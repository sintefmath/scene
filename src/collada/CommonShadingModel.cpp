#include "scene/Effect.hpp"
#include "scene/Profile.hpp"
#include "scene/Technique.hpp"
#include "scene/Pass.hpp"
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;



bool
Importer::parseCommonShadingModel( Technique* technique,
                                   const ShadingModelType shading_model,
                                   xmlNodePtr model_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseCommonShadingModel." + technique->profile()->effect()->id() );


    CommonShadingModel* sm = technique->createCommonShadingModel(shading_model );

    for( xmlNodePtr n=model_node->children; n!=NULL; n=n->next ) {
        ShadingModelComponentType type = SHADING_COMP_COMP_N;

        if( strEq( n->name, "emission" ) ) {
                type = SHADING_COMP_EMISSION;
        }
        else if( strEq( n->name, "ambient" ) ) {
                type = SHADING_COMP_AMBIENT;
        }
        else if( strEq( n->name, "diffuse" ) ) {
                type = SHADING_COMP_DIFFUSE;
        }
        else if( strEq( n->name, "specular" ) ) {
                type = SHADING_COMP_SPECULAR;
        }
        else if( strEq( n->name, "shininess" ) ) {
                type = SHADING_COMP_SHININESS;
        }
        else if( strEq( n->name, "reflective" ) ) {
                type = SHADING_COMP_REFLECTIVE;
        }
        else if( strEq( n->name, "reflectivity" ) ) {
                type = SHADING_COMP_REFLECTIVITY;
        }
        else if( strEq( n->name, "transparent" ) ) {
                type = SHADING_COMP_TRANSPARENT;
        }
        else if( strEq( n->name, "transparency" ) ) {
                type = SHADING_COMP_TRANSPARENCY;
        }
        else if( strEq( n->name, "index_of_refraction" ) ) {
                type = SHADING_COMP_REFINDEX;
        }
        else {
            SCENELOG_ERROR( log, "In <" << model_node->name << ">, unexpected node <" << n->name << ">" );
            return false;
        }

        bool color;
        switch( type ) {
        case SHADING_COMP_EMISSION:
        case SHADING_COMP_DIFFUSE:
        case SHADING_COMP_AMBIENT:
        case SHADING_COMP_SPECULAR:
        case SHADING_COMP_REFLECTIVE:
        case SHADING_COMP_TRANSPARENT:
            color = true;
            break;
        case SHADING_COMP_SHININESS:
        case SHADING_COMP_REFLECTIVITY:
        case SHADING_COMP_TRANSPARENCY:
        case SHADING_COMP_REFINDEX:
        case SHADING_COMP_COMP_N:
            color = false;
            break;
        }

        xmlNodePtr nn = n->children;
        if( nn==NULL ) {
            SCENELOG_ERROR( log, "Malformed shading model parameter." );
            continue;
        }
        if( (!color) && strEq( nn->name, "float" ) ) {
            std::vector<float> val;
            if( parseBodyAsFloats( val, nn, 1 ) ) {
                Value v = Value::createFloat( val[0] );
                sm->setComponentValue( type, v );
            }
        }
        else if( color && strEq( nn->name, "color" ) ) {
            std::vector<float> vals;
            if( parseBodyAsFloats( vals, nn, 4 ) ) {
                Value v = Value::createFloat4( vals[0], vals[1], vals[2], vals[3] );
                sm->setComponentValue( type, v );
            }
        }
        else if( strEq( nn->name, "param" ) ) {
            std::string ref = attribute( nn, "ref" );
            sm->setComponentParameterReference( type, ref );
        }
        else if( color && strEq( nn->name, "texture" ) ) {
            SCENELOG_WARN( log, "<texture> in <profile_COMMON> is currently ignored." );
            continue;
        }
        else {
            SCENELOG_ERROR( log, "In <" << n->name << ">, unexpected node <" << nn->name << ">" );
            continue;
        }
    }
    return true;
}


struct Component
{
    ShadingModelComponentType   m_type;
    std::string                 m_name;
};
Component components[ SHADING_COMP_COMP_N ] = {
    { SHADING_COMP_EMISSION,     "emission" },
    { SHADING_COMP_AMBIENT,      "ambient" },
    { SHADING_COMP_DIFFUSE,      "diffuse" },
    { SHADING_COMP_SPECULAR,     "specular" },
    { SHADING_COMP_SHININESS,    "shininess" },
    { SHADING_COMP_REFLECTIVE,   "reflective" },
    { SHADING_COMP_REFLECTIVITY, "reflectivity" },
    { SHADING_COMP_TRANSPARENT,  "transparent" },
    { SHADING_COMP_TRANSPARENCY, "transparency" },
    { SHADING_COMP_REFINDEX,     "index_of_refraction" }
};


xmlNodePtr
Exporter::createCommonShadingModel( Context& context, const CommonShadingModel* sm ) const
{
    if( sm == NULL ) {
        return NULL;
    }

    xmlNodePtr sm_node = NULL;
    switch( sm->shadingModel() ) {
    case SHADING_BLINN:
        sm_node = newNode( NULL, "blinn" );
        break;
    case SHADING_CONSTANT:
        sm_node = newNode( NULL, "constant" );
        break;
    case SHADING_LAMBERT:
        sm_node = newNode( NULL, "lambert" );
        break;
    case SHADING_PHONG:
        sm_node = newNode( NULL, "phong" );
        break;
    default:
        return NULL;
    }
    for( unsigned int i=0; i<SHADING_COMP_COMP_N; i++ ) {
        ShadingModelComponentType comp = components[i].m_type;
        if( sm->isComponentSet( comp ) ) {
            xmlNodePtr comp_node = newChild( sm_node, NULL, components[i].m_name );
            if( sm->isComponentValue( comp ) ) {
                const Value* val = sm->componentValue( comp );
                if( val->type() == VALUE_TYPE_FLOAT ) {
                    xmlNodePtr val_node = newChild( comp_node, NULL, "float" );
                    setBody( val_node, val->floatData(), 1 );
                }
                else if( val->type() == VALUE_TYPE_FLOAT4 ) {
                    xmlNodePtr val_node = newChild( comp_node, NULL, "color" );
                    setBody( val_node, val->floatData(), 4 );
                }
            }
            else if( sm->isComponentParameterReference( comp ) ) {
                xmlNodePtr param_node = newChild( comp_node, NULL, "param" );
                addProperty( param_node, "ref", sm->componentParameterReference( comp ) );
            }
            else if( sm->isComponentImageReference( comp ) ) {
                xmlNodePtr tex_node = newChild( comp_node, NULL, "texture" );
                addProperty( tex_node, "texture", sm->componentImageReference( comp ) );
            }
        }
    }
    return sm_node;
}




    } // of namespace XML
} // of namespace Scene
