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

#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Geometry.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;

bool
Importer::parseGeometry( Scene::Geometry*  geometry,
                        xmlNodePtr        geometry_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parseGeometry" );
    if( !assertNode( geometry_node, "geometry" ) ) {
        return false;
    }

    // zero or one <asset> followed by exactly one primitives node.

    xmlNodePtr n = geometry_node->children;
    for( ; n!=NULL; n=n->next ) {

        if( xmlStrEqual( n->name, BAD_CAST "asset" ) ) {
           Asset asset;
           if( parseAsset( asset, n ) ) {
               geometry->setAsset( asset );
           }
        }
        else if( xmlStrEqual( n->name, BAD_CAST "convex_mesh" ) ) {
            SCENELOG_WARN( log, "<convex_mesh> is currently not handled, ignoring." );
            break;
        }
        else if( xmlStrEqual( n->name, reinterpret_cast<const xmlChar*>( "mesh" ) ) ) {
            if(!parseMesh( geometry, n ) ) {
                SCENELOG_ERROR( log, "Failed to parse <mesh>, ignoring." );
            }
            break;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "spline" ) ) {
            SCENELOG_WARN( log, "<spline> is currently not handled, ignoring." );
            break;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "brep" ) ) {
            SCENELOG_WARN( log, "<brep> is currently not handled, ignoring." );
            break;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "extra" ) ) {
            // silently ignore <extra>
        }
        else {
            SCENELOG_WARN( log, "Unexpected node " << reinterpret_cast<const char*>(n->name) );
        }
    }

    if( n == NULL ) {
        SCENELOG_ERROR( log, "No primitives defined." );
        return false;
    }
    else {
        // Nag about any extra nodes
        for( n=n->next; n!=NULL; n=n->next ) {
            SCENELOG_WARN( log, "Unexpected node " << reinterpret_cast<const char*>(n->name) );
        }
        return true;
    }
}

xmlNodePtr
Exporter::createGeometry( Context& context,
                         const Scene::Geometry* geometry ) const
{
    if( geometry == NULL ) {
        return NULL;
    }
    if( (geometry->id().length() >= 8) && (geometry->id().substr(0,8) == "builtin." ) ) {
        return NULL;
    }

    xmlNodePtr geo_node = newNode( NULL, "geometry" );
    if( !geometry->id().empty() ) {
        addProperty( geo_node, "id", geometry->id() );
    }
    xmlAddChild( geo_node,
                 createAsset( context,
                              m_database.library<Geometry>().asset(),
                              geometry->asset() ) );

    xmlAddChild( geo_node, createMesh( context, geometry ) );
    return geo_node;
}



    } // of namespace Scene
} // of namespace Scene
