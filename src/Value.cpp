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

#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include "scene/Value.hpp"
#include "scene/Utils.hpp"
#include "scene/Log.hpp"

namespace Scene {
    using std::string;
    using std::stringstream;
    using std::runtime_error;
    using std::min;
    using std::max;
    using std::vector;

/*
Value::Value( const Value& value )
{
    m_type = value.m_type;
    m_instance_image = value.m_instance_image;
    m_payload = value.m_payload;
//    Logger log = getLogger( "Scene.Value.Value" );
//    SCENELOG_DEBUG( log, "const copy, type=0x" << std::hex << m_type << std::dec <<
//                    ", value=" << debugString() );

}
*/

void
Value::set( const ValueType type )
{
    Logger log = getLogger( "Scene.Value.set" );
    m_type = type;
    m_value_changed.touch();

    switch( m_type ) {
    case VALUE_TYPE_INT:
        m_payload.m_ints[0] = 0;
        break;
    case VALUE_TYPE_FLOAT:
        m_payload.m_floats[0] = 0.f;
        break;
    case VALUE_TYPE_FLOAT2:
        for(size_t i=0; i<2; i++) {
            m_payload.m_floats[i] = 0.f;
        }
        break;
    case VALUE_TYPE_FLOAT3:
        for(size_t i=0; i<3; i++) {
            m_payload.m_floats[i] = 0.f;
        }
        break;
    case VALUE_TYPE_FLOAT4:
        for(size_t i=0; i<4; i++) {
            m_payload.m_floats[i] = 0.f;
        }
        break;
    case VALUE_TYPE_FLOAT3X3:
        for(size_t i=0; i<9; i++) {
            m_payload.m_floats[i] = 0.f;
        }
        break;

    case VALUE_TYPE_FLOAT4X4:
        for(size_t i=0; i<16; i++) {
            m_payload.m_floats[i] = 0.f;
        }
        m_payload.m_floats[0]  = 1.f;
        m_payload.m_floats[5]  = 1.f;
        m_payload.m_floats[10] = 1.f;
        m_payload.m_floats[15] = 1.f;
        break;

    case VALUE_TYPE_ENUM:
        m_payload.m_enums[0] = GL_NONE;
        break;

    case VALUE_TYPE_ENUM2:
        m_payload.m_enums[0] = GL_NONE;
        m_payload.m_enums[1] = GL_NONE;
        break;

    case VALUE_TYPE_BOOL:
        m_payload.m_bools[0] = GL_FALSE;
        break;

    case VALUE_TYPE_SAMPLER1D:
    case VALUE_TYPE_SAMPLER2D:
    case VALUE_TYPE_SAMPLER3D:
    case VALUE_TYPE_SAMPLERCUBE:
    case VALUE_TYPE_SAMPLERDEPTH:
    case VALUE_TYPE_N:
        SCENELOG_ERROR( log, "Unimplemented code path " << __FILE__ << "@" << __LINE__ );
        break;
    }
}

Value
Value::createFloat3x3( )
{
    Value value;
    value.m_type = VALUE_TYPE_FLOAT3X3;
    value.m_value_changed.touch();
    value.m_payload.m_floats[0] = 1.f; value.m_payload.m_floats[1] = 0.f; value.m_payload.m_floats[2] = 0.f;
    value.m_payload.m_floats[3] = 0.f; value.m_payload.m_floats[4] = 1.f; value.m_payload.m_floats[5] = 0.f;
    value.m_payload.m_floats[6] = 0.f; value.m_payload.m_floats[7] = 0.f; value.m_payload.m_floats[8] = 1.f;
    return value;
}

Value
Value::createFloat3x3( const float* src )
{
    Value value;
    value.m_type = VALUE_TYPE_FLOAT3X3;
    value.m_value_changed.touch();
    for(size_t i=0; i<9; i++) {
        value.m_payload.m_floats[i] = src[i];
    }
    return value;
}

Value
Value::createFloat3x3( const float e00, const float e01, const float e02,
                       const float e10, const float e11, const float e12,
                       const float e20, const float e21, const float e22 )
{
    Value value;
    value.m_type = VALUE_TYPE_FLOAT3X3;
    value.m_value_changed.touch();
    value.m_payload.m_floats[0] = e00; value.m_payload.m_floats[1] = e10; value.m_payload.m_floats[2] = e20;
    value.m_payload.m_floats[3] = e01; value.m_payload.m_floats[4] = e11; value.m_payload.m_floats[5] = e21;
    value.m_payload.m_floats[6] = e02; value.m_payload.m_floats[7] = e12; value.m_payload.m_floats[8] = e22;
    return value;
}


Value
Value::createFloat4x4( )
{
    Value value;
    value.m_type = VALUE_TYPE_FLOAT4X4;
    value.m_value_changed.touch();
    value.m_payload.m_floats[0]  = 1.f; value.m_payload.m_floats[1]  = 0.f; value.m_payload.m_floats[2]  = 0.f; value.m_payload.m_floats[3]  = 0.f;
    value.m_payload.m_floats[4]  = 0.f; value.m_payload.m_floats[5]  = 1.f; value.m_payload.m_floats[6]  = 0.f; value.m_payload.m_floats[7]  = 0.f;
    value.m_payload.m_floats[8]  = 0.f; value.m_payload.m_floats[9]  = 0.f; value.m_payload.m_floats[10] = 1.f; value.m_payload.m_floats[11] = 0.f;
    value.m_payload.m_floats[12] = 0.f; value.m_payload.m_floats[13] = 0.f; value.m_payload.m_floats[14] = 0.f; value.m_payload.m_floats[15] = 1.f;
    return value;
}

Value
Value::createFloat4x4( const float e00, const float e01, const float e02, const float e03,
                       const float e10, const float e11, const float e12, const float e13,
                       const float e20, const float e21, const float e22, const float e23,
                       const float e30, const float e31, const float e32, const float e33 )
{
    Value value;
    value.m_type = VALUE_TYPE_FLOAT4X4;
    value.m_value_changed.touch();
    value.m_payload.m_floats[ 0] = e00; value.m_payload.m_floats[ 1] = e10;  value.m_payload.m_floats[ 2] = e20; value.m_payload.m_floats[ 3] = e30;
    value.m_payload.m_floats[ 4] = e01; value.m_payload.m_floats[ 5] = e11;  value.m_payload.m_floats[ 6] = e21; value.m_payload.m_floats[ 7] = e31;
    value.m_payload.m_floats[ 8] = e02; value.m_payload.m_floats[ 9] = e12;  value.m_payload.m_floats[10] = e22; value.m_payload.m_floats[11] = e32;
    value.m_payload.m_floats[12] = e03; value.m_payload.m_floats[13] = e13;  value.m_payload.m_floats[14] = e23; value.m_payload.m_floats[15] = e33;
    return value;
}

Value
Value::createInt( const int val )
{
    Value value;
    value.m_type = VALUE_TYPE_INT;
    value.m_value_changed.touch();
    value.m_payload.m_ints[0] = val;
    return value;
}


Value
Value::createFloat( const float val )
{
    Value value;
    value.m_type = VALUE_TYPE_FLOAT;
    value.m_value_changed.touch();
    value.m_payload.m_floats[0] = val;
    return value;
}

Value
Value::createFloat2( const float float0, const float float1 )
{
    Value value;
    value.m_type = VALUE_TYPE_FLOAT2;
    value.m_value_changed.touch();
    value.m_payload.m_floats[0] = float0;
    value.m_payload.m_floats[1] = float1;
    return value;
}

Value
Value::createFloat3( const float float0, const float float1, const float float2 )
{
    Value value;
    value.m_type = VALUE_TYPE_FLOAT3;
    value.m_value_changed.touch();
    value.m_payload.m_floats[0] = float0;
    value.m_payload.m_floats[1] = float1;
    value.m_payload.m_floats[2] = float2;
    return value;
}

Value
Value::createFloat4( const float float0, const float float1, const float float2, const float float3 )
{
    Value value;
    value.m_type = VALUE_TYPE_FLOAT4;
    value.m_value_changed.touch();
    value.m_payload.m_floats[0] = float0;
    value.m_payload.m_floats[1] = float1;
    value.m_payload.m_floats[2] = float2;
    value.m_payload.m_floats[3] = float3;
    return value;
}

Value
Value::createBool( const GLboolean bool0 )
{
    Value value;
    value.m_value_changed.touch();
    value.m_type = VALUE_TYPE_BOOL;
    value.m_payload.m_bools[0] = bool0;
    return value;
}

Value
Value::createEnum( const GLenum enum0 )
{
    Value value;
    value.m_value_changed.touch();
    value.m_type = VALUE_TYPE_ENUM;
    value.m_payload.m_enums[0] = enum0;
    return value;
}

Value
Value::createEnum2( const GLenum enum0, const GLenum enum1 )
{
    Value value;
    value.m_value_changed.touch();
    value.m_type = VALUE_TYPE_ENUM2;
    value.m_payload.m_enums[0] = enum0;
    value.m_payload.m_enums[1] = enum1;
    return value;
}



const std::string
Value::floats( size_t count, bool abbreviate ) const
{
    stringstream o;

    size_t mid = abbreviate ? 2 : count;

    for(size_t i=0; i<min(mid, count); i++) {
        if( i > 0 ) {
            o << " ";
        }
        o << m_payload.m_floats[i];
    }
    if( count > mid+2 ) {
        o << " ...";
    }

    for(size_t i=max(mid,count-2); i<count; i++) {
        o << " " << m_payload.m_floats[i];
    }
    return o.str();
}


const std::string
Value::ints( size_t count ) const
{
    stringstream o;
    for(size_t i=0; i<count; i++) {
        if( i > 0 ) {
            o << " ";
        }
        o << m_payload.m_ints[i];
    }
    return o.str();
}


const std::string
Value::enums( size_t count ) const
{
    stringstream o;
    o << std::hex;
    for(size_t i=0; i<count; i++) {
        if( i > 0 ) {
            o << " ";
        }
        o << "0x" << m_payload.m_enums[i];
    }
    o << std::dec;
    return o.str();
}

const std::string
Value::bools( size_t count ) const
{
    stringstream o;

    for(size_t i=0; i<count; i++) {
        if( i>0 ) {
            o << " ";
        }
        o << (boolData()[i]==GL_TRUE?"TRUE":"FALSE");
    }

    return o.str();
}

bool
Value::setBools( const std::string& source, const size_t count )
{
    Logger log = getLogger( "Scene.Value.setBool" );

    m_value_changed.touch();
    size_t p=0;
    const size_t l = source.length();
    for(size_t i=0; i<count; i++) {

        // beginning of token
        for( ; p<l && isspace( source[p] ); p++ ) ;

        // end of token
        size_t q=p;
        for( ; q<l && !isspace( source[q] ); q++ ) ;

        if( p < q ) {
            string token = source.substr( p, q-p );

            if( token == "TRUE" ) {
                m_payload.m_enums[i] = GL_TRUE;
            }
            else if( token == "1" ) {
                m_payload.m_enums[i] = GL_TRUE;
            }
            else if( token == "FALSE" ) {
                m_payload.m_enums[i] = GL_FALSE;
            }
            else if( token == "0" ) {
                m_payload.m_enums[i] = GL_FALSE;
            }
            else {
                SCENELOG_WARN( log, "unrecognized token '" << token << "'." );
                return false;
            }
            p = q;
        }
        else {
            return false;
        }
    }
    return true;
}


Value
Value::createFloat4x4( const float *src )
{
    Value value;

    value.m_type = VALUE_TYPE_FLOAT4X4;
    value.m_value_changed.touch();
    for(size_t i=0; i<16; i++) {
        value.m_payload.m_floats[i] = src[i];
    }
    return value;
}


const std::string
Value::samplerString( ) const
{
    stringstream o;
    o << "image='" << m_instance_image << "'";

    o << ", wrap_s=";
    switch( m_payload.m_sampler.m_wrap_s ) {
    case GL_REPEAT: o << "GL_REPEAT"; break;
    case GL_MIRRORED_REPEAT: o << "GL_MIRRORED_REPEAT"; break;
    case GL_CLAMP_TO_EDGE: o << "GL_CLAMP_TO_EDGE"; break;
    case GL_CLAMP_TO_BORDER: o << "GL_CLAMP_TO_BORDER"; break;
    default: o << "?"; break;
    }

    if( m_type != VALUE_TYPE_SAMPLER1D ) {
        o << ", wrap_t=";
        switch( m_payload.m_sampler.m_wrap_t ) {
        case GL_REPEAT: o << "GL_REPEAT"; break;
        case GL_MIRRORED_REPEAT: o << "GL_MIRRORED_REPEAT"; break;
        case GL_CLAMP_TO_EDGE: o << "GL_CLAMP_TO_EDGE"; break;
        case GL_CLAMP_TO_BORDER: o << "GL_CLAMP_TO_BORDER"; break;
        default: o << "?"; break;
        }
    }
    if( m_type == VALUE_TYPE_SAMPLER3D ) {
        o << ", wrap_p=";
        switch( m_payload.m_sampler.m_wrap_p ) {
        case GL_REPEAT: o << "GL_REPEAT"; break;
        case GL_MIRRORED_REPEAT: o << "GL_MIRRORED_REPEAT"; break;
        case GL_CLAMP_TO_EDGE: o << "GL_CLAMP_TO_EDGE"; break;
        case GL_CLAMP_TO_BORDER: o << "GL_CLAMP_TO_BORDER"; break;
        default: o << "?"; break;
        }
    }
    o << ", min=";
    switch( m_payload.m_sampler.m_min_filter ) {
    case GL_NEAREST: o << "NEAREST"; break;
    case GL_LINEAR: o << "LINEAR"; break;
    case GL_NEAREST_MIPMAP_NEAREST: o << "NEAREST_MIPMAP_NEAREST"; break;
    case GL_NEAREST_MIPMAP_LINEAR: o << "NEAREST_MIPMAP_LINEAR"; break;
    case GL_LINEAR_MIPMAP_NEAREST: o << "LINEAR_MIPMAP_NEAREST"; break;
    case GL_LINEAR_MIPMAP_LINEAR: o << "LINEAR_MIPMAP_LINEAR"; break;
    default: o << "?"; break;
    }
    o << ", mag=";
    switch( m_payload.m_sampler.m_mag_filter ) {
    case GL_NEAREST: o << "NEAREST"; break;
    case GL_LINEAR: o << "LINEAR"; break;
    case GL_NEAREST_MIPMAP_NEAREST: o << "NEAREST_MIPMAP_NEAREST"; break;
    case GL_NEAREST_MIPMAP_LINEAR: o << "NEAREST_MIPMAP_LINEAR"; break;
    case GL_LINEAR_MIPMAP_NEAREST: o << "LINEAR_MIPMAP_NEAREST"; break;
    case GL_LINEAR_MIPMAP_LINEAR: o << "LINEAR_MIPMAP_LINEAR"; break;
    default: o << "?"; break;
    }



    return o.str();
}


const std::string
Value::debugString() const
{

    string prefix = "";
    int count = 0;
    switch( m_type ) {
    case VALUE_TYPE_INT:
        return "int(" + ints( 1 ) + ")";
    case VALUE_TYPE_FLOAT:
        return "float(" + floats( 1 ) + ")";
    case VALUE_TYPE_FLOAT2:
        return "float2(" + floats( 2 ) + ")";
    case VALUE_TYPE_FLOAT3:
        return "float3(" + floats( 3 ) + ")";
    case VALUE_TYPE_FLOAT4:
        return "float4(" + floats( 4 ) + ")";
    case VALUE_TYPE_FLOAT3X3:
        return "float3x3(" + floats( 9 ) + ")";
    case VALUE_TYPE_FLOAT4X4:
        return "float4x4(" + floats( 16, true ) + ")";
    case VALUE_TYPE_BOOL:
        return "bool(" + bools( 1 ) + ")";
    case VALUE_TYPE_ENUM:
        return "enum(" + enums(1) + ")";
        break;
    case VALUE_TYPE_ENUM2:
        return "enum2(" + enums(2) + ")";
        break;
    case VALUE_TYPE_SAMPLER1D:
        return "sampler1D(" + samplerString() + ")";
    case VALUE_TYPE_SAMPLER2D:
        return "sampler2D(" + samplerString() + ")";
    case VALUE_TYPE_SAMPLER3D:
        return "sampler3D(" + samplerString() + ")";
    case VALUE_TYPE_SAMPLERCUBE:
        return "samplerCUBE(" + samplerString() + ")";
    case VALUE_TYPE_SAMPLERDEPTH:
        return "samplerDEPTH(" + samplerString() + ")";
    case VALUE_TYPE_N:
        return "undefined";
    default:
        return "error";
    }

    stringstream o;

    o << prefix << "(";
    for(int i=0; i<min(2, count); i++) {
        if( i > 0 ) {
            o << ", ";
        }
        o << m_payload.m_floats[i];
    }
    if( count > 4 ) {
        o << ", ...";
    }

    for(int i=max(2,count-2); i<count; i++) {
        o << ", " << m_payload.m_floats[i];
    }
    o << ")";
    return o.str();
}


bool
Value::isSampler() const
{
    switch( m_type ) {
    case VALUE_TYPE_SAMPLER1D:
    case VALUE_TYPE_SAMPLER2D:
    case VALUE_TYPE_SAMPLER3D:
    case VALUE_TYPE_SAMPLERCUBE:
    case VALUE_TYPE_SAMPLERDEPTH:
        return true;
    default:
        return false;
    }
}


Value
Value::createSampler2D( const std::string instance_image,
                 const GLenum wrap_s,
                 const GLenum wrap_t,
                 const GLenum min_filter,
                 const GLenum mag_filter )
{
    Value value;
    value.m_type = VALUE_TYPE_SAMPLER2D;
    value.m_value_changed.touch();
    value.m_instance_image = instance_image;
    value.m_payload.m_sampler.m_wrap_s = wrap_s;
    value.m_payload.m_sampler.m_wrap_t = wrap_t;
    value.m_payload.m_sampler.m_wrap_p = GL_NONE;
    value.m_payload.m_sampler.m_min_filter = min_filter;
    value.m_payload.m_sampler.m_mag_filter = mag_filter;
    return value;
}

Value
Value::createSampler3D( const std::string instance_image,
                 const GLenum wrap_s,
                 const GLenum wrap_t,
                 const GLenum wrap_p,
                 const GLenum min_filter,
                 const GLenum mag_filter )
{
    Value value;
    value.m_type = VALUE_TYPE_SAMPLER3D;
    value.m_value_changed.touch();
    value.m_instance_image = instance_image;
    value.m_payload.m_sampler.m_wrap_s = wrap_s;
    value.m_payload.m_sampler.m_wrap_t = wrap_t;
    value.m_payload.m_sampler.m_wrap_p = wrap_p;
    value.m_payload.m_sampler.m_min_filter = min_filter;
    value.m_payload.m_sampler.m_mag_filter = mag_filter;
    return value;
}

Value
Value::createSamplerCUBE( const std::string instance_image,
                   const GLenum wrap_s,
                   const GLenum wrap_t,
                   const GLenum min_filter,
                   const GLenum mag_filter )
{
    Value value;
    value.m_type = VALUE_TYPE_SAMPLERCUBE;
    value.m_value_changed.touch();
    value.m_instance_image = instance_image;
    value.m_payload.m_sampler.m_wrap_s = wrap_s;
    value.m_payload.m_sampler.m_wrap_t = wrap_t;
    value.m_payload.m_sampler.m_wrap_p = GL_NONE;
    value.m_payload.m_sampler.m_min_filter = min_filter;
    value.m_payload.m_sampler.m_mag_filter = mag_filter;
    return value;
}

Value
Value::createSamplerDEPTH( const std::string instance_image,
                    const GLenum wrap_s,
                    const GLenum wrap_t,
                    const GLenum min_filter,
                    const GLenum mag_filter )
{
    Value value;
    value.m_type = VALUE_TYPE_SAMPLERDEPTH;
    value.m_value_changed.touch();
    value.m_instance_image = instance_image;
    value.m_payload.m_sampler.m_wrap_s = wrap_s;
    value.m_payload.m_sampler.m_wrap_t = wrap_t;
    value.m_payload.m_sampler.m_wrap_p = GL_NONE;
    value.m_payload.m_sampler.m_min_filter = min_filter;
    value.m_payload.m_sampler.m_mag_filter = mag_filter;
    return value;
}





} // of namespace Scene
