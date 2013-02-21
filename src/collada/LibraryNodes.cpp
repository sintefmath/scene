#include "scene/DataBase.hpp"
#include "scene/Node.hpp"
#include "scene/VisualScene.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;
        using std::unordered_map;

const static string ipackage = "Scene.XML.Importer";
//const static string epackage = "Scene.XML.Exporter";

bool
Importer::parseLibraryNodes( Context        context,
                             const Asset&   asset_parent,
                             xmlNodePtr     lib_nodes_node )
{
    Logger log = getLogger( ipackage + ".parseLibraryNodes" );
    if(!assertNode( lib_nodes_node, "library_nodes" ) ) {
        return false;
    }

    xmlNodePtr n = lib_nodes_node->children;

    Asset asset = asset_parent;
    if( checkNode( n, "asset" ) ) {
        if( !parseAsset( context, asset, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <asset>" );
            return false;
        }
        n = n->next;
    }
    m_database.library<Node>().setAsset( asset );

    while( checkNode( n, "node" ) ) {
        if(!parseNode( NULL,
                       m_database.library<Node>().asset(),
                       n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <node>" );
        }
        n = n->next;
    }
    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );

    return true;
}

xmlNodePtr
Exporter::createLibraryNodes( Context& context ) const
{
    const Library<Node>& lib = m_database.library<Node>();

    xmlNodePtr ln_node = newNode( NULL, "library_nodes" );
    for( size_t i=0; i<lib.size(); i++ ) {
        const Node* node = lib.get( i );
        if( node->parent() == NULL ) {
            // don't write node hierarchies that are part of visual scenes
            for( size_t j=0; j<m_database.library<VisualScene>().size(); j++ ) {
                const VisualScene* vs = m_database.library<VisualScene>().get( j );
                if( vs->nodesId() == node->id() ) {
                    node = NULL;
                    break;
                }
            }
            if( node != NULL ) {
                xmlAddChild( ln_node, createNode( context, node ) );
            }
        }
    }

    return ln_node;
}


    } // of namespace Scene
} // of namespace Scene

