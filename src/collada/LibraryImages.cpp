#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {


bool
Importer::parseLibraryImages( Context      context,
                              const Asset& asset_parent,
                              xmlNodePtr lib_images_node )
{
    Logger log = getLogger( "Scene.XML.parseLibraryImages" );
    if( !assertNode( lib_images_node, "library_images" ) ) {
        return false;
    }

    xmlNodePtr n = lib_images_node->children;

    Asset asset = asset_parent;
    if( checkNode( n, "asset" ) ) {
        if( !parseAsset( context, asset, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <asset>" );
        }
        n = n->next;
    }
    m_database.library<Image>().setAsset( asset );

    while( checkNode( n, "image" ) ) {
        if(!parseImage( context, m_database.library<Image>().asset(), n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <image>" );
        }
        n = n->next;
    }

    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );
    return true;
}


    } // of namespace XML
} // of namespace Scene
