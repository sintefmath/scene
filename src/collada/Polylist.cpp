#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Geometry.hpp"
#include "scene/SourceBuffer.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"


namespace Scene {
    namespace Collada {
        using std::unordered_map;
        using std::max;
        using std::string;
        using std::vector;


/*bool
Builder::parseInputShared( Geometry::PrimitiveSet& prim_set,
                           const unordered_map<string,Geometry::VertexInput>& inputs,
                           xmlNodePtr input_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parseInputShared" );
    if(!assertNode( input_node, "input" ) ) {
        return false;
    }

    string offset_str = attribute( input_node, "offset" );
    if( offset_str.empty() ) {
        SCENELOG_ERROR( log, "Required attribute 'offset' empty." );
        return false;
    }
    size_t offset;
    try {
        offset = boost::lexical_cast<size_t>( offset_str );
    }
    catch( const boost::bad_lexical_cast& e ) {
        SCENELOG_ERROR( log, "Failed to parse attribute 'offset': " << e.what() );
        return false;
    }

    prim_set.m_index_elements = max( prim_set.m_index_elements, offset+1 );

    // Currently, we are only interested in the number of elements for each
    // index. In the future, actually use the data of this node.

    //    string semantic_str = attribute( input_node, "semantic" );
    //    string source_str = attribute( input_node, "source" );
    //    string set_str = attribute( input_node, "set" );

    return true;
}
*/

#if 0

bool
Importer::parsePolylist( Geometry*                    geometry,
                        const std::unordered_map<std::string,Geometry::VertexInput>& inputs,
                        xmlNodePtr                      polylist_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parsePolylist" );
    if(!assertNode( polylist_node, "polylist" ) ) {
        return false;
    }

    // count attribute
    string count_str = attribute( polylist_node, "count" );
    size_t count;

    if( count_str.empty() ) {
        SCENELOG_ERROR( log, "Required attribute count empty." );
        return false;
    }
    try {
        count = boost::lexical_cast<size_t>( count_str );
    }
    catch( const boost::bad_lexical_cast& e ) {
        SCENELOG_ERROR( log, "Failed to parse count attribute: " << e.what() );
        return false;
    }

    std::string material_symbol = attribute( polylist_node, "material" );


    Primitives prim_set;

    // material attribute
    prim_set.m_material_symbol = material_symbol;
    //prim_set.m_index_offset_vertex = 0;
    prim_set.m_index_tuple_width = 1;


    // Parse inputs. These define the number of elements for each index. We only
    // use one element, see comment in xml/Input.cpp @ parseInputShared.
    xmlNodePtr n = polylist_node->children;
    while( n != NULL && xmlStrEqual( n->name, BAD_CAST "input" ) ) {
        if( !parseInputShared( prim_set, inputs, n) ) {
            return false;
        }
        n = n->next;
    }

    // Get vertex count for each primitive
    std::vector<int> vcount;
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "vcount" ) ) {
        if(! parseBodyAsInts( vcount, n, count ) ) {
            SCENELOG_ERROR( log, "Failed to parse <vcount>." );
            return false;
        }
        n = n->next;
    }

    // Get total number of vertices
    size_t total = 0;
    for(size_t i=0; i<vcount.size(); i++) {
        if( vcount[i] < 3 ) {
            SCENELOG_ERROR( log, "Vertex count must be at least three." );
            return false;
        }
        else {
            total += vcount[i];
        }
    }

    // Get indices.
    std::vector<int> p;
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "p" ) ) {
        if(!parseBodyAsInts( p, n,
                             total,
                             0,
                             prim_set.m_index_tuple_width ) ) {
            SCENELOG_ERROR( log, "Failed to parse <p>." );
            return false;
        }
    }


    // Extract
    size_t offset = 0;
    std::vector<int> triangles;
    std::vector<int> quads;
    for(size_t i=0; i<vcount.size(); i++) {
        if( vcount[i] == 3 ) {
            // triangle
            triangles.push_back( p[ offset+0 ] );
            triangles.push_back( p[ offset+1 ] );
            triangles.push_back( p[ offset+2 ] );
        }
        else if (vcount[i] == 4 ) {
            // quadrilateral
            quads.push_back( p[ offset+0 ] );
            quads.push_back( p[ offset+1 ] );
            quads.push_back( p[ offset+2 ] );
            quads.push_back( p[ offset+3 ] );
        }
        else {
            // Pretty naive triangulation
            SCENELOG_WARN( log, "Triangulating " << vcount[i] << "-gon." );
            for(int k=2; k<vcount[i]; k++) {
                triangles.push_back( p[ offset+0] );
                triangles.push_back( p[ offset+k-1] );
                triangles.push_back( p[ offset+k] );
            }
        }
        offset += vcount[i];
    }


    if( !triangles.empty() ) {
        string index_name = geometry->id() + "_tris";
        SourceBuffer* ix = m_database.library<SourceBuffer>().add( index_name );
        ix->contents( triangles );

        geometry->addPrimitiveSet( PRIMITIVE_TRIANGLES,
                                   material_symbol,
                                   index_name,
                                   triangles.size()/3 );

//        Primitives prim_set_t = prim_set;
//        prim_set_t.m_type = PRIMITIVE_TRIANGLES;
//        prim_set_t.m_index_source_id = index_name;
//        prim_set_t.m_count = triangles.size()/3;
//        prim_set_t.m_index_elements = 1;
//        geometry->addPrimitiveSet( prim_set_t );
    }

    if( !quads.empty() ) {
        string index_name = geometry->id() + "_quads";
        SourceBuffer* ix = m_database.library<SourceBuffer>().add( index_name );
        ix->contents( quads );

        geometry->addPrimitiveSet( PRIMITIVE_QUADS,
                                   material_symbol,
                                   index_name,
                                   quads.size()/4 );

//        Primitives prim_set_q = prim_set;
//        prim_set_q.m_type = PRIMITIVE_QUADS;
//        prim_set_q.m_index_source_id = index_name;
//        prim_set_q.m_count = quads.size()/4;
//        prim_set_q.m_index_elements = 1;
//       geometry->addPrimitiveSet( prim_set_q );
    }

    return true;
}

#endif



    } // of namespace Scene
} // of namespace Scene
