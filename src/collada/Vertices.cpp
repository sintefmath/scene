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

#include <boost/lexical_cast.hpp>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Geometry.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;
        using std::unordered_map;
        using boost::lexical_cast;

/*
bool
Importer::parseVertices( Geometry*  geo,
                        const unordered_map<std::string,Geometry::VertexInput>& inputs,
                        xmlNodePtr vertices_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parseVertices" );
    if( !assertNode( vertices_node, "vertices" ) ) {
        return false;
    }

    bool position_set = false;
    for( xmlNodePtr n = vertices_node->children; n!=NULL; n=n->next) {

        if( xmlStrEqual( n->name, BAD_CAST "input" ) ) {
            string semantic_str = attribute( n, "semantic" );
            if( semantic_str.empty() ) {
                SCENELOG_ERROR( log, "<input> with empty required semantic attribute." );
                return false;
            }
            VertexSemantic semantic = VERTEX_SEMANTIC_N;
            for( int i=0; i<VERTEX_SEMANTIC_N; i++ ) {
                if( semantic_str == m_vertex_semantics[i] ) {
                    semantic = (VertexSemantic)i;
                    break;
                }
            }

            if( semantic == VERTEX_POSITION ) {
                position_set = true;
            }
            else if( semantic == VERTEX_SEMANTIC_N ) {
                SCENELOG_ERROR( log, "Unrecognized vertex semantic '" << semantic_str << "'.");
                return false;
            }


            string source_str = attribute( n, "source" );
            if( !source_str.empty() && source_str[0] == '#' ) {
                source_str = source_str.substr( 1 );
            }
            if( source_str.empty() ) {
                SCENELOG_ERROR( log, "<input> with empty required source attribute." );
                return false;
            }

            auto it = inputs.find( source_str );
            if( it == inputs.end() ) {
                SCENELOG_ERROR( log, "Failed to resolve input source '"<< source_str << "'.");
                return false;
            }
            const Scene::Geometry::VertexInput& input = it->second;
            geo->setVertexSource( semantic,
                                  input.m_source_buffer_id,
                                  input.m_components,
                                  input.m_count,
                                  input.m_stride,
                                  input.m_offset );
        }
        else if( xmlStrEqual( n->name, BAD_CAST "extra" ) )  {
            // silently ignore <extra>
        }
        else {
            SCENELOG_WARN( log, "Unexpected node " << reinterpret_cast<const char*>(n->name) );
        }
    }
    if(!position_set) {
        SCENELOG_ERROR( log, "no <input semantic=POSITION> in <vertices>." );
        return false;
    }

    return true;
}
*/
xmlNodePtr
Exporter::createVertices( const Scene::Geometry* geometry ) const
{
    xmlNodePtr vertices_node = xmlNewNode( NULL, BAD_CAST "vertices" );

    for( int i=0; i<VERTEX_SEMANTIC_N; i++) {
        const VertexSemantic semantic = (VertexSemantic)i;
        const Geometry::VertexInput& input = geometry->vertexInput( semantic );
        if( input.m_enabled ) {
            xmlNodePtr input_node = xmlNewChild( vertices_node,
                                                 NULL,
                                                 BAD_CAST "input",
                                                 NULL );
            xmlNewProp( input_node, BAD_CAST "semantic", BAD_CAST m_vertex_semantics[i].c_str() );

            // 'source' attribute, must match string in createSource.
            const string src_name = "#" +
                                    geometry->id() +
                                    "_source_" +
                                    lexical_cast<string>( static_cast<int>(semantic) );
            xmlNewProp( input_node, BAD_CAST "source", BAD_CAST src_name.c_str() );

        }
    }

    return vertices_node;
}

    }
}
