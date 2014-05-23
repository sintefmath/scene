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

#ifndef _WIN32
#include <hash_fun.h>
#endif
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "scene/Log.hpp"
#include "scene/Effect.hpp"
#include "scene/Profile.hpp"
#include "scene/Pass.hpp"
#include "scene/SourceBuffer.hpp"
#include "scene/Material.hpp"
#include "scene/runtime/Resolver.hpp"
#include "scene/runtime/RenderAction.hpp"

namespace Scene {
    namespace Runtime {
        using std::for_each;
        using std::equal;
        using std::list;
        using std::vector;
        using std::string;
        using boost::lexical_cast;
        using std::unordered_map;
        using std::pair;
        using std::make_pair;

static const string package = "Scene.Runtime.Resolver";

const LayerMask
Resolver::layerMask( const std::string& layer )
{
    auto it = m_layer_masks.find( layer );
    if( it != m_layer_masks.end() ) {
        return it->second;
    }
    size_t bit = m_layer_masks.size();
    LayerMask mask = 1u<<bit;
    m_layer_masks[ layer ] = mask;

    Logger log = getLogger( package + "layerMask" );
    SCENELOG_TRACE( log, "Layer '"<< layer << "' has layer mask 0x" << std::hex << mask << std::dec );
    return mask;
}

const LayerMask
Resolver::layerMask( const Render* render )
{
    CacheKey<1> key( render );
    auto it = m_layer_mask_render.find( key );
    if( it != m_layer_mask_render.end() ) {
        if( it->second.m_timestamp.asRecentAs( render->evaluateScene()->visualScene()->structureChanged() ) ) {
        //if( !render->evaluateScene()->visualScene()->asset().majorChanges( it->second.m_timestamp ) ) {
            return it->second.m_mask;
        }
    }
    CachedLayerMask cached_mask;
    cached_mask.m_timestamp.touch();
    cached_mask.m_mask = 0u;
    for( size_t i=0; i<render->layers(); i++ ) {
        cached_mask.m_mask |= layerMask( render->layer(i) );
    }
    m_layer_mask_render[ key ] = cached_mask;

    Logger log = getLogger( package + "layerMask" );
    SCENELOG_TRACE( log, "Render item " << render << " (sid='"<< render->sid() << "') has layer mask 0x" << std::hex << cached_mask.m_mask << std::dec );

    return cached_mask.m_mask;
}

const LayerMask
Resolver::layerMask( const Node* node )
{
    CacheKey<1> key( node );
    auto it = m_layer_mask_node.find( key );
    if( it != m_layer_mask_node.end() ) {
        if( it->second.m_timestamp.asRecentAs( node->structureChanged() ) ) {
        //if( !node->asset().majorChanges( it->second.m_timestamp ) ) {
            return it->second.m_mask;
        }
    }

    CachedLayerMask cached_mask;
    cached_mask.m_timestamp.touch();
    cached_mask.m_mask = 0u;
    for( size_t i=0; i<node->layers(); i++ ) {
        cached_mask.m_mask |= layerMask( node->layer(i) );
    }
    m_layer_mask_node[ key ] = cached_mask;

    Logger log = getLogger( package + "layerMask" );
    SCENELOG_TRACE( log, "Node " << node->debugString() << " has layer mask 0x" << std::hex << cached_mask.m_mask << std::dec );

    return cached_mask.m_mask;
}


const RenderAction*
Resolver::setViewCoordSys( const Node*    visual_scene_node,
                   const Render*  render )
{

    const Node* camera_node = NULL;
    const Camera* camera = NULL;
    if( !render->cameraNodeId().empty() ) {
        camera_node = m_database.library<Node>().get( render->cameraNodeId() );
        if( camera_node != NULL && camera_node->instanceCameras() > 0 ) {
            camera = m_database.library<Camera>().get( camera_node->instanceCameraURL(0) );
        }
    }

    const Light* lights[SCENE_LIGHTS_MAX];
    const Camera* light_projections[SCENE_LIGHTS_MAX];
    const Node* light_nodes[ SCENE_LIGHTS_MAX];
    for(size_t j=0; j<SCENE_LIGHTS_MAX; j++) {
        lights[j] = NULL;
        light_projections[j] = NULL;
        light_nodes[j] = NULL;
        if( !render->lightNodeId(j).empty() ) {
            light_nodes[j] = m_database.library<Node>().get( render->lightNodeId(j) );
            if( light_nodes[j] != NULL && light_nodes[j]->instanceLights() > 0 ) {
                lights[j] = m_database.library<Light>().get( light_nodes[j]->instanceLightRef(0) );
            }
            if( light_nodes[j]->instanceCameras() > 0 ) {
                light_projections[j] = m_database.library<Camera>().get( light_nodes[j]->instanceCameraURL(0) );
            }
        }
    }

    std::list<const Node*> camera_path;
    std::list<const Node*> light_paths[SCENE_LIGHTS_MAX];
    if( visual_scene_node != NULL ) {
        if( camera_node != NULL ) {
            findNodePath( camera_path, visual_scene_node, camera_node );
        }
        for( size_t j=0; j<SCENE_LIGHTS_MAX; j++ ) {
            if( light_nodes[j] != NULL ) {
                findNodePath( light_paths[j], visual_scene_node, light_nodes[j] );
            }
        }
    }

    RenderAction* action = RenderAction::createSetViewCoordSys( camera,
                                                        camera_path,
                                                        lights,
                                                        light_projections,
                                                        light_paths );
    m_views.push_back( action );
    return action;
}


const RenderAction*
Resolver::setLocalCoordSys( const std::list<const Node*>&  node_path )
{
    RenderAction* action = RenderAction::createSetLocalCoordSys( node_path );
    m_set_local.push_back( action );
    return action;
}


const NodePath*
Resolver::nodePath( VisualScene* visual_scene, Node* instancer, Node* node )
{

    NodePath::Id id( visual_scene, instancer, node );

    NodePath* np = NULL;
    auto it = m_nodepath_cache.find( id );
    if( it != m_nodepath_cache.end() ) {
        if( it->second->timeStamp().asRecentAs( m_database.structureChanged() ) ) {
        //if( !m_database.asset().majorChanges(it->second->timeStamp() ) ) {
            return it->second;
        }
        else {
            np = it->second;
        }
    }
    else {
    }


    return NULL;
}


Resolver::Resolver( const Scene::DataBase& database, ProfileType profile, const std::string& platform )
: m_database( database ),
  m_profile( profile ),
  m_platform( platform ),
  m_def_framebuffer( NULL ),
  m_def_raster( NULL ),
  m_def_pixel_ops( NULL ),
  m_def_fb_ctrl( NULL )
{
    clear();
}

void
Resolver::clear()
{


    // Note: The cached items that also has a default value (state objects), may
    // have the default be represented multiple times inside the cache. Thus,
    // we must be a bit careful so that is deleted only once.
    for(auto it=m_set_framebuffer_cache.begin(); it!=m_set_framebuffer_cache.end(); ++it ) {
        if( it->second != m_def_framebuffer ) {
            delete it->second;
        }
    }
    m_set_framebuffer_cache.clear();
    if( m_def_framebuffer != NULL ) {
        delete m_def_framebuffer;
        m_def_framebuffer = NULL;
    }


    for(auto it=m_set_raster_cache.begin(); it!=m_set_raster_cache.end(); ++it ) {
        if( it->second != m_def_raster ) {
            delete it->second;
        }
    }
    m_set_raster_cache.clear();
    if( m_def_raster != NULL ) {
        delete m_def_raster;
        m_def_raster = NULL;
    }

    for(auto it=m_set_pixel_ops_cache.begin(); it!=m_set_pixel_ops_cache.end(); ++it) {
        if( it->second != m_def_pixel_ops ) {
            delete it->second;
        }
    }
    m_set_pixel_ops_cache.clear();
    if( m_def_pixel_ops != NULL ) {
        delete m_def_pixel_ops;
        m_def_pixel_ops = NULL;
    }


    for( auto it=m_set_fb_ctrl_cache.begin(); it!=m_set_fb_ctrl_cache.end(); ++it ) {
        if( it->second != m_def_fb_ctrl ) {
            delete it->second;
        }
    }
    m_set_fb_ctrl_cache.clear();
    if( m_def_fb_ctrl != NULL ) {
        delete m_def_fb_ctrl;
        m_def_fb_ctrl = NULL;
    }

    // The rest should be unique

    for_each( m_nodepath_cache.begin(),
              m_nodepath_cache.end(),
             []( std::pair<const NodePath::Id,NodePath*> a){ delete a.second; } );
    m_nodepath_cache.clear();


    for_each( m_set_pass_cache.begin(),
              m_set_pass_cache.end(),
             []( std::pair<const string,RenderAction*> a){ delete a.second; } );
    m_set_pass_cache.clear();

    for_each( m_set_inputs_cache.begin(),
              m_set_inputs_cache.end(),
             []( std::pair<const string,RenderAction*> a){ delete a.second; } );
    m_set_inputs_cache.clear();

    for_each( m_set_uniforms_cache.begin(),
              m_set_uniforms_cache.end(),
             []( std::pair<const string,RenderAction*> a){ delete a.second; } );
    m_set_uniforms_cache.clear();

    for_each( m_set_samplers_cache.begin(),
              m_set_samplers_cache.end(),
             []( std::pair<const string,RenderAction*> a){ delete a.second; } );
    m_set_samplers_cache.clear();

    for_each( m_draw_cache.begin(),
              m_draw_cache.end(),
             []( std::pair<const string,RenderAction*> a){ delete a.second; } );
    m_draw_cache.clear();

    for_each( m_resolved_params_cache.begin(),
              m_resolved_params_cache.end(),
             []( std::pair<const string,ResolvedParams*> a){ delete a.second; } );
    m_resolved_params_cache.clear();



    for_each( m_views.begin(), m_views.end(), [](RenderAction* a){ delete a; } );
    m_views.clear();

    for_each( m_set_local.begin(), m_set_local.end(), [](RenderAction* a){ delete a; } );
    m_set_local.clear();

    m_layer_masks.clear();
    m_layer_mask_render.clear();
    m_layer_mask_node.clear();
}

void
Resolver::purge()
{
    for_each( m_views.begin(), m_views.end(), [](RenderAction* a){ delete a; } );
    m_views.clear();

    for_each( m_set_local.begin(), m_set_local.end(), [](RenderAction* a){ delete a; } );
    m_set_local.clear();
}

Resolver::~Resolver()
{
    clear();
}


bool
Resolver::findNodePath( std::list<const Node*>&  path,
                        const Node*              source_node,
                        const Node*              target_node )
{
    Logger log = getLogger( package + ".findNodePath" );


    path.clear();
    path.push_back( source_node );
    if( !findNodePathRecurse( path, source_node, target_node ) ) {
        path.pop_back();
        SCENELOG_ASSERT( log, path.size() == 0 );
        return false;
    }
    else {
        SCENELOG_ASSERT( log, path.size() % 2 == 0 );
        return true;
    }
}

bool
Resolver::findNodePathRecurse( std::list<const Node*>&  path,
                               const Node*              current_node,
                               const Node*              target_node )
{
    Logger log = getLogger( package + ".findNodePathRecurse" );

    SCENELOG_ASSERT( log, path.size() % 2 == 1 );

    // Check this
    if( current_node == target_node ) {
        path.push_back( current_node );
        return true;
    }

    // Search children
    for( size_t i=0; i<current_node->children(); i++ ) {
        if( findNodePathRecurse( path, current_node->child(i), target_node ) ) {
            return true;
        }
    }

    // Try instancing
    path.push_back( current_node );
    for( size_t i=0; i<current_node->instanceNodes(); i++ ) {
        const Node* instancee_node = m_database.library<Node>().get( current_node->instanceNode(i) );
        if( instancee_node != NULL ) {
            path.push_back( instancee_node );
            if( findNodePathRecurse( path, instancee_node, target_node ) ) {
                return true;
            }
            path.pop_back();
        }
    }
    path.pop_back();
    return false;
}


const RenderAction*
Resolver::setRaster( const Pass* pass )
{
    if( pass == NULL ) {
        if( m_def_raster == NULL ) {
            m_def_raster = RenderAction::createSetRaster( "default", NULL, NULL );
        }
        return m_def_raster;
    }
    const string id = pass->key();
    auto it = m_set_raster_cache.find( id );
    if( it != m_set_raster_cache.end() ) {
        return it->second;
    }
    RenderAction* action = RenderAction::createSetRaster( id, NULL, pass );
    if( action == NULL ) {
        if( m_def_raster == NULL ) {
            m_def_raster = RenderAction::createSetRaster( "default", NULL, NULL );
        }
        action = m_def_raster;
    }
    m_set_raster_cache[ id ] = action;
    return action;
}

const RenderAction*
Resolver::setRenderTarget( const std::vector<Bind>&  bind,
                           const Material*           material,
                           const Pass*               pass )
{
    Logger log = getLogger( package + ".setRenderTarget" );

    if( material == NULL || pass == NULL ) {
        if( m_def_framebuffer == NULL ) {
            m_def_framebuffer = RenderAction::createSetRenderTarget( m_database, "default", NULL, NULL );
        }
        return m_def_framebuffer;
    }

    const ResolvedParams* params = resolveParams( bind, material, pass );
    if( params == NULL ) {
        SCENELOG_FATAL( log, "params==NULL @" << __LINE__ );
        return NULL;
    }
    const string id = params->m_id;

    auto it = m_set_framebuffer_cache.find( id );
    if( it != m_set_framebuffer_cache.end() ) {
        return it->second;
    }

    RenderAction* action = RenderAction::createSetRenderTarget( m_database,
                                                                id,
                                                                params,
                                                                pass );
    if( action != NULL && action->m_set_render_targets.m_items.empty() ) {
        delete action;
        if( m_def_framebuffer == NULL ) {
            m_def_framebuffer = RenderAction::createSetRenderTarget( m_database, "default", NULL, NULL );
        }
        action = m_def_framebuffer;
    }

    m_set_framebuffer_cache[ id ] = action;
    return action;
}


const RenderAction*
Resolver::setPixelOps( const Pass* pass )
{
    if( pass == NULL ) {
        if( m_def_pixel_ops == NULL ) {
            m_def_pixel_ops = RenderAction::createSetPixelOps( "default", NULL, NULL );
        }
        return m_def_pixel_ops;
    }
    const string id = pass->key();
    auto it = m_set_pixel_ops_cache.find( id );
    if( it != m_set_pixel_ops_cache.end() ) {
        return it->second;
    }
    RenderAction* action = RenderAction::createSetPixelOps( id, NULL, pass );
    if( action == NULL ) {
        // No changes from default
        if( m_def_pixel_ops == NULL ) {
            std::vector<Bind> bind;
            m_def_pixel_ops = RenderAction::createSetPixelOps( "default", NULL, NULL );
        }
        action = m_def_pixel_ops;
    }
    m_set_pixel_ops_cache[ id ] = action;
    return action;
}

const RenderAction*
Resolver::setFBCtrl( const Pass* pass )
{
    if( pass == NULL ) {
        if( m_def_fb_ctrl == NULL ) {
            std::vector<Bind> bind;
            m_def_fb_ctrl = RenderAction::createSetFBCtrl( "default", NULL, NULL );
        }
        return m_def_fb_ctrl;
    }
    const string id = pass->key();
    auto it = m_set_fb_ctrl_cache.find( id );
    if( it != m_set_fb_ctrl_cache.end() ) {
        return it->second;
    }
    RenderAction* action = RenderAction::createSetFBCtrl( id, NULL, pass );
    if( action == NULL ) {
        if( m_def_fb_ctrl == NULL ) {
            std::vector<Bind> bind;
            m_def_fb_ctrl = RenderAction::createSetFBCtrl( "default", NULL, NULL );
        }
        action = m_def_fb_ctrl;
    }
    m_set_fb_ctrl_cache[ id ] = action;
    return action;
}

const RenderAction*
Resolver::setPass( const Pass* pass )
{
    Logger log = getLogger( package + ".setPass" );

    const string id = pass->key();

    auto it = m_set_pass_cache.find( id );
    if( it != m_set_pass_cache.end() ) {
        if( 1 ) {
            return it->second;
        }
        delete it->second;
        m_set_pass_cache.erase( it );
    }

    SCENELOG_TRACE( log, "Creating id=" <<id );
    RenderAction* action = RenderAction::createSetPass( id, pass );
    m_set_pass_cache[ action->m_id ] = action;
    return action;
}

const RenderAction*
Resolver::setInputs( const Pass*      pass,
                           const Geometry*  geometry,
                           const Primitives*  primitives )
{
    Logger log = getLogger( package + ".resolveInputs" );

    const string id = pass->key() + "@" + primitives->key();

    auto it = m_set_inputs_cache.find( id );
    if( it != m_set_inputs_cache.end() ) {
        // Todo: check timestamps
        if( 1 ) {
            return it->second;
        }

        delete it->second;
        m_set_inputs_cache.erase( it );
    }

    SCENELOG_TRACE( log, "Creating id=" << id );
    RenderAction* action = RenderAction::createSetInputs( m_database,
                                                          id,
                                                          pass,
                                                          geometry,
                                                          primitives );
    if( action == NULL ) {
        SCENELOG_FATAL( log, "action==NULL @" << __LINE__ );
        return NULL;
    }
    m_set_inputs_cache[ action->m_id ] = action;
    return action;
}


ResolvedParams*
Resolver::resolveParams( const std::vector<Bind>&  bind,
                         const Material*  material,
                         const Pass*      pass )
{
    Logger log = getLogger( package + ".resolveParams" );

    if( !bind.empty() ) {
        SCENELOG_WARN( log, "non-empty bind currently ignored..." );
    }

    const Technique* technique = pass->technique();
    const Profile* profile = technique->profile();
    const Effect* effect = profile->effect();

    const string id = pass->key() + "@" + material->id();

    auto it = m_resolved_params_cache.find( id );
    if( it != m_resolved_params_cache.end() ) {
        // Make sure that pointers are the same
        if( (material  == it->second->m_material) &&
            (effect    == it->second->m_effect ) &&
            (profile   == it->second->m_profile ) &&
            (technique == it->second->m_technique ) &&
            (pass      == it->second->m_pass ) )
        {
            SCENELOG_TRACE( log, "resolve_timestamp=" << it->second->m_timestamp.debugString() );

            // Make sure that no significant changes have been done
            if( it->second->m_timestamp.asRecentAs( material->structureChanged() ) &&
                it->second->m_timestamp.asRecentAs( effect->structureChanged() ) &&
                it->second->m_timestamp.asRecentAs( profile->structureChanged() ) )
            {
            //if( !material->asset().majorChanges( it->second->m_timestamp ) &&
            //    !profile->asset().majorChanges( it->second->m_timestamp ) &&
            //    !profile->asset().majorChanges( it->second->m_timestamp ) )
            //{
                // We can use the cached version
                SCENELOG_TRACE( log, "Found existing, timestamp=" << it->second->m_timestamp.debugString() );
                return it->second;
            }
            else {
                SCENELOG_TRACE( log, "outdated" );
            }

        }
        else {
            SCENELOG_TRACE( log, "pointers do not match." );
        }

        // Something has changed, do resolve again
        delete it->second;
        m_resolved_params_cache.erase( it );
    }

    ResolvedParams* params = new ResolvedParams;
    params->m_id = id;
    params->m_timestamp.touch();
    params->m_material = material;
    params->m_effect = effect;
    params->m_profile = profile;
    params->m_technique = technique;
    params->m_pass = pass;

    // Extract effect parameters
    for( size_t i=0; i<effect->parameters(); i++) {
        const Parameter* p = effect->parameter( i );
        if( !p->value()->defined() ) {
            SCENELOG_FATAL( log, "Value for parameter sid='" << p->sid() <<
                            "' is undefined." );
            return NULL;
        }

        ResolvedParams::Item item;
        item.m_semantic = p->semantic();
        item.m_value = p->value();
        params->m_map[ p->sid() ] = item;
    }

    // Extract profile parameters
    for( size_t i=0; i<profile->parameters(); i++) {
        const Parameter* p = profile->parameter( i );
        if( !p->value()->defined() ) {
            SCENELOG_FATAL( log, "Value for parameter sid='" << p->sid() <<
                            "' is undefined." );
            return NULL;
        }
        auto it = params->m_map.find( p->sid() );
        if( it != params->m_map.end() ) {
            SCENELOG_WARN( log, "Profile parameter sid='" << p->sid() <<
                           " overrides effect parameter with same sid." );
        }
        ResolvedParams::Item item;
        item.m_semantic = p->semantic();
        item.m_value = p->value();
        params->m_map[ p->sid() ] = item;
    }

    // Change parameters that are updated by the material
    for( size_t i=0; i<material->setParams(); i++ ) {
        const string& reference = material->setParamReference( i );

        auto it = params->m_map.find( reference );
        if( it != params->m_map.end() ) {
            const Value* value = material->setParamValue( i );
            if( value->type() != it->second.m_value->type() ) {
                SCENELOG_WARN( log, "Material setparam ref='" << reference <<
                               "' has wrong type, ignoring (id='" << id << "')." );
            }
            else {
                it->second.m_value = value;
            }
        }
        else {
            SCENELOG_WARN( log, "unable to find parameter ref='" << reference <<
                           "', ignoring (id='" << id << "')." );
        }
    }

    m_resolved_params_cache[ params->m_id ] = params;

    SCENELOG_TRACE( log, "Created new, timestamp=" << params->m_timestamp.debugString() );

    return params;
}

const RenderAction*
Resolver::setSamplers( const ResolvedParams* params )
{
    if( params == NULL ) {
        return NULL;
    }
    Logger log = getLogger( package + ".setSamplers" );
    const string id = params->m_id;
    //SCENELOG_DEBUG( log, params->m_timestamp.string() );

    auto it = m_set_samplers_cache.find( id );
    if( it != m_set_samplers_cache.end() ) {
        if( it->second->m_timestamp.asRecentAs( params->m_timestamp ) ) {
        //if( it->second->m_timestamp.asFreshAs( params->m_timestamp ) ) {
            return it->second;
        }
        SCENELOG_TRACE( log,
                       "params_timestamp=" << params->m_timestamp.debugString() <<
                       ", set_samplers_timestamp=" << it->second->m_timestamp.debugString() );
        delete it->second;
        m_set_uniforms_cache.erase( it );
    }

    RenderAction* action = RenderAction::createSetSamplers( m_database,
                                                            id,
                                                            params,
                                                            params->m_pass );
    if( action != NULL ) {
        m_set_samplers_cache[ id ] = action;
    }
    return action;
}


const RenderAction*
Resolver::setUniforms( const RenderAction*               set_samplers,
                       const std::vector<Bind>&          bind,
                       const Material*                   material,
                       const Pass*                       pass )
{
    Logger log = getLogger( package + ".setUniforms" );

    if( !bind.empty() ) {
        SCENELOG_WARN( log, "Non-empty bind-list, ignored!" );
    }

    const ResolvedParams* params = resolveParams( bind, material, pass );
    if( params == NULL ) {
        SCENELOG_FATAL( log, "params==NULL @" << __LINE__ );
        return NULL;
    }

    const string id = params->m_id;

    auto it = m_set_uniforms_cache.find( id );
    if( it != m_set_uniforms_cache.end() ) {
        if( it->second->m_timestamp.asRecentAs( params->m_timestamp ) ) {
            return it->second;
        }
        SCENELOG_DEBUG( log, "Cached version out of date (id='" << id << "')" );
        delete it->second;
        m_set_uniforms_cache.erase( it );
    }

    SCENELOG_TRACE( log, "Creating id=" <<id );
    RenderAction* action = RenderAction::createSetUniforms( id,
                                                            set_samplers,
                                                            params,
                                                            pass );
    m_set_uniforms_cache[ action->m_id ] = action;
    return action;
}

const RenderAction*
Resolver::draw( const Geometry*    geometry,
                const Primitives*  primitives,
                const Pass*        pass )
{
    if( (geometry == NULL) || (primitives==NULL) || (pass==NULL) ) {
        return NULL;
    }

    const string id = primitives->key() + pass->key();

    auto it = m_draw_cache.find( id );
    if( it != m_draw_cache.end() ) {
        RenderAction* cached = it->second;
        if( cached->m_timestamp.asRecentAs( geometry->structureChanged() ) ) {
//        if( !geometry->asset().majorChanges( cached->m_timestamp ) ) {
//        if( cached->m_timestamp.asFreshAs( geometry->asset(). timeStamp() ) ) {
            return cached;
        }
        delete cached;
        m_draw_cache.erase( it );
    }

    RenderAction* action;
    if( primitives->indexBufferId().empty() ) {
        action = RenderAction::createDraw( id, geometry, primitives, pass );
    }
    else {
        action = RenderAction::createDrawIndexed( m_database, id, geometry, primitives, pass );
    }
    m_draw_cache[ action->m_id ] = action;
    return action;
}



    } // of namespace Runtime
} // of namespace Scene
