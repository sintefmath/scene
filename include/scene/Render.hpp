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

#include <vector>
#include <string>
#include "scene/Scene.hpp"

namespace Scene {


/** Describes one pass of rendering to be applied. */
class Render
{
public:
    Render( EvaluateScene* evaluate_scene );

    /** Return the evaluate scene item of which this render item belongs. */
    EvaluateScene*
    evaluateScene() { return m_evaluate_scene; }

    /** Return the evaluate scene item of which this render item belongs. */
    const EvaluateScene*
    evaluateScene() const { return m_evaluate_scene; }

    const std::string&
    sid() const;

    void
    setSid( const std::string& sid );

    const std::string&
    cameraNodeId() const;

    void
    setCameraNodeId( const std::string& id );

    size_t
    layers() const;

    const std::string&
    layer( size_t ix ) const;

    void
    addLayer( const std::string& layer );

    /** \name Instance material (rendering)
      *
      * This mechanism allows a material to be directly instanced, instead of
      * being instanced through a geometry. This is useful for image-level
      * processing.
      *
      * The instance material id denotes which material to instance.
      *
      * Technique override allows very specific usage of a material, bypassing
      * the normal path of processing the material's technique hint and
      * rendering all passes. Technique override is specified using
      * the sid of the technique to use, and optionally the sid of a specific
      * pass to use. If no pass-sid is specified, all passes are processed.
      *
      */
    /** @{ */

    const std::string&
    instanceMaterialId() const;

    void
    setInstanceMaterialId( const std::string& id );

    const std::string&
    instanceMaterialTechniqueOverrideSid() const;

    const std::string&
    instanceMaterialTechniqueOverridePassSid() const;

    void
    setInstanceMaterialTechniqueOverride( const std::string& technique_sid,
                                          const std::string& pass_sid = "" );
    /** \} */

    /** name Binding light nodes
      *
      * Specify the ordering of lights (wrt uniform semantics).
      *
      */
    /** @{ */
    const std::string&
    lightNodeId( size_t light_index ) const;

    void
    setLightNodeId( size_t light_index, const std::string& id );

    /** @} */

    /** A list of layers to render. If empty, we do all layers. */
    std::vector<std::string>               m_layers;

protected:
    /** The evaluate scene item of which this render item belongs. */
    EvaluateScene*                         m_evaluate_scene;

    std::string                            m_sid;

    /** Describes an optional camera node to be used for this pass. */
    std::string                            m_camera_node;



    /** If not empty, instantiate material pass instead of normal rendering. */
    std::string                            m_material_id;

    /** Material instancing; Which technique of the effect to use. If empty, use the default. */
    std::string                            m_tech_override_ref;

    /** Material instancing; Which pass of the technique to use. If empty, do all passes. */
    std::string                            m_tech_override_pass;

    /** The id of the nodes that contain the light sources to be used. */
    std::string                            m_light_nodes[ SCENE_LIGHTS_MAX ];

private:
    Render();

};


} // of namespace Scene
