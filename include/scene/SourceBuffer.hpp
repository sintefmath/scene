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

#pragma once

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
