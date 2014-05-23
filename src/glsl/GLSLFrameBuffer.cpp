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
#include <scene/Image.hpp>
#include <scene/glsl/GLSLRuntime.hpp>

namespace Scene {
    namespace Runtime {



GLSLFrameBuffer::GLSLFrameBuffer()
    : m_fbo(0)
{
}

GLSLFrameBuffer::~GLSLFrameBuffer()
{
    release();
}

GLuint
GLSLFrameBuffer::fbo() const {
    for(auto it=m_textures.begin(); it!=m_textures.end(); ++it ) {
        (*it)->taint();
    }
    return m_fbo;
}


bool
GLSLFrameBuffer::pull( const std::vector<GLSLTexture*>& textures,
                       const SetRenderTargets&          set_fb )
{
    Logger log = getLogger( "Scene.Runtime.GLSLFrameBuffer.pull" );

    release();
    glGenFramebuffers( 1, &m_fbo );
    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );

    m_textures = textures;

    std::vector<GLenum> draw_buffers;
    for(size_t i=0; i<textures.size(); i++) {
        m_width = textures[i]->width();
        m_height = textures[i]->height();

        switch( textures[i]->target() ) {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
            glFramebufferTexture2D( set_fb.m_items[i].m_target,
                                    set_fb.m_items[i].m_attachment,
                                    set_fb.m_items[i].m_textarget,
                                    textures[i]->texture(),
                                    set_fb.m_items[i].m_level );
            draw_buffers.push_back( set_fb.m_items[i].m_attachment );
            break;

        default:
            break;

        }
        glDrawBuffers( draw_buffers.size(), &draw_buffers[0] );
    }

    GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if( status == GL_FRAMEBUFFER_COMPLETE ) {
        SCENELOG_DEBUG( log, "Framebuffer complete" );

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        GLSLRuntime::checkGL( log );
        return true;
    }
    else {


        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        release();

        switch( status ) {
#define HELPER(a) case (a): SCENELOG_ERROR( log, (#a) ); break
        HELPER( GL_FRAMEBUFFER_UNDEFINED );
        HELPER( GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT );
        HELPER( GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT );
        HELPER( GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER );
        HELPER( GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER );
        HELPER( GL_FRAMEBUFFER_UNSUPPORTED );
        HELPER( GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE );
        HELPER( GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS );
#undef HELPER
        default:
            SCENELOG_ERROR( log, "FBO error 0x" << std::hex << status << std::dec );
            break;
        }
        GLSLRuntime::checkGL( log );
        return false;
    }
}

void
GLSLFrameBuffer::release()
{
    glDeleteFramebuffers( 1, &m_fbo );
}

    } // of namespace Runtime
} // of namespace Scene


