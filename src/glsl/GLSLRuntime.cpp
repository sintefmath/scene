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

#include <algorithm>
#include <scene/Log.hpp>
#include <scene/Image.hpp>
#include <scene/SourceBuffer.hpp>
#include <scene/glsl/GLSLRuntime.hpp>

namespace Scene {
    namespace Runtime {
        using std::string;
        using std::for_each;

bool
GLSLRuntime::checkGL( Logger& log, const string& message )
{
    GLenum error = glGetError();
    if( error == GL_NO_ERROR ) {
        return true;
    }

    do {
        switch( error )
        {
#define HELPER(a) case (a): SCENELOG_ERROR( log, message << "GL error: " << (#a) ); break
        HELPER( GL_NO_ERROR );
        HELPER( GL_INVALID_ENUM );
        HELPER( GL_INVALID_VALUE );
        HELPER( GL_INVALID_OPERATION );
        HELPER( GL_STACK_OVERFLOW );
        HELPER( GL_STACK_UNDERFLOW );
        HELPER( GL_OUT_OF_MEMORY );
        HELPER( GL_TABLE_TOO_LARGE );
#undef HELPER
        default:
            SCENELOG_ERROR( log, message << "GL error: 0x" << std::hex << error << std::dec );
        }

        error = glGetError();
    } while (error != GL_NO_ERROR);
    return false;
}

GLSLRuntime::GLSLRuntime( const Scene::DataBase& database )
    : m_database( database ),
      m_resolver( m_database, PROFILE_GLSL, "" )
{
    setReferenceSize( 64, 64 );
}

GLSLRuntime::~GLSLRuntime()
{
    clear();
}

void
GLSLRuntime::clear()
{
    for_each( m_framebuffer_cache.begin(),
              m_framebuffer_cache.end(),
             []( std::pair<const string,GLSLFrameBuffer*> a){ delete a.second; } );
    m_framebuffer_cache.clear();

    for_each( m_shader_cache.begin(),
              m_shader_cache.end(),
             []( std::pair<const string,GLSLShader*> a){ delete a.second; } );
    m_shader_cache.clear();

    for_each( m_buffer_cache.begin(),
              m_buffer_cache.end(),
             []( std::pair<const string,GLSLBuffer*> a){ delete a.second; } );
    m_buffer_cache.clear();

    for_each( m_vbo_cache.begin(),
              m_vbo_cache.end(),
             []( std::pair<const string,GLSLVertexArray*> a){ delete a.second; } );
    m_vbo_cache.clear();

    for_each( m_samplers_cache.begin(),
              m_samplers_cache.end(),
             []( std::pair<const string,GLSLSamplers*> a){ delete a.second; } );
    m_samplers_cache.clear();

    for_each( m_texture_cache.begin(),
              m_texture_cache.end(),
             []( std::pair<const string,GLSLTexture*> a){ delete a.second; } );
    m_texture_cache.clear();

    m_resolver.clear();
}

void
GLSLRuntime::setReferenceSize( size_t w, size_t h )
{
    m_ref_viewport_width = w;
    m_ref_viewport_heigth = h;
    m_ref_viewport_timestamp.touch();
}



GLSLFrameBuffer*
GLSLRuntime::frameBuffer( const RenderAction* set_fb)
{
    Logger log = getLogger( "Scene.Runtime.GLSLRuntime.frameBuffer" );
    SCENELOG_DEBUG( log, "==============" );

    std::vector<GLSLTexture*> textures( set_fb->m_set_render_targets.m_items.size() );
    for(size_t i=0; i<textures.size(); i++) {
        textures[i] = texture( set_fb->m_set_render_targets.m_items[i].m_image );
        if( textures[i] == NULL ) {
            return NULL;
        }
    }

    GLSLFrameBuffer* fb = NULL;

    const string key = set_fb->m_id;
    auto it = m_framebuffer_cache.find( key );
    if( it != m_framebuffer_cache.end() ) {
        bool tainted = false;
        tainted |= !it->second->timeStamp().asRecentAs( set_fb->m_timestamp );
        for(size_t i=0; i<textures.size(); i++ ) {
            tainted |= !it->second->timeStamp().asRecentAs( textures[i]->timeStamp() );
        }
        if( !tainted ) {
            return it->second;
        }
        else {
            fb = it->second;
        }
    }
    else {
        fb = new GLSLFrameBuffer;
        m_framebuffer_cache[ key ] = fb;
    }

    if( fb->pull( textures, set_fb->m_set_render_targets ) ) {
        return fb;
    }
    else {
        return NULL; // failure to pull.
    }
}


GLSLBuffer*
GLSLRuntime::buffer( const SourceBuffer* buffer )
{
    Logger log = getLogger( "Scene.Runtime.GLSLRuntime.buffer" );
    if( buffer == NULL ) {
        SCENELOG_FATAL( log, "buffer==NULL @" << __LINE__ );
        return NULL;
    }
    SCENELOG_TRACE( log, "Retrieving buffer " << buffer->id() );

    auto it = m_buffer_cache.find( buffer->id() );
    if( it != m_buffer_cache.end() ) {
        if( !it->second->timeStamp().asRecentAs( buffer->valueChanged() ) ) {
            SCENELOG_TRACE( log, "Pulling." );
            it->second->pull( buffer );
        }
        return it->second;
    }
    GLSLBuffer* glsl_buffer = new GLSLBuffer;
    glsl_buffer->pull( buffer );
    m_buffer_cache[ buffer->id() ] = glsl_buffer;
    return glsl_buffer;
}


GLSLTexture*
GLSLRuntime::texture( const Image* image )
{
    Logger log = getLogger( "Scene.Runtime.GLSLRuntime.texture" );
    const string key = image->key();
    auto it = m_texture_cache.find( key );
    if( it != m_texture_cache.end() ) {
        if( !it->second->timeStamp().asRecentAs( image->valueChanged() ) ) {
            it->second->pull( image );
        }
        return it->second;
    }
    GLSLTexture* texture = new GLSLTexture;
    texture->pull( image );
    m_texture_cache[ key ] = texture;
    return texture;
}



GLSLShader*
GLSLRuntime::shader( const Pass* pass )
{
    const string key = pass->key();
    auto it = m_shader_cache.find( key );
    if( it != m_shader_cache.end() ) {
        if( !it->second->timeStamp().asRecentAs( pass->structureChanged() ) ) {
            it->second->pull( pass );
        }
        return it->second;
    }
    GLSLShader* shader= new GLSLShader;
    m_shader_cache[ key ] = shader;
    if( shader->pull( pass ) ) {
        return shader;
    }
    else {
        return NULL;
    }
}

GLSLVertexArray*
GLSLRuntime::vbo( const RenderAction* set_inputs )
{
    Logger log = getLogger( "Scene.Runtime.GLSLRuntime.vbo" );

    if( set_inputs->m_type != RenderAction::ACTION_SET_INPUTS ) {
        SCENELOG_FATAL( log, "action is not of SET_INPUT type." );
        return NULL;
    }

    const Pass* pass = set_inputs->m_set_inputs.m_pass;
    const Primitives* primitives = set_inputs->m_set_inputs.m_primitives;

//    const string key = pass->key() + "@" + primitives->key();
    const string key = set_inputs->m_id;

    auto it = m_vbo_cache.find( key );

    GLSLVertexArray* vbo = NULL;
    if( it != m_vbo_cache.end() ) {

        bool tainted = false;
        tainted |= !it->second->timeStamp().asRecentAs( pass->structureChanged() );
        tainted |= !it->second->timeStamp().asRecentAs( primitives->geometry()->structureChanged() );
//        tainted |= primitives->geometry()->asset().majorChanges( it->second->timeStamp() );
//        tainted |= !it->second->timeStamp().asRecentAs( primitives->geometry()->timeStamp() );
        for( size_t i=0; i<set_inputs->m_set_inputs.m_items.size(); i++ ) {
            tainted |= it->second->timeStamp().asRecentAs(
                        set_inputs->m_set_inputs.m_items[i].m_source->structureChanged() );
        }

        if(!tainted) {
            return it->second;
        }
        else {
            vbo = it->second;
        }
    }
    else {
        vbo = new GLSLVertexArray;
        m_vbo_cache[ key ] = vbo;
    }

    const GLSLShader* program = shader( set_inputs->m_set_inputs.m_pass );

    std::vector<const GLSLBuffer*> buffers(set_inputs->m_set_inputs.m_items.size());
    for( size_t i=0; i<buffers.size(); i++ ) {
        buffers[i] = buffer( set_inputs->m_set_inputs.m_items[i].m_source );
    }
    vbo->pull( buffers, program, set_inputs->m_set_inputs.m_items );

    return vbo;
}

GLSLSamplers*
GLSLRuntime::samplers( const RenderAction* set_samplers )
{
    Logger log = getLogger( "Scene.Runtime.GLSLRuntime.samplers" );
    if( set_samplers->m_type != RenderAction::ACTION_SET_SAMPLERS ) {
        SCENELOG_FATAL( log, "action is not of SET_SAMPLERS type." );
        return NULL;
    }

    GLSLSamplers* samplers = NULL;
    auto it = m_samplers_cache.find( set_samplers->m_id );
    if( it != m_samplers_cache.end() ) {
        if( it->second->timeStamp().asRecentAs( set_samplers->m_timestamp ) ) {
            return it->second;
        }
        samplers = it->second;
    }
    else {
        samplers = new GLSLSamplers;
        m_samplers_cache[ set_samplers->m_id ] = samplers;
    }

    std::vector<GLSLTexture*> textures( set_samplers->m_set_samplers.m_items.size() );
    for( size_t i=0; i<textures.size(); i++) {
        textures[i] = texture( set_samplers->m_set_samplers.m_items[i].m_image );
    }
    samplers->pull( textures, set_samplers->m_set_samplers );
    return samplers;
}

    } // of namespace Runtime
} // of namespace Scene
