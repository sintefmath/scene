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

#include <scene/Log.hpp>
#include <scene/Geometry.hpp>
#include <scene/SourceBuffer.hpp>
#include <scene/tools/BBoxTool.hpp>
#include <scene/runtime/TransformCache.hpp>

namespace Scene {
    namespace Tools {

    static const std::string package = "Scene.Tools";

bool
updateBoundingBox( Geometry* geometry )
{
    if( geometry == NULL ) {
        return false;
    }

//    geometry->updateBoundingBox();

    Logger log = getLogger( package + ".updateBoundingBox[" + geometry->id() + "]" );
    DataBase& db = geometry->db();

    const Geometry::VertexInput& pos = geometry->vertexInput( VERTEX_POSITION );
    if( pos.m_enabled == false ) {
        SCENELOG_TRACE( log, "Geometry has no position information, skipping" );
        return false;
    }
    if( pos.m_components == 0 ) {
        SCENELOG_WARN( log, "Vertex position data has no components, skipping" );
        return false;
    }
    const SourceBuffer* pos_buf = db.library<SourceBuffer>().get( pos.m_source_buffer_id );
    if( pos_buf == NULL ) {
        SCENELOG_WARN( log, "Unable to retrieve buffer with vertex position data " << pos.m_source_buffer_id );
        return false;
    }
    if( pos_buf->elementType() != ELEMENT_FLOAT ) {
        SCENELOG_WARN( log, "Only float vertex position data is currently handled, skipping" );
        return false;
    }

    bool changed = !geometry->boundingBoxUpdated().asRecentAs( geometry->valueChanged() ) ||
                   !geometry->boundingBoxUpdated().asRecentAs( pos_buf->valueChanged() );

    // Fetch indices and check if index buffers have changed since last bbox update
    std::vector<const SourceBuffer*> indices( geometry->primitiveSets() );
    for( size_t i=0; i<geometry->primitiveSets(); i++ ) {
        if( geometry->primitives(i)->isIndexed() ) {
            indices[i] = db.library<SourceBuffer>().get( geometry->primitives(i)->indexBufferId() );
            if( indices[i] == NULL ) {
                SCENELOG_WARN( log, "Unable to retrieve index buffer" );
                return false;
            }
            if( indices[i]->elementType() != ELEMENT_INT ) {
                SCENELOG_WARN( log, "Indices are not of type int" );
                return false;
            }
            changed = changed || !geometry->boundingBoxUpdated().asRecentAs( indices[i]->valueChanged() );
        }
        else {
            indices[i] = NULL;
        }
    }
    if( !changed ) {
        SCENELOG_TRACE( log, "Nothing has changed that can affect the bounding box" );
        return true;
    }

    // update bbox
    const bool m1 = pos.m_components > 1;
    const bool m2 = pos.m_components > 2;
    unsigned int v_str = pos.m_stride;
    if( v_str == 0 ) {
        v_str = pos.m_components;
    }

    // Max vertex index without going outside bounds
    unsigned int max_v_ix = pos_buf->elementCount()/pos.m_components;

    bool  bb_empty = true;
    float bb_min[3] = { 0.f, 0.f, 0.f };
    float bb_max[3] = { 0.f, 0.f, 0.f };
    for( size_t i=0; i<geometry->primitiveSets(); i++ ) {

        const Primitives* p = geometry->primitives(i);
        // pointer to vertex pos data
        const float* ap = pos_buf->floatData() + pos.m_offset;


        if( p->isIndexed() ) {
            // Primitive set is indexed.
            unsigned int index_offset = 0;
            unsigned int index_stride = 1;
            if( p->hasSharedInputs() ) {
                // Primitive set has shared inputs, determine which index in the
                // index tuple that references position data.
                index_offset = p->sharedInputTupleOffset( VERTEX_POSITION );
                index_stride = p->sharedInputTupleSize();
            }
            // Determine (and check) number of vertices
            unsigned int N = p->vertexCount();
            if( indices[i]->elementCount() < index_stride*N ) {
                SCENELOG_WARN( log, "vertexcount out of range." );
                N = indices[i]->elementCount()/index_stride;
            }


            // pointer to indices
            const int* ip = indices[i]->intData() +
                            p->indexOffset() +
                            index_offset;

            unsigned int illegal_indices = 0;
            for( unsigned int i=0; i<N; i++ ) {
                const unsigned int ix = static_cast<unsigned int>( ip[ index_stride*i ] );
                if( ix < max_v_ix ) {
                    const float* vp = ap + v_str*ix;
                    const float p[3] = { vp[0],
                                         m1 ? vp[1] : 0.f,
                                         m2 ? vp[2] : 0.f };
                    if( bb_empty ) {
                        bb_min[0] = bb_max[0] = p[0];
                        bb_min[1] = bb_max[1] = p[1];
                        bb_min[2] = bb_max[2] = p[2];
                        bb_empty = false;
                    }
                    else {
                        bb_min[0] = p[0] < bb_min[0] ? p[0] : bb_min[0];
                        bb_min[1] = p[1] < bb_min[1] ? p[1] : bb_min[1];
                        bb_min[2] = p[2] < bb_min[2] ? p[2] : bb_min[2];
                        bb_max[0] = bb_max[0] < p[0] ? p[0] : bb_max[0];
                        bb_max[1] = bb_max[1] < p[1] ? p[1] : bb_max[1];
                        bb_max[2] = bb_max[2] < p[2] ? p[2] : bb_max[2];
                    }
                }
                else {
                    illegal_indices++;
                }
            }
            if( illegal_indices > 0 ) {
                SCENELOG_WARN( log, "Primitive set has " << illegal_indices << " illegal indices" );
            }

        }
        else {
            // Primitive set is not indexed
            unsigned int N = p->vertexCount();
            if( max_v_ix < N ) {
                SCENELOG_WARN( log, "vertexcount out of range." );
                N =  max_v_ix;
            }
            for( unsigned int i=0; i<N; i++ ) {
                const float* vp = ap + v_str*i;
                const float p[3] = { vp[0],
                                     m1 ? vp[1] : 0.f,
                                     m2 ? vp[2] : 0.f };
                if( bb_empty ) {
                    bb_min[0] = bb_max[0] = p[0];
                    bb_min[1] = bb_max[1] = p[1];
                    bb_min[2] = bb_max[2] = p[2];
                    bb_empty = false;
                }
                else {
                    bb_min[0] = p[0] < bb_min[0] ? p[0] : bb_min[0];
                    bb_min[1] = p[1] < bb_min[1] ? p[1] : bb_min[1];
                    bb_min[2] = p[2] < bb_min[2] ? p[2] : bb_min[2];
                    bb_max[0] = bb_max[0] < p[0] ? p[0] : bb_max[0];
                    bb_max[1] = bb_max[1] < p[1] ? p[1] : bb_max[1];
                    bb_max[2] = bb_max[2] < p[2] ? p[2] : bb_max[2];
                }
            }
        }
    }
    if( bb_empty == false ) {
        geometry->setBoundingBox( Value::createFloat4( bb_min[0], bb_min[1], bb_min[2], 1.f ),
                                  Value::createFloat4( bb_max[0], bb_max[1], bb_max[2], 1.f ) );
        SCENELOG_TRACE( log, "bbox=["
                        << bb_min[0] << ", " << bb_min[1] << ", " << bb_min[2] << "]x["
                        << bb_max[0] << ", " << bb_max[1] << ", " << bb_max[2] << "]" );
    }
    else {
        geometry->clearBoundingBox();
        SCENELOG_TRACE( log, "bbox=[ empty ]" );
    }
    return true;
}

void
updateBoundingBoxes( DataBase& database )
{
    for( size_t i=0; i<database.library<Geometry>().size(); i++ ) {
        updateBoundingBox( database.library<Geometry>().get(i) );
    }

}



bool
visualSceneExtents( Scene::Value& bbmin,
                    Scene::Value& bbmax,
                    const Runtime::RenderList& renderlist )
{

    Runtime::TransformCache cache( renderlist.resolver().database(), false );

    // First, tag the transforms we need
    for( size_t i=0; i<renderlist.items(); i++ ) {
        const Runtime::RenderList::Item& item = renderlist.item(i);
        cache.pathTransformMatrix( item.m_set_local_coordsys->m_node_path );
    }

    // Then, let the cache be updated
    cache.update( 1, 1 );

    // And then, run through the geometries, transform to world, and update global bbox.
    bool nonempty = false;
    float bb_min[3] = {0.f, 0.f, 0.f};
    float bb_max[3] = {0.f, 0.f, 0.f};

    for( size_t i=0; i<renderlist.items(); i++ ) {
        const Runtime::RenderList::Item& item = renderlist.item(i);

        const Geometry* g = NULL;
        if( item.m_draw != NULL ) {
            g = item.m_draw->m_geometry;
        }
        else if( item.m_draw_indexed != NULL ) {
            g = item.m_draw_indexed->m_geometry;
        }
        else {
            continue;
        }

        const Value* l_bbmin;
        const Value* l_bbmax;
        if( g->boundingBox( l_bbmin, l_bbmax ) ) {
            const float* M = cache.pathTransformMatrix( item.m_set_local_coordsys->m_node_path )->floatData();
            for( unsigned int k=0; k<8; k++ ) {
                float p[3] = {
                    (k & 0x1) == 0 ? l_bbmin->floatData()[0] : l_bbmax->floatData()[0],
                    (k & 0x2) == 0 ? l_bbmin->floatData()[1] : l_bbmax->floatData()[1],
                    (k & 0x4) == 0 ? l_bbmin->floatData()[2] : l_bbmax->floatData()[2]
                };
                float r = 1.f/(M[3]*p[0] + M[7]*p[1] + M[11]*p[2] + M[15]);
                float q[3] = {
                    r*(M[0]*p[0] + M[4]*p[1] + M[ 8]*p[2] + M[12]),
                    r*(M[1]*p[0] + M[5]*p[1] + M[ 9]*p[2] + M[13]),
                    r*(M[2]*p[0] + M[6]*p[1] + M[10]*p[2] + M[14]),
                };
                if( nonempty == false ) {
                    bb_min[0] = bb_max[0] = q[0];
                    bb_min[1] = bb_max[1] = q[1];
                    bb_min[2] = bb_max[2] = q[2];
                    nonempty = true;
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
    bbmin = Value::createFloat3( bb_min[0], bb_min[1], bb_min[2] );
    bbmax = Value::createFloat3( bb_max[0], bb_max[1], bb_max[2] );
    return nonempty;
}



    } // of namespace Tools
} // of namespace Scene
