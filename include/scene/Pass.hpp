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
#include <unordered_map>
#include "scene/Value.hpp"
#include "scene/Scene.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {


class Pass : public StructureValueSequences
{
    friend class Technique;
public:

    const Technique*
    technique() const { return m_technique; }

    ~Pass();

    const std::string&
    sid() const { return m_sid; }

    const std::string
    key() const;

    const std::string&
    shaderSource( const ShaderStage stage ) const;

    void
    setShaderSource( const ShaderStage stage, const std::string& source );


    /** Get number of specified vertex attributes. */
    const size_t
    attributes() const { return m_attributes.size(); }

    /** Get the semantic value of a given vertex attribute index. */
    const VertexSemantic
    attributeSemantic( const size_t ix ) const;

    /** Get the shader symbol of a given vertex attribute index. */
    const std::string&
    attributeSymbol( const size_t ix ) const;

    /** Assign a shader symbol to a particular semantic value. */
    void
    setAttribute( const VertexSemantic semantic, const std::string& symbol );

    /** Remove all attribute bindings. */
    void
    clearAttributes();


    /** Returns the number of uniform bindings. */
    const size_t
    uniforms() const;

    /** Returns the uniform symbol of a given uniform binding. */
    const std::string&
    uniformSymbol( size_t ix ) const;

    /** Returns true if the given uniform is assigned to a parameter. */
    bool
    uniformIsParameterReference( size_t ix ) const;

    /** Returns the parameter reference of a given uniform binding. */
    const std::string&
    uniformParameterReference( size_t ix ) const;

    /** Returns the assigned value of a given uniform binding. */
    const Value*
    uniformValue( size_t ix ) const;

    /** Assign a reference to a parameter to a shader uniform symbol. */
    void
    setUniform( const std::string& symbol, const std::string param_ref );

    /** Assign a given value to a shader uniform symbol. */
    void
    setUniform( const std::string& symbol, const Value& value );

    /** Remove all uniform bindings. */
    void
    clearUniforms();


    void
    addState( const StateType  type,
              const Value&     value );

    const size_t
    states() const { return m_states.size(); }

    StateType
    stateType( size_t ix ) const;

    bool
    stateIsParameterReference( size_t ix ) const;

    const std::string&
    stateParameterReference( size_t ix ) const;

    const Value*
    stateValue( size_t ix ) const;

    void
    clearStates();


    /**
      *
      * Either image_ref or param_ref set, not both.
      *
      * \param index MRT index.
      * \param slice layer or cube face.
      * \param mip   mip-map level
      */
    void
    addRenderTarget( RenderTarget        target,
                     size_t              index,
                     const std::string&  image_ref,
                     const std::string&  param_ref,
                     size_t              slice = 0,
                     size_t              mip = 0 );

    void
    setRenderTargetClear( RenderTarget target,
                          size_t       index,
                          bool         clear );

    const size_t
    renderTargets() const { return m_render_targets.size(); }

    const std::string&
    renderTargetParameterReference( const size_t ix ) const { return m_render_targets[ix].m_param_ref; }

    const std::string&
    renderTargetImageReference( const size_t ix ) const { return m_render_targets[ix].m_image_ref; }

    const RenderTarget
    renderTargetTarget( const size_t ix ) const { return m_render_targets[ix].m_target; }

    const size_t
    renderTargetIndex( const size_t ix ) const { return m_render_targets[ix].m_index; }

    const size_t
    renderTargetFace( const size_t ix ) const { return m_render_targets[ix].m_slice; }

    const size_t
    renderTargetSlice( const size_t ix ) const { return m_render_targets[ix].m_slice; }

    const size_t
    renderTargetMipLevel( const size_t ix ) const { return m_render_targets[ix].m_mip; }

    const bool
    renderTargetClear( const RenderTarget target ) const { return true; }

    /** Specify what to render when this pass is invoked through instance_material.
      *
      * DRAW_GEOMETRY
      * DRAW_SCENE_GEOMETRY - Render all scene geometry using this effect.
      * DRAW_SCENE_IMAGE - Normal rendering, but use this pass's render targets.
      * DRAW_FULL_SCREEN_QUAD - Render this pass using a FSQ geoemtry.
      */
    Draw
    draw() const { return m_draw; }

    void
    setDraw( Draw draw );


    void
    setPrimitiveOverride( PrimitiveType  source_type, ///< Primitive type to override.
                          PrimitiveType  target_type, ///< Primitive type to override with.
                          unsigned int   vertices,    ///< Number of vertices per primitive (for patches).
                          unsigned int   count_num,   ///< Numerator for calculating new primitive count.
                          unsigned int   count_den    ///< Denominator for calculating new primtive count.
                          );

    const bool
    primitiveOverride( PrimitiveType source_type ) const;

    const PrimitiveType
    primitiveOverrideType( PrimitiveType source_type ) const;

    const unsigned int
    primitiveOverrideVertices( PrimitiveType source_type ) const;

    const unsigned int
    primitiveOverrideCountNum( PrimitiveType source_type ) const;

    const unsigned int
    primitiveOverrideCountDen( PrimitiveType source_type ) const;


protected:
    DataBase&                      m_db;
    Effect*                        m_effect;
    Profile*                       m_profile;
    Technique*                     m_technique;
    std::string                    m_sid;  // optional
    struct State {
        StateType                  m_type;
        std::string                m_param;
        int                        m_index;
        Value                      m_value;
    };
    std::vector<State*>            m_states;
    struct Attribute {
        VertexSemantic             m_semantic;
        std::string                m_symbol;
    };
    std::vector<Attribute>         m_attributes;
    struct Uniform {
        std::string                m_symbol;
        std::string                m_reference;
        Value*                     m_value;
    };
    std::vector<Uniform>           m_uniforms;
    struct RenderTargetItem {
        RenderTarget               m_target;
        size_t                     m_index;
        std::string                m_image_ref;
        std::string                m_param_ref;
        size_t                     m_slice;
        size_t                     m_mip;
        bool                       m_clear;
    };
    std::string                    m_shader_sources[STAGE_N];
    std::vector<RenderTargetItem>  m_render_targets;
    Draw                           m_draw;

    struct PrimitiveOverride {
        PrimitiveType               m_type;
        unsigned int                m_vertices;
        unsigned int                m_count_num;
        unsigned int                m_count_den;
    }                           m_primitive_override[ PRIMITIVE_N ];


    Pass( DataBase& db,
          Effect*    effect,
          Profile*   profile,
          Technique* technique,
          const std::string& sid="" );

};



}
