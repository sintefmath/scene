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

#include <string>
#include "scene/Log.hpp"
#include "scene/Utils.hpp"
#include "scene/Bind.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;

bool
Importer::parseBind( Bind&       bind,
                     xmlNodePtr  bind_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseBind" );

    string semantic = attribute( bind_node, "semantic" );
    if( semantic == "MODELVIEW_MATRIX" ) {
        bind.m_semantic = RUNTIME_MODELVIEW_MATRIX;
    }
    else if( semantic == "PROJECTION_MATRIX" ) {
        bind.m_semantic = RUNTIME_PROJECTION_MATRIX;
    }
    else if( semantic == "MODELVIEW_PROJECTION_MATRIX" ) {
        bind.m_semantic = RUNTIME_MODELVIEW_PROJECTION_MATRIX;
    }
    else if( semantic == "NORMAL_MATRIX" ) {
        bind.m_semantic = RUNTIME_NORMAL_MATRIX;
    }
    else {
        SCENELOG_ERROR( log, "semantic '" << semantic << "' not recognized." );
        return false;
    }

    bind.m_target = attribute( bind_node, "target" );
    if( bind.m_target.empty() ) {
        SCENELOG_ERROR( log, "Required attribute 'target' empty." );
        return false;
    }

    return true;
}


xmlNodePtr
Exporter::createBind( Context& context, const Bind& bind ) const
{
    xmlNodePtr bind_node = newNode( NULL, "bind" );
    addProperty( bind_node, "semantic", uniformSemantic( bind.m_semantic ) );
    addProperty( bind_node, "target", bind.m_target );
    return bind_node;
}

    } // of namespace XML
} // of namespace Scene
