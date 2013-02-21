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
