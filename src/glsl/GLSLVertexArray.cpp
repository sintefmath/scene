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
#include <scene/SourceBuffer.hpp>
#include <scene/glsl/GLSLRuntime.hpp>

namespace Scene {
    namespace Runtime {

GLSLVertexArray::GLSLVertexArray()
{
    m_vertex_array = 0;
}

GLSLVertexArray::~GLSLVertexArray()
{
    release();
}

void
GLSLVertexArray::release()
{
    Logger log = getLogger( "Scene.Runtime.GLSLVertexArray.release" );
    if( m_vertex_array != 0 ) {
        SCENELOG_DEBUG( log, "Released vertex array " << m_vertex_array );
        glDeleteVertexArrays( 1, &m_vertex_array );
        m_vertex_array = 0;
    }
}

void
GLSLVertexArray::pull( const std::vector<const GLSLBuffer*> sources,
                       const GLSLShader*                    shader,
                       const std::vector<SetInputs::Item>&  items )
{
    Logger log = getLogger( "Scene.Runtime.GLSLVertexArray.pull" );

    release();
    glGenVertexArrays( 1, &m_vertex_array );
    SCENELOG_DEBUG( log, "Created vertex array " << m_vertex_array );
    glBindVertexArray( m_vertex_array );

    for( size_t i=0; i<sources.size(); i++ ) {
        GLint loc = shader->attribLocation(i);
        if( loc < 0 ) {
            continue;
        }
        if( sources[i]->elementType() != shader->attribElementType(i) ) {
            SCENELOG_ERROR( log, "Element type mismatch" <<
                            ", buffer element type=" << reinterpret_cast<void*>( sources[i]->elementType() ) <<
                            ", attrib element type=" << reinterpret_cast<void*>( shader->attribElementType(i) ) );
            continue;
        }
        glBindBuffer( GL_ARRAY_BUFFER, sources[i]->buffer() );
        glEnableVertexAttribArray( loc );
        glVertexAttribPointer( loc,
                               shader->attribComponents(i),
                               shader->attribElementType(i),
                               GL_FALSE,
                               shader->attribElementSize(i) * items[i].m_stride,
                               reinterpret_cast<GLvoid*>( shader->attribElementSize(i) * items[i].m_offset ) );
        SCENELOG_DEBUG( log,
                        "loc=" << loc <<
                        ", components=" << shader->attribComponents(i) <<
                        ", stride=" << shader->attribElementSize(i) * items[i].m_stride  << " bytes"
                        ", offset=" << shader->attribElementSize(i) * items[i].m_offset  << " bytes");
    }
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );
}


    } // of namespace Runtime
} // of namespace Scene


