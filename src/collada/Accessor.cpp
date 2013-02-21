#include <boost/lexical_cast.hpp>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Geometry.hpp"
#include "scene/SourceBuffer.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;
        using boost::lexical_cast;


bool
Importer::parseAccessor( Scene::Geometry::VertexInput&  input,
                        xmlNodePtr                     accessor_node )
{
    Logger log = getLogger( "Scene.XML.Builder.parseAccessor" );

#ifdef DEBUG
    if( !xmlStrEqual( accessor_node->name, BAD_CAST "accessor" ) )  {
        SCENELOG_FATAL( log, "Node is not <accessor>" );
        return false;
    }
#endif

    // attribute 'count', required.
    const string count_str = attribute( accessor_node, "count" );
    if( count_str.empty() ) {
        SCENELOG_ERROR( log, "Required attribute 'count' empty." );
        return false;
    }
    input.m_count = boost::lexical_cast<size_t>( count_str );

    // attribute 'offset', optional, defaults to 0
    const string offset_str = attribute( accessor_node, "offset" );
    if( !offset_str.empty() ) {
        input.m_offset = boost::lexical_cast<size_t>( offset_str );
    }
    else {
        input.m_offset = 0;
    }

    // attribute 'source', required
    input.m_source_buffer_id = attribute( accessor_node, "source" );
    if( !input.m_source_buffer_id.empty() && input.m_source_buffer_id[0] == '#' ) {
        input.m_source_buffer_id = input.m_source_buffer_id.substr(1);
    }
    if( input.m_source_buffer_id.empty() ) {
        SCENELOG_ERROR( log, "Required attribute 'source' missing." );
        return false;
    }


    // <param> children determines components. We ignore the type and use the
    // element type of the source buffer.
    input.m_components = 0;
    for(xmlNodePtr n = accessor_node->children; n!=NULL; n=n->next ) {
        if( xmlStrEqual( n->name, BAD_CAST "param" ) ) {
            input.m_components++;
        }
        else {
            SCENELOG_WARN( log, "Unexpected node " << reinterpret_cast<const char*>(n->name) );
        }
    }

    // attribute 'stride', optional, defaults to tightly packed data.
    const string stride_str = attribute( accessor_node, "stride" );
    if( !stride_str.empty() ) {
        input.m_stride = lexical_cast<size_t>( stride_str );
    }
    else {
        input.m_stride = input.m_components;
    }


    return true;
}

xmlNodePtr
Exporter::createAccessor( Context&           context,
                          const std::string& source_buffer_id,
                          const unsigned int count,
                          const unsigned int components,
                          const unsigned int offset,
                          const unsigned int stride ) const
{
    const Scene::SourceBuffer* buffer = m_database.library<SourceBuffer>().get( source_buffer_id );
    if( buffer == NULL ) {
        return NULL;
    }

    xmlNodePtr acc_node = newNode( NULL, "accessor" );
    addProperty( acc_node, "count", count );
    if( offset != 0 ) {
        addProperty( acc_node, "offset", offset );
    }
    addProperty( acc_node, "source", "#" + buffer->id() );
    if( stride != components ) {
        addProperty( acc_node, "stride", stride );
    }

    std::string element_type;
    switch( buffer->elementType() ) {
    case ELEMENT_FLOAT:
        element_type = "float";
        break;
    case ELEMENT_INT:
        element_type = "int";
        break;
    }

    for( unsigned int i=0; i<components; i++ ) {
        xmlNodePtr p_node = newChild( acc_node, NULL, "param" );
        addProperty( p_node, "type", element_type );
    }

    return acc_node;
}



    } // of namespace Scene
} // of namespace Scene
