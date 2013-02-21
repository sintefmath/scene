#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Library.hpp"
#include "scene/Light.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"


namespace Scene {
    namespace Collada {
        using std::string;

bool
Importer::parseLibraryLights( const Asset& parent_asset, xmlNodePtr library_lights_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseLibraryLights" );
    if( !assertNode( library_lights_node, "library_lights" ) ) {
        return false;
    }

    bool success = true;
    xmlNodePtr n = library_lights_node->children;


    // <library_lights>/<asset>
    Asset library_asset = parent_asset;
    if( checkNode( n, "asset" ) ) {
        Asset asset;
        if( parseAsset( asset, n ) ) {
            library_asset = asset;
        }
        else {
            SCENELOG_WARN( log, "Failed to parse asset, ignoring." );
        }
        n = n->next;
    }

    // <library_lights>/<light>
    while( checkNode( n, "light" ) ) {
        std::string id = attribute( n, "id" );
        if( id.empty() ) {
            // COLLADA allows light without id, but I can't see the usefulness
            // for this as the id is required for referencing the light in any
            // way.
            SCENELOG_ERROR( log, "<light> with empty 'id' attribute, ignoring." );
        }
        else {
            string context = "In <light id='" + id + "'>, ";

            Light* light = m_database.library<Light>().add( id );
            if( light == NULL ) {
                SCENELOG_ERROR( log, context << "failed to create object." );
            }
            else {
                xmlNodePtr m = n->children;

                // <library_lights>/<light>/<asset>
                Asset light_asset = library_asset;
                if( checkNode( m, "asset" ) ) {
                    Asset asset;
                    if( parseAsset( asset, m ) ) {
                        light_asset = asset;
                    }
                    else {
                        SCENELOG_WARN( log, context << "failed to parse asset." );
                        success = false;
                    }
                    m = m->next;
                }
                // <library_lights>/<light>/<technique_common>
                if( checkNode( m, "technique_common" ) ) {
                    xmlNodePtr l = m->children;
                    xmlNodePtr o = NULL;

                    bool allow_att = false;
                    bool allow_falloff = false;

                    // <library_lights>/<light>/<technique_common>/<ambient>
                    if( checkNode( l, "ambient" ) ) {
                        light->setType( Light::LIGHT_AMBIENT );
                        o = l->children;
                        l = l->next;
                    }
                    // <library_lights>/<light>/<technique_common>/<directional>
                    else if( checkNode( l, "directional" ) ) {
                        light->setType( Light::LIGHT_DIRECTIONAL );
                        o = l->children;
                        l = l->next;
                    }
                    // <library_lights>/<light>/<technique_common>/<point>
                    else if( checkNode( l, "point" ) ) {
                        light->setType( Light::LIGHT_POINT );
                        allow_att = true;
                        o = l->children;
                        l = l->next;
                    }
                    // <library_lights>/<light>/<technique_common>/<spot>
                    else if( checkNode( l, "spot" ) ) {
                        light->setType( Light::LIGHT_SPOT );
                        allow_att = true;
                        allow_falloff = true;
                        o = l->children;
                        l = l->next;
                    }

                    if( checkNode( o, "color" ) ) {
                        std::vector<float> color;
                        if( parseBodyAsFloats( color, o, 3 ) ) {
                            light->setColor( color[0], color[1], color[2] );
                        }
                        else {
                            SCENELOG_ERROR( log, context << "malformed <color>");
                            success = false;
                        }
                        o = o->next;
                    }
                    else {
                        SCENELOG_ERROR( log, context << "expected <color>." );
                        success = false;
                    }

                    if( allow_att && checkNode( o, "constant_attenuation" ) ) {
                        string sid = attribute( o, "sid" );
                        if(!sid.empty() ) {
                            light->setConstantAttenuationSid( sid );
                        }

                        std::vector<float> attenuation;
                        if( parseBodyAsFloats( attenuation, o, 1 ) ) {
                            light->setConstantAttenuation( attenuation[0] );
                        }
                        else {
                            SCENELOG_ERROR( log, context << "malformed <constant_attenuation>" );
                        }
                        o = o->next;
                    }

                    if( allow_att && checkNode( o, "linear_attenuation" ) ) {
                        string sid = attribute( o, "sid" );
                        if(!sid.empty() ) {
                            light->setLinearAttenuationSid( sid );
                        }

                        std::vector<float> attenuation;
                        if( parseBodyAsFloats( attenuation, o, 1 ) ) {
                            light->setLinearAttenuation( attenuation[0] );
                        }
                        else {
                            SCENELOG_ERROR( log, context << "malformed <linear_attenuation>" );
                        }
                        o = o->next;
                    }

                    if( allow_att && checkNode( o, "quadratic_attenuation" ) ) {
                        string sid = attribute( o, "sid" );
                        if(!sid.empty() ) {
                            light->setQuadraticAttenuationSid( sid );
                        }

                        std::vector<float> attenuation;
                        if( parseBodyAsFloats( attenuation, o, 1 ) ) {
                            light->setQuadraticAttenuation( attenuation[0] );
                        }
                        else {
                            SCENELOG_ERROR( log, context << "malformed <quadratic_attenuation>" );
                        }
                        o = o->next;
                    }

                    if( allow_falloff && checkNode( o, "falloff_angle" ) ) {
                        string sid = attribute( o, "sid" );
                        if(!sid.empty()) {
                            light->setFalloffAngleSid( sid );
                        }
                        std::vector<float> falloff;
                        if( parseBodyAsFloats( falloff, o, 1 ) ) {
                            light->setFalloffAngle( falloff[0] );
                        }
                        else {
                            SCENELOG_ERROR( log, context << "malformed <falloff_angle>" );
                        }
                        o = o->next;
                    }

                    if( allow_falloff && checkNode( o, "falloff_exponent" ) ) {
                        string sid = attribute( o, "sid" );
                        if(!sid.empty()) {
                            light->setFalloffExponentSid( sid );
                        }
                        std::vector<float> falloff;
                        if( parseBodyAsFloats( falloff, o, 1 ) ) {
                            light->setFalloffExponent( falloff[0] );
                        }
                        else {
                            SCENELOG_ERROR( log, context << "malformed <falloff_exponent>" );
                        }
                        o = o->next;
                    }


                    m = m->next;
                }
                else {
                    SCENELOG_ERROR( log, context << "expected <technique_common>" );
                }
                // <library_lights>/<light>/<technique>
                while( checkNode( m, "technique" ) ) {
                    // ignore
                    m = m->next;
                }
                // <library_lights>/<light>/<extra>
                while( checkNode( m, "extra" ) ) {
                    // ignore
                    m = m->next;
                }
                nagAboutRemainingNodes( log, m );
                light->setAsset( light_asset );
            }
        }
        n = n->next;
    }

    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );


    m_database.library<Light>().setAsset( library_asset );
    return success;
}


xmlNodePtr
Exporter::createLibraryLights( Context& context ) const
{
    const Library<Light>& lib = m_database.library<Light>();
    if( lib.size() == 0 ) {
        return NULL;
    }
    xmlNodePtr lib_lights_node = newNode( NULL, "library_lights" );
    xmlAddChild( lib_lights_node, createAsset( context, m_database.asset(), lib.asset() ) );
    for( size_t i=0; i<lib.size(); i++ ) {
        xmlAddChild( lib_lights_node, createLight( context, lib.get(i) ) );
    }

    return lib_lights_node;
}





    } // of namespace XML
} // of namespace Scene
