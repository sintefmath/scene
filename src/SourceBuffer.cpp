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

#include <cstring>
#include <stdexcept>

#include "scene/SourceBuffer.hpp"
#include "scene/DataBase.hpp"
#include "scene/Utils.hpp"
#include "scene/Log.hpp"


namespace Scene {
    using std::string;
    using std::runtime_error;


SourceBuffer::SourceBuffer( DataBase& db, const std::string& id )
: m_db( db ),
  m_id( id ),
  m_element_size( 0u ),
  m_element_count( 0u )
{

}

SourceBuffer::SourceBuffer( Library<SourceBuffer>* library_source_buffers, const std::string& id )
    : m_db( *library_source_buffers->dataBase() ),
      m_id( id ),
      m_element_size( 0u ),
      m_element_count( 0u )
{
}

const int*
SourceBuffer::intData() const
{
    if( m_element_type != ELEMENT_INT ) {
        Logger log = getLogger( "Scene.SourceBuffer.intData" );
        SCENELOG_FATAL( log, "Wrong element type." );
        return NULL;
    }
    return reinterpret_cast<const int*>( &m_host_data[0] );
}

const float*
SourceBuffer::floatData() const
{
    if( m_element_type != ELEMENT_FLOAT ) {
        Logger log = getLogger( "Scene.SourceBuffer.floatData" );
        SCENELOG_FATAL( log, "Wrong element type." );
        return NULL;
    }
    return reinterpret_cast<const float*>( &m_host_data[0] );
}



void
SourceBuffer::contents( const std::vector<int>& data )
{

    m_element_type = ELEMENT_INT;
    m_element_size = sizeof(int);
    m_element_count = data.size();
    m_host_data.resize( m_element_size*m_element_count );
    memcpy( m_host_data.data(), data.data(), m_host_data.size() );

    structureChanged();
    m_db.library<SourceBuffer>().moveForward( *this );
    m_db.moveForward( *this );


    Logger log = getLogger( "Scene.SourceBuffer.contents" );
    SCENELOG_TRACE( log,
                    "id=" << m_id <<
                    ", etyp=" << m_element_type <<
                    ", esiz=" << m_element_size <<
                    ", ecnt=" << m_element_count <<
                    ", bsiz=" << (m_host_data.size()) );

}

void
SourceBuffer::contents( const std::vector<float>& data )
{

    m_element_type = ELEMENT_FLOAT;
    m_element_size = sizeof(float);
    m_element_count = data.size();
    m_host_data.resize( m_element_size*m_element_count );
    memcpy( &m_host_data[0], &data[0], m_host_data.size() );

    structureChanged();
    m_db.library<SourceBuffer>().moveForward( *this );
    m_db.moveForward( *this );

    Logger log = getLogger( "Scene.SourceBuffer.contents" );
    SCENELOG_TRACE( log,
                    "id=" << m_id <<
                    ", etyp=" << m_element_type <<
                    ", esiz=" << m_element_size <<
                    ", ecnt=" << m_element_count <<
                    ", bsiz=" << (m_host_data.size()) );

}


} // of namespace Scene

