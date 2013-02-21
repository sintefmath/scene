#include <scene/Log.hpp>
#include <scene/SourceBuffer.hpp>
#include <scene/glsl/GLSLRuntime.hpp>

namespace Scene {
    namespace Runtime {

GLSLBuffer::GLSLBuffer()
{
    m_buffer = 0;
}

GLSLBuffer::~GLSLBuffer()
{
    Logger log = getLogger( "Scene.Runtime.GLSLBuffer.~GLSLBuffer" );

    if( m_buffer != 0 ) {
        glDeleteBuffers( 1, &m_buffer );
        SCENELOG_DEBUG( log, "Released GL buffer " << m_buffer );
    }
}


void
GLSLBuffer::pull(const SourceBuffer *buffer)
{
    Logger log = getLogger( "Scene.Runtime.GLSLBuffer.pull" );

    if( m_buffer == 0 ) {
        glGenBuffers( 1, &m_buffer );
        SCENELOG_DEBUG( log, "Created GL buffer " << m_buffer );
    }

    switch( buffer->elementType() ) {
    case ELEMENT_FLOAT:
        m_element_type = GL_FLOAT;
        m_element_size = sizeof( GLfloat );
        break;
    case ELEMENT_INT:
        m_element_type = GL_INT;
        m_element_size = sizeof( GLint );
        break;
    }
    m_element_count = buffer->elementCount();
    m_buffer_size = m_element_size * m_element_count;

    SCENELOG_DEBUG( log, "Upload to GPU: buf=" << m_buffer <<
                   ", type=" << reinterpret_cast<void*>( m_element_type ) <<
                   ", count=" << m_element_count <<
                   ", size=" << m_buffer_size << " bytes." );

    glBindBuffer( GL_ARRAY_BUFFER, m_buffer );
    glBufferData( GL_ARRAY_BUFFER, m_buffer_size, buffer->voidData(), GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}


    } // of namespace Runtime
} // of namespace Scene

