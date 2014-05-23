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
#include <unordered_map>
#include <scene/SeqPos.hpp>
#include "scene/Bind.hpp"
#include "scene/Scene.hpp"
#include "scene/Geometry.hpp"
#include "scene/VisualScene.hpp"
#include "scene/runtime/Resolver.hpp"
#include "scene/runtime/RenderAction.hpp"

namespace Scene {
    namespace Runtime {

/** Represents a general renderlist not tied to a specific runtime.
  *
  * A renderlist consists of a series of actions that alter the state, and is
  * assumed to be processed from front to back. The render list includes
  * operations that restores the state to default values.
  */
class RenderList
{
public:

    struct Item {
        // The render action pointers will be removed
        const RenderAction*               m_action_set_pass;
        const RenderAction*               m_action_set_input;
        const RenderAction*               m_action_set_uniform;
        const RenderAction*               m_action_set_samplers;
        const RenderAction*               m_action_set_framebuffer;

        const SetRenderTargets*         m_set_render_targets;
        const SetPass*                  m_set_pass;
        const SetInputs*                m_set_inputs;
        const SetUniforms*              m_set_uniforms;
        const SetSamplers*              m_set_samplers;
        const SetRaster*                m_set_raster;
        const SetPixelOps*              m_set_pixel_ops;
        const SetFBCtrl*                m_set_fb_ctrl;
        const SetLocalCoordSys*         m_set_local_coordsys;
        const SetViewCoordSys*          m_set_view_coordsys;
        const Draw*                     m_draw;
        const DrawIndexed*              m_draw_indexed;
    };


    RenderList( Resolver& resolver );

    const Resolver&
    resolver() const { return m_resolver; }


    /**  Build the render list if needed.
      *
      * Checks if the visual scene id match with the render list, and if so,
      * if the render list is as fresh as the database. If not, the render list
      * is completely rebuilt.
      *
      * \returns true If the render list has been rebuilt.
      */
    bool
    build( const std::string& visual_scene );



    /** Get the bounding box of the current visual scene.
      *
      * Returns the bounding box for the geometry in the node-hierarchy of the
      * current visual scene. The bounding box is in the coordinate system of
      * the root node of the visual scene.
      *
      * \returns True if the bounding box is defined.
      */
/*
    bool
    boundingBox( Value& min, Value& max );
*/
    /** Clears the content of the render list.
      *
      * \note Also clear whatever that uses the render list items (e.g.
      * GLSLRenderList etc.)
      */
    void
    clear();

    /** Get the number of items in the render list. */
    size_t
    size() const { return m_operations.size(); }

    size_t
    items() const { return m_items.size(); }

    const Item&
    item( size_t index ) const { return m_items[ index ]; }


    /** Get a particular item in the render list. */
    const RenderAction*
    operator[]( size_t index ) const { return m_operations[index]; }

protected:
    /** Resolver used to deduce references and pointers into actions. */
    Resolver&                         m_resolver;
    /** Name of the visual scene that this render list is an instance of. */
    std::string                       m_visual_scene;
    /** Timestamp for when the render list was built. */
    SeqPos                            m_list_created;
    /** Temp variables that holds current state when building the render list. */
    /** @{ */
    /** The current framebuffer. */
    const RenderAction*               m_set_framebuffer_current;
    /** The current pass. */
    const RenderAction*               m_set_pass_current;
    /** The current vertex attribute input. */
    const RenderAction*               m_set_input_current;
    /** The current set of uniforms. */
    const RenderAction*               m_set_uniform_current;
    /** The current set of samplers. */
    const RenderAction*               m_set_samplers_current;
    /** The current local coordinate system used to specify objects. */
    const RenderAction*               m_set_local_current;
    /** The current set of raster operations state. */
    const RenderAction*               m_set_raster_current;
    /** The current set of pixel operations state. */
    const RenderAction*               m_set_pixel_ops_current;
    /** The current set of framebuffer control state. */
    const RenderAction*               m_set_fb_ctrl_current;
    /** The current transform state. */
    const RenderAction*               m_set_transforms_current;
    /** @} */

    /** The actual render list, a sequence of operations. */
    std::vector<const RenderAction*>  m_operations;

    std::vector<Item>                   m_items;

    /** Purge the current render list and build it from scratch, */
    void
    rebuild( );

    /** Helper struct used when traversing the scene graph. */
    struct Context
    {
        const Camera*                    m_camera;
        const Node*                      m_node_path[SCENE_PATH_MAX];
        LayerMask                        m_layer_mask;

        std::list<const Node*>           m_camera_path_;
        std::list<const Node*>           m_node_path_;
        const Node*                      m_current_node;
    };

    /** Add the required operations on the render list to render an item.
      *
      * Gets the actions from the resolver and checks if they are different
      * from the current set of actions. If so, they are pushed onto the render
      * list and current actions are updated.
      */
    void
    addRenderItem( const Context&            context,
                   const Material*           render_target_material,
                   const Pass*               render_target_pass,
                   const Material*           material,
                   const Pass*               pass,
                   const CommonShadingModel* common,
                   const Geometry*           geometry,
                   const Primitives*         primitives );

    /** Debug helper class to dump the contents of the render list to the log. */
    void
    dumpRenderList() const;


    /** Given a material id, deduce the material, effect, technique etc to use.
      *
      * The profile and platform are passed to the material's techniqueHint
      * to get the appropriate technique.
      *
      * In the case of render/instance_material, the technique and pass might
      * be overridden. These values are optionally passed in override_tech and
      * override_pass.
      *
      * \param[in]  material_id    The name of the material to process.
      * \param[out] material       Pointer to the corresponding material to use.
      * \param[out] effect         Pointer to the corresponding effect to use.
      * \param[out] profile        Pointer to the corresponding profile to use.
      * \param[out] technique      Pointer to the corresponding technique to use.
      * \param[out] pass           Pointer to the corresponding pass to apply. If
      *                            all passes of the technique is to be applied,
      *                            NULL is returned.
      * \param[in]  override_tech  Use this technique instead of the default.
      * \param[in]  override_pass  Use this pass instead of the default set of
      *                            passes.
      */
    bool
    getMaterialChildren( const Material*&    material,
                         const Effect*&      effect,
                         const Profile*&     profile,
                         const Technique*&   technique,
                         const Pass*&        pass,
                         const std::string&  material_id,
                         const std::string&  override_tech = "",
                         const std::string&  override_pass = "" );

    void
    processNodeTransforms( const Node* parent,
                           const Node* current );

    /** Recursively process a scene graph node.
      *
      * First, if the list of layers in the context is non-empty, it is checked
      * if this node is member of any of those layers. If not, the recusion
      * is terminated. Otherwise, the node and all sub-nodes are included,
      * regardless of which layers the children belongs to. I.e. a the property
      * of belonging to a layer is inheritated down the node hierarchy.
      *
      * If the node has any instance_node items, these nodes are recursed into.
      *
      * If the node has any children, these nodes are recursed into.
      *
      * If the node has any instance_geometry items, the geometry is instanced
      * using this node's transforms.
      *
      *
      * \param[in] context                 The context to use and to copy for
      *                                    further recursions.
      * \param[in] render_target_material  Passed to instanceGeometry.
      * \param[in] render_target_pass      Passed to instanceGeometry.
      * \param[in] override_material       Passed to instanceGeometry.
      */
    void
    processNode( Context&         parent_context,
                 const Material*  render_target_material,
                 const Pass*      render_target_pass,
                 bool             override_material );


    /** Instantiate geometry in a given node context.
      *
      * \param[in] node                    The node that instantiated the geometry.
      * \param[in] context                 The context in which the node was
      *                                    processed (due to instancing of nodes,
      *                                    a node can be processed in multiple
      *                                    contexts).
      * \param[in] render_target_material  The material that instantiates an
      *                                    effect with the pass that has the
      *                                    specification of the render target.
      * \param[in] render_target_pass      The pass that has the specification
      *                                    of the render target. Must be a child
      *                                    of the effect that is instantiated by
      *                                    render_target_material.
      * \param[in] override_material       If true, apply the render target
      *                                    material instead of the material
      *                                    specified in the instance_geometry.
      *                                    Used when traversing instance_material.
      */
    void
    instanceGeometry( const Node*               node,
                      const Context&            context,
                      const Material*           render_target_material,
                      const Pass*               render_target_pass,
                      bool                      override_material );

    /** Invoked when render's material id is non-empty (instance_material pipeline).
      *
      * The instance material pipeline is used to render into a specific render
      * target, render all the geometry using a single material (shadow buffering),
      * or to render full-screen passes.
      *
      * What to do is dependent on the 'draw' attribute of the render_target_pass.
      *
      * If draw is DRAW_SCENE_GEOMETRY, the geometry of the visual scene is
      * rendered into the render target of the pass using that pass.
      *
      * If draw is DRAW_SCENE_IMAGE, the visual scene is rendered normally, but
      * the result is rendered into the render target of the pass.
      *
      * If draw is DRAW_FULL_SCREEN_QUAD, the visual scene is not rendered, but
      * a full-screen quad is rendered using that pass.
      */
    void
    processInstanceMaterialPass( const VisualScene*               visual_scene,
                                 Context&                         context,
                                 const std::vector<std::string>&  layers,
                                 const Material*                  material,
                                 const Pass*                      pass );
    void
    traverseVisualSceneNodeHierarchy( const VisualScene*             visual_scene,
                                      Context&                       parent_context,
                                      const Material*                rt_material,
                                      const Pass*                    rt_pass,
                                      bool                           use_rt_as_material );


    /** Process a render item of a visual scene.
     *
     * Entry point for processing a set of render operations of a visual scene.
     * If the material_id of the render item is empty, the node hierarchy is
     * processed normally, i.e., using the specified materials on the geometries,
     * rendering to the default buffer.
     *
     * If the render item's material id is non-empty, the instance_material
     * pipeline is used instead. If the pass is empty, all the passes of the
     * effect/technique instantitated by the material is applied, otherwise
     * only that single pass is processed. See processInstanceMaterialPass for
     * details on the instance_material pipeline.
     *
     * \param[in] visual_scene  The visual scene to process.
     * \param[in] render        The render item to process. This render item
     *                          must belong to the visual scene.
     */
    void
    processRender( const VisualScene*  visual_scene,
                   const Render*       render );

};



    } // of namespace Runtime
} // of namespace Scene

