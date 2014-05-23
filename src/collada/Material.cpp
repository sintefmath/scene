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

#include "scene/Material.hpp"
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;

bool
Importer::parseMaterial( const Asset&  asset_parent,
                         xmlNodePtr    material_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseMaterial" );
    if( !assertNode( material_node, "material" ) ) {
        return false;
    }

    const string id = attribute( material_node, "id" );
    if( id.empty() ) {
        SCENELOG_ERROR( log, "Required attribute 'id' empty." );
        return false;
    }

    Material* m = m_database.library<Material>().add( id );
    if( m == NULL ) {
        SCENELOG_ERROR( log, "Failed to create material '" << id << "'." );
        return false;
    }

    // An optional <asset>
    xmlNodePtr n = material_node->children;
    if( n != NULL && xmlStrEqual( n->name, BAD_CAST "asset" ) ) {
        Asset asset;
        if( parseAsset( asset, n ) ) {
            m->setAsset( asset );
        }
        else {
            SCENELOG_ERROR( log, "Failed to parse <asset>" );
            return false;
        }
        n=n->next;
    }
    else {
        m->setAsset( asset_parent );
    }

    // Exactly one <instance_effect>
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "instance_effect" ) ) {
        if( !parseInstanceEffect( m, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <instance_effect>." );
            return false;
        }
        n=n->next;
    }
    else {
        SCENELOG_ERROR( log, "Couldn't find required <instance_effect> child." );
        return false;
    }

    // Nought to many <extra>'s
    while( n != NULL && xmlStrEqual(n->name, BAD_CAST "extra" ) ) {
        n = n->next;
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
Exporter::createMaterial( Exporter::Context&      context,
                          const Scene::Material*  material ) const
{

    xmlNodePtr material_node = newNode( NULL, "material" );
    addProperty( material_node, "id", material->id() );
    if( !m_lean_export && !(material->asset() == m_database.library<Material>().asset()) ) {
        xmlAddChild( material_node, createAsset( material->asset() ) );
    }
    xmlAddChild( material_node, createInstanceEffect( context, material ) );
    return material_node;
}


    } // of namespace XML
} // of namespace Scene
