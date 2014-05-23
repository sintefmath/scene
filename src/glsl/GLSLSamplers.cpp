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


GLSLSamplers::GLSLSamplers()
{
}

GLSLSamplers::~GLSLSamplers()
{
    release();
}

void
GLSLSamplers::release()
{
    if( !m_samplers.empty() ) {
        glDeleteSamplers( m_samplers.size(), &m_samplers[0] );
        m_samplers.clear();
    }
}

void
GLSLSamplers::pull( const std::vector<GLSLTexture*>& textures,
                    const SetSamplers&               setsamplers )
{
    Logger log = getLogger( "Scene.Runtime.GLSLSamplers.pull" );
    release();

    if( textures.size() != setsamplers.m_items.size() ) {
        SCENELOG_FATAL( log, "textures.size() != setsamplers.m_items.size()" );
        return;
    }

    m_textures = textures;

    m_targets.resize( m_textures.size() );
    m_textures_name.resize( m_textures.size() );
    m_samplers.resize( m_textures.size() );
    glGenSamplers( m_samplers.size(), &m_samplers[0] );

    glActiveTexture( GL_TEXTURE0 );
    for(size_t i=0; i<m_samplers.size(); i++) {
        GLSLTexture* tex = m_textures[i];
        m_textures_name[i] = m_textures[i]->texture();
        m_targets[i] = m_textures[i]->target();

        SCENELOG_DEBUG( log, "  unit " << i <<
                        ", sampler=" << m_samplers[i] <<
                        ", texture=" << m_textures[i]->texture() << " (" << m_textures[i]->debugName() << ")" );


        glBindSampler( 0, m_samplers[i] );
        glBindTexture( tex->target(), 0 );

        glTexParameteri( m_targets[i], GL_TEXTURE_WRAP_S, setsamplers.m_items[i].m_wrap_s );
        if( m_targets[i] != GL_TEXTURE_1D ) {
            glTexParameteri( m_targets[i], GL_TEXTURE_WRAP_T, setsamplers.m_items[i].m_wrap_t );
            if( m_targets[i] == GL_TEXTURE_3D ) {
                glTexParameteri( m_targets[i], GL_TEXTURE_WRAP_R, setsamplers.m_items[i].m_wrap_p );
            }
        }

        glTexParameteri( tex->target(), GL_TEXTURE_MIN_FILTER, setsamplers.m_items[i].m_min_filter  );
        glTexParameteri( tex->target(), GL_TEXTURE_MAG_FILTER, setsamplers.m_items[i].m_mag_filter );


        glBindTexture( m_targets[i], 0 );

        glBindSampler( 0, 0 );
    }

}

    } // of namespace Runtime
} // of namespace Scene

