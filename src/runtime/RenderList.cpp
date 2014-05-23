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

#include <cstring>
#include <algorithm>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "scene/Log.hpp"
#include "scene/Profile.hpp"
#include "scene/DataBase.hpp"
#include "scene/VisualScene.hpp"
#include "scene/Geometry.hpp"
#include "scene/Material.hpp"
#include "scene/InstanceGeometry.hpp"
#include "scene/SourceBuffer.hpp"
#include "scene/runtime/RenderList.hpp"
#include "scene/runtime/TransformCache.hpp"

namespace Scene {
    namespace Runtime {
        using std::vector;
        using std::unordered_map;
        using std::string;
        using std::pair;
        using std::make_pair;
        using std::for_each;
        using boost::lexical_cast;

static const string package = "Scene.Runtime.RenderList";


RenderList::RenderList( Resolver& resolver )
    : m_resolver( resolver )
{
    m_list_created.invalidate();
}


bool
RenderList::build( const std::string& visual_scene_id )
{
    //Logger log = getLogger( package + ".build" );

    bool rebuilt = false;

    if( (visual_scene_id != m_visual_scene) ||
        !m_list_created.asRecentAs( m_resolver.database().structureChanged() ) )
    {
    //if( visual_scene_id != m_visual_scene || m_resolver.database().asset().majorChanges( m_list_created ) ) {
        m_visual_scene = visual_scene_id;
        rebuild();
        rebuilt = true;
    }

    return rebuilt;
}

void
RenderList::clear()
{
    Logger log = getLogger( package + ".clear" );
    SCENELOG_DEBUG( log, "invoked" );
    m_operations.clear();
    SCENELOG_DEBUG( log, m_list_created.debugString() );
    m_list_created.invalidate();
    SCENELOG_DEBUG( log, m_list_created.debugString() );
}


/*
bool
RenderList::boundingBox( Value& min, Value& max )
{

    Logger log = getLogger( package + ".boundingBox" );

    struct Instance {
        const Geometry* m_geometry;
        const Value*    m_transform;
    };

    std::list<Instance> instances;
    TransformCache cache( m_resolver.database() );

    const Value* last_transform = NULL;
    for( size_t i=0; i<m_operations.size(); i++ ) {
        if( m_operations[i]->m_type == RenderAction::ACTION_SET_LOCAL_COORDSYS ) {
            last_transform = cache.pathTransformMatrix( m_operations[i]->m_set_local.m_node_path );
        }
        else if( m_operations[i]->m_type == RenderAction::ACTION_DRAW ) {
            Instance instance;
            instance.m_geometry = m_operations[i]->m_draw.m_geometry;
            instance.m_transform = last_transform;
            instances.push_back( instance );
        }
        else if( m_operations[i]->m_type == RenderAction::ACTION_DRAW_INDEXED ) {
            Instance instance;
            instance.m_geometry = m_operations[i]->m_draw_indexed.m_geometry;
            instance.m_transform = last_transform;
            instances.push_back( instance );
        }
    }
    cache.update( 1, 1 );

    bool bb_nonempty = false;
    float bb_min[3] = {0.f, 0.f, 0.f};
    float bb_max[3] = {0.f, 0.f, 0.f};
    for( auto it=instances.begin(); it!=instances.end(); ++it ) {
        Instance& instance = *it;
        if( instance.m_geometry == NULL ) {
            continue;
        }
        if( instance.m_transform == NULL ) {
            continue;
        }
        const Value* g_bb_min;
        const Value* g_bb_max;

        if( instance.m_geometry->boundingBox( g_bb_min, g_bb_max ) ) {
            const float* M = instance.m_transform->floatData();
            for( unsigned int k=0; k<8; k++ ) {
                float p[3] = {
                    (k & 0x1) == 0 ? g_bb_min->floatData()[0] : g_bb_max->floatData()[0],
                    (k & 0x2) == 0 ? g_bb_min->floatData()[1] : g_bb_max->floatData()[1],
                    (k & 0x4) == 0 ? g_bb_min->floatData()[2] : g_bb_max->floatData()[2]
                };
                float r = 1.f/(M[3]*p[0] + M[7]*p[1] + M[11]*p[2] + M[15]);
                float q[3] = {
                    r*(M[0]*p[0] + M[4]*p[1] + M[ 8]*p[2] + M[12]),
                    r*(M[1]*p[0] + M[5]*p[1] + M[ 9]*p[2] + M[13]),
                    r*(M[2]*p[0] + M[6]*p[1] + M[10]*p[2] + M[14]),
                };
                if( bb_nonempty == false ) {
                    bb_min[0] = bb_max[0] = q[0];
                    bb_min[1] = bb_max[1] = q[1];
                    bb_min[2] = bb_max[2] = q[2];
                    bb_nonempty = true;
                }
                else {
                    bb_min[0] = q[0] < bb_min[0] ? q[0] : bb_min[0];
                    bb_min[1] = q[1] < bb_min[1] ? q[1] : bb_min[1];
                    bb_min[2] = q[2] < bb_min[2] ? q[2] : bb_min[2];
                    bb_max[0] = bb_max[0] < q[0] ? q[0] : bb_max[0];
                    bb_max[1] = bb_max[1] < q[1] ? q[1] : bb_max[1];
                    bb_max[2] = bb_max[2] < q[2] ? q[2] : bb_max[2];
                }
            }
        }
    }
    min = Value::createFloat3( bb_min[0], bb_min[1], bb_min[2] );
    max = Value::createFloat3( bb_max[0], bb_max[1], bb_max[2] );
    return bb_nonempty;
}
*/

void
RenderList::rebuild( )
{
    Logger log = getLogger( package + ".rebuild" );

    SCENELOG_DEBUG( log, "Rebuilding," );

    m_resolver.purge();
    m_operations.clear();
    m_items.clear();


    m_set_framebuffer_current = NULL;
    m_set_pass_current = NULL;
    m_set_input_current = NULL;
    m_set_uniform_current = NULL;
    m_set_samplers_current = NULL;
    m_set_local_current = NULL;
    m_set_raster_current = NULL;
    m_set_pixel_ops_current = NULL;
    m_set_fb_ctrl_current = NULL;
    m_set_transforms_current = NULL;


    const VisualScene* visual_scene = NULL;

    if( m_visual_scene == "" && (m_resolver.database().library<VisualScene>().size() > 0 ) ) {
        visual_scene = m_resolver.database().library<VisualScene>().get( 0 );
    }
    else {
        visual_scene = m_resolver.database().library<VisualScene>().get( m_visual_scene );
    }

    if( visual_scene == NULL ) {
        SCENELOG_ERROR( log, "Failed to retrieve visual scene '" << m_visual_scene << "'." );
        return;
    }

    SCENELOG_DEBUG( log, "Processing visual scene '" << visual_scene->id() << "'." );


    size_t evaluate_scenes = visual_scene->evaluateScenes();
    for(size_t i=0; i<evaluate_scenes; i++ ) {
        SCENELOG_DEBUG( log, "Processing evaluate scene " << i );

        const EvaluateScene* evaluate = visual_scene->evaluateScene( i );
        if( evaluate == NULL ) {
            SCENELOG_FATAL( log, "Got null-pointer @ " << __LINE__ );
            continue;
        }

        for(size_t i=0; i<evaluate->renderItems(); i++) {
            processRender( visual_scene, evaluate->renderItem(i) );
        }
    }

    // Restore state
    if( !m_operations.empty() ) {
        std::vector<Bind> bind;
        const RenderAction* def_framebuffer = m_resolver.setRenderTarget( bind, NULL, NULL );
        if( m_set_framebuffer_current != def_framebuffer ) {
            m_operations.push_back( def_framebuffer );
        }
        const RenderAction* def_raster = m_resolver.setRaster( NULL );
        if( m_set_raster_current != def_raster ) {
            m_operations.push_back( def_raster );
        }
        const RenderAction* def_pix_ops = m_resolver.setPixelOps( NULL );
        if( m_set_pixel_ops_current != def_pix_ops ) {
            m_operations.push_back( def_pix_ops );
        }
        const RenderAction* def_fb_ctrl = m_resolver.setFBCtrl( NULL );
        if( m_set_fb_ctrl_current != def_fb_ctrl ) {
            m_operations.push_back( def_fb_ctrl );
        }
    }

    m_list_created.touch();
    dumpRenderList();

    SCENELOG_DEBUG( log, "# items in render list = " << m_operations.size() );
}

void
RenderList::dumpRenderList() const
{
    for( size_t i=0; i<m_operations.size(); i++) {
        m_operations[i]->debugDump();
    }
}

void
RenderList::addRenderItem( const Context&              context,
                           const Material*             render_target_material,
                           const Pass*                 render_target_pass,
                           const Material*             material,
                           const Pass*                 pass,
                           const CommonShadingModel*   common,
                           const Geometry*             geometry,
                           const Primitives*           primitives )
{

    std::vector<Bind> dummy_bind;

    Logger log = getLogger( package + ".addRenderItem" );
    if( material == NULL ) {
        SCENELOG_ERROR( log, "material==NULL" );
        return;
    }
    if( geometry == NULL ) {
        SCENELOG_ERROR( log, "geometry==NULL" );
        return;
    }
    if( primitives == NULL ) {
        SCENELOG_ERROR( log, "primitives==NULL" );
        return;
    }


    // --- set framebuffer state
    const RenderAction* set_framebuffer = NULL;
    if( render_target_material != NULL ) {
        set_framebuffer = m_resolver.setRenderTarget( dummy_bind,
                                                        render_target_material,
                                                        render_target_pass );
        if( set_framebuffer == NULL ) {
            SCENELOG_ERROR( log, "Failed to resolve render target, skipping batch." );
            return;
        }
    }
    else {
        set_framebuffer = m_resolver.setRenderTarget( dummy_bind,
                                                      NULL,
                                                      NULL );
    }
    if( m_set_framebuffer_current != set_framebuffer ) {
        m_set_framebuffer_current = set_framebuffer;
        m_operations.push_back( m_set_framebuffer_current );
    }

    const RenderAction* set_raster = m_resolver.setRaster( pass );
    if( m_set_raster_current != set_raster ) {
        m_operations.push_back( set_raster );
        m_set_raster_current = set_raster;
    }

    const RenderAction* set_pixel_ops = m_resolver.setPixelOps( pass );
    if( m_set_pixel_ops_current != set_pixel_ops ) {
        m_operations.push_back( set_pixel_ops );
        m_set_pixel_ops_current = set_pixel_ops;
    }

    const RenderAction* set_fb_ctrl = m_resolver.setFBCtrl( pass );
    if( m_set_fb_ctrl_current != set_fb_ctrl ) {
        m_operations.push_back( set_fb_ctrl );
        m_set_fb_ctrl_current = set_fb_ctrl;
    }


    // --- set transforms ------------------------------------------------------

    std::list<const Node*> node_path = context.m_node_path_;
    if( context.m_current_node != NULL ) {
        node_path.push_back( context.m_current_node );
    }
    const RenderAction* set_local_coordsys = m_resolver.setLocalCoordSys( node_path );
    if( m_set_local_current != set_local_coordsys ) {
        m_operations.push_back( set_local_coordsys );
        m_set_local_current = set_local_coordsys;
    }

#ifdef DEBUG
    std::string nodepath_str;
    for(auto it=node_path.begin(); it!=node_path.end(); ++it ) {
        nodepath_str += "/" + (*it)->debugString();
    }
    nodepath_str += ":" + geometry->id();
    SCENELOG_DEBUG( log, "Instancing " << nodepath_str );
#endif

    // --- set shader state ----------------------------------------------------
    if( pass == NULL ) {
        SCENELOG_ERROR( log, "pass == NULL" );
        return;
    }

    // shader
    const RenderAction* set_pass = m_resolver.setPass( pass );
    if( m_set_pass_current != set_pass ) {
        m_operations.push_back( set_pass );
        m_set_pass_current = set_pass;
    }

    // parameters
    const ResolvedParams* resolved_params = m_resolver.resolveParams( dummy_bind,
                                                                      material,
                                                                      pass );
    if( resolved_params == NULL ) {
        SCENELOG_ERROR( log, "Failed to resolve params" );
        return;
    }

    // samplers
    const RenderAction* set_samplers = m_resolver.setSamplers( resolved_params );
    if( m_set_samplers_current != set_samplers ) {
        if( set_samplers != NULL ) {
            m_operations.push_back( set_samplers );
            m_set_samplers_current = set_samplers;
        }
    }

    // uniforms
    const RenderAction* set_uniforms = m_resolver.setUniforms( set_samplers,
                                                               dummy_bind,
                                                               material,
                                                               pass );

    m_operations.push_back( set_uniforms );
    m_set_uniform_current = set_uniforms;

    // inputs
    const RenderAction* set_inputs = m_resolver.setInputs( pass, geometry, primitives );
    if( set_inputs == NULL ) {
        SCENELOG_ERROR( log, "Failed to resolve inputs, skipping batch." );
        return;
    }
    if( m_set_input_current != set_inputs ) {
        m_operations.push_back( set_inputs );
        m_set_input_current = set_inputs;
    }

    // invoke draw
    const RenderAction* draw = m_resolver.draw( geometry, primitives, pass );

    if( draw != NULL ) {
        m_operations.push_back( draw );
    }

#ifdef SCENE_RL_CHUNKS
    Item item;
    item.m_action_set_framebuffer = set_framebuffer;
    item.m_action_set_pass        = set_pass;
    item.m_action_set_input       = set_inputs;
    item.m_action_set_uniform     = set_uniforms;
    item.m_action_set_samplers    = set_samplers;

    item.m_set_render_targets   = &set_framebuffer->m_set_render_targets;
    item.m_set_pass             = &set_pass->m_set_pass;
    item.m_set_inputs           = &set_inputs->m_set_inputs;
    item.m_set_uniforms         = &set_uniforms->m_set_uniforms;
    item.m_set_samplers         = &set_samplers->m_set_samplers;
    item.m_set_local_coordsys   = &set_local_coordsys->m_set_local;
    item.m_set_view_coordsys    = &m_set_transforms_current->m_set_view;
    item.m_set_raster           = &set_raster->m_set_rasterization;
    item.m_set_pixel_ops        = &set_pixel_ops->m_set_pixel_ops;
    item.m_set_fb_ctrl          = &set_fb_ctrl->m_set_fb_ctrl;

    if( draw->m_type == RenderAction::ACTION_DRAW ) {
        item.m_draw = &draw->m_draw;
        item.m_draw_indexed = NULL;
    }
    else if( draw->m_type == RenderAction::ACTION_DRAW_INDEXED ) {
        item.m_draw = NULL;
        item.m_draw_indexed = &draw->m_draw_indexed;
    }
    else {
        return;
    }

    m_items.push_back( item );
#endif
}

bool
RenderList::getMaterialChildren( const Material*&    material,
                                 const Effect*&      effect,
                                 const Profile*&     profile,
                                 const Technique*&   technique,
                                 const Pass*&        pass,
                                 const std::string&  material_id,
                                 const std::string&  override_tech,
                                 const std::string&  override_pass )
{
    Logger log = getLogger( package + ".getMaterialChildren" );


    material = m_resolver.database().library<Material>().get( material_id );
    if( material == NULL ) {
        SCENELOG_WARN( log, "Unable to retrieve material '" << material_id << '\'' );
        return false;
    }
    else {
        effect = m_resolver.database().library<Effect>().get( material->effectId() );
        if( effect == NULL ) {
            SCENELOG_WARN( log, "Unable to retrieve effect '" << material->effectId() << '\'' );
            return false;
        }
        else {
            profile = effect->profile( m_resolver.profile() );
            if( profile == NULL && m_resolver.profile() != PROFILE_COMMON ) {
                SCENELOG_INFO( log, "Looking for common profile." );
                profile = effect->profile( PROFILE_COMMON );
            }
            if( profile == NULL ) {
                SCENELOG_WARN( log, "Unable to retrieve profile '" << m_resolver.profile() << '\'' );
                return false;
            }
            else {

                if( override_tech.empty() ) {
                    technique = profile->technique( material->techniqueHint( m_resolver.profile(), m_resolver.platform() ) );
                    if( technique == NULL ) {
                        SCENELOG_WARN( log, "No suitable techniques." );
                        return false;
                    }
                    pass = NULL;
                    return true;
                }
                else {
                    technique = profile->technique( override_tech );
                    if( technique == NULL ) {
                        SCENELOG_ERROR( log, "Technique '" << override_tech << "' not found in effect '" << effect->id() << '\'' );
                        return false;
                    }
                    if( override_pass.empty() ) {
                        pass = NULL;
                        return true;
                    }
                    else {
                        pass = technique->pass( override_pass );
                        if( pass == NULL ) {
                            SCENELOG_ERROR( log, "Pass '" << override_pass << "' not found in technique '" << override_tech << '\'' );
                            return false;
                        }
                        else {
                            return true;
                        }
                    }
                }
            }
        }
    }
}


void
RenderList::processNode( Context&         context,
                         const Material*  render_target_material,
                         const Pass*      render_target_pass,
                         bool             override_material )
{
    Logger log = getLogger( package + ".processRenderNodeList" );

    SCENELOG_INFO( log, "Processing " << context.m_current_node->debugString() );

    // Check layer spec. If list of layers to include is empty, render all
    // layers. Otherwise, we check if the node is contained.

    LayerMask new_layer_mask = context.m_layer_mask;
    LayerMask node_layer_mask = m_resolver.layerMask( context.m_current_node );

    bool include_node;
    if( (m_resolver.profile() & context.m_current_node->profileMask() ) != 0u ) {
        if( context.m_layer_mask == 0u ) {
            include_node = true;
        }
        else {
            if( node_layer_mask == 0u ) {
                include_node = true;
            }
            else {
                include_node = (context.m_layer_mask & node_layer_mask) != 0;
                if( include_node ) {
                    // We got a match, clear the layer mask for further recursion
                    //new_layer_mask = 0u;
                    SCENELOG_DEBUG( log, "Got a specific match on node " << context.m_current_node->debugString() <<
                                    ", render_layer_mask=" << context.m_layer_mask <<
                                    ", node_layer_mask=" << node_layer_mask <<
                                    ", all children will be processed." );

                }
                else {
                    SCENELOG_DEBUG( log, "No match on node " << context.m_current_node->debugString() <<
                                    ", render_layer_mask=" << context.m_layer_mask <<
                                    ", node_layer_mask=" << node_layer_mask <<
                                    ", skipping this sub-tree." );
                }
            }
        }
    }
    else {
        include_node = false;
    }

    if(!include_node) {
        return;
    }

    // Instance node
    for( size_t j=0; j<context.m_current_node->instanceNodes(); j++ ) {
        const string& id = context.m_current_node->instanceNode(j);
        const Node* n = m_resolver.database().library<Node>().get( id );
        if( n == NULL ) {
            SCENELOG_ERROR( log, "Unable to find node " << id );
        }
        else {


            Context recurse_context = context;
            recurse_context.m_current_node = n;

            recurse_context.m_node_path_.push_back( context.m_current_node );          // Instancer
            recurse_context.m_node_path_.push_back( recurse_context.m_current_node );  // Instancee
            recurse_context.m_layer_mask = new_layer_mask;

            SCENELOG_DEBUG( log, "Instancing " << n->debugString() );
            processNode( recurse_context,
                        render_target_material,
                        render_target_pass,
                        override_material );
        }
    }

    // Recurse into children
    for( size_t i=0; i<context.m_current_node->children(); i++ ) {

        Context recurse_context = context;
        recurse_context.m_current_node = context.m_current_node->child(i);
        recurse_context.m_layer_mask = new_layer_mask;

        processNode( recurse_context,
                     render_target_material,
                     render_target_pass,
                     override_material );
    }

    // Instance geometry
    if( include_node && context.m_current_node->geometryInstances() > 0 ) {
        SCENELOG_DEBUG( log, "Instancing geometry of node " << context.m_current_node->debugString() );

        instanceGeometry( context.m_current_node,
                         context,
                         render_target_material,
                         render_target_pass,
                         override_material );
    }
}




void
RenderList::instanceGeometry( const Node*                node,
                              const Context&             context,
                              const Material*            render_target_material,
                              const Pass*                render_target_pass,
                              bool                       override_material )
{
    Logger log = getLogger( package + ".instanceGeometry" );

    SCENELOG_INFO( log, "Instancing geometry of node " << node->debugString() );


    for(size_t i=0; i<node->geometryInstances(); i++) {
        const InstanceGeometry* instance = node->geometryInstance( i );

        const Geometry* geometry = m_resolver.database().library<Geometry>().get( instance->geometryId() );
        if( geometry == NULL ) {
            SCENELOG_ERROR( log, "Failed to retrieve geometry '" << instance->geometryId() << "'." );
        }
        else {

            SCENELOG_DEBUG( log, "geometry='" << geometry->id() << "'" );
            for(size_t i=0; i<geometry->primitiveSets(); i++) {
                const Primitives* primitives = geometry->primitives( i );
                if( primitives->hasSharedInputs() ) {
                    SCENELOG_WARN( log, "geometry '" << geometry->id() << "', primitive set " << i << " has shared inputs, skipping." );
                    continue;
                }


                const string& material_symbol = primitives->materialSymbol();

                const string& target_id = instance->materialBindingTargetId( material_symbol );
                if( target_id.empty() ) {
                    SCENELOG_ERROR( log,
                                  "Geometry '" << geometry->id() << "', " <<
                                  "primitive set " << i << ": " <<
                                  "Unable to find material binding for '" << material_symbol << "'." );
                }
                else {
                    const Material* material = NULL;
                    const Effect* effect = NULL;
                    const Profile* profile = NULL;
                    const Technique* technique = NULL;
                    const Pass* pass = NULL;

                    bool success = true;
                    if( !getMaterialChildren( material, effect, profile, technique, pass, target_id ) ) {
                        SCENELOG_WARN( log, "Missing children, using default material." );
                        if(!getMaterialChildren( material, effect, profile, technique, pass, "phong"  ) ) {
                            SCENELOG_FATAL( log, "Unable to use default material." );
                            success = false;
                        }
                    }
                    if( success ) {
                        if( technique->profile()->type() == PROFILE_COMMON ) {
                            addRenderItem( context,
                                           render_target_material,
                                           render_target_pass,
                                           material,
                                           NULL,
                                           technique->commonShadingModel(),
                                           geometry,
                                           primitives );

                        }
                        else if( override_material ) {
                            addRenderItem( context,
                                           render_target_material,
                                           render_target_pass,
                                           render_target_material,
                                           render_target_pass,
                                           NULL,
                                           geometry,
                                           primitives );
                        }
                        else {
                            for(size_t k=0; k<technique->passes(); k++) {
                                addRenderItem( context,
                                               render_target_material,
                                               render_target_pass,
                                               material,
                                               technique->pass(k),
                                               NULL,
                                               geometry,
                                               primitives );
                            }
                        }
                    }
                }
            }
        }
    }
}

void
RenderList::traverseVisualSceneNodeHierarchy( const VisualScene*             visual_scene,
                                              Context&                       context,
                                              const Material*                rt_material,
                                              const Pass*                    rt_pass,
                                              bool                           use_rt_as_material )
{

    const Node* visual_scene_node = m_resolver.database().library<Node>().get( visual_scene->nodesId() );
    if( visual_scene_node != NULL ) {
        context.m_node_path_.push_back( visual_scene_node );

        for(size_t i=0; i<visual_scene_node->children(); i++ ) {

            context.m_current_node = visual_scene_node->child(i);
            processNode( context,
                         rt_material,
                         rt_pass,
                         use_rt_as_material );

        }

        context.m_node_path_.pop_back();
    }
}


void
RenderList::processInstanceMaterialPass( const VisualScene*               visual_scene,
                                         Context&                         context,
                                         const std::vector<std::string>&  layers,
                                         const Material*                  material,
                                         const Pass*                      pass )
{
    Logger log = getLogger( package + ".processInstanceMaterialPass" );


    const Node* visual_scene_node = NULL;
    if( !visual_scene->nodesId().empty() ) {
        visual_scene_node = m_resolver.database().library<Node>().get( visual_scene->nodesId() );
    }


    Scene::Draw draw = pass->draw();
    if( draw == DRAW_GEOMETRY ) {
        SCENELOG_ERROR( log, "DRAW_GEOMETRY: Not really sure what to draw..." );
        return;
    }
    else if( draw == DRAW_SCENE_GEOMETRY ) {
        traverseVisualSceneNodeHierarchy( visual_scene,
                                          context,
                                          material,
                                          pass,
                                          true );
    }
    else if( draw == DRAW_SCENE_IMAGE ) {
        traverseVisualSceneNodeHierarchy( visual_scene,
                                          context,
                                          material,
                                          pass,
                                          false );
    }
    else if( draw == DRAW_FULL_SCREEN_QUAD ) {
        const Geometry* geometry = m_resolver.database().library<Geometry>().get( "builtin.full_screen_quad" );
        if( geometry == NULL ) {
            SCENELOG_FATAL( log, "Cannot find full-screen quad geometry!" );
            return;
        }
        if( geometry->primitiveSets() < 1 ) {
            SCENELOG_FATAL( log, "FSQ geometry contain no primitives!");
            return;
        }

        const Primitives* primitives = geometry->primitives(0);
        addRenderItem( context,
                       material,
                       pass,
                       material,
                       pass,
                       NULL,
                       geometry,
                       primitives );
   }
    else {
        SCENELOG_ERROR( log, "Unhandled draw mode 0x" << std::hex << draw << std::dec );
        return;
    }

}



void
RenderList::processRender( const VisualScene*          visual_scene,
                           const Render*  render )
{
    Logger log = getLogger( package + ".processRender" );


    SCENELOG_DEBUG( log,
                   "Processing visual scene '" << visual_scene->id() << '\'' <<
                   " render instance " << render << " (sid='"<< render->sid() <<"')." );


    const Node* visual_scene_node = NULL;
    if( !visual_scene->nodesId().empty() ) {
        visual_scene_node = m_resolver.database().library<Node>().get( visual_scene->nodesId() );
    }

    m_set_transforms_current = m_resolver.setViewCoordSys( visual_scene_node, render );
    m_operations.push_back( m_set_transforms_current );

    // Get the camera to use
    Context context;
    context.m_layer_mask = m_resolver.layerMask( render );

    if( visual_scene_node != NULL ) {
        if( !render->cameraNodeId().empty() ) {
            const Node* camera_node = m_resolver.database().library<Node>().get( render->cameraNodeId() );
            if( camera_node == NULL ) {
                SCENELOG_ERROR( log, "Unable to find camera '" << render->cameraNodeId() << "'." );
            }
            else {
                if( camera_node->instanceCameras() > 0 ) {
                    context.m_camera = m_resolver.database().library<Camera>().get( camera_node->instanceCameraURL(0) );
                }
                if( m_resolver.findNodePath( context.m_camera_path_, visual_scene_node, camera_node ) ) {
                }
            }
        }
    }

    context.m_camera = NULL;
    if( !context.m_camera_path_.empty() ) {
        const Node* camera_node = context.m_camera_path_.back();
        if( camera_node->instanceCameras() == 0 ) {
            SCENELOG_WARN( log, "No camera instanced in camera node." );
        }
        else {
            context.m_camera = m_resolver.database().library<Camera>().get( camera_node->instanceCameraURL(0) );
            if( context.m_camera == NULL ) {
                SCENELOG_ERROR( log, "Unable to locate camera '" << camera_node->instanceCameraURL(0) << "'." );
            }
        }
    }

    context.m_current_node = NULL;

    if( render->instanceMaterialId().empty() ) {
        if( visual_scene_node != NULL ) {
           traverseVisualSceneNodeHierarchy( visual_scene,
                                             context,
                                             NULL,
                                             NULL,
                                             false );

        }
    }
    else {
        // Instance material pipeline

        const Material* material = NULL;
        const Effect* effect = NULL;
        const Profile* profile = NULL;
        const Technique* technique = NULL;
        const Pass* pass = NULL;

        if( !getMaterialChildren( material,
                                  effect,
                                  profile,
                                  technique,
                                  pass,
                                  render->instanceMaterialId(),
                                  render->instanceMaterialTechniqueOverrideSid(),
                                  render->instanceMaterialTechniqueOverridePassSid() ) )
        {
            SCENELOG_ERROR( log, "Unable to find material - technique chain" <<
                            ", override_tech='" << render->instanceMaterialTechniqueOverrideSid() << '\'' <<
                            ", override_pass='" << render->instanceMaterialTechniqueOverridePassSid() << '\'' );
            return;
        }

        if( pass != NULL ) {
            processInstanceMaterialPass( visual_scene,
                                         context,
                                         render->m_layers,
                                         material,
                                         pass );
        }
        else {
            for(size_t i=0; i<technique->passes(); i++ ) {
                processInstanceMaterialPass( visual_scene,
                                             context,
                                             render->m_layers,
                                             material,
                                             technique->pass(i) );
            }
        }
    }
}


    } // of namespace Runtime
} // of namespace Scene

