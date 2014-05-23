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

#include "scene/Value.hpp"
#include "scene/Scene.hpp"
#include "scene/Geometry.hpp"

namespace Scene {

class Primitives
{
    friend class Geometry;
public:

    Primitives( Geometry* geometry = NULL );

    const std::string
    key() const;

    const Geometry*
    geometry() const
    { return m_geometry; }

    const std::string&
    materialSymbol() const
    { return m_material_symbol; }

    void
    setMaterialSymbol( const std::string& symbol,
                       const bool taint=true );

    const PrimitiveType
    primitiveType() const { return m_primitive_type; }

    /** Return the number vertices in the set, i.e. number of primitives times number of vertices per primitive. */
    const size_t
    vertexCount() const { return m_primitive_count*m_vertices_per_primitive; }


    /** Return number of vertices per primitive. */
    const unsigned int
    verticesPerPrimitive() const { return m_vertices_per_primitive; }

    /** Return true if primitive is indexed. */
    const bool
    isIndexed() const { return !m_index_buffer_id.empty(); }

    /** Return id of buffer with indices. */
    const std::string&
    indexBufferId() const { return m_index_buffer_id; }

    const size_t
    indexOffset() const { return m_index_buffer_offset; }

    /** \name Shared inputs. */
    /** \{ */

    /** Returns true if primitive set has any shared inputs. */
    bool
    hasSharedInputs() const
    { return m_has_shared_inputs; }

    /** Number of indices in an index tuple. */
    const unsigned int
    sharedInputTupleSize() const { return m_index_tuple_width; }

    /** Returns true if shared input for a specific semantic is enabled. */
    bool
    sharedInputEnabled( VertexSemantic semantic ) const
    { return m_shared_inputs[semantic].m_enabled; }

    /** Returns the offset in the index tuple for a specific semantic. */
    unsigned int
    sharedInputTupleOffset( VertexSemantic semantic ) const
    { return m_shared_inputs[semantic].m_tuple_offset; }

    /** Returns the name of the source buffer that stores the data for a specific semantic. */
    const std::string&
    sharedInputSourceBuffer( VertexSemantic semantic ) const
    { return m_shared_inputs[semantic].m_source_buffer_id; }

    /** Returns the number of components for a specific semantic. */
    unsigned int
    sharedInputComponents( VertexSemantic semantic ) const
    { return m_shared_inputs[semantic].m_components; }

    /** Number of elements defined for a specific semantic. */
    unsigned int
    sharedInputCount( VertexSemantic semantic ) const
    { return m_shared_inputs[semantic].m_primitive_count; }

    /** Offset (in single values) into data buffer for a specific semantic. */
    unsigned int
    sharedInputOffset( VertexSemantic semantic ) const
    { return m_shared_inputs[semantic].m_offset; }

    /** Stride (in single values) between elements in data buffer for a specific semantic. */
    unsigned int
    sharedInputStride( VertexSemantic semantic ) const
    { return m_shared_inputs[semantic].m_stride; }

    void
    setSharedVertexSource( const unsigned int   index_offset,
                           VertexSemantic       semantic,
                           const std::string&   source_buffer_id,
                           const unsigned int   components,
                           const unsigned int   count,
                           const unsigned int   stride,
                           const unsigned int   offset );

    void
    clearSharedInputs();

    /** \} */

    void
    set( const PrimitiveType    type,
         const unsigned int     primitive_count,
         const unsigned int     vertices_per_primitive,
         const bool             taint = true );

    void
    set( const PrimitiveType    type,
         const unsigned int     primitive_count,
         const unsigned int     vertices_per_primitive,
         const std::string&     index_buffer_id,
         const unsigned int     index_buffer_offset,
         const bool             taint = true );


protected:
    Geometry*                       m_geometry;
    bool                            m_has_shared_inputs;
    struct {
        bool                            m_enabled;
        unsigned int                    m_tuple_offset;
        std::string                     m_source_buffer_id;
        unsigned int                    m_components;
        unsigned int                    m_primitive_count;
        unsigned int                    m_offset;
        unsigned int                    m_stride;
    }                               m_shared_inputs[ VERTEX_SEMANTIC_N ];

    PrimitiveType                   m_primitive_type;
    int                             m_primitive_count;
    int                             m_vertices_per_primitive; // required for patches
    std::string                     m_material_symbol;
    std::string                     m_index_buffer_id;
    size_t                          m_index_buffer_offset;
    unsigned int                    m_index_tuple_width;


};


} // of namespace Scene
