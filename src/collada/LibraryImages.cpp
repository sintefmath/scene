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
