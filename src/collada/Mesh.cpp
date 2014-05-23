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

#include <list>
#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <iterator>
#include <boost/lexical_cast.hpp>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Geometry.hpp"
#include "scene/SourceBuffer.hpp"
#include "scene/Utils.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::unordered_map;
        using std::string;
        using std::vector;

static const struct {
    const std::string   m_string;
    VertexSemantic      m_semantic;
}   semantic_map[ ] = {
    { "VERTEX",     VERTEX_POSITION },
    { "POSITION",   VERTEX_POSITION },
    { "COLOR",      VERTEX_COLOR },
    { "NORMAL",     VERTEX_NORMAL },
    { "TEXCOORD",   VERTEX_TEXCOORD },
    { "TANGENT",    VERTEX_TANGENT },
    { "BINORMAL",   VERTEX_BINORMAL },
    { "UV",         VERTEX_UV },
    { "",           VERTEX_SEMANTIC_N }
};

static const struct {
    const std::string   m_string;
    PrimitiveType       m_type;
    bool                m_simple;
    bool                m_vcount_array;
    unsigned int        m_vertex_count;
}   primitive_map[] = {
    { "points",     PRIMITIVE_POINTS,       true,   false, 1   },
    { "lines",      PRIMITIVE_LINES,        true,   false, 2   },
    { "linestrips", PRIMITIVE_N,            false,  false, 0   },
    { "patches",    PRIMITIVE_PATCHES,      true,   false, 0   },
    { "polygons",   PRIMITIVE_N,            false,  false, 0   },
    { "polylist",   PRIMITIVE_TRIANGLES,    false,  true,  3   }, // see parsePolyList
    { "triangles",  PRIMITIVE_TRIANGLES,    true,   false, 3   },
    { "quads",      PRIMITIVE_QUADS,        true,   false, 4   },
    { "trifans",    PRIMITIVE_N,            false,  false, 0   },
    { "tristrips",  PRIMITIVE_N,            false,  false, 0   },
    { "", PRIMITIVE_N, false }
};



bool
Importer::parseMesh( Scene::Geometry*  geometry,
                    xmlNodePtr        mesh_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parseMesh" );
    assertNode( mesh_node, "mesh" );

    SCENELOG_INFO( log, "Parsing <mesh>, geometry.id='"<< geometry->id() <<"'." );


    unordered_map<string,Geometry::VertexInput> inputs;

    // First, At least one, or more, <source>'s
    xmlNodePtr n=mesh_node->children;
    for( ; checkNode( n, "source" ); n=n->next ) {
        const string id = attribute( n, "id" );
        if( id.empty() ) {
            nagAboutParseError( log, n, "Required attribute 'id' is missing/malformed, giving up." );
            return false;
        }

        Scene::Geometry::VertexInput input;
        if( parseSource( input, n ) ) {
            inputs[ id ] = input;
        }
        else {
            nagAboutParseError( log, n );
            return false;
        }
    }


    // Parse unshared inputs in required node <vertices>
    if( n == NULL ) {
        SCENELOG_ERROR( log, "Premature end of mesh node" );
        return false;
    }
    else if( !checkNode( n, "vertices" ) ) {
        nagAboutParseError( log, n, "Expected <vertices>" );
        return false;
    }
    else {
        xmlNodePtr m = n->children;
        for( ; m!=NULL && strEq( m->name, "input" ); m=m->next ) {
            std::string sem_str = attribute( m, "semantic" );
            VertexSemantic sem = VERTEX_SEMANTIC_N;
            for( unsigned int i=0; !semantic_map[i].m_string.empty(); i++ ) {
                if( semantic_map[i].m_string == sem_str ) {
                    sem = semantic_map[i].m_semantic;
                }
            }
            if( sem == VERTEX_SEMANTIC_N ) {
                SCENELOG_ERROR( log, "Unknown input semanitic '" << sem_str << "', skipping" );
                continue;
            }
            std::string src_str = cleanRef( attribute( m, "source" ) );
            auto it = inputs.find( src_str );
            if( it == inputs.end() ) {
                SCENELOG_ERROR( log, "Unable to resolve input source '" << src_str << "', skipping" );
                continue;
            }
            geometry->setVertexSource( sem,
                                       it->second.m_source_buffer_id,
                                       it->second.m_components,
                                       it->second.m_count,
                                       it->second.m_stride,
                                       it->second.m_offset );

        }
        ignoreExtraNodes( log, m );
        nagAboutRemainingNodes( log, m );
        n = n->next;
    }


    // And then, the zero or more primitive sets
    bool success = true;
    for( ; n!=NULL; n=n->next ) {
        Primitives* primitives = geometry->addPrimitiveSet();

        unsigned int match = ~0u;

        unsigned int vertex_count = 0;

        // determine primitive type
        for( unsigned int i=0; !primitive_map[i].m_string.empty(); i++ ) {
            if( strEq( n->name, primitive_map[i].m_string ) ) {
                match = i;
//                type         = primitive_map[i].m_type;
                vertex_count = primitive_map[i].m_vertex_count;
                break;
            }
        }
        if( match == ~0u ) {
            nagAboutParseError( log, n, "Unexpected node" );
            geometry->removePrimitiveSet( primitives );
            continue;
        }
        if( primitive_map[ match ].m_type == PRIMITIVE_N ) {
            nagAboutParseError( log, n, "Unsupported primitive type" );
            geometry->removePrimitiveSet( primitives );
            continue;
        }
        // if patches type, require the vertices attribute
        if( primitive_map[ match ].m_type  == PRIMITIVE_PATCHES ) {
            if( !attribute( vertex_count, n, "vertices" ) ) {
                SCENELOG_ERROR( log, "Required attribute 'vertices' is missing/malformed, skipping." );
                geometry->removePrimitiveSet( primitives );
                continue;
            }
        }
        if( vertex_count < 1 ) {
            SCENELOG_ERROR( log, "Illegal vertex count " << vertex_count << ", skipping." );
            geometry->removePrimitiveSet( primitives );
            continue;
        }
        // get the number of indices
        unsigned int prim_count = 0;
        if( !attribute( prim_count, n, "count" ) ) {
            SCENELOG_ERROR( log, "Required attribute 'count' is missing/malformed, skipping." );
            geometry->removePrimitiveSet( primitives );
            continue;
        }

        primitives->setMaterialSymbol( attribute( n, "material" ) );


        // process child nodes
        xmlNodePtr m = n->children;

        // process shared inputs
        std::list<xmlNodePtr> shared_inputs;
        for( ; checkNode( m, "input" ); m=m->next ) {
            unsigned int offset;
            if( !attribute( offset, m, "offset" ) ) {
                nagAboutParseError( log, m, "Required attribute 'offset' missing/malformed, skipping input." );
                continue;
            }

            VertexSemantic semantic = VERTEX_SEMANTIC_N;
            std::string sem_str = attribute( m, "semantic" );
            for( unsigned int i=0; i<VERTEX_SEMANTIC_N; i++ ) {
                if( semantic_map[i].m_string == sem_str ) {
                    semantic = semantic_map[i].m_semantic;
                }
            }
            if( semantic == VERTEX_SEMANTIC_N ) {
                nagAboutParseError( log, m, "Required attribute 'semantic' (='" + sem_str +"'') is missing/malformed, skipping input." );
                continue;
            }
            else if( semantic == VERTEX_POSITION ) {
                primitives->setSharedVertexSource( offset, semantic, "", 0, 0, 0, 0 );
            }
            else {
                std::string src_str = cleanRef( attribute( m, "source" ) );
                auto jt = inputs.find( src_str );
                if( jt == inputs.end() ) {
                    nagAboutParseError( log, m, "Unable to resolve shared input source '" + src_str + "', skipping input." );
                    continue;
                }

                primitives->setSharedVertexSource( offset,
                                                   semantic,
                                                   jt->second.m_source_buffer_id,
                                                   jt->second.m_components,
                                                   jt->second.m_count,
                                                   jt->second.m_stride,
                                                   jt->second.m_offset );

            }
        }



        // get vcount if needed
        size_t expected_tuples = 0;

        std::vector<int> vcount;
        if( primitive_map[ match ].m_vcount_array ) {
            if( m == NULL ) {
                nagAboutParseError( log, n, "Premature end of node" );
                geometry->removePrimitiveSet( primitives );
                continue;
            }
            else if( !strEq( m->name, "vcount" ) ) {
                nagAboutParseError( log, m, "Expected <vcount>" );
                geometry->removePrimitiveSet( primitives );
                continue;
            }
            else if( !parseBodyAsInts( vcount, m, prim_count ) ) {
                nagAboutParseError( log, m, "Failed to parse body" );
                geometry->removePrimitiveSet( primitives );
                continue;
            }
            else {
                expected_tuples = std::accumulate( vcount.begin(), vcount.end(), 0 );
                m = m->next;
            }
        }
        else {
            expected_tuples = vertex_count * prim_count;
        }



        if( m != NULL && strEq( m->name, "p" ) ) {
            std::vector<int> p;
            if( !parseBodyAsInts( p, m, expected_tuples * primitives->sharedInputTupleSize()  ) ) {
                SCENELOG_ERROR( log, "Failed to parse <p>, skipping primitives." );
                geometry->removePrimitiveSet( primitives );
                continue;
            }


            // tessellated polygons to triangles
            if( primitive_map[ match ].m_vcount_array ) {
                std::vector<int> tessellated;

                size_t offset = 0;
                for( size_t i=0; i<vcount.size(); i++ ) {
                    if( vcount[i] == 3 ) {
                        std::copy_n( p.begin() + primitives->sharedInputTupleSize()*offset, 3*primitives->sharedInputTupleSize(), std::back_insert_iterator< std::vector<int> >( tessellated ) );
                    }
                    else if( vcount[i] == 4 ) {
                        std::copy_n( p.begin() + primitives->sharedInputTupleSize()*(offset+0), 3*primitives->sharedInputTupleSize(), std::back_insert_iterator< std::vector<int> >( tessellated ) );
                        std::copy_n( p.begin() + primitives->sharedInputTupleSize()*(offset+2), 2*primitives->sharedInputTupleSize(), std::back_insert_iterator< std::vector<int> >( tessellated ) );
                        std::copy_n( p.begin() + primitives->sharedInputTupleSize()*(offset+0), 1*primitives->sharedInputTupleSize(), std::back_insert_iterator< std::vector<int> >( tessellated ) );
                    }
                    else {


                    }
                    offset += vcount[i];
                }
                p.swap( tessellated );
                SCENELOG_DEBUG( log, "Tessellated " << prim_count << " polygons into " << (p.size()/(primitives->sharedInputTupleSize()*3)) << " triangles (tuple width=" << primitives->sharedInputTupleSize() << ")" );
                prim_count = p.size()/(primitives->sharedInputTupleSize()*3);
            }


            string indices_id = geometry->id() + "_ix_" + boost::lexical_cast<string>( geometry->primitiveSets() );
            SourceBuffer* indices = m_database.library<SourceBuffer>().add( indices_id );
            indices->contents( p );

            primitives->set( primitive_map[ match ].m_type,
                             prim_count,
                             vertex_count,
                             indices->id(),
                             0 );
            m = m->next;
        }
        else {
            primitives->set( primitive_map[ match ].m_type, prim_count, vertex_count );
        }

        SCENELOG_INFO( log, "Parsed primitiveset, type=" << primitive_map[ match ].m_type <<
                       ", prim_count=" << prim_count << ", vertex_count=" << vertex_count <<
                       ", tuple_width=" << primitives->sharedInputTupleSize() );


        ignoreExtraNodes( log, m );
        nagAboutRemainingNodes( log, m );
    }
    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );
    return success;
}


xmlNodePtr
Exporter::createMesh( Context& context,
                     const Scene::Geometry* geometry ) const
{
    Logger log = getLogger( "Scene.XML.Exporter.createMesh" );
    if( geometry == NULL ) {
        return NULL;
    }

    xmlNodePtr mesh_node = newNode( NULL, "mesh" );
    // Add <source>s referenced by unshared input
    for( unsigned int k=0; k<VERTEX_SEMANTIC_N; k++ ) {
        VertexSemantic semantic = static_cast<VertexSemantic>( k );
        if( geometry->unsharedInputEnabled( semantic ) ) {
            xmlAddChild( mesh_node, createSource( context,
                                                  geometry->unsharedInputSourceBufferId( semantic ),
                                                  geometry->unsharedInputCount( semantic ),
                                                  geometry->unsharedInputComponents( semantic ),
                                                  geometry->unsharedInputOffset( semantic ),
                                                  geometry->unsharedInputStride( semantic ) ) );
        }
    }
    // Add <source>s referenced by shared input
    for( size_t i=0; i<geometry->primitiveSets(); i++ ) {
        const Primitives* primitives = geometry->primitives(i);
        if( primitives->hasSharedInputs() ) {
            for( unsigned int k=0; k<VERTEX_SEMANTIC_N; k++ ) {
                VertexSemantic semantic = static_cast<VertexSemantic>( k );
                if( semantic == VERTEX_POSITION ) {
                    continue;   // should be the <vertices> node
                }
                if( primitives->sharedInputEnabled( semantic ) ) {
                    xmlAddChild( mesh_node,
                                 createSource( context,
                                               primitives->sharedInputSourceBuffer( semantic ),
                                               primitives->sharedInputCount( semantic ),
                                               primitives->sharedInputComponents( semantic ),
                                               primitives->sharedInputOffset( semantic ),
                                               primitives->sharedInputStride( semantic ) ) );
                }
            }
        }
    }
    // <vertices> declares shared input
    xmlNodePtr vertices_node = newChild( mesh_node, NULL, "vertices" );
    addProperty( vertices_node, "id", geometry->id() + "_vertices" );
    for( unsigned int k=0; k<VERTEX_SEMANTIC_N; k++ ) {
        VertexSemantic semantic = static_cast<VertexSemantic>( k );
        if( geometry->unsharedInputEnabled( semantic ) ) {
            xmlNodePtr input_node = newChild( vertices_node, NULL, "input" );
            addProperty( input_node, "semantic", vertexSemantic( semantic ) );
            addProperty( input_node, "source", "#" + sourceId( geometry->unsharedInputSourceBufferId( semantic ),
                                                               geometry->unsharedInputCount( semantic ),
                                                               geometry->unsharedInputComponents( semantic ),
                                                               geometry->unsharedInputOffset( semantic ),
                                                               geometry->unsharedInputStride( semantic ) ) );
        }
    }
    // Primitive sets
    for( size_t i=0; i<geometry->primitiveSets(); i++ ) {
        const Primitives* primitives = geometry->primitives(i);
        xmlNodePtr prim_node = NULL;
        switch( primitives->primitiveType() ) {
        case PRIMITIVE_POINTS:
            prim_node = newChild( mesh_node, NULL, "points" );
            break;
        case PRIMITIVE_LINES:
            prim_node = newChild( mesh_node, NULL, "lines" );
            break;
        case PRIMITIVE_TRIANGLES:
            prim_node = newChild( mesh_node, NULL, "triangles" );
            break;
        case PRIMITIVE_QUADS:
            prim_node = newChild( mesh_node, NULL, "quads" );
            break;
        case PRIMITIVE_PATCHES:
            prim_node = newChild( mesh_node, NULL, "patches" );
            addProperty( mesh_node, "vertices", primitives->verticesPerPrimitive() );
            break;
        case PRIMITIVE_N:
            break;
        }
        if( prim_node == NULL ) {
            continue;
        }
        addProperty( prim_node, "count", primitives->vertexCount()/primitives->verticesPerPrimitive() );
        if( !primitives->materialSymbol().empty() ) {
            addProperty( prim_node, "material", primitives->materialSymbol() );
        }

        // Add shared input
        if( primitives->hasSharedInputs() ) {
            for( unsigned int k=0; k<VERTEX_SEMANTIC_N; k++ ) {
                VertexSemantic semantic = static_cast<VertexSemantic>( k );
                if( primitives->sharedInputEnabled( semantic ) ) {
                    xmlNodePtr input_node = newChild( prim_node, NULL, "input" );
                    if( semantic == VERTEX_POSITION ) {
                        addProperty( input_node, "semantic", "VERTEX" );
                        addProperty( input_node, "source", "#" + geometry->id() + "_vertices" );
                    }
                    else {
                        addProperty( input_node, "semantic", vertexSemantic( semantic ) );
                        addProperty( input_node, "source", "#" + sourceId( primitives->sharedInputSourceBuffer( semantic ),
                                                                           primitives->sharedInputCount( semantic ),
                                                                           primitives->sharedInputComponents( semantic ),
                                                                           primitives->sharedInputOffset( semantic ),
                                                                           primitives->sharedInputStride( semantic ) ) );
                    }
                    addProperty( input_node, "offset", primitives->sharedInputTupleOffset( semantic ) );
                }
            }
        }

        if( primitives->isIndexed() ) {
            const SourceBuffer* indices = m_database.library<SourceBuffer>().get( primitives->indexBufferId() );
            if( indices == NULL ) {
                SCENELOG_ERROR( log, "Unable to retrieve index buffer '" << primitives->indexBufferId() << "'" );
            }
            else if (indices->elementType() != ELEMENT_INT ) {
                SCENELOG_ERROR( log, "Index buffer has not int type but " << elementType( indices->elementType() ) );
            }
            else if(indices->elementCount() < (primitives->indexOffset() + primitives->vertexCount()*primitives->sharedInputTupleSize() ) ) {
                SCENELOG_ERROR( log, "Index buffer has less elements than required" );
            }
            else {
                xmlNodePtr p_node = newChild( prim_node, NULL, "p" );
                setBody( p_node,
                         indices->intData() + primitives->indexOffset(),
                         primitives->vertexCount()*primitives->sharedInputTupleSize() );
            }
        }
    }

    return mesh_node;
}



    } // of namespace Scene
} // of namespace Scene
