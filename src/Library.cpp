#include "scene/Log.hpp"
#include "scene/Camera.hpp"
#include "scene/Material.hpp"
#include "scene/Effect.hpp"
#include "scene/VisualScene.hpp"
#include "scene/SourceBuffer.hpp"
#include "scene/Geometry.hpp"
#include "scene/Light.hpp"
#include "scene/Image.hpp"
#include "scene/DataBase.hpp"
#include "scene/Library.hpp"


namespace Scene {


template<class T>
Library<T>::Library()
    : m_database( NULL )
{
}

template<class T>
void
Library<T>::setDatabase( DataBase* database )
{
    m_database = database;
}


template<class T>
Library<T>::~Library()
{
    clear();
}

template<class T>
void
Library<T>::clear()
{
    Logger log = getLogger( m_instance_name + ".clear" );
    SCENELOG_DEBUG( log, "Deleting all contents." );
    for( auto it=m_objects.begin(); it!=m_objects.end(); ++it ) {
        delete *it;
    }
    m_objects.clear();
    m_map.clear();

    touchStructureChanged();
    m_database->moveForward( *this );
}

template<class T>
const T*
Library<T>::get( const std::string& id , bool search_fallback ) const
{
    auto it = m_map.find( id );
    if( it != m_map.end() ) {
        return m_objects[ it->second ];
    }
    else if( search_fallback ) {
        if( m_database != NULL ) {
            const DataBase* fallback = m_database->fallback();
            if( fallback != NULL ) {
                return fallback->library<T>().get( id, true );
            }
        }
    }
    return NULL;
}


template<class T>
T*
Library<T>::get( const std::string& id, bool clone_from_fallback )
{
    Logger log = getLogger( m_instance_name + ".get" );
    auto it = m_map.find( id );
    if( it != m_map.end() ) {
        return m_objects[ it->second ];
    }
    else if( clone_from_fallback ) {
        if( m_database == NULL ) {
            SCENELOG_FATAL( log, "m_database == NULL" );
        }
        else {
            const DataBase* fallback = m_database->fallback();
            if( fallback != NULL ) {
                const T* fallback_obj = fallback->library<T>().get( id, true );
                if( fallback_obj != NULL ) {
                    SCENELOG_FATAL( log, "Deep copy not implemented." );
                }
            }
        }
    }
    return NULL;
}

template<class T>
T*
Library<T>::get( size_t index )
{
    return m_objects[ index ];
}

template<class T>
const T*
Library<T>::get( size_t index ) const
{
    return m_objects[ index ];
}


template<class T>
size_t
Library<T>::size() const
{
    return m_objects.size();
}

template<class T>
T*
Library<T>::add( const std::string& id )
{
    auto it = m_map.find( id );
    if( it != m_map.end() ) {
        Logger log = getLogger( m_instance_name + ".add" );
        SCENELOG_ERROR( log, "id '" << id << "' already exists." );
        return NULL;
    }
    else {
        if( !id.empty() ) {
            // TODO: Add flag to check if id is required
            m_map[ id ] = m_objects.size();
        }
        m_objects.push_back( new T( this, id ) );
        touchStructureChanged();
        m_database->moveForward( *this );
        return m_objects.back();
    }
}

template<class T>
const Asset&
Library<T>::asset() const
{
    return m_asset;
}

template<class T>
void
Library<T>::setAsset( const Asset& asset )
{
    m_asset = asset;
}

template<class T>
DataBase*
Library<T>::dataBase()
{
    return m_database;
}

template<class T>
const DataBase*
Library<T>::dataBase() const
{
    return m_database;
}

template<class T>
void
Library<T>::remove( T* pointer )
{

    bool found = false;
    size_t ix;

    // First, try searching by ID
    const std::string id = pointer->id();
    if( !id.empty() ) {
        // It has an ID, remove from map
        auto it = m_map.find(id );
        if( it != m_map.end() ) {
            found = true;
            ix = it->second;
            m_map.erase( it );
#ifdef DEBUG
            if( m_objects[ix] != pointer ) {
                Logger log = getLogger( m_instance_name + ".remove" );
                SCENELOG_ERROR( log, "Pointer and ID mismatch (id='" << id << "')." );
                found = false;
            }
#endif
        }
    }

    // Then, try searching by address
    if(!found) {
        for( size_t i=0; i<m_objects.size(); i++ ) {
            if( m_objects[i] == pointer ) {
                ix = i;
                found = true;
                break;
            }
        }
    }

    if( !found ) {
        Logger log = getLogger( m_instance_name + ".remove" );
        SCENELOG_WARN( log, "Pointer " << pointer << " not found." );
        return;
    }

    m_objects[ ix ] = m_objects.back();
    m_objects.resize( m_objects.size()-1 );

    // Update map
    for(auto it=m_map.begin(); it!=m_map.end(); ++it ) {
        if( it->second == m_objects.size() ) {
            it->second = ix;
        }
    }

    // Remove object
    delete pointer;

    touchStructureChanged();
    m_database->moveForward( *this );
}

template<class T>
const std::string
Library<T>::generateId() const
{
    std::stringstream o;
    for(size_t i=0; i<m_objects.size()+1; i++ ) {
        o.str( m_autoid_prefix );
        o << i;
        const std::string str = o.str();
        auto it = m_map.find( str );
        if( it == m_map.end() ) {
            return str;
        }
    }
    Logger log = getLogger( m_instance_name + ".remove" );
    SCENELOG_FATAL( log, "Failed to generate unique ID." );
    return "error";
}


template class Library<Geometry>;
template<> const std::string Library<Geometry>::m_instance_name = "Scene.Library<Geometry>";
template<> const std::string Library<Geometry>::m_autoid_prefix = "geometry";

template class Library<Image>;
template<> const std::string Library<Image>::m_instance_name = "Scene.Library<Image>";
template<> const std::string Library<Image>::m_autoid_prefix = "image";

template class Library<Effect>;
template<> const std::string Library<Effect>::m_instance_name = "Scene.Library<Effect>";
template<> const std::string Library<Effect>::m_autoid_prefix = "effect";

template class Library<Material>;
template<> const std::string Library<Material>::m_instance_name = "Scene.Library<Material>";
template<> const std::string Library<Material>::m_autoid_prefix = "material";

template class Library<Camera>;
template<> const std::string Library<Camera>::m_instance_name = "Scene.Library<Camera>";
template<> const std::string Library<Camera>::m_autoid_prefix = "camera";

template class Library<Light>;
template<> const std::string Library<Light>::m_instance_name = "Scene.Library<Light>";
template<> const std::string Library<Light>::m_autoid_prefix = "light";

template class Library<Node>;
template<> const std::string Library<Node>::m_instance_name = "Scene.Library<Node>";
template<> const std::string Library<Node>::m_autoid_prefix = "node";

template class Library<SourceBuffer>;
template<> const std::string Library<SourceBuffer>::m_instance_name = "Scene.Library<SourceBuffer>";
template<> const std::string Library<SourceBuffer>::m_autoid_prefix = "buffer";

template class Library<VisualScene>;
template<> const std::string Library<VisualScene>::m_instance_name = "Scene.Library<VisualScene>";
template<> const std::string Library<VisualScene>::m_autoid_prefix = "visual_scene";


} // of namespace Scene

