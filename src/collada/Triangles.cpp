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

#include <unordered_map>
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
        using std::string;
        using std::vector;
        using boost::lexical_cast;


    /*

      Note: functionality replaced by parseSimplePrimitives.

bool
Importer::parseTriangles( Geometry*                    geometry,
                         const std::unordered_map<std::string,Geometry::VertexInput>& inputs,
                         xmlNodePtr                      triangles_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parseTriangles" );
    if(!assertNode( triangles_node, "triangles" ) ) {
        return false;
    }


    Primitives prim_set;
    prim_set.m_type = PRIMITIVE_TRIANGLES;
    prim_set.m_index_offset_vertex = 0;
    prim_set.m_index_elements = 1;
    prim_set.m_vertices = 3;

    // count attribute
    string count_str = attribute( triangles_node, "count" );
    if( count_str.empty() ) {
        SCENELOG_ERROR( log, "Required attribute count empty." );
        return false;
    }
    try {
        prim_set.m_count = boost::lexical_cast<size_t>( count_str );
    }
    catch( const boost::bad_lexical_cast& e ) {
        SCENELOG_ERROR( log, "Failed to parse count attribute: " << e.what() );
        return false;
    }

    // material attribute
    prim_set.m_material_symbol = attribute( triangles_node, "material" );

    // Parse inputs. These define the number of elements for each index. We only
    // use one element, see comment in xml/Input.cpp @ parseInputShared.
    xmlNodePtr n = triangles_node->children;
    while( n != NULL && xmlStrEqual( n->name, BAD_CAST "input" ) ) {
        if( !parseInputShared( prim_set, inputs, n) ) {
            return false;
        }
        n = n->next;
    }

    // Get indices if present
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "p" ) ) {
        vector<int> p;
        if(!parseBodyAsInts( p, n,
                             prim_set.m_vertices * prim_set.m_count,
                             prim_set.m_index_offset_vertex,
                             prim_set.m_index_elements ) ) {
            SCENELOG_ERROR( log, "Failed to parse <p>." );
            return false;
        }

        prim_set.m_index_source_id = geometry->id() +
                                     "_indices_" +
                                     boost::lexical_cast<string>( geometry->primitiveSets() );
        SourceBuffer* ix = m_database.addSourceBuffer( prim_set.m_index_source_id );
        ix->contents( p );

        // and iterate one step forwards
        n = n->next;
    }
    else {
        prim_set.m_index_source_id = "";
    }

    // and plow through the rest of the nodes
    for( ; n!=NULL; n=n->next ) {
        if( xmlStrEqual( n->name, BAD_CAST "extra" ) ) {
            // ignore
        }
        else {
            SCENELOG_WARN( log, "Unexpected node " <<
                           reinterpret_cast<const char*>( n->name) );
        }
    }


    prim_set.m_index_offset_vertex = 0;
    prim_set.m_index_elements = 1;

    geometry->addPrimitiveSet( prim_set );

    return true;
}
*/
/*
xmlNodePtr
Exporter::createTriangles( Context& context,
                           const Primitives& ps ) const
{
    Logger log = getLogger( "Scene.Builder.createTriangles" );

    xmlNodePtr triangle_node = xmlNewNode( NULL, BAD_CAST "triangles" );

    // Add attribute 'count', required.
    const string count_str = lexical_cast<string>( ps.m_primitive_count );
    xmlNewProp( triangle_node, BAD_CAST "count", BAD_CAST count_str.c_str() );

    // Add attribute 'material', optional
    if( !ps.m_material_symbol.empty() ) {
        xmlNewProp( triangle_node, BAD_CAST "material", BAD_CAST ps.m_material_symbol.c_str() );
        context.m_referenced_materials[ ps.m_material_symbol ] = true;
    }

    // Add <p> child if indexed
    if( !ps.m_index_buffer_id.empty() ) {
        const SourceBuffer* buffer = m_database.library<SourceBuffer>().get( ps.m_index_buffer_id );
        if( buffer == NULL ) {
            SCENELOG_ERROR( log, "Failed to locate index buffer '" << ps.m_index_buffer_id << "'." );
            return NULL;
        }


        xmlNodePtr p_node = xmlNewChild( triangle_node,
                                         NULL,
                                         BAD_CAST "p",
                                         NULL );
        setBody( p_node, buffer->intData(), buffer->elementCount() );
    }


    return triangle_node;
}
*/



    } // of namespace Scene
} // of namespace Scene
