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

#include <sstream>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;


Exporter::Exporter( const Scene::DataBase& database )
: m_database( database ),
  m_lean_export( false )
{
}

xmlNodePtr
Exporter::create(  bool lib_geometry,
                   bool lib_image,
                   bool lib_camera,
                   bool lib_light,
                   bool lib_effect,
                   bool lib_material,
                   bool lib_nodes,
                   bool lib_visual_scene,
                   int profile_mask)
{
    Logger log = getLogger( "Scene.XML.Builder.create" );

    Context context;
    context.m_lib_geometry     = lib_geometry;
    context.m_lib_image        = lib_image;
    context.m_lib_camera       = lib_camera;
    context.m_lib_light        = lib_light;
    context.m_lib_effect       = lib_effect;
    context.m_lib_material     = lib_material;
    context.m_lib_nodes        = lib_nodes;
    context.m_lib_visual_scene = lib_visual_scene;
    context.m_profile_mask     = profile_mask;

    xmlNodePtr collada_node = createCollada( context );
    if( collada_node == NULL ) {
        SCENELOG_ERROR( log, "Failed to create COLLADA node." );
    }
    return collada_node;
}

void
Exporter::setBody( xmlNodePtr node, const float* values, size_t count ) const
{
    if( values == NULL ) {
        Logger log = getLogger( "Scene.XML.Builder.setBody" );
        SCENELOG_FATAL( log, "Got null pointer to floats." );
        return;
    }

    std::stringstream o;
    if( count > 0 ) {
        o << values[0];
    }
    for(size_t i=1; i<count; i++) {
        o << ' ' << values[i];
    }

    xmlNodeSetContent( node, reinterpret_cast<const xmlChar*>( o.str().c_str() ) );
}

xmlNodePtr
Exporter::newChild( xmlNodePtr parent, xmlNsPtr ns, const std::string& name, const std::string& content ) const
{
    return xmlNewChild( parent,
                        ns,
                        reinterpret_cast<const xmlChar*>( name.c_str() ),
                        reinterpret_cast<const xmlChar*>( content.c_str() ) );
}

xmlNodePtr
Exporter::newChild( xmlNodePtr parent, xmlNsPtr ns, const std::string& name ) const
{
    return xmlNewChild( parent,
                        ns,
                        reinterpret_cast<const xmlChar*>( name.c_str() ),
                        NULL );
}

xmlNodePtr
Exporter::newNode( xmlNsPtr ns, const std::string& name ) const
{
    return xmlNewNode( ns, reinterpret_cast<const xmlChar*>( name.c_str() ) );
}

void
Exporter::addProperty( xmlNodePtr node, const std::string& key, const std::string& value ) const
{
    xmlNewProp( node,
                reinterpret_cast<const xmlChar*>( key.c_str() ),
                reinterpret_cast<const xmlChar*>( value.c_str() ) );
}

void
Exporter::addProperty( xmlNodePtr node, const std::string& key, const int value ) const
{
    std::stringstream tmp;
    tmp << value;
    xmlNewProp( node,
                reinterpret_cast<const xmlChar*>( key.c_str() ),
                reinterpret_cast<const xmlChar*>( tmp.str().c_str() ) );
}

const std::string
Exporter::sourceId( const std::string& source_buffer_id,
                    const unsigned int count,
                    const unsigned int components,
                    const unsigned int offset,
                    const unsigned int stride ) const
{
    std::stringstream tmp;
    tmp << source_buffer_id
        << "_"
        << count
        << "_"
        << components
        << "_"
        << offset
        << "_"
        << stride;

    return tmp.str();
}


void
Exporter::setBody( xmlNodePtr node, const int* values, size_t count ) const
{
    if( values == NULL ) {
        Logger log = getLogger( "Scene.XML.Builder.setBody" );
        SCENELOG_FATAL( log, "Got null pointer to ints." );
        return;
    }

    std::stringstream o;
    if( count > 0 ) {
        o << values[0];
    }
    for(size_t i=1; i<count; i++) {
        o << ' ' << values[i];
    }

    xmlNodeSetContent( node, reinterpret_cast<const xmlChar*>( o.str().c_str() ) );
}



    } // of namespace Scene
} // of namespace Scene
