#include <string>
#include <vector>
#include <stdexcept>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

#ifdef USE_POSIX
#include <cstring>
#include <clocale>
#endif

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;
        using std::runtime_error;


bool
Importer::parseCollada( xmlNodePtr collada_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parseCollada" );

    Context context;
    context.m_up_axis = Context::Y_UP;
    context.m_unit = 1.f;

    if( !assertNode( collada_node, "COLLADA" ) ) {
        return false;
    }
#ifdef USE_POSIX
    char* loc_ctype = strdup( setlocale( LC_CTYPE, NULL ) );
    setlocale( LC_CTYPE, "C" );
    char* loc_numeric = strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );
    char* loc_time = strdup( setlocale( LC_TIME, NULL ) );
    setlocale( LC_TIME, "C" );
#endif

    bool success = true;

    const std::string version_str = attribute( collada_node, "version" );
    if( version_str == "1.4.0" ) {
        context.m_version = Context::VERSION_1_4_X;
    }
    else if( version_str == "1.4.1" ) {
        context.m_version = Context::VERSION_1_4_X;
    }
    else if( version_str == "1.5.0" ) {
        context.m_version = Context::VERSION_1_5_0;
    }
    else {
        SCENELOG_WARN( log, "Unrecognized version='" << version_str << "', assuming 1.4.1." );
        context.m_version = Context::VERSION_1_4_X;
    }

    xmlNodePtr n = collada_node->children;


    Asset collada_asset = m_database.asset();
    if( checkNode( n, "asset" ) ) {
        Asset asset;
        if( parseAsset( context, asset, n ) ) {
            collada_asset = asset;
        }
        else {
            SCENELOG_ERROR( log, "failed to parse <asset>" );
            success = false;
        }
        n = n->next;
    }
    else {
        SCENELOG_WARN( log, "expected required <asset> element" );
    }

    while( n != NULL ) {
        if( checkNode( n, "library_animation_clips" ) ) {
            SCENELOG_WARN( log, "In <COLLADA>, ignoring unsupported <" << n->name << ">" );
        }
        else if( checkNode( n, "library_animations" ) ) {
            SCENELOG_WARN( log, "In <COLLADA>, ignoring unsupported <" << n->name << ">" );
        }
        else if( checkNode( n, "library_articulated_systems" ) ) {
            SCENELOG_WARN( log, "In <COLLADA>, ignoring unsupported <" << n->name << ">" );
        }
        else if( checkNode( n, "library_cameras" ) ) {
            if(!parseLibraryCameras( collada_asset, n ) ) {
                nagAboutParseError( log, n );
                success = false;
            }
        }
        else if( checkNode( n, "library_controllers" ) ) {
            SCENELOG_WARN( log, "In <COLLADA>, ignoring unsupported <" << n->name << ">" );
        }
        else if( checkNode( n, "library_effects" ) ) {
            if(!parseLibraryEffects( n) ) {
                SCENELOG_ERROR( log, "In <COLLADA>, failed to parse <library_effects>" );
                success = false;
            }
        }
        else if( checkNode( n, "library_force_fields" ) ) {
            SCENELOG_WARN( log, "In <COLLADA>, ignoring unsupported <" << n->name << ">" );
        }
        else if( checkNode( n, "library_formulas" ) ) {
            SCENELOG_WARN( log, "In <COLLADA>, ignoring unsupported <" << n->name << ">" );
        }
        else if( checkNode( n, "library_geometries" ) ) {
            if(!parseLibraryGeometries( n ) ) {
                nagAboutParseError( log, n );
                success = false;
            }
        }
        else if( checkNode( n, "library_images" ) ) {
            if(!parseLibraryImages( context, collada_asset, n ) ) {
                nagAboutParseError( log, n );
                success = false;
            }
        }
        else if( checkNode( n, "library_joints" ) ) {
            SCENELOG_WARN( log, "In <COLLADA>, ignoring unsupported <" << n->name << ">" );
        }
        else if( checkNode( n, "library_kinematic_models" ) ) {
            SCENELOG_WARN( log, "In <COLLADA>, ignoring unsupported <" << n->name << ">" );
        }
        else if( checkNode( n, "library_kinematic_scenes" ) ) {
            SCENELOG_WARN( log, "In <COLLADA>, ignoring unsupported <" << n->name << ">" );
        }
        else if( checkNode( n, "library_lights" ) ) {
            if(!parseLibraryLights( collada_asset, n ) ) {
                nagAboutParseError( log, n );
                success = false;
            }
        }
        else if( checkNode( n, "library_materials" ) ) {
            if(!parseLibraryMaterials( n ) ) {
                nagAboutParseError( log, n );
                success = false;
            }
        }
        else if( checkNode( n, "library_nodes" ) ) {
            if(!parseLibraryNodes( context, collada_asset, n) ) {
                nagAboutParseError( log, n );
                success = false;
            }
        }
        else if( checkNode( n, "library_physics_materials" ) ) {
            SCENELOG_WARN( log, "In <COLLADA>, ignoring unsupported <" << n->name << ">" );
        }
        else if( checkNode( n, "library_physics_models" ) ) {
            SCENELOG_WARN( log, "In <COLLADA>, ignoring unsupported <" << n->name << ">" );
        }
        else if( checkNode( n, "library_physics_scenes" ) ) {
            SCENELOG_WARN( log, "In <COLLADA>, ignoring unsupported <" << n->name << ">" );
        }
        else if( checkNode( n, "library_visual_scenes" ) ) {
            if(!parseLibraryVisualScenes( context, collada_asset, n) ) {
                nagAboutParseError( log, n );
                success = false;
            }
        }
        else {
            break;
        }
        n = n->next;
    }


    if( checkNode(n, "scene" ) ) {
        // ignore for now
        n = n->next;
    }

    while( checkNode( n, "extra" ) ) {
        xmlNodePtr m = n->children;
        if( checkNode( m, "asset" ) ) {
            // ignore
            m = m->next;
        }
        for( ; checkNode( m, "technique" ); m = m->next ) {
            const string profile = attribute( m, "profile" );
            if( profile == "Scene" || profile == "scene" ) {
                xmlNodePtr o = m->children;
                for( ; o != NULL; o = o->next ) {
                    if( checkNode( o, "include" ) ) {
                        Importer importer( m_database );
                        success = success && importer.parse( resolvePath( attribute( o, "file" ) ) );
                    }
                }
            }
        }
        n = n->next;
    }

    while( n != NULL ) {
        SCENELOG_WARN( log, "In <COLLADA>, unexpected node <" << reinterpret_cast<const char*>( n->name ) << ">." );
        n = n->next;
    }

    m_database.setAsset( collada_asset );

#ifdef USE_POSIX
    setlocale( LC_CTYPE, loc_ctype );
    free( loc_ctype );
    setlocale( LC_NUMERIC, loc_numeric );
    free( loc_numeric );
    setlocale( LC_TIME, loc_time );
    free( loc_time );
#endif
    return success;
}

xmlNodePtr
Exporter::createCollada( Context& context ) const
{
//    Logger log = getLogger( "Scene.XML.Builder.createCollada" );

    xmlNodePtr collada_node = newNode( NULL, "COLLADA" );
    xmlAddChild( collada_node, createAsset( m_database.asset() ) );

    if( context.m_lib_geometry ) {
        xmlAddChild( collada_node, createLibraryGeometries( context ) );
    }
    if( context.m_lib_image ) {
        xmlAddChild( collada_node, createLibraryCameras( context ) );
    }
    if( context.m_lib_light ) {
        xmlAddChild( collada_node, createLibraryLights( context ) );
    }
    if( context.m_lib_effect ) {
        xmlAddChild( collada_node, createLibraryEffects( context ) );
    }
    if( context.m_lib_material ) {
        xmlAddChild( collada_node, createLibraryMaterials( context ) );
    }
    if( context.m_lib_nodes ) {
        xmlAddChild( collada_node, createLibraryNodes( context ) );
    }
    if( context.m_lib_visual_scene ) {
        xmlAddChild( collada_node, createLibraryVisualScenes( context ) );
    }

    return collada_node;
}




    } // of namespace Scene
} // of namespace Scene
