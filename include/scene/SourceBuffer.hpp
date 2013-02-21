#ifndef SOURCEBUFFER_HPP
#define SOURCEBUFFER_HPP

#include <string>
#include <vector>
#include "scene/Scene.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {

class SourceBuffer : public StructureValueSequences
{
    friend class DataBase;
    friend class LibrarySourceBuffers;

    friend class Library<SourceBuffer>;

public:

    void
    contents( const std::vector<float>& data );

    void
    contents( const std::vector<int>& data );


    const std::string&
    id() const { return m_id; }

    const ElementType
    elementType() const { return m_element_type; }

    const size_t
    elementCount() const { return m_element_count; }

    const void*
    voidData() const { return m_host_data.data(); }

    const int*
    intData() const;

    const float*
    floatData() const;

protected:

    DataBase&                   m_db;
    std::string                 m_id;
    ElementType                 m_element_type;
    size_t                      m_element_size;
    size_t                      m_element_count;
    std::vector<unsigned char>  m_host_data;

    SourceBuffer( DataBase& db, const std::string& id );

    SourceBuffer( Library<SourceBuffer>* library_source_buffers, const std::string& id );

};

} // of namespace Scene
#endif // SOURCEBUFFER_HPP
