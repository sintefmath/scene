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

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "scene/Geometry.hpp"
#include "scene/Primitives.hpp"
#include "scene/SourceBuffer.hpp"
#include "scene/DataBase.hpp"
#include "scene/Utils.hpp"
#include "scene/Log.hpp"

namespace Scene {
    using std::string;
    using std::unordered_map;
    using std::runtime_error;

static const string package = "Scene.Geometry";

//Geometry::Geometry( DataBase& db, const std::string& id )
//    : m_db( db ),
//      m_id( id ),
//      m_bbox_nonempty( false )
//{
//    unsharedInputClearAll();
//}

Geometry::Geometry( Library<Geometry>* library_geometries, const std::string& id )
    : m_db( *library_geometries->dataBase() ),
      m_id( id ),
      m_bbox_nonempty( false )
{
    unsharedInputClearAll();
//    for(size_t i=0; i<VERTEX_SEMANTIC_N; i++) {
//        m_vertex_inputs[i].m_enabled = false;
//    }
}

Geometry::~Geometry()
{
    for( auto it=m_primitive_sets.begin(); it!=m_primitive_sets.end(); ++it ) {
        delete *it;
    }
}


void
Geometry::unsharedInputClearAll()
{
    for(size_t i=0; i<VERTEX_SEMANTIC_N; i++) {
        m_vertex_inputs[i].m_enabled = false;
    }
}




bool
Geometry::hasSharedInputs() const
{
    bool retval = false;
    for( auto it=m_primitive_sets.begin(); it!=m_primitive_sets.end() && !retval; ++it ) {
        retval = retval | (*it)->hasSharedInputs();
    }
    return retval;
}

bool
Geometry::flatten()
{
    Logger log = getLogger( package + "." + m_id + ".flatten" );
    if( !hasSharedInputs() ) {
        SCENELOG_WARN( log, "No need to flatten a geometry without shared inputs." );
        return true;
    }

    struct Tuple {
        unsigned int    m_tuple[ VERTEX_SEMANTIC_N ];
        bool operator<(const Tuple& b) const {
            for( unsigned int i=0; i<VERTEX_SEMANTIC_N; i++ ) {
                if( m_tuple[i] < b.m_tuple[i] ) {
                    return true;
                }
                else if( m_tuple[i] > b.m_tuple[i] ) {
                    return false;
                }
            }
            return false;
        }

    };


//    VertexInput inputs[ VERTEX_SEMANTIC_N ] = m_vertex_inputs;
    VertexInput inputs[ VERTEX_SEMANTIC_N ];
    for (int i = 0; i < VERTEX_SEMANTIC_N; ++i) {
        inputs[i] = m_vertex_inputs[i];
    }

    std::map< Tuple, unsigned int> remap;

    std::vector< int > indices;
    std::vector< unsigned int > offsets;

    for( auto it=m_primitive_sets.begin(); it!=m_primitive_sets.end(); ++it ) {

        SCENELOG_DEBUG( log, "Processing primitive " );

        unsigned int tuple_offsets[ VERTEX_SEMANTIC_N ];
        for( unsigned int i=0; i<VERTEX_SEMANTIC_N; i++ ) {
            tuple_offsets[i] = ~0u;
        }

        // First, set tuple index for the common sources (defined in <vertex>).
        // If we have no shared inputs, this defaults to zero. Otherwise, we
        // grap the tuple offset from VERTEX_POSITION (which is an alias for
        // semantic=VERTEX within a shared input).
        unsigned int vertex_tuple_offset = 0;
        if( (*it)->hasSharedInputs() ) {
            if( (*it)->sharedInputEnabled(VERTEX_POSITION) ) {
                vertex_tuple_offset = (*it)->sharedInputTupleOffset( VERTEX_POSITION );
            }
            else {
                SCENELOG_WARN( log, "Input semantic VERTEX is required for shared inputs, giving up." );
                return false;
            }
        }
        for( unsigned int i=0; i<VERTEX_SEMANTIC_N; i++ ) {
            if( unsharedInputEnabled( (VertexSemantic)i ) ) {
                tuple_offsets[i] = vertex_tuple_offset;
            }
        }

        // Then, process the shared inputs from the polygon set. We update the
        // input sources, and make sure that there are no inconsistencies, and
        // update the tuple indices.
        for( unsigned int i=1; i<VERTEX_SEMANTIC_N; i++ ) {
            VertexSemantic sem = (VertexSemantic)i;
            if( (*it)->sharedInputEnabled(sem) ) {
                if( inputs[i].m_enabled ) {
                    if( (inputs[i].m_source_buffer_id != (*it)->sharedInputSourceBuffer(sem) ) ||
                        (inputs[i].m_components       != (*it)->sharedInputComponents(sem)   ) ||
                        (inputs[i].m_count            != (*it)->sharedInputCount(sem)        ) ||
                        (inputs[i].m_stride           != (*it)->sharedInputStride(sem)       ) ||
                        (inputs[i].m_offset           != (*it)->sharedInputOffset(sem)       ) )
                    {
                        SCENELOG_ERROR( log, "Mismatch in shared input source definitions, giving up." );
                        return false;
                    }
                }
                else {
                    inputs[i].m_enabled          = true;
                    inputs[i].m_source_buffer_id = (*it)->sharedInputSourceBuffer(sem);
                    inputs[i].m_components       = (*it)->sharedInputComponents(sem);
                    inputs[i].m_count            = (*it)->sharedInputCount(sem);
                    inputs[i].m_stride           = (*it)->sharedInputStride(sem);
                    inputs[i].m_offset           = (*it)->sharedInputOffset(sem);
                }
                tuple_offsets[ i ] = (*it)->sharedInputTupleOffset( sem );
            }
        }
        for( unsigned int i=0; i<VERTEX_SEMANTIC_N; i++ ) {
            if( tuple_offsets[i] != ~0u ) {
                SCENELOG_DEBUG( log, "sem=" << i << ", offset=" << tuple_offsets[i] );
            }
        }


        // Extract indicies
        offsets.push_back( indices.size() );
        if( (*it)->isIndexed() ) {
            SourceBuffer* index_buf = m_db.library<SourceBuffer>().get( (*it)->indexBufferId() );
            const int* ix = index_buf->intData();

            for( unsigned int p=0; p<(*it)->vertexCount(); p++ ) {

                Tuple t;
                for( unsigned int i=0; i<VERTEX_SEMANTIC_N; i++ ) {
                    if( tuple_offsets[ i ] == ~0u ) {
                        t.m_tuple[i] = ~0u;
                    }
                    else {
                        t.m_tuple[i] = ix[ (*it)->indexOffset() +
                                           (*it)->sharedInputTupleSize()*p +
                                           tuple_offsets[ i ] ];
                    }
                }

                unsigned int ix;
                auto it = remap.find( t );
                if( it == remap.end() ) {
                    ix = remap.size();
                    remap[ t ] = ix;
                }
                else {
                    ix = it->second;
                }
                indices.push_back( ix );
            }
        }
        // Create indices for non-indexed shapes
        else {
            for( unsigned int p=0; p<(*it)->vertexCount(); p++ ) {
                Tuple t;
                for( unsigned int i=0; i<VERTEX_SEMANTIC_N; i++ ) {
                    t.m_tuple[i] = ( tuple_offsets[i] == ~0u ? ~0u : p);
                }
                unsigned int ix;
                auto it = remap.find( t );
                if( it == remap.end() ) {
                    ix = remap.size();
                    remap[ t ] = ix;
                }
                else {
                    ix = it->second;
                }
                indices.push_back( ix );

            }
        }
    }
    offsets.push_back( indices.size() );



    // Create buffer
    unsigned int interleaved_offsets[ VERTEX_SEMANTIC_N +1 ];
    const float* interleaved_sources[ VERTEX_SEMANTIC_N ];
    interleaved_offsets[0] = 0;
    for( unsigned int i=0; i<VERTEX_SEMANTIC_N; i++ ) {
        if( inputs[i].m_enabled ) {
            interleaved_offsets[i+1] = interleaved_offsets[i] + inputs[i].m_components;
            const SourceBuffer* sb = m_db.library<SourceBuffer>().get( inputs[i].m_source_buffer_id );
            if( sb == NULL ) {
                SCENELOG_ERROR( log, "Unable to resolve source buffer, giving up." );
                return false;
            }
            interleaved_sources[i] = sb->floatData();
        }
        else {
            interleaved_offsets[i+1] = interleaved_offsets[i];
            interleaved_sources[i] = NULL;
        }
        SCENELOG_DEBUG( log, "interleaved offset " << i << "=" << interleaved_offsets[i] );
    }
    unsigned int interleaved_stride = interleaved_offsets[ VERTEX_SEMANTIC_N ];
    if( interleaved_stride == 0 ) {
        SCENELOG_ERROR( log, "No source data, giving up" );
        return false;
    }

    std::vector<float> interleaved( interleaved_stride*remap.size() );
    for( auto it=remap.begin(); it!=remap.end(); ++it ) {
#if 0
        SCENELOG_TRACE( log, "map " <<
                        it->second << " = { " <<
                        it->first.m_tuple[0] << ", " <<
                        it->first.m_tuple[1] << ", " <<
                        it->first.m_tuple[2] << ", " <<
                        it->first.m_tuple[3] << ", " <<
                        it->first.m_tuple[4] << ", " <<
                        it->first.m_tuple[5] << ", " <<
                        it->first.m_tuple[6] << ", " <<
                        it->first.m_tuple[7] << "} " );
        float* dst = interleaved.data() + it->second*interleaved_stride;
#endif
        for( unsigned int i=0; i<VERTEX_SEMANTIC_N; i++ ) {
            if( it->first.m_tuple[i] == ~0u ) {
                // ignore
            }
            else {
                std::copy_n( interleaved_sources[i] +
                             inputs[i].m_offset +
                             inputs[i].m_stride*it->first.m_tuple[i],
                             inputs[i].m_components,
                             interleaved.begin() +
                             it->second*interleaved_stride +
                             interleaved_offsets[i] );

            }

        }
        std::stringstream o;
        for( unsigned int i=0; i<interleaved_stride; i++ ) {
            if( i!=0 ) {
                o << ", ";
            }
            o << interleaved[ it->second*interleaved_stride + i ];
        }
        SCENELOG_TRACE( log, "data: " << o.str() );
    }

    SCENELOG_DEBUG( log, "Interleaved attribute tuple is of size " << interleaved_offsets[ VERTEX_SEMANTIC_N ] );

    SourceBuffer* interleaved_buffer = m_db.library<SourceBuffer>().add( m_id + "_attributes_float_interleaved" );
    if( interleaved_buffer == NULL ) {
        SCENELOG_ERROR( log, "Failed to create interleaved attribute buffer, giving up." );
        return false;
    }
    SourceBuffer* index_buffer = m_db.library<SourceBuffer>().add( m_id + "_flat_indices" );
    if( index_buffer == NULL ) {
        SCENELOG_ERROR( log, "Failed to create buffer for flattened indices, giving up" );
        return false;
    }

    interleaved_buffer->contents( interleaved );
    unsharedInputClearAll();
    for( unsigned int i=0; i<VERTEX_SEMANTIC_N; i++ ) {
        if( inputs[i].m_enabled ) {
            setVertexSource( (VertexSemantic)i,
                             interleaved_buffer->id(),
                             interleaved_offsets[i+1]-interleaved_offsets[i],
                             remap.size(),
                             interleaved_stride,
                             interleaved_offsets[i] );
        }
    }

    index_buffer->contents( indices );
    SCENELOG_DEBUG( log, "offsets.size=" << offsets.size() );
    SCENELOG_DEBUG( log, "m_primitive_sets.size=" << m_primitive_sets.size() );

    for( size_t i=0; i<m_primitive_sets.size(); i++ ) {
        Primitives* p = m_primitive_sets[i];
        p->clearSharedInputs();
        p->set( p->primitiveType(),
               (offsets[i+1]-offsets[i])/p->verticesPerPrimitive(),
               p->verticesPerPrimitive(),
               index_buffer->id(),
               offsets[i] );
    }

    return false;
}

const SeqPos&
Geometry::boundingBoxUpdated() const
{
    return m_bbox_timestamp;
}

bool
Geometry::boundingBox( const Value*& min, const Value*& max ) const
{
    min = &m_bbox_min;
    max = &m_bbox_max;
    return m_bbox_nonempty;
}

void
Geometry::clearBoundingBox()
{
    m_bbox_nonempty = false;
    m_bbox_timestamp.touch();
    valueChanged().moveForward( m_bbox_timestamp );
}

void
Geometry::setBoundingBox( const Value& bbmin, const Value& bbmax )
{
    m_bbox_min = bbmin;
    m_bbox_max = bbmax;
    m_bbox_nonempty = true;
    m_bbox_timestamp.touch();
    valueChanged().moveForward( m_bbox_timestamp );
}

void
Geometry::setVertexSource( VertexSemantic      semantic,
                           const std::string&  source_buffer_id,
                           int                 components,
                           int              count,
                           int              stride,
                           int              offset )
{
    const string prefix = "Scene::Geometry::setVertexSource: ";

    if( semantic >= VERTEX_SEMANTIC_N ) {
        throw runtime_error( prefix + "Illegal semantic." );
    }

    VertexInput& i = m_vertex_inputs[semantic];
    i.m_enabled = true;
    i.m_source_buffer_id = source_buffer_id;
    i.m_components = components;
    i.m_count = count;
    if( stride == 0 ) {
        i.m_stride = components;
    }
    else {
        i.m_stride = stride;
    }

    i.m_offset = offset;

    touchStructureChanged();
    m_db.library<Geometry>().moveForward( *this );
    m_db.moveForward( *this );
}


bool
Geometry::unsharedInputEnabled( VertexSemantic semantic ) const
{
    return m_vertex_inputs[ semantic ].m_enabled;
}

const std::string&
Geometry::unsharedInputSourceBufferId( VertexSemantic semantic ) const
{
    return m_vertex_inputs[ semantic ].m_source_buffer_id;
}

const unsigned int
Geometry::unsharedInputCount( VertexSemantic semantic ) const
{
    return m_vertex_inputs[ semantic ].m_count;
}

const unsigned int
Geometry::unsharedInputComponents( VertexSemantic semantic ) const
{
    return m_vertex_inputs[ semantic ].m_components;
}

const unsigned int
Geometry::unsharedInputOffset( VertexSemantic semantic ) const
{
    return m_vertex_inputs[ semantic ].m_offset;
}

const unsigned int
Geometry::unsharedInputStride( VertexSemantic semantic ) const
{
    return m_vertex_inputs[ semantic ].m_stride;
}



Primitives*
Geometry::addPrimitiveSet( )
{
    m_primitive_sets.push_back( new Primitives( this ) );
    touchStructureChanged();
    m_db.library<Geometry>().moveForward( *this );
    m_db.moveForward( *this );
    return m_primitive_sets.back();
}

void
Geometry::removePrimitiveSet( const Primitives* set )
{
    auto it = std::find( m_primitive_sets.begin(),
                         m_primitive_sets.end(),
                         set );
    if( it == m_primitive_sets.end() ) {
        Logger log = getLogger( package + ".removePrimitiveSet" );
        SCENELOG_ERROR( log, "Cannot find primitive set to remove" );
    }
    else {
        delete *it;
        m_primitive_sets.erase( it );
        touchStructureChanged();
        m_db.library<Geometry>().moveForward( *this );
        m_db.moveForward( *this );
    }
}



} // of namespace Scene
