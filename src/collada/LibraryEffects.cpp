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

#include <algorithm>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Effect.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::sort;
        using std::string;
        using std::vector;
        using std::unordered_map;

bool
Importer::parseLibraryEffects( xmlNodePtr library_effects_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parseLibraryEffects" );

#ifdef DEBUG
    if( !xmlStrEqual( library_effects_node->name, BAD_CAST "library_effects" ) ) {
        SCENELOG_FATAL( log, "Node is not <library_effects>" );
        return false;
    }
#endif

    bool success = true;
    for( xmlNodePtr m=library_effects_node->children; m!=NULL; m=m->next ) {

        if( xmlStrEqual( m->name, BAD_CAST "asset" ) ) {
        }
        else if( xmlStrEqual( m->name, BAD_CAST "effect" ) ) {
            if( !parseEffect( m_database.library<Effect>().asset(), m ) ) {
                SCENELOG_ERROR( log, "Failed to parse <effect>" );
                return false;
            }
        }
        else if( xmlStrEqual( m->name, BAD_CAST "extra" ) ) {
        }
        else if( xmlStrEqual( m->name, BAD_CAST "comment" ) ) {
        }
        else {
            SCENELOG_WARN( log, "Unexpected node " <<
                           reinterpret_cast<const char*>(m->name) );
        }
    }

    return success;
}

xmlNodePtr
Exporter::createLibraryEffects( Context&  context ) const
{
    const Library<Effect>& lib = m_database.library<Effect>();
    if( lib.size() == 0 ) {
        return NULL;
    }

    xmlNodePtr le_node = newNode( NULL, "library_effects" );
    xmlAddChild( le_node, createAsset( context, m_database.asset(), lib.asset() ) );
    for(size_t i=0; i<lib.size(); i++ ) {
        xmlAddChild( le_node, createEffect( context, lib.get( i ) ) );
    }
    return le_node;
}


    } // of namespace Scene
} // of namespace Scene
