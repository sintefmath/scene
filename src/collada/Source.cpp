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
#include "scene/Utils.hpp"
#include "scene/SourceBuffer.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;
        using boost::lexical_cast;



bool
Importer::parseSource( Scene::Geometry::VertexInput&  input,
                      xmlNodePtr                     source_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parseSource" );

    if(!assertNode( source_node, "source" ) ) {
            return false;
    }


    xmlNodePtr n = source_node->children;

    // An optional <asset>
    if( n == NULL ) {
        SCENELOG_ERROR( log, "Premature end of <source> element." );
        return false;
    }
    else if( xmlStrEqual( n->name, reinterpret_cast<const xmlChar*>( "asset" ) ) ) {
        // silently ignore <asset>
        n = n->next;
    }


    // An optional source buffer
    if( n == NULL ) {
    }
    else if( xmlStrEqual( n->name, BAD_CAST "bool_array" ) ) {
        SCENELOG_WARN( log, "<bool_array> currently ignored." );
        n = n->next;
    }
    else if( xmlStrEqual( n->name, reinterpret_cast<const xmlChar*>( "float_array" ) ) ) {

        // QA: The COLLADA spec doesn't require an ID, but we need it. However,
        // the buffer must be accessed through an accessor, and the source
        // attribute of an accessor is required. Thus, it beats me how one could
        // actually get hold of the data in an anonymous source buffer. So we
        // require the ID.
        string id = attribute( n, "id" );
        if( id.empty() ) {
            SCENELOG_ERROR( log, "<float_array> with empty id attribute." );
            return false;
        }

        Scene::SourceBuffer* source = m_database.library<SourceBuffer>().add( id );
        if( source == NULL ) {
            SCENELOG_ERROR( log, "Failed to create source buffer '" << id << "'." );
            return false;
        }
        if( !parseFloatArray( source, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <float_array id='" << id << "'>." );
            return false;
        }
        n = n->next;
    }
    else if( xmlStrEqual( n->name, BAD_CAST "IDREF_array" ) ) {
        SCENELOG_WARN( log, "<IDREF_array> currently ignored." );
        n = n->next;
    }
    else if( xmlStrEqual( n->name, BAD_CAST "int_array" ) ) {
        // Handle int array
        n = n->next;
    }
    else if( xmlStrEqual( n->name, BAD_CAST "Name_array" ) ) {
        SCENELOG_WARN( log, "<Name_array> currently ignored." );
        n = n->next;
    }
    else if( xmlStrEqual( n->name, BAD_CAST "SIDREF_array" ) ) {
        SCENELOG_WARN( log, "<SIDREF_array> currently ignored." );
        n = n->next;
    }
    else if( xmlStrEqual( n->name, BAD_CAST "token_array" ) ) {
        SCENELOG_WARN( log, "<token_array> currently ignored." );
        n = n->next;
    }

    // There should be able to throw in an accessor here...

    // An optional technique_common (we require this)
    xmlNodePtr technique_common_node = NULL;
    if( n == NULL ) {
    }
    if( xmlStrEqual( n->name, BAD_CAST "technique_common" ) ) {
        technique_common_node = n;
        n = n->next;
    }

    // And just plow over the remaining nodes
    for( ; n!=NULL; n=n->next ) {
        if( xmlStrEqual( n->name, BAD_CAST "technique" ) ) {
            SCENELOG_WARN( log, "<technique> currently ignored in this context." );
        }
        else {
            SCENELOG_WARN( log, "Unexpected node " << reinterpret_cast<const char*>(n->name) );
        }
    }

    // Process the technique_common node
    if( technique_common_node == NULL ) {
        SCENELOG_WARN( log, "No <technique_common> found." );
        return false;
    }
    n = technique_common_node->children;

    if( n==NULL ) {
        SCENELOG_WARN( log, "Premature end of <technique_common>" );
        return false;
    }
    else if( xmlStrEqual( n->name, BAD_CAST "asset" ) ) {
        // silently ignore <asset>
        n = n->next;
    }

    if( n==NULL ) {
        SCENELOG_WARN( log, "Premature end of <technique_common>" );
        return false;
    }
    else if( xmlStrEqual( n->name, BAD_CAST "accessor" ) ) {
        parseAccessor( input, n );

        n = n->next;
    }

    // And just plow over the remaining nodes
    for( ; n!=NULL; n=n->next ) {
        SCENELOG_WARN( log, "Unexpected node " << reinterpret_cast<const char*>(n->name) );
    }
    return true;
}


xmlNodePtr
Exporter::createSource( Context&            context,
                        const std::string&  source_buffer_id,
                        const unsigned int  count,
                        const unsigned int  components,
                        const unsigned int  offset,
                        const unsigned int  stride  ) const
{
    Logger log = getLogger( "Scene.XML.Exporter.createSource" );

    const std::string src_id = sourceId( source_buffer_id,
                                         count,
                                         components,
                                         offset,
                                         stride );
    auto it = context.m_exported_sources.find( src_id );
    if( it != context.m_exported_sources.end() ) {
        SCENELOG_TRACE( log, "Source " << src_id << " already exported" );
        return NULL;
    }
    context.m_exported_sources[ src_id ] = true;

    xmlNodePtr src_node = newNode( NULL, "source" );
    addProperty( src_node, "id", src_id );


    auto ref = context.m_exported_source_buffers.find( source_buffer_id );
    if( ref == context.m_exported_source_buffers.end() ) {
        const SourceBuffer* buffer = m_database.library<SourceBuffer>().get( source_buffer_id );
        if( buffer == NULL ) {
            SCENELOG_ERROR( log, "Unable to find source buffer '" << source_buffer_id << "'" );
        }
        else {
            switch( buffer->elementType() ) {
            case ELEMENT_FLOAT:
                xmlAddChild( src_node, createFloatArray( context, buffer ) );
                break;
            case ELEMENT_INT:
                SCENELOG_FATAL( log, "integer source buffers not yet supported" );
                break;
            }
            context.m_exported_source_buffers[ source_buffer_id ] = true;
        }
    }

    xmlNodePtr tc_node = newChild( src_node, NULL, "technique_common" );
    xmlAddChild( tc_node, createAccessor( context,
                                          source_buffer_id,
                                          count,
                                          components,
                                          offset,
                                          stride ) );

    return src_node;
}



    } // of namespace Scene
} // of namespace Scene
