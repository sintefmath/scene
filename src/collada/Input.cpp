
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

/*
bool
Importer::parseInputShared( Primitives& prim_set,
                           const unordered_map<string,Geometry::VertexInput>& inputs,
                           xmlNodePtr input_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parseInputShared" );
    if(!assertNode( input_node, "input" ) ) {
        return false;
    }

    // Indexed rendering from buffers in OpenGL only supports one index per
    // vertex, i.e, vertex positons, normals, etc for a given vertex must
    // all have the same index. COLLADA allows defining indices with multiple
    // element, e.g. two elements per index. This is incompatible with
    // OpenGL rendering. Converting is not completely trivial, so here we just
    // use a single index element, the one with the position. On the COLLADA
    // website there  is a conditioner (Deindexer) that should fix geometries
    // with multiple indices.


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

    prim_set.m_index_tuple_width = max( prim_set.m_index_tuple_width, offset+1 );
    string semantic_str = attribute( input_node, "semantic" );
    if( semantic_str == "VERTEX" ) {
        prim_set.m_index_offset_vertex = offset;
    }

    return true;
}
*/


    } // of namespace Scene
} // of namespace Scene
