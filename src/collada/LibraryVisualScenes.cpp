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

#include <unordered_map>
#include "scene/Log.hpp"
#include "scene/Asset.hpp"
#include "scene/DataBase.hpp"
#include "scene/Geometry.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;
        using std::unordered_map;

bool
Importer::parseLibraryVisualScenes( Context         context,
                                    const Asset&    asset_parent,
                                    xmlNodePtr      lib_vis_scene_node )
{
    Logger log = getLogger( "Scene.XML.parseLibraryVisualScenes" );

    if(!assertNode( lib_vis_scene_node, "library_visual_scenes" ) ) {
        return false;
    }


    bool success = true;

    xmlNodePtr n = lib_vis_scene_node->children;

    Asset lib_asset = asset_parent;
    if( checkNode( n, "asset" ) ) {
        Asset asset;
        if( parseAsset( context, asset, n ) ) {
            lib_asset = asset;
        }
        else {
            SCENELOG_ERROR( log, "In <library_visual_scenes>, failed to parse <asset>." );
            success = false;
        }
        n = n->next;
    }


    while( checkNode( n, "visual_scene" ) ) {
        if( !parseVisualScene( context, lib_asset, n ) ) {
            SCENELOG_ERROR( log, "In <library_visual_scenes>, failed to parse <visual_scene>" );
            success = false;
        }
        n = n->next;
    }
    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );

    m_database.library<VisualScene>().setAsset( lib_asset );
    return success;
}

xmlNodePtr
Exporter::createLibraryVisualScenes( Exporter::Context& context ) const
{
    const Library<VisualScene>& lib = m_database.library<VisualScene>();
    if( lib.size() == 0 ) {
        return NULL;
    }

    xmlNodePtr lvs_node = newNode( NULL, "library_visual_scenes" );
    xmlAddChild( lvs_node, createAsset( context, m_database.asset(), lib.asset() ) );
    for( size_t i=0; i<lib.size(); i++ ) {
        xmlAddChild( lvs_node, createVisualScene( context, lib.get( i ) ) );
    }
    return lvs_node;
}




    } // of namespace Scene
} // of namespace Scene

