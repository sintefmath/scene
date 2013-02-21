#include <boost/lexical_cast.hpp>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Geometry.hpp"
#include "scene/Primitives.hpp"

namespace Scene {
    using std::string;

Primitives::Primitives( Geometry* geometry )
    : m_geometry( geometry ),
      m_has_shared_inputs( false ),
      m_primitive_type( PRIMITIVE_N ),
      m_primitive_count( 0 ),
      m_vertices_per_primitive( 0 ),
      m_material_symbol( "" ),
      m_index_buffer_id( "" ),
      m_index_buffer_offset( 0 ),
      m_index_tuple_width( 0 )
{
    clearSharedInputs();
}

void
Primitives::setMaterialSymbol( const std::string& symbol,
                               const bool taint )
{
    m_material_symbol = symbol;
    if( taint && m_geometry != NULL ) {
        m_geometry->touchStructureChanged();
        m_geometry->db().library<Geometry>().moveForward( *m_geometry );
        m_geometry->db().moveForward( *m_geometry );
    }
}


void
Primitives::set( const PrimitiveType    type,
                 const unsigned int     primitive_count,
                 const unsigned int     vertices_per_primitive,
                 const bool             taint )
{
    m_primitive_type = type;
    m_primitive_count = primitive_count;
    m_vertices_per_primitive = vertices_per_primitive;
    m_index_buffer_id.clear();
    m_index_buffer_offset = 0;

    if( taint && m_geometry != NULL ) {
        m_geometry->touchStructureChanged();
        m_geometry->db().library<Geometry>().moveForward( *m_geometry );
        m_geometry->db().moveForward( *m_geometry );
    }
}

void
Primitives::set( const PrimitiveType    type,
                 const unsigned int     primitive_count,
                 const unsigned int     vertices_per_primitive,
                 const std::string&     index_buffer_id,
                 const unsigned int     index_buffer_offset,
                 const bool             taint )
{
    m_primitive_type = type;
    m_primitive_count = primitive_count;
    m_vertices_per_primitive = vertices_per_primitive;
    m_index_buffer_id = index_buffer_id;
    m_index_buffer_offset = index_buffer_offset;

    if( taint && m_geometry != NULL ) {
        m_geometry->touchStructureChanged();
        m_geometry->db().library<Geometry>().moveForward( *m_geometry );
        m_geometry->db().moveForward( *m_geometry );
    }
}


void
Primitives::clearSharedInputs()
{
    for( int i=0; i<VERTEX_SEMANTIC_N; i++ ) {
        m_shared_inputs[i].m_enabled = false;
    }
    m_has_shared_inputs = false;
    m_index_tuple_width = 1;
}

void
Primitives::setSharedVertexSource( const unsigned int   index_offset,
                                   VertexSemantic       semantic,
                                   const std::string&   source_buffer_id,
                                   const unsigned int   components,
                                   const unsigned int   count,
                                   const unsigned int   stride,
                                   const unsigned int   offset )
{
    m_shared_inputs[ semantic ].m_enabled           = true;
    m_shared_inputs[ semantic ].m_tuple_offset      = index_offset;
    m_shared_inputs[ semantic ].m_source_buffer_id  = source_buffer_id;
    m_shared_inputs[ semantic ].m_components        = components;
    m_shared_inputs[ semantic ].m_primitive_count   = count;
    m_shared_inputs[ semantic ].m_offset            = offset;
    m_shared_inputs[ semantic ].m_stride            = stride;
    m_index_tuple_width = std::max( index_offset+1u, m_index_tuple_width );
    m_has_shared_inputs = true;
}


const std::string
Primitives::key() const
{
    Logger log = getLogger( "Scene.Primitives.key" );

    if( m_geometry == NULL ) {
        SCENELOG_FATAL( log, "invoked on primitives that are not associated a geoemtry!" );
        return "";
    }

    return m_geometry->key() + "/" +
            boost::lexical_cast<string>( this );
}



} // of namespace Scene
