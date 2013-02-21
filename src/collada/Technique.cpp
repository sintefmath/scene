#include "scene/Effect.hpp"
#include "scene/Profile.hpp"
#include "scene/Technique.hpp"
#include "scene/CommonShadingModel.hpp"
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;
        using std::unordered_map;

    static const std::string ipackage = "Scene.XML.Importer";
    static const std::string opackage = "Scene.XML.Exporter";

bool
Importer::parseTechnique( Profile* profile,
                          const unordered_map<string,string>& code_blocks,
                          xmlNodePtr technique_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseTechnique" );
    if(!assertNode( technique_node, "technique" ) ) {
        return false;
    }

    const string sid = attribute( technique_node, "sid" );
    Technique* technique = profile->createTechnique( sid );
    if( technique == NULL ) {
        SCENELOG_ERROR( log, "Failed to create technique." );
        return false;
    }

    xmlNodePtr n = technique_node->children;
    if( n != NULL && strEq( n->name, "asset" ) ) {
        Asset asset;
        if(!parseAsset( asset, n ) ) {
            nagAboutParseError( log, n );
            return false;
        }
        technique->setAsset( asset );
    }
    else {
        technique->setAsset( profile->asset() );
    }

    if( profile->type() == PROFILE_COMMON ) {
        ShadingModelType model = SHADING_CONSTANT;
        if( n == NULL ) {
            SCENELOG_ERROR( log, "Expected <profile_COMMON>/<technique>/<blinn|constant|lambert|phong>, got NULL" );
            return false;
        }
        else if( strEq( n->name, "blinn" ) ) {
            model = SHADING_BLINN;
        }
        else if( strEq( n->name, "constant" ) ) {
            model = SHADING_CONSTANT;
        }
        else if( strEq( n->name, "lambert" ) ) {
            model = SHADING_LAMBERT;
        }
        else if( strEq( n->name, "phong" ) ) {
            model = SHADING_PHONG;
        }
        else {
            SCENELOG_ERROR( log, "Expected <profile_COMMON>/<technique>/<blinn|constant|lambert|phong>, got " << n->name );
            return false;
        }
        if( !parseCommonShadingModel( technique, model, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <" << n->name << ">" );
            return false;
        }
        n = n->next;
    }
    else {
        for( ; n!=NULL && strEq( n->name, "annotate" ); n=n->next) {
            SCENELOG_DEBUG( log, "<annotate> ignored." );
        }

        for( ; n!=NULL && strEq( n->name, "pass" ); n=n->next) {

            if(!parsePass( technique, code_blocks, n ) ) {
                SCENELOG_ERROR( log, "Failed to parse <pass>" );
                return false;
            }

        }
    }
    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );
    return true;
}

xmlNodePtr
Exporter::createTechnique( Context& context, const Technique* technique ) const
{
    if( technique == NULL ) {
        return NULL;
    }

    xmlNodePtr t_node = newNode( NULL, "technique" );
    addProperty( t_node, "sid", technique->sid() );
    createAsset( context, technique->profile()->asset(), technique->asset() );

    if( technique->profile()->type() == PROFILE_COMMON ) {
        xmlAddChild( t_node, createCommonShadingModel( context, technique->commonShadingModel() ) );
    }
    else {
        for( size_t i=0; i<technique->passes(); i++ ) {
            xmlAddChild( t_node, createPass( context, technique->pass( i ) ) );
        }
    }
    return t_node;
}


    } // of namespace XML
} // of namespace Scene
