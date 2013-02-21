#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {


bool
Importer::parseLibraryImages( const Asset& asset_parent,
                              xmlNodePtr lib_images_node )
{
    Logger log = getLogger( "Scene.XML.parseLibraryImages" );
    if( !assertNode( lib_images_node, "library_images" ) ) {
        return false;
    }

    xmlNodePtr n = lib_images_node->children;

    if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "asset" ) ) {
        Asset asset;
        if( parseAsset( asset, n ) ) {
            m_database.library<Image>().setAsset( asset );
        }
        else {
            SCENELOG_ERROR( log, "Failed to parse <asset>" );
           return false;
        }

        n=n->next;
    }
    else {
        m_database.library<Image>().setAsset( asset_parent );
    }

    while( n!=NULL && xmlStrEqual( n->name, BAD_CAST "image" ) ) {
        if(!parseImage( m_database.library<Image>().asset(), n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <image>" );
            return false;
        }
        n=n->next;
    }

    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );
    return true;
}


    } // of namespace XML
} // of namespace Scene
