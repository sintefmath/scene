/* Copyright STIFTELSEN SINTEF 2014
 * 
 * This file is part of Scene.
 * 
 * Scene is free software: you can redistribute it and/or modifyit under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * Scene is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more
 * details.
 *  
 * You should have received a copy of the GNU Affero General Public License
 * along with the Scene.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#pragma comment( lib, "WSock32" )
#else
#include <sys/time.h>
#endif

#ifndef M_2_PI
#define M_2_PI 1.57079632679489661923
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

inline
double
getCurrentTime()
{
#if defined(__unix) || defined(__APPLE__)
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return tv.tv_sec+tv.tv_usec*1e-6;
#elif defined(_WIN32)
    LARGE_INTEGER f;
    LARGE_INTEGER t;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&t);
    return t.QuadPart/(double) f.QuadPart;
#endif
}


/** Return the cardinal axis that most closely map to the camera's y-axis.
 *
 * \param q  The rotation that takes world coords to camera coords.
 *
 * \returns The cardinal axis.
 *
 * \author Christopher Dyken, <christopher.dyken@sintef.no>
 */
inline
glm::vec3
upAxis( const glm::quat& q )
{
    glm::vec3 y_w = glm::conjugate( q )*glm::vec3( 0.f, 1.f, 0.f );
    glm::vec3 a_w = glm::abs( y_w );
    if( (a_w.x > a_w.y) && (a_w.x > a_w.z ) ) {
        return glm::vec3( y_w.x > 0.f ? 1.f : -1.f, 0.f, 0.f );
    }
    else if( a_w.y > a_w.z ) {
        return glm::vec3( 0.f, y_w.y > 0.f ? 1.f : -1.f, 0.f );
    }
    else {
        return glm::vec3( 0.f, 0.f, y_w.z > 0.f ? 1.f : -1.f );
    }
}


/** Creates the great-circle rotation from a to b.
 * \param a The 'from'-vector.
 * \param b The 'to'-vector.
 * \returns The quaternion q with roll removed.
 * \author Christopher Dyken, <christopher.dyken@sintef.no>
 */
inline
glm::quat
greatCircle( const glm::vec3& a, const glm::vec3& b )
{
    float t = glm::dot( a, b );
    if( std::fabs(t) < 0.999f ) {
        float p = (180.f/M_PI)*std::acos(t);
        glm::vec3 v = glm::normalize( glm::cross(a,b) );
        return glm::angleAxis( p, v );
    }
    else {
        return glm::quat();
    }
}


/** Removes camera roll by forcing the camera x-axis to be orthogonal to up.
 *
 * \param q  The rotation that takes world coords to camera coords.
 * \param up The world-space up-axis, normalized.
 *
 * \returns The quaternion q with roll removed.
 *
 * \author Christopher Dyken, <christopher.dyken@sintef.no>
 */
inline
glm::quat
removeRoll( const glm::quat& q, const glm::vec3& up )
{
    glm::vec3 x_w = glm::conjugate( q ) * glm::vec3( 1.f, 0.f, 0.f );
    glm::vec3 t_w = glm::normalize( x_w - glm::dot(x_w,up)*up );
    glm::quat r = greatCircle( x_w, t_w );
    return q*glm::conjugate(r);
}

/** Returns a point on the unit sphere.
 *
 * Converts a pair of normalized screen coords to a point on a sphere that is
 * exactly enclosed by the window.
 */
inline
glm::vec3
getPointOnUnitSphere( glm::vec2 p )
{
    float r2 = p.x*p.x + p.y * p.y;
    if(r2 < 0.25f)
    {
        return glm::vec3(2.f * p[0], 2.f * p[1], std::sqrt(1.f-4.f*r2));
    }
    else
    {
        float r = 1.f / std::sqrt(r2);
        return glm::vec3(r * p[0], r * p[1], 0.f);
    }
}

/** Returns a plane in object space that corresponds to a norm
 * device coordinate.
 */
inline
glm::vec3
getPointOnPlaneFromNormDevCoords( const glm::mat4& mvp,
                                  const glm::vec4& plane,
                                  const glm::vec2& nd )
{
    glm::vec4 horizontal, vertical;
    //        getPlanesFromNormDevCoords(horizontal, vertical, mvp, nd);
    horizontal = mvp * glm::vec4(1.f, 0.f, 0.f, -nd[0]);
    vertical = mvp * glm::vec4(0.f, -1.f, 0.f, nd[1]);

    //        return intersection(horizontal, vertical, plane);
    glm::vec3 t_horizontal = glm::vec3(horizontal.x, horizontal.y, horizontal.z);
    glm::vec3 t_vertical = glm::vec3(vertical.x, vertical.y, vertical.z);
    glm::vec3 t_plane = glm::vec3(plane.x, plane.y, plane.z);

    float triple3d = 1 / glm::dot(t_horizontal, glm::cross(t_vertical, t_plane));
    glm::vec3 num = glm::vec3(-horizontal.w * glm::cross(t_vertical, t_plane) +
                              -vertical.w * glm::cross(t_plane, t_horizontal) +
                              -plane.w * glm::cross(t_horizontal, t_vertical));
    return num * triple3d;
}

/** Maps a pixel window coordinate to norm device pixel centers */
inline
glm::vec2
normDevFromWinCoords( const glm::vec2 &win_size,
                      const glm::vec2 &wincoords )
{
    return glm::vec2( 2.f * (wincoords[0]+0.5f)/win_size[0] - 1.f,
                     -2.f * (wincoords[1]+0.5f)/win_size[1] + 1.f );
}

#if 0
// disabled as glm::outerProduct has some trouble.
/** Create a random rotation matrix.
 *
 * Implemention of the method described in Arvo, 'Fast Random Rotation Matrices'.
 *
 * \param x  Three uniformly distributed random numbers in [0,1]
 * \returns A corresponding uniformly distributed rotation matrix.
 *
 * \author Christopher Dyken, <christopher.dyken@sintef.no>
 */
inline
glm::mat3
randomRotation( glm::vec3 x )
{
    const float x1 = M_2_PI*x.x;
    const float c1 = std::cos( x1 );
    const float s1 = std::sin( x1 );
    const glm::mat3 R( c1, -s1, 0.f,
                       s1, c1, 0.f,
                       0.f, 0.f, 1.f );

    const float x2 = M_2_PI*x.y;
    const float c2 = std::cos( x2 );
    const float s2 = std::sin( x2 );

    const float xy3 = std::sqrt( x.z );
    const float z3 = std::sqrt( 1.f - x.z );

    const glm::vec3 v( c2*xy3,
                       s2*xy3,
                       z3 );
    const glm::mat3 H = glm::mat3(1.f) - 2.f*glm::outerProduct( v, v );
    return -1.f*H*R;
}
#endif

/** Calculates polar decomposition A = UH of a 3x3 matrix.
 *
 * Implementation of Algorithm 2.1. in Higham and Schreiber, 'Fast Polar
 * Decomposition of an Arbitrary Matrix'.
 *
 * \param A[in]  A non-singular 3x3 matrix (i.e. a rotation).
 * \param U[out] A unitary matrix.
 * \param H[out] Positive semi-definition Hermittian matrix.
 * \author Christopher Dyken, <christopher.dyken@sintef.no>
 */
inline
float
polarDecomposition( glm::mat3& U, glm::mat3& H, const glm::mat3& A )
{
    glm::mat3 X = A;
    for( unsigned int it=0; it<100; it++ ) {
        glm::mat3 Y = glm::inverse( X );        // Y = X^{-1}

        float x_1 = 0.f;                        // 1-norm of X
        float x_i = 0.f;                        // Infty-norm of X
        float y_1 = 0.f;                        // 1-norm of Y
        float y_i = 0.f;                        // Infty-norm of Y
        for( unsigned int i=0; i<9; i++ ) {
            const float x = std::fabs( glm::value_ptr( X )[i] );
            x_1 = x_1 + x;
            x_i = std::max( x_i, x );
            const float y = std::fabs( glm::value_ptr( Y )[i] );
            y_1 = y_1 + y;
            y_i = std::max( y_i, y );
        }

        float gamma = std::sqrt( std::sqrt(  (y_1*y_i)/(x_1*x_i) ) );

        glm::mat3 Xn = (0.5f*gamma)*X + (0.5f/gamma)*glm::transpose(Y);
        float e_d = 0.f;                        // 1-norm of Xn-X
        float e_n = 0.f;                        // 1-norm of Xn
        for( unsigned int i=0; i<9; i++ ) {
            e_d += std::fabs( glm::value_ptr( Xn )[i] - glm::value_ptr( X )[i] );
            e_n += std::fabs( glm::value_ptr( Xn )[i] );
        }
        X = Xn;
        if( e_d <= std::numeric_limits<float>::epsilon()*e_n ) {
            break;
        }
    }
    U = X;
    H = 0.5f*( glm::transpose(U)*A + glm::transpose(A)*U );
    return 0.f;
}

/** Extracts the upper left 3x3 matrix of a 4x4 matrix. */
inline
glm::mat3
upperLeft3x3( const glm::mat4& M )
{
    return glm::mat3( M[0][0], M[0][1], M[0][2],
            M[1][0], M[1][1], M[1][2],
            M[2][0], M[2][1], M[2][2] );
}

/** Extracts the translation part of an affine transformation matrix. */
inline
glm::vec3
translation( const glm::mat4& M )
{
    return glm::vec3( M[3][0], M[3][1], M[3][2] );
}
