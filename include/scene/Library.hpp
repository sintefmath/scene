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

protected:
    DataBase*                                m_database;
    Asset                                    m_asset;
    std::vector<T*>                          m_objects;
    std::unordered_map<std::string,index_t>  m_map;

    Library();

    void
    setDatabase( DataBase* database );

    static const std::string                 m_instance_name;



};





} // of namespace Scene
