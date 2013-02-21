#include <sstream>
#include "scene/Log.hpp"
#include "scene/Render.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"


namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;


xmlNodePtr
Exporter::createRender( Context& context, const Render* render ) const
{
    if( render == NULL ) {
        return NULL;
    }

    xmlNodePtr render_node = newNode( NULL, "render" );
    if( !render->sid().empty() ) {
        addProperty( render_node, "sid", render->sid() );
    }
    if( !render->cameraNodeId().empty() ) {
        addProperty( render_node, "camera_node", "#" + render->cameraNodeId() );
    }

    for( size_t i=0; i<render->layers(); i++ ) {
        newChild( render_node, NULL, "layer", render->layer(i) );
    }
    if( !render->instanceMaterialId().empty() ) {
        xmlNodePtr im_node = newChild( render_node, NULL, "instance_material" );
        addProperty( im_node, "url", "#" + render->instanceMaterialId() );
        if( !render->instanceMaterialTechniqueOverrideSid().empty() ) {
            xmlNodePtr to_node = newChild( im_node, NULL, "technique_override" );
            addProperty( to_node, "ref", render->instanceMaterialTechniqueOverrideSid() );
            if( !render->instanceMaterialTechniqueOverridePassSid().empty() ) {
                addProperty( to_node, "pass", render->instanceMaterialTechniqueOverridePassSid() );
            }
        }
    }

    xmlNodePtr ex_t_node = NULL;
    for( size_t i=0; i<SCENE_LIGHTS_MAX; i++ ) {
        if( !render->lightNodeId( i ).empty() ) {
            if( ex_t_node == NULL ) {
                ex_t_node = newChild( newChild( render_node, NULL, "extra" ),
                                      NULL,
                                      "technique" );
                addProperty( ex_t_node, "profile", "scene" );
            }
            xmlNodePtr ln_node = newChild( ex_t_node, NULL, "light_node" );

            std::stringstream tmp;
            tmp << i;
            addProperty( ln_node, "index", tmp.str() );
            addProperty( ln_node, "ref", render->lightNodeId(i) );
        }
    }

    return render_node;
}




    } // of namespace XML
} // of namespace Scene
