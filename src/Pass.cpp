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

#include <string>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "scene/Scene.hpp"
#include "scene/Pass.hpp"
#include <scene/Profile.hpp>
#include "scene/DataBase.hpp"
#include "scene/Utils.hpp"
#include "scene/Log.hpp"

namespace Scene {
    using std::string;
    using std::vector;
    using std::runtime_error;
    using std::stringstream;

Pass::Pass( DataBase&  db,
            Effect*    effect,
            Profile*   profile,
            Technique* technique,
            const string& sid )
    : m_db( db ),
      m_effect( effect ),
      m_profile( profile ),
      m_technique( technique ),
      m_sid( sid ),
      m_draw( DRAW_GEOMETRY )
{
    for( unsigned int i=0; i<PRIMITIVE_N; i++ ) {
        m_primitive_override[i].m_type = PRIMITIVE_N;
    }
}

void
Pass::setPrimitiveOverride( PrimitiveType  source_type,
                            PrimitiveType  target_type,
                            unsigned int   vertices,
                            unsigned int   count_num,
                            unsigned int   count_den )
{
    Logger log = getLogger( "Scene.Pass.setPrimitiveOverride" );
    if( source_type >= PRIMITIVE_N ) {
        SCENELOG_ERROR( log, "Illegal source primitive type." );
        return;
    }
    m_primitive_override[ source_type ].m_type = target_type;
    m_primitive_override[ source_type ].m_vertices = vertices;
    m_primitive_override[ source_type ].m_count_num = count_num;
    m_primitive_override[ source_type ].m_count_den = count_den;
}

const bool
Pass::primitiveOverride( PrimitiveType source_type ) const
{
    Logger log = getLogger( "Scene.Pass.primitiveOverride" );
    if( source_type >= PRIMITIVE_N ) {
        SCENELOG_ERROR( log, "Illegal source primitive type." );
        return false;
    }
    return m_primitive_override[ source_type ].m_type != PRIMITIVE_N;
}

const PrimitiveType
Pass::primitiveOverrideType( PrimitiveType source_type ) const
{
    Logger log = getLogger( "Scene.Pass.primitiveOverrideType" );
    if( source_type >= PRIMITIVE_N ) {
        SCENELOG_ERROR( log, "Illegal source primitive type." );
        return PRIMITIVE_N;
    }
    return m_primitive_override[ source_type ].m_type;
}

const unsigned int
Pass::primitiveOverrideVertices( PrimitiveType source_type ) const
{
    Logger log = getLogger( "Scene.Pass.primitiveOverrideVertices" );
    if( source_type >= PRIMITIVE_N ) {
        SCENELOG_ERROR( log, "Illegal source primitive type." );
        return 0;
    }
    return m_primitive_override[ source_type ].m_vertices;
}

const unsigned int
Pass::primitiveOverrideCountNum( PrimitiveType source_type ) const
{
    Logger log = getLogger( "Scene.Pass.primitiveOverrideCountNum" );
    if( source_type >= PRIMITIVE_N ) {
        SCENELOG_ERROR( log, "Illegal source primitive type." );
        return 0;
    }
    return m_primitive_override[ source_type ].m_count_num;
}

const unsigned int
Pass::primitiveOverrideCountDen( PrimitiveType source_type ) const
{
    Logger log = getLogger( "Scene.Pass.primitiveOverrideCountDen" );
    if( source_type >= PRIMITIVE_N ) {
        SCENELOG_ERROR( log, "Illegal source primitive type." );
        return 0;
    }
    return m_primitive_override[ source_type ].m_count_den;
}


Pass::~Pass()
{
    for( auto s=m_states.begin(); s!=m_states.end(); ++s ) {
        delete *s;
    }
    m_states.clear();
}

const std::string&
Pass::shaderSource( const ShaderStage stage ) const
{
    return m_shader_sources[ stage ];
}

void
Pass::setShaderSource( const ShaderStage stage, const std::string& source )
{
    m_shader_sources[ stage ] = source;

    touchStructureChanged();
    m_technique->moveForward( *this );
    m_profile->moveForward( *this );
    m_effect->moveForward( *this );
    m_db.library<Effect>().moveForward( *this );
    m_db.moveForward( *this );
}

const VertexSemantic
Pass::attributeSemantic( const size_t ix ) const
{
    return m_attributes[ix].m_semantic;
}

const std::string&
Pass::attributeSymbol( const size_t ix ) const
{
    return m_attributes[ix].m_symbol;
}


void
Pass::setAttribute( const VertexSemantic semantic, const std::string& symbol )
{
    for(size_t i=0; i<m_attributes.size(); i++) {
        if( m_attributes[i].m_semantic == semantic ) {
            m_attributes[i].m_symbol = semantic;
            return;
        }
    }
    m_attributes.resize( m_attributes.size() + 1 );
    m_attributes.back().m_semantic = semantic;
    m_attributes.back().m_symbol = symbol;

    touchStructureChanged();
    m_technique->moveForward( *this );
    m_profile->moveForward( *this );
    m_effect->moveForward( *this );
    m_db.library<Effect>().moveForward( *this );
    m_db.moveForward( *this );
}

void
Pass::clearAttributes()
{
    m_attributes.clear();

    touchStructureChanged();
    m_technique->moveForward( *this );
    m_profile->moveForward( *this );
    m_effect->moveForward( *this );
    m_db.library<Effect>().moveForward( *this );
    m_db.moveForward( *this );
}

const size_t
Pass::uniforms() const
{
    return m_uniforms.size();
}

const std::string&
Pass::uniformSymbol( size_t ix ) const
{
    return m_uniforms[ix].m_symbol;
}

bool
Pass::uniformIsParameterReference( size_t ix ) const
{
    return m_uniforms[ix].m_value == NULL;
}

const std::string&
Pass::uniformParameterReference( size_t ix ) const
{
    return m_uniforms[ix].m_reference;
}

const Value*
Pass::uniformValue( size_t ix ) const
{
    return m_uniforms[ix].m_value;
}

void
Pass::setUniform( const std::string& symbol, const std::string param_ref )
{
    for(size_t i=0; i<m_uniforms.size(); i++) {
        if( m_uniforms[i].m_symbol == symbol ) {
            m_uniforms[i].m_reference = param_ref;
            if( m_uniforms[i].m_value != NULL ) {
                delete m_uniforms[i].m_value;
                DEADBEEF( m_uniforms[i].m_value );
            }
            return;
        }
    }
    m_uniforms.resize( m_uniforms.size()+1 );
    m_uniforms.back().m_symbol = symbol;
    m_uniforms.back().m_reference = param_ref;
    m_uniforms.back().m_value = NULL;

    touchStructureChanged();
    m_technique->moveForward( *this );
    m_profile->moveForward( *this );
    m_effect->moveForward( *this );
    m_db.library<Effect>().moveForward( *this );
    m_db.moveForward( *this );
}

void
Pass::setUniform( const std::string& symbol, const Value& value )
{
    for(size_t i=0; i<m_uniforms.size(); i++) {
        if( m_uniforms[i].m_symbol == symbol ) {
            m_uniforms[i].m_reference.clear();
            if( m_uniforms[i].m_value != NULL ) {
                *m_uniforms[i].m_value = value;
            }
            else {
                m_uniforms[i].m_value = new Value( value );
            }
            return;
        }
    }
    m_uniforms.resize( m_uniforms.size()+1 );
    m_uniforms.back().m_symbol = symbol;
    m_uniforms.back().m_reference.clear();
    m_uniforms.back().m_value = new Value( value );

    touchStructureChanged();
    m_technique->moveForward( *this );
    m_profile->moveForward( *this );
    m_effect->moveForward( *this );
    m_db.library<Effect>().moveForward( *this );
    m_db.moveForward( *this );
}

void
Pass::clearUniforms()
{
    for(size_t i=0; i<m_uniforms.size(); i++) {
        if( m_uniforms[i].m_value != NULL ) {
            delete m_uniforms[i].m_value;
        }
    }
    m_uniforms.clear();

    touchStructureChanged();
    m_technique->moveForward( *this );
    m_profile->moveForward( *this );
    m_effect->moveForward( *this );
    m_db.library<Effect>().moveForward( *this );
    m_db.moveForward( *this );
}


const std::string
Pass::key() const
{
    if( m_sid.empty() ) {
        return m_technique->key() + "/" +
                boost::lexical_cast<string>( this );
    }
    else {
        return m_technique->key() + "/" + m_sid;
    }
}


void
Pass::addState( const StateType  type,
                const Value&     value )
{
    State* state = new State;
    state->m_type = type;
    state->m_value = value;
    state->m_param = "";
    state->m_index = 0;
    m_states.push_back( state );

    touchStructureChanged();
    m_technique->moveForward( *this );
    m_profile->moveForward( *this );
    m_effect->moveForward( *this );
    m_db.library<Effect>().moveForward( *this );
    m_db.moveForward( *this );
}


StateType
Pass::stateType( size_t ix ) const
{
    return m_states[ix]->m_type;
}

const Value*
Pass::stateValue( size_t ix ) const
{
    return &(m_states[ix]->m_value);
}

bool
Pass::stateIsParameterReference( size_t ix ) const
{
    return !m_states[ix]->m_param.empty();
}

const std::string&
Pass::stateParameterReference( size_t ix ) const
{
    return m_states[ix]->m_param;
}

void
Pass::clearStates()
{
    for(size_t i=0; i<m_states.size(); i++) {
        delete m_states[i];
    }
    m_states.clear();

    touchStructureChanged();
    m_technique->moveForward( *this );
    m_profile->moveForward( *this );
    m_effect->moveForward( *this );
    m_db.library<Effect>().moveForward( *this );
    m_db.moveForward( *this );
}



void
Pass::addRenderTarget( RenderTarget        target,
                       size_t              index,
                       const std::string&  image_ref,
                       const std::string&  param_ref,
                       size_t              slice,
                       size_t              mip )
{
    Logger log = getLogger( "Scene.Pass.addRenderTarget" );
    if( image_ref.empty() && param_ref.empty() ) {
        SCENELOG_ERROR( log, "Neither image or param ref is set, ignoring." );
        return;
    }
    if( !image_ref.empty() && !param_ref.empty() ) {
        SCENELOG_ERROR( log, "Both image and param ref is set, ignoring." );
        return;
    }

    RenderTargetItem item;
    item.m_target = target;
    item.m_index = index;
    item.m_image_ref = image_ref;
    item.m_param_ref = param_ref;
    item.m_slice = slice;
    item.m_mip = mip;
    item.m_clear = false;
    for( size_t i=0; i<m_render_targets.size(); i++ ) {
        if( (m_render_targets[i].m_target == target ) &&
            (m_render_targets[i].m_index == index ) )
        {
            m_render_targets[i] = item;
            return;
        }
    }
    m_render_targets.push_back( item );

    touchStructureChanged();
    m_technique->moveForward( *this );
    m_profile->moveForward( *this );
    m_effect->moveForward( *this );
    m_db.library<Effect>().moveForward( *this );
    m_db.moveForward( *this );
}

void
Pass::setRenderTargetClear( RenderTarget target,
                            size_t       index,
                            bool         clear )
{
    Logger log = getLogger( "Scene.Pass.setRenderTargetClear" );
    for( size_t i=0; i<m_render_targets.size(); i++ ) {
        if( (m_render_targets[i].m_target == target ) &&
            (m_render_targets[i].m_index == index ) )
        {
            m_render_targets[i].m_clear = clear;
            touchStructureChanged();
            m_technique->moveForward( *this );
            m_profile->moveForward( *this );
            m_effect->moveForward( *this );
            m_db.library<Effect>().moveForward( *this );
            m_db.moveForward( *this );
            return;
        }
    }
    SCENELOG_ERROR( log, "Cannot set clear state on undefined render target." );
}

void
Pass::setDraw( Draw draw )
{
    m_draw = draw;
    touchStructureChanged();
    m_technique->moveForward( *this );
    m_profile->moveForward( *this );
    m_effect->moveForward( *this );
    m_db.library<Effect>().moveForward( *this );
    m_db.moveForward( *this );
}




}
