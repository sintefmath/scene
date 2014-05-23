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

#ifdef __SSE4_2__
#include <xmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#endif

#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <scene/Value.hpp>
#include <scene/Node.hpp>
#include <scene/Camera.hpp>
#include <scene/runtime/TransformCompute.hpp>

namespace Scene {
    namespace Runtime {
#ifdef __SSE4_2__
static const std::string toString( const __m128& val )
{
    float t[4];
    _mm_storeu_ps( t, val );
    std::stringstream o;
    o << "m128[" << t[0]
      << ", " << t[1]
      << ", " << t[2]
      << ", " << t[3]
      << "]";
    return o.str();
}
#endif


//#ifdef __SSE4_2__
#define _MM_MUL4TX4_PS(a,b,c,d,e) {                     \
    __m128 t;                                           \
    __m128 c0 = _mm_load_ps( (e) + 0 );                 \
    __m128 c1 = _mm_load_ps( (e) + 4 );                 \
    __m128 c2 = _mm_load_ps( (e) + 8 );                 \
    __m128 c3 = _mm_load_ps( (e) + 12 );                \
    t   =               _mm_dp_ps( (a), c0, 0xf1 );     \
    t   = _mm_or_ps( t, _mm_dp_ps( (a), c1, 0xf2 ) );   \
    t   = _mm_or_ps( t, _mm_dp_ps( (a), c2, 0xf4 ) );   \
    (a) = _mm_or_ps( t, _mm_dp_ps( (a), c3, 0xf8 ) );   \
    t   =               _mm_dp_ps( (b), c0, 0xf1 );     \
    t   = _mm_or_ps( t, _mm_dp_ps( (b), c1, 0xf2 ) );   \
    t   = _mm_or_ps( t, _mm_dp_ps( (b), c2, 0xf4 ) );   \
    (b) = _mm_or_ps( t, _mm_dp_ps( (b), c3, 0xf8 ) );   \
    t   =               _mm_dp_ps( (c), c0, 0xf1 );     \
    t   = _mm_or_ps( t, _mm_dp_ps( (c), c1, 0xf2 ) );   \
    t   = _mm_or_ps( t, _mm_dp_ps( (c), c2, 0xf4 ) );   \
    (c) = _mm_or_ps( t, _mm_dp_ps( (c), c3, 0xf8 ) );   \
    t   =               _mm_dp_ps( (d), c0, 0xf1 );     \
    t   = _mm_or_ps( t, _mm_dp_ps( (d), c1, 0xf2 ) );   \
    t   = _mm_or_ps( t, _mm_dp_ps( (d), c2, 0xf4 ) );   \
    (d) = _mm_or_ps( t, _mm_dp_ps( (d), c3, 0xf8 ) );   \
    }
//#else
#define _MUL4TX4_PS(a,b) {                                                  \
    const float*p = (b);                                                    \
    float t[4];                                                             \
    t[0] = (a)[0]*p[0]  + (a)[1]*p[1]  + (a)[2]*p[2]  + (a)[3]*p[3];        \
    t[1] = (a)[0]*p[4]  + (a)[1]*p[5]  + (a)[2]*p[6]  + (a)[3]*p[7];        \
    t[2] = (a)[0]*p[8]  + (a)[1]*p[9]  + (a)[2]*p[10] + (a)[3]*p[11];       \
    t[3] = (a)[0]*p[12] + (a)[1]*p[13] + (a)[2]*p[14] + (a)[3]*p[15];       \
    (a)[0] = t[0]; (a)[1] = t[1]; (a)[2]= t[2]; (a)[3] = t[3];              \
    t[0] = (a)[4]*p[0]  + (a)[5]*p[1]  + (a)[6]*p[2]  + (a)[7]*p[3];        \
    t[1] = (a)[4]*p[4]  + (a)[5]*p[5]  + (a)[6]*p[6]  + (a)[7]*p[7];        \
    t[2] = (a)[4]*p[8]  + (a)[5]*p[9]  + (a)[6]*p[10] + (a)[7]*p[11];       \
    t[3] = (a)[4]*p[12] + (a)[5]*p[13] + (a)[6]*p[14] + (a)[7]*p[15];       \
    (a)[4] = t[0]; (a)[5] = t[1]; (a)[6]= t[2]; (a)[7] = t[3];              \
    t[0] = (a)[8]*p[0]  + (a)[9]*p[1]  + (a)[10]*p[2]  + (a)[11]*p[3];      \
    t[1] = (a)[8]*p[4]  + (a)[9]*p[5]  + (a)[10]*p[6]  + (a)[11]*p[7];      \
    t[2] = (a)[8]*p[8]  + (a)[9]*p[9]  + (a)[10]*p[10] + (a)[11]*p[11];     \
    t[3] = (a)[8]*p[12] + (a)[9]*p[13] + (a)[10]*p[14] + (a)[11]*p[15];     \
    (a)[8] = t[0]; (a)[9] = t[1]; (a)[10]= t[2]; (a)[11] = t[3];            \
    t[0] = (a)[12]*p[0]  + (a)[13]*p[1]  + (a)[14]*p[2]  + (a)[15]*p[3];    \
    t[1] = (a)[12]*p[4]  + (a)[13]*p[5]  + (a)[14]*p[6]  + (a)[15]*p[7];    \
    t[2] = (a)[12]*p[8]  + (a)[13]*p[9]  + (a)[14]*p[10] + (a)[15]*p[11];   \
    t[3] = (a)[12]*p[12] + (a)[13]*p[13] + (a)[14]*p[14] + (a)[15]*p[15];   \
    (a)[12] = t[0]; (a)[13] = t[1]; (a)[14]= t[2]; (a)[15] = t[3];          \
    }

//#endif

void
TransformCompute::cosine( Value* dst, const Value* src )
{
    if( (src == NULL) || !dst->valueChanged().moveForward( src->valueChanged() ) ) {
        return;
    }
    const float* s = src->floatData();
    float* d = dst->m_payload.m_floats;
    d[0] = std::cos( s[0] );
    d[1] = std::cos( s[1] );
    d[2] = std::cos( s[2] );
    d[3] = std::cos( s[3] );
}

void
TransformCompute::reciprocal( Value* dst, const Value* src )
{
    if( (src == NULL) || !dst->valueChanged().moveForward( src->valueChanged() ) ) {
        return;
    }
    const float* s = src->floatData();
    float* d = dst->m_payload.m_floats;
    d[0] = 1.f/s[0];
    d[1] = 1.f/s[1];
    d[2] = 1.f/s[2];
    d[3] = 1.f/s[3];
}

void
TransformCompute::projection( Value* dst, const Camera* c )
{
    if( (c == NULL) || !dst->valueChanged().moveForward( c->valueChanged() ) ) {
        return;
    }
    if( c->cameraType() == CAMERA_PERSPECTIVE ) {
        float f = 1.f/tanf( 0.5f*c->fovY() );
        float g = 1.f/tanf( 0.5f*c->fovX() );
        float dz = c->far() - c->near();
        float h = (c->far() + c->near() )/( -dz );
        float j = (2.f * c->far() * c->near() )/( -dz );
        float* d = dst->m_payload.m_floats;
        d[0]  = g;    d[1]  = 0.f;  d[2]  = 0.f;  d[3] = 0.f;
        d[4]  = 0.f;  d[5]  = f;    d[6]  = 0.f;  d[7] = 0.f;
        d[8]  = 0.f;  d[9]  = 0.f;  d[10] = h;    d[11] = -1.f;
        d[12] = 0.f;  d[13] = 0.f;  d[14] = j;    d[15] = 0.f;
    }
    else if( c->cameraType() == CAMERA_ORTHOGONAL ) {
        // FIXME: Implement orthogonal camera.
    }
    else if( c->cameraType() == CAMERA_CUSTOM_MATRIX ) {
        std::copy_n( c->customMatrix().floatData(),
                     16,
                     dst->m_payload.m_floats );
    }
}

void
TransformCompute::projectionInverse( Value* dst, const Camera* c )
{
    if( (c == NULL) || !dst->valueChanged().moveForward( c->valueChanged() ) ) {
        return;
    }
    if( c->cameraType() == CAMERA_PERSPECTIVE ) {
        // TODO: This code is not checked yet, needs test.
        float dz = c->far() - c->near();
        float gg = tanf( 0.5f*c->fovX() );
        float ff = tanf( 0.5f*c->fovY() );
        float jj = -dz/(2.f * c->far() * c->near() );
        float hj = (c->far() + c->near() )/
                (2.f * c->far() * c->near() );
        float* d = dst->m_payload.m_floats;
        d[0]  = gg;   d[1]  = 0.f;  d[2]  = 0.f;   d[3]  = 0.f;
        d[4]  = 0.f;  d[5]  = ff;   d[6]  = 0.f;   d[7]  = 0.f;
        d[8]  = 0.f;  d[9]  = 0.f;  d[10] = 0.f;   d[11] = jj;
        d[12] = 0.f;  d[13] = 0.f;  d[14] = -1.f;  d[15] = hj;
    }
    else if( c->cameraType() == CAMERA_ORTHOGONAL ) {
        // FIXME: Implement orthogonal camera.
    }
    else if( c->cameraType() == CAMERA_CUSTOM_MATRIX ) {
        const float* v = c->customMatrix().floatData();
        glm::mat4 P = glm::mat4( v[0],  v[1],  v[2],  v[3],
                                 v[4],  v[5],  v[6],  v[7],
                                 v[8],  v[9],  v[10], v[11],
                                 v[12], v[13], v[14], v[15] );
        glm::mat4 Q = glm::inverse( P );
        std::copy_n( glm::value_ptr(Q), 16, dst->m_payload.m_floats );
    }
}


void
TransformCompute::nodeTransform( Value* dst, const Node* n )
{
    if( (n == NULL) || !dst->valueChanged().moveForward( n->valueChanged() ) ) {
        return;
    }
    glm::mat4 M;
    for( size_t i=0; i<n->transforms(); i++ ) {
        const TransformType type = n->transformType(i);
        const Value& src = n->transformValue(i);
        const float* v = src.floatData();

        if( type == TRANSFORM_LOOKAT ) {
            glm::vec3 eye( v[0], v[3], v[6] );
            glm::vec3 coi( v[1], v[4], v[7] );
            glm::vec3 up( v[2], v[5], v[8] );
            glm::vec3 z = glm::normalize( eye - coi );
            glm::vec3 x = glm::normalize( glm::cross( up, z ) );
            glm::vec3 y = glm::cross( z, x );
            glm::mat4 O = glm::mat4( x[0], x[1], x[2], 0.f,
                                     y[0], y[1], y[2], 0.f,
                                     z[0], z[1], z[2], 0.f,
                                     eye.x, eye.y, eye.z, 1.f );
            M = M * O;
        }
        else if( type == TRANSFORM_MATRIX ) {
            M *= glm::mat4( v[0],  v[1],  v[2],  v[3],
                            v[4],  v[5],  v[6],  v[7],
                            v[8],  v[9],  v[10], v[11],
                            v[12], v[13], v[14], v[15] );
        }
        else if( type == TRANSFORM_TRANSLATE ) {
            M *= glm::mat4( 1.f, 0.f, 0.f, 0.f,
                            0.f, 1.f, 0.f, 0.f,
                            0.f, 0.f, 1.f, 0.f,
                            v[0], v[1], v[2], 1.f );
        }
        else if( type == TRANSFORM_SCALE ) {
            M *= glm::mat4( v[0], 0.f, 0.f, 0.f,
                            0.f, v[1], 0.f, 0.f,
                            0.f, 0.f, v[2], 0.f,
                            0.f, 0.f, 0.f, 1.f );
        }
        else if( type == TRANSFORM_ROTATE ) {
            glm::vec3 u = glm::normalize( glm::vec3( v[0], v[1], v[2] ) );
            const float x = u[0];
            const float y = u[1];
            const float z = u[2];
            const float c = cosf( v[3] );
            const float s = sinf( v[3] );
            M *= glm::mat4( x*x*(1.f-c) + c,   y*x*(1.f-c) + z*s, z*x*(1.f-c) - y*s, 0.f,
                            x*y*(1.f-c) - z*s, y*y*(1.f-c) + c,   z*y*(1.f-c) + x*s, 0.f,
                            x*z*(1.f-c) + y*s, y*z*(1.f-c) - x*s, z*z*(1.f-c) + c,   0.f,
                            0.f,               0.f,               0.f,               1.f );
        }
        else {
        }
    }
    std::copy_n( glm::value_ptr(M), 16, dst->m_payload.m_floats );
}

void
TransformCompute::nodeInverseTransform( Value* dst, const Node* n )
{
    if( (n == NULL) || !dst->valueChanged().moveForward( n->valueChanged() ) ) {
        return;
    }
    glm::mat4 M;
    for( size_t i=0; i<n->transforms(); i++ ) {
        const TransformType type = n->transformType(i);
        const Value& src = n->transformValue(i);
        const float* v = src.floatData();

        if( type == TRANSFORM_LOOKAT ) {
            glm::vec3 eye( v[0], v[3], v[6] );
            glm::vec3 coi( v[1], v[4], v[7] );
            glm::vec3 up( v[2], v[5], v[8] );
            glm::vec3 z = glm::normalize( eye - coi );
            glm::vec3 x = glm::normalize( glm::cross( up, z ) );
            glm::vec3 y = glm::cross( z, x );
            glm::mat4 O = glm::mat4( x[0], y[0], z[0], 0.f,
                                     x[1], y[1], z[1], 0.f,
                                     x[2], y[2], z[2], 0.f,
                                     -glm::dot( x, eye ), -glm::dot( y, eye ), -glm::dot(z, eye ), 1.f) *M;
            M = O * M;
        }
        else if( type == TRANSFORM_MATRIX ) {
            M = glm::inverse( glm::mat4( v[0],  v[1],  v[2],  v[3],
                                         v[4],  v[5],  v[6],  v[7],
                                         v[8],  v[9],  v[10], v[11],
                                         v[12], v[13], v[14], v[15] ) ) * M;
        }
        else if( type == TRANSFORM_TRANSLATE ) {
            M = glm::mat4( 1.f, 0.f, 0.f, 0.f,
                           0.f, 1.f, 0.f, 0.f,
                           0.f, 0.f, 1.f, 0.f,
                           -v[0], -v[1], -v[2], 1.f ) * M;

        }
        else if( type == TRANSFORM_SCALE ) {
            M = glm::mat4( 1.f/v[0], 0.f, 0.f, 0.f,
                           0.f, 1.f/v[1], 0.f, 0.f,
                           0.f, 0.f, 1.f/v[2], 0.f,
                           0.f, 0.f, 0.f, 1.f ) * M;
        }

        else if( type == TRANSFORM_ROTATE ) {
            glm::vec3 u = glm::normalize( glm::vec3( v[0], v[1], v[2] ) );
            const float x = u[0];
            const float y = u[1];
            const float z = u[2];
            const float c = cosf( -v[3] );
            const float s = sinf( -v[3] );
            glm::mat4 O = glm::mat4( x*x*(1.f-c) + c,   y*x*(1.f-c) + z*s, z*x*(1.f-c) - y*s, 0.f,
                                     x*y*(1.f-c) - z*s, y*y*(1.f-c) + c,   z*y*(1.f-c) + x*s, 0.f,
                                     x*z*(1.f-c) + y*s, y*z*(1.f-c) - x*s, z*z*(1.f-c) + c,   0.f,
                                     0.f,               0.f,               0.f,               1.f );

            M = O * M;
        }
        else {
        }
    }
    std::copy_n( glm::value_ptr(M), 16, dst->m_payload.m_floats );
}

void
TransformCompute::transformZAxis( Value* dst, const unsigned int N, const Value** src )
{
    bool modified = false;
    for( unsigned int i=0 ; i<N; i++ ) {
        bool m = dst->valueChanged().moveForward( src[i]->valueChanged() );
        modified = modified | m;
    }
    if( !modified || (N<1) ) {
        return;
    }
    if( N == 1 ) {
        const float* s = src[0]->floatData();
        const float V[4] = { s[2], s[6], s[10], s[14] };
        std::copy_n( V, 4, dst->m_payload.m_floats );
    }
    else {
        const float* p = src[0]->floatData();
        float M[16] = {                                                         // load first matrix transposed
            p[0], p[4], p[8], p[12],
            p[1], p[5], p[9], p[13],
            p[2], p[6], p[10], p[14],
            p[3], p[7], p[11], p[15]
        };
        for( unsigned int i=1; i<N; i++ ) {
            _MUL4TX4_PS( M, src[i]->floatData() );
        }
        const float V[4] = { M[8], M[9], M[10], M[11] };
        std::copy_n( V, 4, dst->m_payload.m_floats );
    }
}

void
TransformCompute::transformOrigin( Value* dst, const unsigned int N, const Value** src )
{
    bool modified = false;
    for( unsigned int i=0 ; i<N; i++ ) {
        bool m = dst->valueChanged().moveForward( src[i]->valueChanged() );
        modified = modified | m;
    }
    if( !modified || (N<1) ) {
        return;
    }
    if( N == 1 ) {
        std::copy_n( src[0]->floatData() + 12, 4, dst->m_payload.m_floats );
    }
    else {
        const float* p = src[0]->floatData();
        float M[16] = {                                                         // load first matrix transposed
            p[0], p[4], p[8], p[12],
            p[1], p[5], p[9], p[13],
            p[2], p[6], p[10], p[14],
            p[3], p[7], p[11], p[15]
        };
        for( unsigned int i=1; i<N; i++ ) {
            _MUL4TX4_PS( M, src[i]->floatData() );
        }
        const float V[4] = { M[3], M[7], M[11], M[15] };
        std::copy_n( V, 4, dst->m_payload.m_floats );
    }

}

void
TransformCompute::transposedUpper3x3( Value* dst, const unsigned int N, const Value** src )
{
    bool modified = false;
    for( unsigned int i=0 ; i<N; i++ ) {
        bool m = dst->valueChanged().moveForward( src[i]->valueChanged() );
        modified = modified | m;
    }
    if( !modified || (N<1) ) {
        return;
    }
    if( N == 1 ) {
        const float* m = src[0]->floatData();
        float* d = dst->m_payload.m_floats;
        d[0] = m[0];  d[1] = m[4];  d[2] = m[8];
        d[3] = m[1];  d[4] = m[5];  d[5] = m[9];
        d[6] = m[2];  d[7] = m[6];  d[8] = m[10];
    }
    else {
#ifdef __SSE4_2__
        __m128 Ar0 = _mm_load_ps( src[0]->floatData() + 0 );
        __m128 Ar1 = _mm_load_ps( src[0]->floatData() + 4 );
        __m128 Ar2 = _mm_load_ps( src[0]->floatData() + 8 );
        __m128 Ar3 = _mm_load_ps( src[0]->floatData() + 12 );
        _MM_TRANSPOSE4_PS( Ar0, Ar1, Ar2, Ar3 );
        unsigned int Nn = N-1;
        for( unsigned int i=1; i<Nn; i++ ) {
            _MM_MUL4TX4_PS( Ar0, Ar1, Ar2, Ar3, src[i]->floatData() );
        }
        __m128 Bc0 = _mm_load_ps( src[Nn]->floatData() + 0 );
        __m128 Bc1 = _mm_load_ps( src[Nn]->floatData() + 4 );
        __m128 Bc2 = _mm_load_ps( src[Nn]->floatData() + 8 );
        __m128 t;
        t =               _mm_dp_ps( Ar0, Bc0, 0xf1 );   // d[0] = Ar0 Bc0
        t = _mm_or_ps( t, _mm_dp_ps( Ar0, Bc1, 0xf2 ) ); // d[1] = Ar0 Bc1
        t = _mm_or_ps( t, _mm_dp_ps( Ar0, Bc2, 0xf4 ) ); // d[2] = Ar0 Bc2
        t = _mm_or_ps( t, _mm_dp_ps( Ar1, Bc0, 0xf8 ) ); // d[3] = Ar1 Bc0
        _mm_store_ps( dst->m_payload.m_floats + 0, t );
        t =               _mm_dp_ps( Ar1, Bc1, 0xf1 );   // d[4] = Ar1 Bc1
        t = _mm_or_ps( t, _mm_dp_ps( Ar1, Bc2, 0xf2 ) ); // d[5] = Ar1 Bc2
        t = _mm_or_ps( t, _mm_dp_ps( Ar2, Bc0, 0xf4 ) ); // d[6] = Ar2 Bc0
        t = _mm_or_ps( t, _mm_dp_ps( Ar2, Bc1, 0xf8 ) ); // d[7] = Ar2 Bc1
        _mm_store_ps( dst->m_payload.m_floats + 4, t );
        t =               _mm_dp_ps( Ar2, Bc2, 0xf1 );   // d[8] = Ar2 Sc2
        _mm_store_ps( dst->m_payload.m_floats + 8, t );
#else
        const float* p = src[0]->floatData();
        float M[16] = {                                                         // load first matrix transposed
            p[0], p[4], p[8], p[12],
            p[1], p[5], p[9], p[13],
            p[2], p[6], p[10], p[14],
            p[3], p[7], p[11], p[15]
        };
        for( unsigned int i=1; i<N; i++ ) {
            _MUL4TX4_PS( M, src[i]->floatData() );
        }
        const float Mt[9] = {
            M[0], M[1], M[2],
            M[4], M[5], M[6],
            M[8], M[9], M[10]
        };
        std::copy_n( Mt, 9, dst->m_payload.m_floats );                         // store
#endif
    }
}


void
TransformCompute::multiplyMatrices( Value* dst, const unsigned int N, const Value** src )
{
    bool modified = false;
    for( unsigned int i=0 ; i<N; i++ ) {
        bool m = dst->valueChanged().moveForward( src[i]->valueChanged() );
        modified = modified | m;
    }
    if( !modified || (N<1) ) {
        return;
    }

#ifdef __SSE4_2__
    __m128 Ar0 = _mm_load_ps( src[0]->floatData() + 0 );
    __m128 Ar1 = _mm_load_ps( src[0]->floatData() + 4 );
    __m128 Ar2 = _mm_load_ps( src[0]->floatData() + 8 );
    __m128 Ar3 = _mm_load_ps( src[0]->floatData() + 12 );
    if( N > 1 ) {
        _MM_TRANSPOSE4_PS( Ar0, Ar1, Ar2, Ar3 );
        for( unsigned int i=1; i<N; i++ ) {
            _MM_MUL4TX4_PS( Ar0, Ar1, Ar2, Ar3, src[i]->floatData() );
        }
        _MM_TRANSPOSE4_PS(  Ar0, Ar1, Ar2, Ar3  );
    }
    _mm_store_ps( dst->m_payload.m_floats +  0, Ar0 );   // _mm_stream_ps?
    _mm_store_ps( dst->m_payload.m_floats +  4, Ar1 );
    _mm_store_ps( dst->m_payload.m_floats +  8, Ar2 );
    _mm_store_ps( dst->m_payload.m_floats + 12, Ar3 );
#else
    if( N == 1 ) {
        std::copy_n( src[0]->m_payload.m_floats, 16, dst->m_payload.m_floats );
    }
    else {
        const float* p = src[0]->floatData();
        float M[16] = {                                                         // load first matrix transposed
            p[0], p[4], p[8], p[12],
            p[1], p[5], p[9], p[13],
            p[2], p[6], p[10], p[14],
            p[3], p[7], p[11], p[15]
        };
        for( unsigned int i=1; i<N; i++ ) {
            _MUL4TX4_PS( M, src[i]->floatData() );
        }
        const float Mt[16] = {                                                  // transpose result
            M[0], M[4], M[8], M[12],
            M[1], M[5], M[9], M[13],
            M[2], M[6], M[10], M[14],
            M[3], M[7], M[11], M[15]
        };
        std::copy_n( Mt, 16, dst->m_payload.m_floats );                         // store
    }
#endif
}


void
TransformCompute::boundingBoxTest( Value* dst, const unsigned int N, const Value** src )
{
    bool modified = false;
    for( unsigned int i=0 ; i<N; i++ ) {
        bool m = dst->valueChanged().moveForward( src[i]->valueChanged() );
        modified = modified | m;
    }
    if( !modified || (N<3) ) {
        return;
    }

#ifdef __SSE4_2__
    __m128 Ar0 = _mm_load_ps( src[2]->floatData() + 0 );                        // fetch first matrix transposed
    __m128 Ar1 = _mm_load_ps( src[2]->floatData() + 4 );
    __m128 Ar2 = _mm_load_ps( src[2]->floatData() + 8 );
    __m128 Ar3 = _mm_load_ps( src[2]->floatData() + 12 );
    _MM_TRANSPOSE4_PS( Ar0, Ar1, Ar2, Ar3 );
    for( unsigned int i=3; i<N; i++ ) {                                         // right multiply matrices
        _MM_MUL4TX4_PS( Ar0, Ar1, Ar2, Ar3, src[i]->floatData() );
    }
    __m128 bmin = _mm_load_ps( src[0]->floatData() );          // load bbox
    __m128 bmax = _mm_load_ps( src[1]->floatData() );

    __m128 p0 = bmin;                                                           // position of the corners
    __m128 p1 = _mm_blend_ps( bmin, bmax, 1 );
    __m128 p2 = _mm_blend_ps( bmin, bmax, 2 );
    __m128 p3 = _mm_blend_ps( bmin, bmax, 3 );
    __m128 p4 = _mm_blend_ps( bmin, bmax, 4 );
    __m128 p5 = _mm_blend_ps( bmin, bmax, 5 );
    __m128 p6 = _mm_blend_ps( bmin, bmax, 6 );
    __m128 p7 = bmax;


    __m128 w0p, w1p;                                                            // calc w coordinates
    w0p = _mm_dp_ps(                 Ar3, p0, 0xf1 );
    w0p = _mm_or_ps( w0p, _mm_dp_ps( Ar3, p1, 0xf2 ) );
    w0p = _mm_or_ps( w0p, _mm_dp_ps( Ar3, p2, 0xf4 ) );
    w0p = _mm_or_ps( w0p, _mm_dp_ps( Ar3, p3, 0xf8 ) );
    w1p = _mm_dp_ps(                 Ar3, p4, 0xf1 );
    w1p = _mm_or_ps( w1p, _mm_dp_ps( Ar3, p5, 0xf2 ) );
    w1p = _mm_or_ps( w1p, _mm_dp_ps( Ar3, p6, 0xf4 ) );
    w1p = _mm_or_ps( w1p, _mm_dp_ps( Ar3, p7, 0xf8 ) );


    static const __m128 signmask = _mm_castsi128_ps( _mm_set1_epi32( 0x80000000 ));
    __m128 w0n = _mm_xor_ps( w0p, signmask );                                   // calc -w
    __m128 w1n = _mm_xor_ps( w1p, signmask );

    __m128 x0, x1;                                                              // calc x coordinates
    x0 = _mm_dp_ps(                Ar0, p0, 0xf1 );
    x0 = _mm_or_ps( x0, _mm_dp_ps( Ar0, p1, 0xf2 ) );
    x0 = _mm_or_ps( x0, _mm_dp_ps( Ar0, p2, 0xf4 ) );
    x0 = _mm_or_ps( x0, _mm_dp_ps( Ar0, p3, 0xf8 ) );
    x1 = _mm_dp_ps(                Ar0, p4, 0xf1 );
    x1 = _mm_or_ps( x1, _mm_dp_ps( Ar0, p5, 0xf2 ) );
    x1 = _mm_or_ps( x1, _mm_dp_ps( Ar0, p6, 0xf4 ) );
    x1 = _mm_or_ps( x1, _mm_dp_ps( Ar0, p7, 0xf8 ) );

    // determine if x-planes break the inside-box test
    bool xp = _mm_movemask_ps( _mm_or_ps( _mm_cmplt_ps( x0, w0p ),      // x < w
                                          _mm_cmplt_ps( x1, w1p ) ) ) != 0;
    bool xn = _mm_movemask_ps( _mm_or_ps( _mm_cmpgt_ps( x0, w0n ),      // -w < x
                                          _mm_cmpgt_ps( x1, w1n ) ) ) != 0;
    bool inside = xp && xn;

    if( inside ) {
        __m128 y0, y1;                                                          // calc y-coordinates
        y0 = _mm_dp_ps(                Ar1, p0, 0xf1 );
        y0 = _mm_or_ps( y0, _mm_dp_ps( Ar1, p1, 0xf2 ) );
        y0 = _mm_or_ps( y0, _mm_dp_ps( Ar1, p2, 0xf4 ) );
        y0 = _mm_or_ps( y0, _mm_dp_ps( Ar1, p3, 0xf8 ) );
        y1 = _mm_dp_ps(                Ar1, p4, 0xf1 );
        y1 = _mm_or_ps( y1, _mm_dp_ps( Ar1, p5, 0xf2 ) );
        y1 = _mm_or_ps( y1, _mm_dp_ps( Ar1, p6, 0xf4 ) );
        y1 = _mm_or_ps( y1, _mm_dp_ps( Ar1, p7, 0xf8 ) );
        // determine if y-planes break the inside-box test
        bool yp = _mm_movemask_ps( _mm_or_ps( _mm_cmplt_ps( y0, w0p ),      // y < w
                                              _mm_cmplt_ps( y1, w1p ) ) ) != 0;
        bool yn = _mm_movemask_ps( _mm_or_ps( _mm_cmpgt_ps( y0, w0n ),      // -w < y
                                              _mm_cmpgt_ps( y1, w1n ) ) ) != 0;
        inside = inside && yp;
        inside = inside && yn;

        if( inside ) {
            __m128 z0, z1;                                                      // calc z-coordinates
            z0 = _mm_dp_ps(                Ar2, p0, 0xf1 );                     // calc z for p0..p3
            z0 = _mm_or_ps( z0, _mm_dp_ps( Ar2, p1, 0xf2 ) );
            z0 = _mm_or_ps( z0, _mm_dp_ps( Ar2, p2, 0xf4 ) );
            z0 = _mm_or_ps( z0, _mm_dp_ps( Ar2, p3, 0xf8 ) );
            z1 = _mm_dp_ps(                Ar2, p4, 0xf1 );                     // calc z for p4..p7
            z1 = _mm_or_ps( z1, _mm_dp_ps( Ar2, p5, 0xf2 ) );
            z1 = _mm_or_ps( z1, _mm_dp_ps( Ar2, p6, 0xf4 ) );
            z1 = _mm_or_ps( z1, _mm_dp_ps( Ar2, p7, 0xf8 ) );
            // determine if z-planes break the inside-box test
            bool zp = _mm_movemask_ps( _mm_or_ps( _mm_cmplt_ps( z0, w0p ),      // y < w
                                                  _mm_cmplt_ps( z1, w1p ) ) ) != 0;
            bool zn = _mm_movemask_ps( _mm_or_ps( _mm_cmpgt_ps( z0, w0n ),      // -w < y
                                                  _mm_cmpgt_ps( z1, w1n ) ) ) != 0;
            inside = inside && zp;
            inside = inside && zn;
        }
    }
    dst->m_payload.m_bools[0] = inside ? GL_TRUE : GL_FALSE;
#else
    const float* p = src[2]->floatData();
    float M[16] = {                                                         // load first matrix transposed
        p[0], p[4], p[8], p[12],
        p[1], p[5], p[9], p[13],
        p[2], p[6], p[10], p[14],
        p[3], p[7], p[11], p[15]
    };
    for( unsigned int i=3; i<N; i++ ) {
        _MUL4TX4_PS( M, src[i]->floatData() );
    }

    const float* bbmin = src[0]->floatData();
    const float* bbmax = src[1]->floatData();
    const float P[4*8] = {                                                  // create bbox corners
        bbmin[0], bbmin[1], bbmin[2], 1.f,
        bbmin[0], bbmin[1], bbmax[2], 1.f,
        bbmin[0], bbmax[1], bbmin[2], 1.f,
        bbmin[0], bbmax[1], bbmax[2], 1.f,
        bbmax[0], bbmin[1], bbmin[2], 1.f,
        bbmax[0], bbmin[1], bbmax[2], 1.f,
        bbmax[0], bbmax[1], bbmin[2], 1.f,
        bbmax[0], bbmax[1], bbmax[2], 1.f,
    };
    unsigned int mask_a = 0u;
    unsigned int mask_b = 0u;
    for( unsigned int i=0; i<8; i++ ) {                                     // transform point
        const float* p = P + 4*i;
        const float h[4] = { M[ 0]*p[0] + M[ 1]*p[1] + M[ 2]*p[2] + M[ 3]*p[3],
                             M[ 4]*p[0] + M[ 5]*p[1] + M[ 6]*p[2] + M[ 7]*p[3],
                             M[ 8]*p[0] + M[ 9]*p[1] + M[10]*p[2] + M[11]*p[3],
                             M[12]*p[0] + M[13]*p[1] + M[14]*p[2] + M[15]*p[3] };
        mask_a = mask_a | (h[0]<h[3]?0x1:0x0) | (h[1]<h[3]?0x2:0x0) | (h[2]<h[3]?0x4:0x0);
        mask_b = mask_b | (h[0]>-h[3]?0x1:0x0) | (h[1]>-h[3]?0x2:0x0) | (h[2]>-h[3]?0x4:0x0);
    }
    dst->m_payload.m_bools[0] = (mask_a & mask_b) == 7u ? GL_TRUE : GL_FALSE;
#endif
}


    } // of namespace Runtime
} // of namespace Scene

