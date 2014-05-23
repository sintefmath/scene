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

#include <scene/Log.hpp>
#include <scene/Pass.hpp>
#include <scene/glsl/GLSLRuntime.hpp>

namespace Scene {
    namespace Runtime {
        using std::string;
        using std::vector;


GLSLShader::GLSLShader()
    : m_program( 0 ),
      m_shader_vertex(0),
      m_shader_geometry(0),
      m_shader_tess_ctrl(0),
      m_shader_tess_eval(0),
      m_shader_fragment(0)
{
}

GLSLShader::~GLSLShader()
{
    release();
}

void
GLSLShader::release()
{
    if( m_program != 0 ) {
        glDeleteProgram( m_program );
        m_program = 0;
    }
    if( m_shader_vertex != 0 ) {
        glDeleteShader( m_shader_vertex );
        m_shader_vertex = 0;
    }

    if( m_shader_geometry != 0 ) {
        glDeleteShader( m_shader_geometry );
        m_shader_geometry = 0;
    }
    if( m_shader_tess_ctrl != 0 ) {
        glDeleteShader( m_shader_tess_ctrl );
        m_shader_tess_ctrl = 0;
    }
    if( m_shader_tess_eval != 0 ) {
        glDeleteShader( m_shader_tess_eval );
        m_shader_tess_eval = 0;
    }
    if( m_shader_fragment != 0 ) {
        glDeleteShader( m_shader_fragment );
        m_shader_fragment = 0;
    }

}

bool
GLSLShader::pull( const Pass* pass )
{
    Logger log = getLogger( "Scene.Runtime.GLSLShader.pull" );
    release();

    bool has_tessellation_shader = false;
    bool has_geometry_shader = false;

    m_program = glCreateProgram();

    const string& src_v = pass->shaderSource( STAGE_VERTEX );
    if( !src_v.empty() ) {
        m_shader_vertex = glCreateShader( GL_VERTEX_SHADER );
        if( !compileShader( m_shader_vertex, src_v ) ) {
            release();
            return false;
        }
        glAttachShader( m_program, m_shader_vertex );
    }


    const string& src_tc = pass->shaderSource( STAGE_TESSELLATION_CONTROL );
    if( !src_tc.empty() ) {
        m_shader_tess_ctrl = glCreateShader( GL_TESS_CONTROL_SHADER );
        if( !compileShader( m_shader_tess_ctrl, src_tc ) ) {
            release();
            return false;
        }
        glAttachShader( m_program, m_shader_tess_ctrl );
        has_tessellation_shader = true;
    }

    const string& src_te = pass->shaderSource( STAGE_TESSELLATION_EVALUATION );
    if( !src_te.empty() ) {
        m_shader_tess_eval = glCreateShader( GL_TESS_EVALUATION_SHADER );
        if( !compileShader( m_shader_tess_eval, src_te ) ) {
            release();
            return false;
        }
        glAttachShader( m_program, m_shader_tess_eval );
    }

    const string& src_g = pass->shaderSource( STAGE_GEOMETRY );
    if( !src_g.empty() ) {
        m_shader_geometry = glCreateShader( GL_GEOMETRY_SHADER );
        if( !compileShader( m_shader_geometry, src_g ) ) {
            release();
            return false;
        }
        glAttachShader( m_program, m_shader_geometry );
        has_geometry_shader = true;
    }

    const string& src_f = pass->shaderSource( STAGE_FRAGMENT );
    if( !src_f.empty() ) {
        m_shader_fragment = glCreateShader( GL_FRAGMENT_SHADER );
        if( !compileShader( m_shader_fragment, src_f ) ) {
            release();
            return false;
        }
        glAttachShader( m_program, m_shader_fragment );
    }

    if( !linkProgram() ) {
        release();
        return false;
    }
    SCENELOG_DEBUG( log, "Built shader program " << pass->key() << " ("<< m_timestamp.string() << ") " << m_program );

    if( has_tessellation_shader ) {
        m_expected_input_primitive_type = GL_PATCHES;
    }
    else if( has_geometry_shader ) {
        glGetProgramiv( m_program, GL_GEOMETRY_INPUT_TYPE, reinterpret_cast<GLint*>( &m_expected_input_primitive_type ) );
    }
    else {
        m_expected_input_primitive_type = GL_ALWAYS;
    }

    string primitive_type_str;
    switch( m_expected_input_primitive_type ) {
    case GL_ALWAYS:         primitive_type_str = "GL_ALWAYS"; break;
    case GL_POINTS:         primitive_type_str = "GL_POINTS"; break;
    case GL_LINES:          primitive_type_str = "GL_LINES"; break;
    case GL_LINE_STRIP:     primitive_type_str = "GL_LINE_STRIP"; break;
    case GL_LINE_LOOP:      primitive_type_str = "GL_LINE_LOOP"; break;
    case GL_TRIANGLES:      primitive_type_str = "GL_TRIANGLES"; break;
    case GL_TRIANGLE_STRIP: primitive_type_str = "GL_TRIANGLE_STRIP"; break;
    case GL_TRIANGLE_FAN:   primitive_type_str = "GL_TRIANGLE_FAN"; break;
    case GL_QUADS:          primitive_type_str = "GL_QUADS"; break;
    case GL_QUAD_STRIP:     primitive_type_str = "GL_QUAD_STRIP"; break;
    case GL_POLYGON:        primitive_type_str = "GL_POLYGON"; break;
    case GL_PATCHES:        primitive_type_str = "GL_PATCHES"; break;
    default:                primitive_type_str = "<unknown>"; break;
    }
    SCENELOG_DEBUG( log, "expected input primitive type: " << primitive_type_str );

    retrieveAttributeInfo( pass );
    retrieveUniformInfo( pass );

    m_timestamp = pass->valueChanged();
    return true;
}

void
GLSLShader::retrieveAttributeInfo( const Pass* pass )
{
    Logger log = getLogger( "Scene.Runtime.GLSLShader.retrieveAttributeInfo" );
//    const Program* program = pass->program();

    GLint active_attribs;
    glGetProgramiv( m_program, GL_ACTIVE_ATTRIBUTES, &active_attribs );
    SCENELOG_DEBUG( log, "GL_ACTIVE_ATTRIBUTES=" << active_attribs );


    m_attributes.resize( pass->attributes() );
    for( size_t i=0; i<m_attributes.size(); i++) {
        m_attributes[i].m_location = glGetAttribLocation( m_program,
                                                          pass->attributeSymbol(i).c_str() );
        if( m_attributes[i].m_location < 0 ) {
            continue;
        }

        // seems like index og GetActiveAttrib doesn't match location, so we seek
        // through all and find the matching name.

        string name;
        for( GLint k=0; k<active_attribs; k++) {
            GLint gl_size;
            GLenum gl_type;
            GLchar gl_name[256];
            glGetActiveAttrib( m_program,
                               k,
                               sizeof(gl_name),
                               NULL,
                               &gl_size,
                               &gl_type,
                               gl_name );
            name = string(gl_name);
            if( name == pass->attributeSymbol(i) ) {
                switch( gl_type ) {
                case GL_FLOAT:
                    m_attributes[i].m_element_type = GL_FLOAT;
                    m_attributes[i].m_element_size = sizeof(GLfloat);
                    m_attributes[i].m_components = 1;
                    break;
                case GL_FLOAT_VEC2:
                    m_attributes[i].m_element_type = GL_FLOAT;
                    m_attributes[i].m_element_size = sizeof(GLfloat);
                    m_attributes[i].m_components = 2;
                    break;
                case GL_FLOAT_VEC3:
                    m_attributes[i].m_element_type = GL_FLOAT;
                    m_attributes[i].m_element_size = sizeof(GLfloat);
                    m_attributes[i].m_components = 3;
                    break;
                case GL_FLOAT_VEC4:
                    m_attributes[i].m_element_type = GL_FLOAT;
                    m_attributes[i].m_element_size = sizeof(GLfloat);
                    m_attributes[i].m_components = 4;
                    break;
                default:
                    SCENELOG_ERROR( log, "attribute gl_type " <<
                                   reinterpret_cast<void*>( gl_type) <<
                                   " not supported." );
                    m_attributes[i].m_location = -1;
                    break;
                }
                goto found_attribute;
            }
        }
        SCENELOG_ERROR( log, "Couldn't find attribute " << pass->attributeSymbol(i) );
        m_attributes[i].m_location = -1;
        continue;

    found_attribute:
        SCENELOG_DEBUG( log, "attribute " << pass->attributeSymbol(i) <<
                        ", gl_name=" << name <<
                        ", location=" << m_attributes[i].m_location <<
                        ", element_type=" << m_attributes[i].m_element_type <<
                        ", components=" << m_attributes[i].m_components );
    }
}


void
GLSLShader::retrieveUniformInfo( const Pass* pass )
{
    Logger log = getLogger( "Scene.Runtime.GLSLShader.retrieveUniformInfo" );

    GLint active_uniforms;
    glGetProgramiv( m_program, GL_ACTIVE_UNIFORMS, &active_uniforms );
    SCENELOG_DEBUG( log, "  ACTIVE_UNIFORMS=" << active_uniforms );


    m_uniforms.resize( pass->uniforms() );
    for( size_t i=0; i<m_uniforms.size(); i++) {
        m_uniforms[i].m_location = glGetUniformLocation( m_program, pass->uniformSymbol(i).c_str() );
        if( m_uniforms[i].m_location < 0 ) {
             m_uniforms[i].m_type = VALUE_TYPE_N;
             SCENELOG_DEBUG( log, "  symbol '" << pass->uniformSymbol(i) << "' not found in shader prog " << m_program );

             continue;
        }

        GLint gl_size;
        GLenum gl_type;
        GLchar name[256];

        glGetActiveUniform( m_program,
                            m_uniforms[i].m_location,
                            sizeof(name),
                            NULL,
                            &gl_size,
                            &gl_type,
                            name );
        switch( gl_type ) {
        case GL_INT:
            m_uniforms[i].m_type = VALUE_TYPE_INT;
            break;
        case GL_FLOAT:
            m_uniforms[i].m_type = VALUE_TYPE_FLOAT;
            break;
        case GL_FLOAT_VEC2:
            m_uniforms[i].m_type = VALUE_TYPE_FLOAT2;
            break;
        case GL_FLOAT_VEC3:
            m_uniforms[i].m_type = VALUE_TYPE_FLOAT3;
            break;
        case GL_FLOAT_VEC4:
            m_uniforms[i].m_type = VALUE_TYPE_FLOAT4;
            break;

        case GL_FLOAT_MAT3:
            m_uniforms[i].m_type = VALUE_TYPE_FLOAT3X3;
            break;
        case GL_FLOAT_MAT4:
            m_uniforms[i].m_type = VALUE_TYPE_FLOAT4X4;
            break;

        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_SHADOW:
        case GL_SAMPLER_2D_SHADOW:
            m_uniforms[i].m_type = VALUE_TYPE_INT;
            break;


            // Types that are not supported yet
        case GL_INT_VEC2:
            SCENELOG_ERROR( log, "GL_INT_VEC2 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_INT_VEC3:
            SCENELOG_ERROR( log, "GL_INT_VEC3 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_INT_VEC4:
            SCENELOG_ERROR( log, "GL_INT_VEC4 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_BOOL:
            SCENELOG_ERROR( log, "GL_BOOL not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_BOOL_VEC2:
            SCENELOG_ERROR( log, "GL_BOOL_VEC2 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_BOOL_VEC3:
            SCENELOG_ERROR( log, "GL_BOOL_VEC3 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_BOOL_VEC4:
            SCENELOG_ERROR( log, "GL_BOOL_VEC4 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_FLOAT_MAT2:
            SCENELOG_ERROR( log, "GL_BOOL_FLOAT_MAT2 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_FLOAT_MAT2x3:
            SCENELOG_ERROR( log, "GL_BOOL_FLOAT_MAT2x3 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_FLOAT_MAT2x4:
            SCENELOG_ERROR( log, "GL_BOOL_FLOAT_MAT2x4 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_FLOAT_MAT3x2:
            SCENELOG_ERROR( log, "GL_BOOL_FLOAT_MAT3x2 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_FLOAT_MAT3x4:
            SCENELOG_ERROR( log, "GL_BOOL_FLOAT_MAT3x4 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_FLOAT_MAT4x2:
            SCENELOG_ERROR( log, "GL_BOOL_FLOAT_MAT4x2 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        case GL_FLOAT_MAT4x3:
            SCENELOG_ERROR( log, "GL_BOOL_FLOAT_MAT4x3 not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            break;
        default:
            SCENELOG_ERROR( log, "uniform gl_type " <<
                           reinterpret_cast<void*>( gl_type) <<
                           " not supported." );
            m_uniforms[i].m_location = -1;
            m_uniforms[i].m_type = VALUE_TYPE_N;
            continue;
        }

        SCENELOG_DEBUG( log, "symbol='" << pass->uniformSymbol(i) <<
                        ", location=" << m_uniforms[i].m_location <<
                        ", type=" << reinterpret_cast<void*>( m_uniforms[i].m_type ) <<
                        ", name=" << name );

    }

}


bool
GLSLShader::compileShader( GLuint shader, const std::string& source )
{
    Logger log = getLogger( "Scene.Runtime.GLSLShader.compileShader" );

    const GLchar* src = source.c_str();
    glShaderSource( shader, 1, &src, NULL );
    glCompileShader( shader );

    // check compilation status
    GLint status;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
    if( status != GL_TRUE ) {
        string error_message = "\nSource:\n" + source + "\nLog:\n";

        GLint loglength;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &loglength );
        if( loglength != 0 ) {
            vector<GLchar> log( loglength );
            glGetShaderInfoLog( shader, loglength, NULL, log.data() );
            error_message += reinterpret_cast<const char*>( log.data() );
        }
        else {
            error_message += "<no log>";
        }
        SCENELOG_ERROR( log, "Failed to compile shader: " << error_message );
        return false;
    }
    return true;
}

bool
GLSLShader::linkProgram()
{
    Logger log = getLogger( "Scene.Runtime.GLSLShader.linkProgram" );

    glLinkProgram( m_program );

    // check link status
    GLint status;
    glGetProgramiv( m_program, GL_LINK_STATUS, &status );
    if( status != GL_TRUE ) {
        string error_message = "\nLog:\n";

        GLint loglength;
        glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &loglength );
        if( loglength != 0 ) {
            vector<GLchar> log( loglength );
            glGetProgramInfoLog( m_program, loglength, NULL, log.data() );
            error_message += reinterpret_cast<const char*>( log.data() );
        }
        else {
            error_message += "<no log>";
        }
        SCENELOG_ERROR( log, "Failed to link program: " << error_message );
        return false;
    }
    return true;
}




    } // of namespace Runtime
} // of namespace Scene
