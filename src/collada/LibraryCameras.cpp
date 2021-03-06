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
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {


bool
Importer::parseLibraryCameras( const Asset& asset_parent,
                               xmlNodePtr lib_cameras_node )
{
    Logger log = getLogger( "Scene.XML.parseLibraryCameras" );
    if( !assertNode( lib_cameras_node, "library_cameras" ) ) {
        return false;
    }
    xmlNodePtr n = lib_cameras_node->children;
    if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "asset" ) ) {
        Asset asset;
        if( parseAsset( asset, n ) ) {
            m_database.library<Camera>().setAsset( asset );
        }
        else {
            SCENELOG_ERROR( log, "Failed to parse <asset>" );
            return false;
        }
        n = n->next;
    }
    else {
        m_database.library<Camera>().setAsset( asset_parent );
    }

    while( n!= NULL && xmlStrEqual( n->name, BAD_CAST "camera" ) ) {
        if( !parseCamera( m_database.library<Camera>().asset(), n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <camera>" );
            return false;
        }
        n = n->next;
    }
    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );
    return true;

}

xmlNodePtr
Exporter::createLibraryCameras( Context& context ) const
{
    const Library<Camera>& lib = m_database.library<Camera>();
    if( lib.size() == 0 ) {
        return NULL;
    }

    xmlNodePtr lc_node = newNode( NULL, "library_cameras" );
    xmlAddChild( lc_node, createAsset( context, m_database.asset(), lib.asset() ) );
    for( size_t i=0; i<lib.size(); i++ ) {
        xmlAddChild( lc_node, createCamera( context, lib.get(i) ) );
    }
    return lc_node;
}


    } // of namespace XML
} // of namespace Scene
