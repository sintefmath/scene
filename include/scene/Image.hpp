#pragma once

#include <vector>
#include "scene/Scene.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {

class Image : public StructureValueSequences
{
    friend class Library<Image>;
    friend class LibraryImages;
public:

    const std::string&
    id() const { return m_id; }

    const std::string&
    key() const { return m_id; }

    /** Create a 2D image with exact size. */
    bool
    init2D( GLenum iformat,
            GLenum format,
            GLenum type,
            size_t width,
            size_t height,
            size_t array_length,
            size_t mips,
            bool   auto_generate );

    /** Create a 2D image with size relative to viewport. */
    bool
    init2D( GLenum iformat,
            GLenum format,
            GLenum type,
            float  width,
            float  height,
            size_t array_length,
            size_t mips,
            bool   auto_generate );

    /** Create a 2D image with exact size. */
    bool
    initCube( GLenum iformat,
              GLenum format,
              GLenum type,
              size_t width,
              size_t array_length,
              size_t mips,
              bool   auto_generate );

    bool
    init3D( GLenum iformat,
            GLenum format,
            GLenum type,
            size_t width,
            size_t height,
            size_t depth,
            size_t array_length,
            size_t mips,
            bool   auto_generate );

    size_t
    texelsInSlice();


    bool
    set( size_t mip_level, size_t slice, const void* data );

    const void*
    get( size_t mip_level, size_t slice ) const;

    ImageType
    type() const { return m_type; }


    GLenum
    suggestedInternalFormat() const { return m_iformat; }

    GLenum
    format() const { return m_format; }

    GLenum
    elementType() const { return m_element_type; }

    size_t
    width() const { return m_width; }

    size_t
    height() const { return m_height; }

    size_t
    depth() const { return m_depth; }

    bool
    buildMipMap() const { return true; }

protected:
    DataBase&          m_database;
    const std::string  m_id;

    ImageType          m_type;
    GLenum             m_iformat;
    GLenum             m_format;
    GLenum             m_element_type;
    size_t             m_width;
    size_t             m_height;
    size_t             m_depth;
//    size_t             m_mip_levels;
    size_t             m_channels;
    size_t             m_element_size;


    Image( DataBase& database, const std::string& id );

    Image( Library<Image>* library_images, const std::string& id );

    bool
    setFormat( GLenum iformat, GLenum format, GLenum type );


    std::vector<unsigned char> m_data;

};





} // of namespace Scene
