#include <sstream>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Light.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"


namespace Scene {
    namespace Collada {
        using std::string;




xmlNodePtr
Exporter::createLight( Context& context, const Light* light ) const
{
    if( light == NULL ) {
        return NULL;
    }

    xmlNodePtr light_node = newNode( NULL, "light" );
    if( !light->id().empty() ) {
        addProperty( light_node, "id", light->id() );
    }
    xmlAddChild( light_node,
                 createAsset( context,
                              m_database.library<Light>().asset(),
                              light->asset() ) );

    xmlNodePtr tc_node = newChild( light_node, NULL, "technique_common" );

    bool include_color = false;
    bool include_attenuation = false;
    bool include_falloff = false;

    xmlNodePtr type_node = NULL;
    switch( light->type() ) {
    case Light::LIGHT_NONE:
        break;
    case Light::LIGHT_AMBIENT:
        include_color = true;
        type_node = newChild( tc_node, NULL, "ambient" );
        break;
    case Light::LIGHT_DIRECTIONAL:
        include_color = true;
        type_node = newChild( tc_node, NULL, "directional" );
        break;
    case Light::LIGHT_POINT:
        include_color = true;
        include_attenuation = true;
        type_node = newChild( tc_node, NULL, "point" );
        break;
    case Light::LIGHT_SPOT:
        include_color = true;
        include_attenuation = true;
        include_falloff = true;
        type_node = newChild( tc_node, NULL, "spot" );
        break;
    }

    if( type_node != NULL ) {
        std::stringstream o;
        if( include_color ) {
            const float* c = light->color()->floatData();
            o.str("");
            o << c[0] << " " << c[1] << " " << c[2];
            newChild( type_node, NULL, "color", o.str() );
        }
        if( include_attenuation ) {
            o.str("");
            o << light->constantAttenuation()->floatData()[0];
            xmlNodePtr c = newChild( type_node, NULL, "constant_attenuation", o.str() );
            if( !light->constantAttenuationSid().empty() ) {
                addProperty( c, "sid", light->constantAttenuationSid() );
            }

            o.str("");
            o << light->linearAttenuation()->floatData()[0];
            xmlNodePtr l = newChild( type_node, NULL, "linear_attenuation", o.str() );
            if( !light->linearAttenuationSid().empty() ) {
                addProperty( l, "sid", light->linearAttenuationSid() );
            }

            o.str("");
            o << light->quadraticAttenuation()->floatData()[0];
            xmlNodePtr q = newChild( type_node, NULL, "quadratic_attenuation", o.str() );
            if( !light->quadraticAttenuationSid().empty() ) {
                addProperty( q, "sid", light->quadraticAttenuationSid() );
            }
        }
        if( include_falloff) {
            o.str("");
            o << light->falloffAngle();
            xmlNodePtr a = newChild( type_node, NULL, "falloff_angle", o.str() );
            if( !light->falloffAngleSid().empty() ) {
                addProperty( a, "sid", light->falloffAngleSid() );
            }

            o.str("");
            o << light->falloffExponent();
            xmlNodePtr e = newChild( type_node, NULL, "falloff_exponent", o.str() );
            if( !light->falloffExponentSid().empty() ) {
                addProperty( e, "sid", light->falloffExponentSid() );
            }
        }
    }

    return light_node;
}


    } // of namespace XML
} // of namespace Scene

