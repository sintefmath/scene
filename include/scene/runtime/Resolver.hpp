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

#pragma once

#include <list>
#include <string>
#include <unordered_map>

#include <scene/SeqPos.hpp>
#include "scene/Bind.hpp"
#include "scene/Scene.hpp"
#include "scene/Geometry.hpp"
#include "scene/VisualScene.hpp"
#include "scene/DataBase.hpp"
#include "scene/runtime/RenderAction.hpp"
#include "scene/runtime/CacheKey.hpp"

namespace Scene {
    namespace Runtime {


    typedef size_t LayerMask;

    /**
      * See documentation in Scene::VisualScene for info on allowed node tree
      * constructs.
      *
      *
      *
      *
      *
      *
      *
      */
    class NodePath
    {
    public:
        typedef CacheKey<3> Id;
        typedef std::unordered_map< Id, NodePath*, Id > Map;

        const SeqPos&
        timeStamp() const { return m_timestamp; }

    protected:

        SeqPos                     m_timestamp;
        VisualScene*                  m_visual_scene;
        Node*                         m_instancer;
        Node*                         m_node;
    };

    struct ResolvedParams
    {
        struct Item {
            RuntimeSemantic  m_semantic;
            const Value*     m_value;
        };
        std::string                          m_id;
        SeqPos                            m_timestamp;
        const Material*                      m_material;
        const Effect*                        m_effect;
        const Profile*                       m_profile;
        const Technique*                     m_technique;
        const Pass*                          m_pass;
        std::unordered_map<std::string,Item> m_map;
    };


    class Resolver
    {
    public:
        Resolver( const Scene::DataBase& database, ProfileType profile, const std::string& platform=""  );

        ~Resolver();

        void
        purge();


        /** Clears all cached data.
          *
          * \note Also clear structures that uses the cached data (RenderList)
          */
        void
        clear();

        /** Get the layer mask corresponding to a single layer.
          *
          * \param[in] layer  The name of the layer.
          * \returns A mask with the bit of the specified layer set.
          */
        const LayerMask
        layerMask( const std::string& layer );

        /** Get the layer mask for nodes to process in a render item.
          *
          * \param[in] render  The render item.
          * \returns A mask with the bits of the layers to process set.
          */
        const LayerMask
        layerMask( const Render* render );

        /** Get the layer mask for which layers a node belongs to.
          *
          * \param[in] node  The node
          * \returns A mask with the bits of the layers that the node belongs to
          * set.
          */
        const LayerMask
        layerMask( const Node* node );

        const RenderAction*
        setViewCoordSys( const Node*    visual_scene_node,
                         const Render*  render );

        const RenderAction*
        setLocalCoordSys( const std::list<const Node*>&  node_path );

        const NodePath*
        nodePath( VisualScene* visual_scene, Node* instancer, Node* node );

        const RenderAction*
        setRaster( const Pass* );

        const RenderAction*
        setPixelOps( const Pass* );

        const RenderAction*
        setFBCtrl( const Pass* );

        const RenderAction*
        setPass( const Pass* );

        const RenderAction*
        setInputs( const Pass*      pass,
                   const Geometry*  geometry,
                   const Primitives*  primitives );

        const RenderAction*
        setSamplers( const ResolvedParams* params );


        const RenderAction*
        setUniforms( const RenderAction*               set_samplers,
                     const std::vector<Bind>&          bind,
                     const Material*                   material,
                     const Pass*                       pass );

        const RenderAction*
        setRenderTarget( const std::vector<Bind>&  bind,
                         const Material*           material,
                         const Pass*               pass );

        const RenderAction*
        draw( const Geometry*    geometry,
              const Primitives*  primitives,
              const Pass*        pass );


        ResolvedParams*
        resolveParams( const std::vector<Bind>&  bind,
                       const Material*           material,
                       const Pass*               pass );


        /** Find the path from a given source node to a target node.
          *
          * Used to find the path to the camera node from the current visual
          * scene root node.
          *
          * Due to node instancing, there can be multiple possible paths, as
          * well as multiple instancing steps as well as a risk for cycles.
          *
          * This method recursively traverses the full scene graph, recording
          * which nodes that has been visited (to detect cycles), searching
          * for the target node.
          *
          * \param[out] The resulting node path, where pairs of nodes represent
          *             direct node to parent traversals (i.e., when node
          *             instancing is not used, this path consists of two nodes,
          *             the root node and the current node.
          * \param[in]  The source node where to start the search.
          * \param[in]  The target node to find.
          * \returns    True if target is a direct or indirect child of source.
          */
        bool
        findNodePath( std::list<const Node*>&  path,
                      const Node*              source_node,
                      const Node*              target_node );


        // TODO: protected
        bool
        findNodePathRecurse( std::list<const Node*>&  path,
                             const Node*              current_node,
                             const Node*              target_node );


        const DataBase&
        database() const { return m_database; }

        const ProfileType
        profile() const { return m_profile; }

        const std::string&
        platform() const { return m_platform; }


    protected:
        const Scene::DataBase&                           m_database;
        const ProfileType                                m_profile;
        const std::string                                m_platform;
        RenderAction*                                    m_def_framebuffer;
        RenderAction*                                    m_def_raster;
        RenderAction*                                    m_def_pixel_ops;
        RenderAction*                                    m_def_fb_ctrl;
        NodePath::Map                                    m_nodepath_cache;

        std::unordered_map<std::string,RenderAction*>    m_set_framebuffer_cache;
        std::unordered_map<std::string,RenderAction*>    m_set_raster_cache;
        std::unordered_map<std::string,RenderAction*>    m_set_pixel_ops_cache;
        std::unordered_map<std::string,RenderAction*>    m_set_fb_ctrl_cache;
        std::unordered_map<std::string,RenderAction*>    m_set_pass_cache;
        std::unordered_map<std::string,RenderAction*>    m_set_inputs_cache;
        std::unordered_map<std::string,RenderAction*>    m_set_uniforms_cache;
        std::unordered_map<std::string,RenderAction*>    m_set_samplers_cache;
        std::unordered_map<std::string,RenderAction*>    m_draw_cache;
        std::unordered_map<std::string,ResolvedParams*>  m_resolved_params_cache;

        struct CachedLayerMask
        {
            SeqPos                                    m_timestamp;
            LayerMask                                    m_mask;
        };

        std::unordered_map<std::string, LayerMask>       m_layer_masks;
        std::unordered_map<CacheKey<1>, CachedLayerMask> m_layer_mask_render;
        std::unordered_map<CacheKey<1>, CachedLayerMask> m_layer_mask_node;


        std::list<RenderAction*>                         m_views;
        std::list<RenderAction*>                         m_set_local;

    };



    } // of namespace Runtime
} // of namespace Scene
