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

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <boost/lexical_cast.hpp>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/SourceBuffer.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;
        using boost::lexical_cast;

static const string ipackage = "Scene.XML.Importer";

bool
Importer::parseFloatArray( Scene::SourceBuffer* source_buffer,
                          xmlNodePtr          float_array_node )
{
    Logger log = getLogger( ipackage + ".parseFloatArray");

    size_t count = 0;
    std::vector<float> data;
    const char* a = NULL;
    xmlChar* p = NULL;
    string count_str;

#ifdef DEBUG
    if( !xmlStrEqual( float_array_node->name, BAD_CAST "float_array" ) )  {
        SCENELOG_FATAL( log, "Node is not <float_array>" );
        return false;
    }
#endif

    count_str = attribute( float_array_node, "count" );
    if( count_str.empty() ) {
        SCENELOG_ERROR( log, "Required count attribute empty." );
        return false;
    }
    count = boost::lexical_cast<size_t>( count_str );

    data.resize( count );

    p = xmlNodeGetContent( float_array_node );
    if( p == NULL ) {
        SCENELOG_ERROR( log, "XML node has no content." );
        return false;
    }

    a = reinterpret_cast<const char*>( p );
    for(size_t i=0; i<count; i++) {
        char *b = NULL;
        errno = 0;
#ifdef _WIN32
        //MSVC 10 does not support strtof yet (c++0x feature)
        data[i] = static_cast<float>(strtod( a, &b ));
#else
        data[i] = strtof( a, &b );
#endif
        if( errno != 0 ) {
            int myerrno = errno;
            SCENELOG_ERROR( log, "strtoX: " << strerror( myerrno ) );
        }

        if( a == b ) {
            SCENELOG_ERROR( log, "Premature end of data" );
            return false;
        }
        a = b;
    }
    xmlFree( p );

    source_buffer->contents( data );
    return true;
}

xmlNodePtr
Exporter::createFloatArray( Context& context,
                          const Scene::SourceBuffer* source_buffer ) const
{
    Logger log = getLogger( "Scene.XML.Builder.createFloatBuffer" );

    const string id_str = source_buffer->id();
    const string count_str = lexical_cast<string>( source_buffer->elementCount() );

    xmlNodePtr float_array_node = xmlNewNode( NULL, BAD_CAST "float_array" );
    xmlNewProp( float_array_node, BAD_CAST "id", BAD_CAST id_str.c_str() );
    xmlNewProp( float_array_node, BAD_CAST "count", BAD_CAST count_str.c_str() );
    setBody( float_array_node,
             source_buffer->floatData(),
             source_buffer->elementCount() );

    return float_array_node;
}

    } // of namespace Scene
} // of namespace Scene
