#include <scene/Log.hpp>
#include <scene/Image.hpp>
#include <scene/glsl/GLSLRuntime.hpp>

namespace Scene {
    namespace Runtime {


GLSLTexture::GLSLTexture()
    : m_texture(0)
{
}

GLSLTexture::~GLSLTexture()
{
    release();
}

void
GLSLTexture::release()
{
    if( m_texture != 0 ) {
        glDeleteTextures( 1, &m_texture );
        m_texture = 0;
    }
}



bool
GLSLTexture::doBuildMipMap()
{
    if( m_build_mipmap && m_tainted ) {
        m_tainted = false;
        return true;
    }
    else {
        return false;
    }

}


void
GLSLTexture::pull(const Image *image)
{
    Logger log = getLogger( "Scene.Runtime.GLSLTexture.pull" );

    if( m_texture == 0 ) {
        glGenTextures( 1, &m_texture );
    }
    m_tainted = true;

    m_debug_name = image->id();

    GLint unpack_alignment;
    glGetIntegerv( GL_UNPACK_ALIGNMENT, &unpack_alignment );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    m_width = image->width();
    m_height = image->height();
    m_depth = image->depth();
    m_build_mipmap = image->buildMipMap();

    if( image->type() == IMAGE_2D ) {
        m_target = GL_TEXTURE_2D;
        glBindTexture( GL_TEXTURE_2D, m_texture );
        const void* data = image->get(0,0);
        glTexImage2D( GL_TEXTURE_2D,
                      0,
                      image->suggestedInternalFormat(),
                      image->width(),
                      image->height(),
                      0,
                      image->format(),
                      image->elementType(),
                      data );
        glTexParameteri( m_target, GL_TEXTURE_WRAP_S, GL_CLAMP );
        glTexParameteri( m_target, GL_TEXTURE_WRAP_T, GL_CLAMP );
        glBindTexture( m_target, 0 );
        SCENELOG_DEBUG( log, "Built TEX2D " << image->key() );
    }
    else if( image->type() == IMAGE_3D ) {
        m_target = GL_TEXTURE_3D;
        glBindTexture( m_target, m_texture );
        glTexImage3D( m_target,
                      0,
                      image->suggestedInternalFormat(),
                      image->width(),
                      image->height(),
                      image->depth(),
                      0,
                      image->format(),
                      image->elementType(),
                      image->get(0,0) );
        glBindTexture( m_target, 0 );
    }
    else if( image->type() == IMAGE_CUBE ) {
        m_target = GL_TEXTURE_CUBE_MAP;
        glBindTexture( m_target, m_texture );
        for(size_t f=0; f<6; f++) {
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + f,
                          0,
                          image->suggestedInternalFormat(),
                          image->width(),
                          image->width(),
                          0,
                          image->format(),
                          image->elementType(),
                          image->get(0,f) );
        }
        glTexParameteri( m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
        glTexParameteri( m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
        glTexParameteri( m_target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );
        SCENELOG_DEBUG( log, "Built TEXCUBE " << image->key() );

        glGenerateMipmap( m_target );

        glBindTexture( m_target, 0 );
    }
    else {
        SCENELOG_ERROR( log, "Code path for image type " << image->type() << " not yet implemented." );
    }

    glPixelStorei( GL_UNPACK_ALIGNMENT, unpack_alignment );

    GLSLRuntime::checkGL( log );
}


    } // of namespace Runtime
} // of namespace Scene

