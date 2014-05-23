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

#include "scene/Parameter.hpp"
#include "scene/Utils.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;

#define ADD_SEMANTIC(a,b) else if( semantic_str == #a ) { semantic = b; return true; }

bool
Importer::parseSemantic( RuntimeSemantic& semantic, xmlNodePtr semantic_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseSemantic" );
    if(!assertNode( semantic_node, "semantic" ) ) {
        return false;
    }

    const string semantic_str = getBody( semantic_node );
    semantic = runtimeSemantic( semantic_str );
    return true;
}

bool
Importer::parseNewParam( Parameter&          parameter,
                         xmlNodePtr          newparam_node,
                         const ValueContext  context )
{
    Logger log = getLogger( "Scene.XML.Importer.parseNewParam" );
    if(!assertNode( newparam_node, "newparam" ) ) {
        return false;
    }

    const string sid = attribute( newparam_node, "sid" );
    if( sid.empty() ) {
        SCENELOG_ERROR( log, "Required attribute 'sid' empty");
        return false;
    }


    xmlNodePtr n = newparam_node->children;
    if( context != VALUE_CONTEXT_COMMON_GROUP ) {
        while( n!= NULL && xmlStrEqual( n->name, BAD_CAST "annotate" ) ) {
            SCENELOG_WARN( log, "<annotate> ignored." );
            n = n->next;
        }
    }

    RuntimeSemantic semantic;
    if( n != NULL && xmlStrEqual( n->name, BAD_CAST "semantic" ) ) {
        if(!parseSemantic( semantic, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <semantic>" );
            return false;
        }
        n = n->next;
    }
    else {
        semantic = RUNTIME_SEMANTIC_N;
    }

    if( context != VALUE_CONTEXT_COMMON_GROUP ) {
        if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "modifier" ) ) {
            SCENELOG_WARN( log, "<modifier> ignored." );
            n = n->next;
        }
    }
    if( n==NULL ) {
        SCENELOG_ERROR( log, "No value specified." );
        return false;
    }

    Value value;
    if( !parseValue( &value, n, context ) ) {
        SCENELOG_ERROR( log, "Failed to parse value" );
        return false;
    }
    parameter.set( sid, semantic, value );
    return true;
}

xmlNodePtr
Exporter::createNewParam( Context&            context,
                          const ValueContext  value_context,
                          const Parameter*    param ) const
{
    xmlNodePtr np_node = newNode( NULL, "newparam" );
    addProperty( np_node, "sid", param->sid() );
    if( param->semantic() != RUNTIME_SEMANTIC_N ) {
        newChild( np_node,
                  NULL,
                  "semantic",
                  runtimeSemantic( param->semantic() ) );
    }
    xmlAddChild( np_node, createValue( param->value(), value_context ) );

    return np_node;
}


    } // of namespace XML
} // of namespace Scene
