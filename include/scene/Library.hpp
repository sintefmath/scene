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
#include <unordered_map>
#include "scene/Scene.hpp"
#include "scene/Asset.hpp"
#include "scene/Geometry.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {

/** A generalized library of a particular type.
  *
  * COLLADA organizes assets into libraries, e.g., library_geometries,
  * library_effects, and so on. Scene uses the same organization. The only
  * difference between these libraries is the type of objects they hold. Thus,
  * this functionality is organized into a generalized library.
  *
  * A library is the owner of the objects.
  */

template<class T>
class Library : public StructureValueSequences
{
    friend class DataBase;
public:
    ~Library();

    T*
    add( const std::string& id );

    size_t
    size() const;

    T*
    get( size_t index );

    const T*
    get( size_t index ) const;

    T*
    get( const std::string& id, bool clone_from_fallback = false );

    const T*
    get( const std::string& id , bool search_fallback = true ) const;

    const Asset&
    asset() const;

    Asset&
    asset() { return m_asset; }

    void
    setAsset( const Asset& asset );

    void
    remove( T* pointer );

    void
    clear();

    DataBase*
    dataBase();

    const DataBase*
    dataBase() const;

    /** Generate an unique id. */
    const std::string
    generateId() const;

protected:
    DataBase*                                m_database;
    Asset                                    m_asset;
    std::vector<T*>                          m_objects;
    std::unordered_map<std::string,index_t>  m_map;

    Library();

    void
    setDatabase( DataBase* database );

    static const std::string                 m_autoid_prefix;
    static const std::string                 m_instance_name;



};





} // of namespace Scene
