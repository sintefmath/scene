#include <algorithm>
#include <cmath>
#include <cstring>
#include "scene/Log.hpp"
#include "scene/Image.hpp"
#include "scene/Library.hpp"

namespace Scene
{
    using std::max;

Image::Image( DataBase& database, const std::string& id )
: m_database( database ),
  m_id( id ),
  m_type( IMAGE_N )
{}

Image::Image( Library<Image>* library_images, const std::string& id )
    : m_database( *library_images->dataBase() ),
      m_id( id ),
      m_type( IMAGE_N )
{
}

bool
Image::setFormat( GLenum iformat, GLenum format, GLenum type )
{
    Logger log = getLogger( "Scene.Image.setFormat" );

    m_iformat = iformat;
    m_format = format;
    switch( m_format ) {
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_LUMINANCE:
        m_channels = 1;
        break;
    case GL_LUMINANCE_ALPHA:
        m_channels = 2;
        break;
    case GL_RGB:
        m_channels = 3;
        break;
    case GL_RGBA:
        m_channels = 4;
        break;
    default:
        SCENELOG_ERROR( log, "Unsupported format");
        return false;
    }
    m_element_type = type;
    switch( m_element_type ) {
    case GL_UNSIGNED_BYTE:
        m_element_size = sizeof(GLubyte);
        break;
    case GL_FLOAT:
        m_element_size = sizeof(GLfloat);
        break;
    default:
        SCENELOG_ERROR( log, "Unsupported type");
        return false;
    }
    return true;
}

bool
Image::initCube( GLenum iformat,
                 GLenum format,
                 GLenum type,
                 size_t width,
                 size_t array_length,
                 size_t mips,
                 bool   auto_generate )
{
    Logger log = getLogger( "Scene.Image.initCube" );
    if( !setFormat( iformat, format, type ) ) {
        return false;
    }
    if( width < 1 ) {
        SCENELOG_ERROR( log, "width < 1" );
        return false;
    }

    m_width = width;
    m_height = width;
    m_depth = width;
    m_type = IMAGE_CUBE;
    m_data.clear();

    return true;
}




bool
Image::init2D( GLenum iformat,
               GLenum format,
               GLenum type,
               size_t width,
               size_t height,
               size_t array_length,
               size_t mips,
               bool   auto_generate  )
{
    Logger log = getLogger( "Scene.Image.init2D" );
    if( !setFormat( iformat, format, type ) ) {
        return false;
    }
    if( width < 1 ) {
        SCENELOG_ERROR( log, "width < 1" );
        return false;
    }
    if( height < 1 ) {
        SCENELOG_ERROR( log, "heigth < 1" );
        return false;
    }

    m_width = width;
    m_height = height;
    m_depth = 1;
    m_type = IMAGE_2D;
    m_data.clear();
    return true;
}

bool
Image::init2D( GLenum iformat,
               GLenum format,
               GLenum type,
               float  width,
               float  height,
               size_t array_length,
               size_t mips,
               bool   auto_generate )
{
    Logger log = getLogger( "Scene.Image.init2D" );

    SCENELOG_FATAL( log, "Unimplemented code path!" );
    return false;
}

bool
Image::init3D( GLenum iformat,
               GLenum format,
               GLenum type,
               size_t width,
               size_t height,
               size_t depth,
               size_t array_length,
               size_t mips,
               bool   auto_generate )
{
    Logger log = getLogger( "Scene.Image.initCube" );
    if( !setFormat( iformat, format, type ) ) {
        return false;
    }
    if( width < 1 ) {
        SCENELOG_ERROR( log, "width < 1" );
        return false;
    }
    if( height < 1 ) {
        SCENELOG_ERROR( log, "heigth < 1" );
        return false;
    }
    if( depth < 1 ) {
        SCENELOG_ERROR( log, "depth < 1" );
        return false;
    }
    m_width = width;
    m_height = height;
    m_depth = depth;
    m_type = IMAGE_3D;
    m_data.clear();
    return true;
}


size_t
Image::texelsInSlice()
{
    switch( m_type ) {
    case IMAGE_2D:
    case IMAGE_3D:
    case IMAGE_CUBE:
        return m_width*m_height*m_channels;
        break;
    case IMAGE_N:
        break;
    }

    return 0;
}

const void*
Image::get( size_t mip_level, size_t slice ) const
{
    Logger log = getLogger( "Scene.Image.get" );

    if( m_data.empty() ) {
        return NULL;
    }

    if( mip_level != 0) {
        SCENELOG_FATAL( log, "Getting of non-zero mip-levels not yet supported." );
        return NULL;
    }

    // Sanity checks
    switch( m_type ) {
    case IMAGE_2D:
        if( slice != 0 ) {
            SCENELOG_FATAL( log, "IMAGE_2D & slice != 0" );
            return NULL;
        }
        break;
    case IMAGE_3D:
        if( slice >= m_depth ) {
            SCENELOG_FATAL( log, "IMAGE_3D & slice >= depth" );
            return NULL;
        }
        break;
    case IMAGE_CUBE:
        if( slice >= 6 ) {
            SCENELOG_FATAL( log, "IMAGE_CUBE & slice >= 6." );
            return NULL;
        }
        break;
    case IMAGE_N:
        SCENELOG_FATAL( log, "Uninitialized image." );
        return NULL;
    }

    size_t slice_mem_size = m_width*m_height*m_channels*m_element_size;

    return &m_data[ slice*slice_mem_size ];
}

bool
Image::set( size_t mip_level, size_t slice, const void* data )
{
    Logger log = getLogger( "Scene.Image.set" );

    if( mip_level != 0 ) {
        SCENELOG_FATAL( log, "Getting of non-zero mip-levels not yet supported." );
        return false;
    }

    size_t slice_mem_size = m_width*m_height*m_channels*m_element_size;

    // Sanity checks
    switch( m_type ) {
    case IMAGE_2D:
        if( slice != 0 ) {
            SCENELOG_FATAL( log, "IMAGE_2D & slice != 0" );
            return false;
        }
        if( m_data.empty() ) {
            m_data.resize( slice_mem_size );
        }
        break;
    case IMAGE_3D:
        if( slice >= m_depth ) {
            SCENELOG_FATAL( log, "IMAGE_3D & slice >= depth" );
            return false;
        }
        if( m_data.empty() ) {
            m_data.resize( m_depth*slice_mem_size );
        }
        break;
    case IMAGE_CUBE:
        if( slice >= 6 ) {
            SCENELOG_FATAL( log, "IMAGE_CUBE & slice >= 6." );
            return false;
        }
        if( m_data.empty() ) {
            m_data.resize( 6*slice_mem_size );
        }
        break;
    case IMAGE_N:
        SCENELOG_FATAL( log, "Uninitialized image." );
        return false;
    }

    memcpy( &m_data[ slice*slice_mem_size ],
            data,
            slice_mem_size );
    return true;
}



}
