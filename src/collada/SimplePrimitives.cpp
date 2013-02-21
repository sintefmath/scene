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

static const string ipackage = "Scene.XML.Importer";
/*

bool
Importer::parseSimplePrimitives( Geometry*                    geometry,
                       const std::unordered_map<std::string,Geometry::VertexInput>& inputs,
                       xmlNodePtr                      primitives_node )
{
    Logger log = getLogger( ipackage + ".parseSimplePrimitives" );

    PrimitiveType prim_type;
    unsigned int prim_vtx_count = 0;

    if( checkNode( primitives_node, "points") ) {
        prim_type = PRIMITIVE_POINTS;
        prim_vtx_count = 1;
    }
    else if( checkNode( primitives_node, "lines") ) {
        prim_type = PRIMITIVE_LINES;
        prim_vtx_count = 2;
    }
    else if( checkNode( primitives_node, "triangles" ) ) {
        prim_type = PRIMITIVE_TRIANGLES;
        prim_vtx_count = 3;
    }
    else if( checkNode( primitives_node, "quads" ) ) {
        prim_type = PRIMITIVE_QUADS;
        prim_vtx_count = 4;
    }
    else if( checkNode( primitives_node, "patches" ) ) {
        prim_type = PRIMITIVE_PATCHES;

        string vertices_str = attribute( primitives_node, "vertices" );
        if( vertices_str.empty() ) {
            SCENELOG_ERROR( log, "<patch> without required attribute 'vertices'." );
            return false;
        }
        try {
            prim_vtx_count = boost::lexical_cast<size_t>( vertices_str );
        }
        catch( const boost::bad_lexical_cast& e ) {
            SCENELOG_ERROR( log, "<patch> with malformed attribute 'vertices': " << e.what() );
            return false;
        }
    }
    else {
        SCENELOG_FATAL( log, "Wrong nodetype." );
        return false;
    }

    // count attribute
    unsigned int prim_count = 0;
    string count_str = attribute( primitives_node, "count" );
    if( count_str.empty() ) {
        SCENELOG_ERROR( log, "Required attribute count empty." );
        return false;
    }
    try {
        prim_count = boost::lexical_cast<size_t>( count_str );
    }
    catch( const boost::bad_lexical_cast& e ) {
        SCENELOG_ERROR( log, "Failed to parse count attribute: " << e.what() );
        return false;
    }


    // material attribute
    string material_symbol = attribute( primitives_node, "material" );

    // Parse inputs. These define the number of elements for each index. We only
    // use one element, see comment in xml/Input.cpp @ parseInputShared.
    xmlNodePtr n = primitives_node->children;
    while( n != NULL && xmlStrEqual( n->name, BAD_CAST "input" ) ) {
        SCENELOG_WARN( log, "ignored shared input element." );

//        if( !parseInputShared( prim_set, inputs, n) ) {
//            return false;
//        }
        n = n->next;
    }

    // Get indices.
    std::vector<int> p;
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "p" ) ) {
        if( parseBodyAsInts( p, n,
                             prim_vtx_count * prim_count ) ) {

            string index_id = geometry->id() + "_indices_" + boost::lexical_cast<string>( geometry->primitiveSets() );
            SourceBuffer* ix = m_database.library<SourceBuffer>().add( index_id );
            ix->contents( p );

            geometry->addPrimitiveSet( prim_type,
                                       material_symbol,
                                       index_id,
                                       prim_count,
                                       prim_vtx_count );
        }
        else {
            SCENELOG_ERROR( log, "Failed to parse <p>." );
            return false;
        }
    }
    else {
        geometry->addPrimitiveSet( prim_type,
                                   material_symbol,
                                   prim_count,
                                   prim_vtx_count );
    }
    return true;
}

*/


    } // of namespace Scene
} // of namespace Scene
