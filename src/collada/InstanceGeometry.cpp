#include <string>
#include "scene/Log.hpp"
#include "scene/Bind.hpp"
#include "scene/Node.hpp"
#include "scene/InstanceGeometry.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"


namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;

bool
Importer::parseInstanceGeometry( Node* node, xmlNodePtr instance_geometry_node )
{

    Logger log = getLogger( "Scene.XML.Importer.parseInstanceGeometry");
    if( !assertNode( instance_geometry_node, "instance_geometry" ) ) {
        return false;
    }


    std::string geometry_url = attribute( instance_geometry_node, "url" );
    if( geometry_url.size() > 0 && geometry_url[0] == '#' ) {
        geometry_url = geometry_url.substr( 1 );
    }
    if( geometry_url.empty() ) {
        SCENELOG_ERROR( log, "Required attribute 'url' empty." );
        return false;
    }
    InstanceGeometry* instgeo = new InstanceGeometry( geometry_url );

    // Parse children

    xmlNodePtr n = instance_geometry_node->children;
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "bind_material" ) ) {
        if(!parseBindMaterial( instgeo, n ) ) {
            delete instgeo;
            return false;
        }
        n=n->next;
    }

    ignoreExtraNodes( log, n );

    nagAboutRemainingNodes( log, n );

    node->add( instgeo );
    return true;
}



xmlNodePtr
Exporter::createInstanceGeometry( Context& context, const InstanceGeometry* instance ) const
{
    if( instance == NULL ) {
        return NULL;
    }

    xmlNodePtr instance_node = newNode( NULL, "instance_geometry" );
    addProperty( instance_node, "url", "#" + instance->geometryId() );

    xmlAddChild( instance_node, createBindMaterial( context, instance ) );

    return instance_node;
}


    } // of namespace XML
} // of namespace Scene
