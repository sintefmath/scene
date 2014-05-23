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
#include "scene/Asset.hpp"
#include "scene/DataBase.hpp"
#include "scene/VisualScene.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;


bool
Importer::parseVisualScene( Context context,
                            const Asset& asset_parent,
                            xmlNodePtr visual_scene_node )
{
    Logger log = getLogger( "Scene.XML.parseVisualScene" );
    if(!assertNode( visual_scene_node, "visual_scene" ) ) {
        return false;
    }

    // ID is optional in COLLADA, but we require it.
    const string id = attribute( visual_scene_node, "id" );
    if(id.empty() ) {
        SCENELOG_ERROR( log, "<visual_scene> with empty 'id' attribute." );
        return false;
    }


    VisualScene* visual_scene = m_database.library<VisualScene>().add( id );
    if( visual_scene == NULL ) {
        SCENELOG_ERROR( log, "Failed to create visual scene '" << id << "'." );
        return false;
    }

    bool success = true;
    xmlNodePtr n = visual_scene_node->children;

    Asset vis_scene_asset = asset_parent;
    if( checkNode( n, "asset" ) ) {
        Asset asset;
        if( parseAsset( context, asset, n ) ) {
            vis_scene_asset = asset;
        }
        else {
            nagAboutParseError( log, n );
            success = false;
        }
        n = n->next;
    }

    Node* visual_scene_nodes = NULL;

    while( checkNode( n, "node" ) ) {

        // Create an entry in library nodes to hold the nodes of this visual scene
        if( visual_scene_nodes == NULL ) {
            const string visual_scene_node_id = visual_scene->id() + "_nodes";
            visual_scene_nodes = m_database.library<Node>().add( visual_scene_node_id );
            if( visual_scene_nodes == NULL ) {
                SCENELOG_ERROR( log, "Failed to create node '" << visual_scene_node_id << "' in library<Node>." );
                success = false;
            }
            else {
                visual_scene->setNodesId( visual_scene_node_id );
            }
        }

        if( visual_scene_nodes != NULL ) {
            if( !parseNode( visual_scene_nodes, vis_scene_asset, n ) ) {
                nagAboutParseError( log, n );
            }
        }
        n = n->next;
    }


    for( ; checkNode( n, "evaluate_scene" ); n = n->next ) {
        EvaluateScene* eval = visual_scene->addEvaluateScene();
        eval->setEnabled( parseBool( attribute( n, "enable" ), true ) );
        eval->setId( attribute( n, "id" ) );
        eval->setSid( attribute( n, "sid" ) );


        xmlNodePtr m = n->children;

        for( ; checkNode( m, "text" ); m = m->next ) {
            // Eat possible whitespace lying around
        }

        if( checkNode( m, "asset" ) ) {
            // ignore for now
            m = m->next;
        }

        for( ; checkNode( m, "render" ); m = m->next ) {

            Render* render = eval->addRenderItem();
            render->setSid( attribute( m, "sid" ) );
            render->setCameraNodeId( cleanRef( attribute( m, "camera_node" ) ) );

            xmlNodePtr o = m->children;
            for( ; checkNode( o, "text" ); o = o->next ) {
                // Eat possible whitespace lying around
            }
            for( ; checkNode( o, "layer"); o = o->next ) {
                render->addLayer( getBody( o ) );
            }
            if( checkNode( o, "instance_material" ) ) {
                string ref = cleanRef( attribute( o, "url" ) );
                if( ref.empty() ) {
                    nagAboutParseError( log, o, "missing required attribute 'url'" );
                    success = false;
                }
                else {
                    render->setInstanceMaterialId( ref );

                    xmlNodePtr p = o->children;
                    for( ; checkNode( p, "text" ); p = p->next ) {
                        // Eat possible whitespace lying around
                    }
                    if( checkNode( p, "technique_override" ) ) {
                        string ref = attribute( p, "ref" );
                        if( ref.empty() ) {
                            nagAboutParseError( log, p, "missing required attribute 'ref'" );
                        }
                        else {
                            render->setInstanceMaterialTechniqueOverride( ref,
                                                                          attribute( p, "pass" ) );
                        }
                        p = p->next;
                    }
                    for( ; checkNode( p, "bind" ); p = p->next ) { }
                    ignoreExtraNodes( log, p );
                    nagAboutRemainingNodes( log, p );
                }
                o = o->next;
            }
            for( ; checkNode( o, "extra" ); o = o->next ) {
                xmlNodePtr q = o->children;
                for( ; checkNode( q, "technique" ); q = q->next ) {
                    const string profile = attribute( q, "profile" );
                    if( profile == "Scene" || profile == "scene" ) {

                        xmlNodePtr p = q->children;
                        for( ; p != NULL; p = p->next) {
                            if( checkNode( p, "light_node" ) ) {
                                // Scene extension: <light_node index=".." ref="..." /> specifies light sources
                                string idx = attribute( p, "index" );
                                string ref = cleanRef( attribute( p, "ref" ) );

                                int index = 0;
                                if( !idx.empty() ) {
                                    index = atoi( idx.c_str() );
                                }
                                if( ( 0 <= index) && (index < SCENE_LIGHTS_MAX ) ) {
                                    render->setLightNodeId( index, ref );
                                }
                                else {
                                    nagAboutParseError( log, p, "has illegal index" );
                                }
                            }
                        }
                    }
                }
            }
            nagAboutRemainingNodes( log, o );
        }
        ignoreExtraNodes( log, m );
        nagAboutRemainingNodes( log, m );
    }
    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );

    visual_scene->setAsset( vis_scene_asset );
    return success;
}

xmlNodePtr
Exporter::createVisualScene( Context& context, const VisualScene* scene ) const
{
    if( scene == NULL ) {
        return NULL;
    }

    xmlNodePtr scene_node = newNode( NULL, "visual_scene" );
    if( !scene->id().empty() ) {
        addProperty( scene_node, "id", scene->id() );
    }
    xmlAddChild( scene_node, createAsset( context, m_database.library<VisualScene>().asset(), scene->asset() ) );
    if( !scene->nodesId().empty() ) {
        const Node* root = m_database.library<Node>().get( scene->nodesId() );
        if( root != NULL ) {
            for( size_t i=0; i<root->children(); i++ ) {
                xmlAddChild( scene_node, createNode( context, root->child(i) ) );
            }
        }
    }

    // evaluate scenes
    for( size_t i=0; i<scene->evaluateScenes(); i++ ) {
        xmlNodePtr eval_node = newChild( scene_node, NULL, "evaluate_scene" );
        const EvaluateScene* eval = scene->evaluateScene(i);
        if( !eval->id().empty() ) {
            addProperty( eval_node, "id", eval->id() );
        }
        if( !eval->sid().empty() ) {
            addProperty( eval_node, "sid", eval->sid() );
        }
        if( eval->enabled() == false ) {
            addProperty( eval_node, "enabled", "false" );
        }
        for( size_t k=0; k<eval->renderItems(); k++ ) {
            xmlAddChild( eval_node, createRender( context, eval->renderItem(k) ) );
        }
    }

    return scene_node;
}


    } // of namespace XML
} // of namespace Scene

