#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#pragma comment( lib, "WSock32" )
#else
#include <sys/time.h>
#endif

#include <cmath>
#include <algorithm>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Utils.hpp"

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#include "ViewManipulator.hpp"


ViewManipulator::ViewManipulator(const glm::vec3 &bb_min, const glm::vec3 &bb_max)
    : m_projection(PROJECTION_PERSPECTIVE),
      m_state( NONE )
{
    m_camera_state_curr.m_fov = 90.f; //since glm requires degrees instead of radians
    setViewVolume(bb_min, bb_max);
    m_win_size[0] = 1;
    m_win_size[1] = 1;
    viewAll();
}


void
ViewManipulator::startMotion(int state, int x, int y)
{
    m_mouse_state_curr = glm::vec2(static_cast<float>(x), static_cast<float>(y));
    m_mouse_state_init = m_mouse_state_prev = m_mouse_state_curr;

    m_camera_state_init = m_camera_state_curr;
    if(state == ROTATE) {
        m_state = ROTATE;
    }
    else if(state == PAN) {
        m_state = PAN;
    }
    else if(state == DOLLY) {
        m_state = DOLLY;
    }
    else if( state == FOLLOW ) {
        m_state = FOLLOW;
        m_up_axis = upAxis( m_camera_state_init.m_orientation );
        m_camera_state_init.m_orientation = removeRoll( m_camera_state_init.m_orientation, m_up_axis );
        m_camera_state_curr.m_orientation = m_camera_state_init.m_orientation;
    }
    else if(state == ZOOM) {
        m_state = ZOOM;
    }
    else if( state == ORIENT ) {
        m_state = ORIENT;
        m_up_axis = upAxis( m_camera_state_init.m_orientation );
        m_camera_state_init.m_orientation = removeRoll( m_camera_state_init.m_orientation, m_up_axis );
        m_camera_state_curr.m_orientation = m_camera_state_init.m_orientation;
    }
}

void
ViewManipulator::motion(int x, int y )
{
    m_mouse_state_prev = m_mouse_state_curr;
    m_mouse_state_curr = glm::vec2(static_cast<float>(x), static_cast<float>(y));

    if(m_state == ROTATE) {
        glm::vec3 axis;
        float angle;
        if(trackball(axis, angle, m_mouse_state_init, m_mouse_state_curr)) {
            glm::quat rot;
            float sinA = static_cast<float>(std::sin(0.5*angle));
            float cosA = static_cast<float>(std::cos(0.5*angle));
            axis = glm::normalize(axis);
            rot.w = cosA;
            rot.x = sinA*axis[0];
            rot.y = sinA*axis[1];
            rot.z = sinA*axis[2];
            m_camera_state_curr.m_orientation = rot * (m_camera_state_init.m_orientation);
        }
        else {
            m_camera_state_curr.m_orientation = m_camera_state_init.m_orientation;
        }
        calculateMatrices();
    }
    else if( m_state == ORIENT ) {
        glm::vec2 d =
                normDevFromWinCoords( m_win_size, m_mouse_state_curr )-
                normDevFromWinCoords( m_win_size, m_mouse_state_init );

        glm::quat rot =
                glm::angleAxis(-45.f*d[0], glm::vec3(0.f, 1.f, 0.f) ) *
                glm::angleAxis( 45.f*d[1], glm::vec3(1.f, 0.f, 0.f) );

        m_camera_state_curr.m_orientation = removeRoll( rot*m_camera_state_init.m_orientation, m_up_axis );
        glm::vec3 v = glm::conjugate( m_camera_state_init.m_orientation ) * glm::vec3( 0.f, 0.f, -m_camera_state_init.m_distance );
        glm::vec3 w = glm::conjugate( m_camera_state_curr.m_orientation ) * glm::vec3( 0.f, 0.f, -m_camera_state_curr.m_distance );
        m_camera_state_curr.m_center_of_interest =
                m_camera_state_init.m_center_of_interest + (w - v);

        calculateMatrices();
    }
    else if(m_state == PAN) {
        glm::vec3 ms_mouse_a, ms_mouse_b;
        ms_mouse_a = mousePosOnInterestPlaneAsObjectCoords(normDevFromWinCoords(m_win_size, m_mouse_state_init));
        ms_mouse_b = mousePosOnInterestPlaneAsObjectCoords(normDevFromWinCoords(m_win_size, m_mouse_state_curr));
        m_camera_state_curr.m_center_of_interest = m_camera_state_init.m_center_of_interest - (ms_mouse_b - ms_mouse_a);
        calculateMatrices();
    }

    else if(m_state == DOLLY) {
        glm::vec4 plane = glm::vec4(0.f, 0.f, -1.f, 1.f) * m_camera_state_init.m_model_view_projection;
        plane.w = -glm::dot(glm::vec3(plane.x, plane.y, plane.z), m_camera_state_init.m_center_of_interest);
        if(plane.w != plane.w) {
            return;
        }
        glm::vec2 a_nd = normDevFromWinCoords(m_win_size, m_mouse_state_init);
        glm::vec2 b_nd = normDevFromWinCoords(m_win_size, m_mouse_state_curr);

        glm::vec3 d = getPointOnPlaneFromNormDevCoords(m_camera_state_init.m_model_view_projection, plane, b_nd) -
                getPointOnPlaneFromNormDevCoords(m_camera_state_init.m_model_view_projection, plane, a_nd);

        const glm::mat4 &m = m_camera_state_init.m_model_view;
        glm::vec3 x = glm::vec3(-m[0][0], m[0][1], m[0][2]);
        glm::vec3 z = glm::vec3(m[2][0], m[2][1], m[2][2]);

        m_camera_state_curr.m_center_of_interest = m_camera_state_init.m_center_of_interest - glm::dot(x, d) * z;
        calculateMatrices();
    }
    else if( m_state == FOLLOW ) {
        glm::vec2 d =
                normDevFromWinCoords(m_win_size, m_mouse_state_curr) -
                normDevFromWinCoords(m_win_size, m_mouse_state_init);

        glm::vec3 dir = glm::conjugate(m_camera_state_init.m_orientation) * glm::vec3( 0.f, 0.f, 1.f );
        dir = glm::normalize( dir - glm::dot(dir,m_up_axis)*m_up_axis );
        m_camera_state_curr.m_center_of_interest =
                m_camera_state_init.m_center_of_interest +
                d[1] * glm::distance( m_bbox_min, m_bbox_max ) * dir;

        /*

                    MV = T(D)R(ORIENT)T(-COI)
m_camera_state_curr.m_model_view = glm::translate( glm::mat4(), cam_pos );
                    m_camera_state_curr.m_model_view = m_camera_state_curr.m_model_view * rotmatrix;
                    m_camera_state_curr.m_model_view = glm::translate( m_camera_state_curr.m_model_view,
                                                                       -m_camera_state_curr.m_center_of_interest );
*/

        calculateMatrices();

    }
    else if(m_state == ZOOM) {
        glm::vec2 d =
                normDevFromWinCoords(m_win_size, m_mouse_state_curr) -
                normDevFromWinCoords(m_win_size, m_mouse_state_init);

        m_camera_state_curr.m_distance = m_camera_state_init.m_distance * (1 + d[1]);

        float distTemp =  0.01f * glm::distance( m_bbox_min, m_bbox_max );

        if(m_camera_state_curr.m_distance < distTemp)
        {
            m_camera_state_curr.m_distance = distTemp;
        }
        calculateMatrices();
    }
}

void
ViewManipulator::update()
{
#if 0
    if(m_state == SPIN) {
        float axis_length = glm::length( m_spin_axis );
        if( axis_length < std::numeric_limits<float>::epsilon() ) {
            m_state = NONE;
        }
        else {

            //doing manual rotation instead of glm since it avoids conversion to and from degrees.
            glm::quat spin;
            float angle = m_spin_speed * m_spin_factor * static_cast<float>(elapsed);
            float sinA = static_cast<float>(std::sin(0.5*angle));
            float cosA = static_cast<float>(std::cos(0.5*angle));

            m_spin_axis = glm::normalize(m_spin_axis);
            spin.w = cosA;
            spin.x = sinA*m_spin_axis[0];
            spin.y = sinA*m_spin_axis[1];
            spin.z = sinA*m_spin_axis[2];
            m_camera_state_curr.m_orientation = spin * m_camera_state_curr.m_orientation;

            calculateMatrices();
        }
    }
#endif
}

void
ViewManipulator::endMotion(int x, int y )
{
#if 0
    MouseState curr;
    curr = glm::vec2(static_cast<float>(x), static_cast<float>(y));
    curr.m_elapsed = static_cast<float>(elapsed);
    if(m_state == ROTATE) {
        if(false && curr.m_elapsed < 0.5 ) {
            float angle;
            if(trackball(m_spin_axis, angle, m_mouse_state_prev,
                         curr))
            {
                m_spin_speed = angle / (m_mouse_state_prev.m_elapsed + m_mouse_state_curr.m_elapsed +
                                        curr.m_elapsed);
                m_state = SPIN;
            }
        }
    }
#endif
    motion( x, y );
    m_state = NONE;
}

void
ViewManipulator::setProjectionType(ProjectionType pt)
{
    m_projection = pt;
    calculateMatrices();
}

void
ViewManipulator::setWindowSize(int w, int h)
{
    m_win_size = glm::vec2(static_cast<float>(w), static_cast<float>(h));
    calculateMatrices();
}

void
ViewManipulator::setOrientation(const glm::mat4 &m)
{
    m_state = NONE;
    m_camera_state_curr.m_orientation = glm::quat(m);
    calculateMatrices();
}

void
ViewManipulator::setOrientation(const glm::quat &q)
{
    m_state = NONE;
    m_camera_state_curr.m_orientation = q;
    calculateMatrices();
}

void
ViewManipulator::setViewVolume( const glm::vec3 &bb_min,
                                const glm::vec3 &bb_max )
{
    m_bbox_min = bb_min;
    m_bbox_max = bb_max;
    viewAll();
    calculateMatrices();
}

void
ViewManipulator::setCamera( const glm::mat4& modelview,
                            const glm::mat4& projection,
                            const bool guess_bbox )
{

    // Calculate fov and guess projection type
    glm::vec4 tp = glm::row( projection, 3 ) - glm::row( projection, 1 );
    glm::vec4 bp = glm::row( projection, 3 ) + glm::row( projection, 1 );
    float fov_y = std::acos( glm::dot( glm::normalize( glm::vec3( tp.x, tp.y, tp.z ) ),
                                       glm::normalize( glm::vec3( bp.x, bp.y, bp.z ) ) ) );

    if( fov_y > 0.1 ) {
        m_projection = PROJECTION_PERSPECTIVE;
        m_camera_state_curr.m_fov = (180.f/M_PI)*fov_y;
    }
    else {
        // todo
    }

    // extract linear transformation
    glm::mat3 A = upperLeft3x3( modelview );
    glm::mat3 Ai = glm::inverse( A );

    // extract rotation
    glm::mat3 R;
    polarDecomposition( R, A, A );
    m_camera_state_curr.m_orientation = glm::quat_cast( R );

    // extract observer position, calc coi, and transform to world
    glm::vec3 op = -translation( modelview );

    if( guess_bbox ) {
        glm::vec4 np = glm::row( projection, 3 ) + glm::row( projection, 2 );
        glm::vec4 fp = glm::row( projection, 3 ) - glm::row( projection, 2 );
        float n = np.w/glm::length( glm::vec3( np.x, np.y, np.z ) );
        float f = -fp.w/glm::length( glm::vec3( fp.x, fp.y, fp.z ) );
        float d = 0.5f*(std::abs(f)-std::abs(n));
        m_camera_state_curr.m_distance = -0.5f*(n+f);

        glm::vec3 mb = glm::vec3( 0.f, 0.f, m_camera_state_curr.m_distance);
        m_camera_state_curr.m_center_of_interest = Ai*(op - mb );

        m_bbox_min = m_camera_state_curr.m_center_of_interest - glm::vec3(d);
        m_bbox_max = m_camera_state_curr.m_center_of_interest + glm::vec3(d);
    }
    else {
        glm::vec3 mb = glm::vec3( 0.f, 0.f, m_camera_state_curr.m_distance);
        m_camera_state_curr.m_center_of_interest = Ai*(op - mb );
    }

    calculateMatrices();
}

void
ViewManipulator::updateViewVolume( const glm::vec3 &bb_min,
                                   const glm::vec3 bb_max )
{
    m_bbox_min = bb_min;
    m_bbox_max = bb_max;
    calculateMatrices();
}

void
ViewManipulator::viewAll()
{
    m_camera_state_curr.m_center_of_interest = 0.5f*( m_bbox_min + m_bbox_max );
    m_camera_state_curr.m_distance = glm::distance( m_bbox_min, m_bbox_max ) / tanf(m_camera_state_curr.m_fov*(M_PI/360.f));
    calculateMatrices();
}

glm::vec3
ViewManipulator::getCurrentViewPoint() const
{
    glm::mat4 inv =  m_camera_state_curr.m_model_view_inverse;
    return glm::vec3(glm::column(inv, 3) );
}


void
ViewManipulator::calculateMatrices()
{

    glm::vec3 cam_pos = glm::vec3( 0.f, 0.f, -m_camera_state_curr.m_distance );
    glm::mat4 rotmatrix = glm::mat4_cast( m_camera_state_curr.m_orientation );

    // modelview matrix
    m_camera_state_curr.m_model_view = glm::translate( glm::mat4(), cam_pos );
    m_camera_state_curr.m_model_view = m_camera_state_curr.m_model_view * rotmatrix;
    m_camera_state_curr.m_model_view = glm::translate( m_camera_state_curr.m_model_view,
                                                       -m_camera_state_curr.m_center_of_interest );
    // create inverse modelview matrix
    m_camera_state_curr.m_model_view_inverse = glm::translate(glm::mat4(), m_camera_state_curr.m_center_of_interest);
    m_camera_state_curr.m_model_view_inverse *= glm::mat4_cast(glm::conjugate(m_camera_state_curr.m_orientation));
    m_camera_state_curr.m_model_view_inverse *= glm::translate(glm::mat4(), -cam_pos);

    // determine near_ and far
    float near_, far_;
    for(int i=0; i<8; i++) {
        glm::vec4 p = glm::vec4( (i&1)==0 ? m_bbox_min.x : m_bbox_max.x,
                                 (i&2)==0 ? m_bbox_min.y : m_bbox_max.y,
                                 (i&4)==0 ? m_bbox_min.z : m_bbox_max.z,
                                 1.f );
        glm::vec4 h = m_camera_state_curr.m_model_view * p;
        float d = (1.f/h.w)*h.z;
        if( i == 0 ) {
            near_ = far_ = d;
        }
        else {
            near_ = std::max( near_, d );
            far_  = std::min( far_,  d );
        }
    }

    // use infty-norm of bbox to get an idea of the scale of the scene, and
    // determine an appropriate epsilon
    float e = std::max( std::numeric_limits<float>::epsilon(),
                        0.001f*std::max( m_bbox_max.x-m_bbox_min.x,
                                         std::max( m_bbox_max.y-m_bbox_min.y,
                                                   m_bbox_max.z-m_bbox_min.z )
                                         )
                        );
    far_  = std::min( -e, far_-e );
    near_ = std::min( 0.01f*far_, std::max(far_, near_ + e ) );
    m_camera_state_curr.m_near = near_;
    m_camera_state_curr.m_far = far_;

    float aspect = m_win_size[0]/m_win_size[1];
    if(m_projection == PROJECTION_PERSPECTIVE) {
        //would like to use our own projection matrix function, since it will remove the need for using degrees
        m_camera_state_curr.m_projection = glm::perspective(m_camera_state_curr.m_fov, aspect, -near_, -far_ );
        m_camera_state_curr.m_projection_inverse = glm::inverse(m_camera_state_curr.m_projection);
    }
    else if(m_projection == PROJECTION_ORTHOGRAPHIC) {
        float h = tan(0.5f * m_camera_state_curr.m_fov) * m_camera_state_curr.m_distance;
        float w = aspect * h;
        m_camera_state_curr.m_projection = glm::ortho(-w, w, -h, h, -near_, -far_);
        m_camera_state_curr.m_projection_inverse = glm::inverse(m_camera_state_curr.m_projection);
    }
    m_camera_state_curr.m_model_view_projection = m_camera_state_curr.m_projection * m_camera_state_curr.m_model_view;
    m_camera_state_curr.m_model_view_projection_inverse = m_camera_state_curr.m_model_view_inverse * m_camera_state_curr.m_projection_inverse;
}

bool
ViewManipulator::trackball(glm::vec3 &axis, float &angle, const glm::vec2 &wc1, const glm::vec2 &wc2)
{
    const glm::vec3 sphere1 = getPointOnUnitSphere( glm::vec2((wc1[0] / m_win_size[0] -0.5f),
            -wc1[1] / m_win_size[1]+0.5f));

    const glm::vec3 sphere2 = getPointOnUnitSphere( glm::vec2((wc2[0] / m_win_size[0] -0.5f),
            -wc2[1] / m_win_size[1]+0.5f));

    // note: length of cross product is proportional to sine of
    // angle, i.e., a trigonometric func can be removed (dyken).

    angle = acos( glm::dot( sphere1, sphere2 ) );
    if( fabs( angle ) < std::numeric_limits<float>::epsilon() ) {
        return false;
    }
    else {
        axis = glm::normalize( glm::cross( sphere1, sphere2 ) );
        if(axis.x != axis.x || axis.y != axis.y || axis.z != axis.z)
        {
            //degenerate axis from normalised cross product of two identical vectors/spheres.
            return false;
        }
        return true;
    }

}

glm::vec3
ViewManipulator::mousePosOnInterestPlaneAsObjectCoords( glm::vec2 mouse_pos)
{
    //find ndc-depth of center of interest plane
    glm::vec4 h = glm::vec4(m_camera_state_init.m_center_of_interest.x,
                            m_camera_state_init.m_center_of_interest.y,
                            m_camera_state_init.m_center_of_interest.z,
                            1.f);
    h = m_camera_state_init.m_model_view_projection * h;
    float z = h.z/h.w;

    //Build ndc mouse position at found depth and project back to object space.
    h.x = mouse_pos.x;
    h.y = mouse_pos.y;
    h.z = z;
    h.w = 1.f;
    h = m_camera_state_init.m_model_view_projection_inverse * h;

    return (1.f/h.w)*glm::vec3(h.x, h.y, h.z);

}
