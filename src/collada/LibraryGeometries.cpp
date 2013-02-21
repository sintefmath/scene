#include <algorithm>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Geometry.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::sort;
        using std::string;
        using std::vector;
        using std::unordered_map;

bool
Importer::parseLibraryGeometries( xmlNodePtr library_geometries_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parseLibraryGeometries" );
    if( !assertNode( library_geometries_node, "library_geometries" ) ) {
        return false;
    }

    for( xmlNodePtr n = library_geometries_node->children; n!=NULL; n=n->next ) {

        if( xmlStrEqual( n->name, BAD_CAST "asset" ) ) {
            // Silently ignore <asset>
        }
        else if( xmlStrEqual( n->name, BAD_CAST "geometry" ) ) {
            const string id = attribute( n, "id" );

            if( id.empty() ) {
                SCENELOG_ERROR( log, "Missing required id attribute." );
                return false;
            }
            else {
                Geometry* geometry = m_database.library<Geometry>().add( id );

                if( geometry == NULL ) {
                    SCENELOG_ERROR( log, "Failed to create geometry '" << id << "'." );
                    return false;
                }
                else if(!parseGeometry( geometry, n ) ) {
                    return false;
                }
                SCENELOG_TRACE( log, "Read geometry '" << id << "'." );
            }

        }
        else if( xmlStrEqual( n->name, BAD_CAST "extra" ) ) {
            // Silently ignoring <extra>
        }
        else {
            SCENELOG_WARN( log, "Unexpected node " << reinterpret_cast<const char*>(n->name) );
        }
    }
    return true;
}


xmlNodePtr
Exporter::createLibraryGeometries( Context& context ) const
{
    const Library<Geometry>& lib = m_database.library<Geometry>();
    if( lib.size() == 0 ) {
        return NULL;
    }
    xmlNodePtr lib_node = newNode( NULL, "library_geometries" );
    xmlAddChild( lib_node, createAsset( context, m_database.asset(), lib.asset() ) );
    for( size_t i=0; i<lib.size(); i++ ) {
        xmlAddChild( lib_node, createGeometry( context, lib.get(i) ) );
    }
    return lib_node;
}




    } // of namespace Scene
} // of namespace Scene


