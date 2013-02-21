#pragma once

#include <string>
#include <vector>
#include "scene/Asset.hpp"
#include "scene/Scene.hpp"
#include "scene/Primitives.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {



/** Groups source buffers into primitives
  *
  * Geometry combines the data of a set of source buffers into a vertex stream,
  * and defines primitives that use this vertex stream. See \ref geometry for
  * discussion.
  */
class Geometry : public StructureValueSequences
{
    friend class Library<Geometry>;

public:
    struct VertexInput
    {
        bool                           m_enabled;
        std::string                    m_source_buffer_id;
        unsigned int                   m_components;
        unsigned int                   m_count;
        unsigned int                   m_offset;
        unsigned int                   m_stride;
    };


    /** Returns true if any of the primitive sets has shared inputs. */
    bool
    hasSharedInputs() const;


    /** Flattens multi-index primitive sets to single index primitive sets.
      *
      * \returns true If successful, false otherwise.
      */
    bool
    flatten();

    /** Return the identity of this Geometry. */
    const std::string&
    id() const { return m_id; }

    const std::string&
    key() const { return m_id; }


    /** \name Un-shared vertex input specification.
      *
      * Shared input is specified in the individual primitive sets:
      *
      * - source buffer id: The id of the SourceBuffer that contains the data
      *   backing store.
      * - count: The number of input elements.
      * - components: The number of components in each input element.
      * - offset: Number of values to offset into the source buffer.
      * - stride: Number of values between the start of two adjacent elements.
      * - The type is derived from the type of the SourceBuffer.
      */
    /** \{ */

    /** clear all unshared inputs for this geometry. */
    bool
    unsharedInputEnabled( VertexSemantic semantic ) const;

    const std::string&
    unsharedInputSourceBufferId( VertexSemantic semantic ) const;

    const unsigned int
    unsharedInputCount( VertexSemantic semantic ) const;

    const unsigned int
    unsharedInputComponents( VertexSemantic semantic ) const;

    const unsigned int
    unsharedInputOffset( VertexSemantic semantic ) const;

    const unsigned int
    unsharedInputStride( VertexSemantic semantic ) const;


    void
    unsharedInputClearAll();


    /** Specify a source to the vertex stream.
      *
      * A vertex stream is a set of sources paired with semantics, and define
      * the input to the shader pipeline.
      *
      * \param[in] semantic          The semantic label of this source, implying
      *                              that the source is paired with the vertex
      *                              input of the shader pipeline with the
      *                              corresponding semantic.
      * \param[in] source_buffer_id  The id of the source buffer that provides
      *                              data for this source.
      * \param[in] components        The number of components of this source
      *                              (e.g. 3 for 3D positions).
      * \param[in] count             The number of elements of this source
      *                              (often # source buffer elements /
      *                              components).
      * \param[in] stride            The source buffer stride between source
      *                              elements. A stride of zero implies that
      *                              the source elements are tightly packed.
      * \param[in] offset            The source buffer offset of the first
      *                              source element.
      */
    void
    setVertexSource( VertexSemantic      semantic,
                     const std::string&  source_buffer_id,
                     int                 components,
                     int                 count,
                     int                 stride = 0u,
                     int                 offset = 0u );



    /** \} */

    Primitives*
    addPrimitiveSet( );

    void
    removePrimitiveSet( const Primitives* set );

    const VertexInput&
    vertexInput( VertexSemantic semantic ) const
    { return m_vertex_inputs[ semantic ]; }

    size_t
    primitiveSets() const { return m_primitive_sets.size(); }

    Primitives*
    primitives( size_t index ) { return m_primitive_sets[index]; }

    const Primitives*
    primitives( size_t index ) const { return m_primitive_sets[index]; }


    Asset&
    asset() { return m_asset; }

    const Asset&
    asset() const { return m_asset; }

    void
    setAsset( const Asset& asset ) { m_asset = asset; }


    DataBase&
    db() { return m_db; }

    /** Get the axis-aligned bounding box of the geometry, if set.
     *
     * \param[out] min The minimum corner of the bounding box.
     * \param[out] max The maximum corner of the bounding box.
     * \returns True if bounding box is set and non-empty.
     */
    bool
    boundingBox( const Value*& min, const Value*& max ) const;

    /** Returns the timestamp of when the bounding box last was updated. */
    const SeqPos&
    boundingBoxUpdated() const;

    /** Clear the geometry's bounding box and set it to empty. */
    void
    clearBoundingBox();

    /** Set the geometry's axis-aligned bounding box. */
    void
    setBoundingBox( const Value& bbmin, const Value& bbmax );

protected:
    Asset                              m_asset;
    VertexInput                        m_vertex_inputs[ VERTEX_SEMANTIC_N ];
    std::vector<Primitives*>           m_primitive_sets;
    DataBase&                          m_db;
    std::string                        m_id;
    bool                               m_bbox_nonempty;
    Value                              m_bbox_min;
    Value                              m_bbox_max;
    SeqPos                             m_bbox_timestamp;

    //Geometry( DataBase& db, const std::string& id );

    Geometry( Library<Geometry>* library_geometries, const std::string& id );

    ~Geometry();


};


} // of namespace Scene
