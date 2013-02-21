
#include <algorithm>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Material.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::sort;
        using std::string;
        using std::vector;
        using std::unordered_map;

bool
Importer::parseLibraryMaterials( xmlNodePtr library_materials_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parseLibraryMaterials" );
#ifdef DEBUG
    if( !xmlStrEqual( library_materials_node->name, BAD_CAST "library_materials" ) )  {
        SCENELOG_FATAL( log, "Node is not <library_materials>" );
        return false;
    }
#endif

    xmlNodePtr n = library_materials_node->children;
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "asset" ) ) {
        Asset asset;
        if( parseAsset( asset, n ) ) {
            m_database.library<Material>().setAsset( asset );
        }
        else {
            SCENELOG_ERROR( log, "Failed to parse <asset>." );
            return false;
        }
        n = n->next;
    }
    else {
        m_database.library<Material>().setAsset( m_database.asset() );
    }

    while( n!= NULL && xmlStrEqual( n->name, BAD_CAST "material" ) ) {
        if( !parseMaterial( m_database.library<Material>().asset(), n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <material>" );
            return false;
        }
        n = n->next;
    }
    while( n!=NULL && xmlStrEqual( n->name, BAD_CAST "extra" ) ) {
        n=n->next;
    }
    // Nag if there is something left
    while( n != NULL ) {
        SCENELOG_WARN( log, "Unexpected node '"<<
                       reinterpret_cast<const char*>( n->name) <<
                       "'." );
        n = n->next;
    }
    return true;
}



xmlNodePtr
Exporter::createLibraryMaterials( Context& context ) const
{
    //Logger log = getLogger( "Scene.XML.Builder.createLibraryMaterials" );
    const Library<Material>& lib = m_database.library<Material>();

    xmlNodePtr library_materials_node = newNode( NULL, "library_materials" );
    if( !(lib.asset() == m_database.asset()) ) {
        xmlAddChild( library_materials_node, createAsset( lib.asset() ) );
    }
    for( size_t i=0; i<lib.size(); i++ ) {
        xmlAddChild( library_materials_node, createMaterial( context, lib.get(i) ) );
    }
    return library_materials_node;
}




    } // of namespace Scene
} // of namespace Scene


