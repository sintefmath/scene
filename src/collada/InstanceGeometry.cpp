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
