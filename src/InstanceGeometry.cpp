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


#include "scene/Log.hpp"
#include "scene/InstanceGeometry.hpp"

namespace Scene {
    using std::string;
    using std::vector;






InstanceGeometry::InstanceGeometry( const std::string geometry_url )
: m_geometry_url( geometry_url )
{}



void
InstanceGeometry::addMaterialBinding( const std::string& symbol, const std::string& target_id )
{
    Logger log = getLogger( "Scene.InstanceGeometry.addMaterialBinding" );

    if( symbol.empty() ) {
        SCENELOG_ERROR( log, "Empty symbol, ignoring." );
        return;
    }
    if( target_id.empty() ) {
        SCENELOG_ERROR( log, "Empty target, ignoring." );
    }

    SCENELOG_TRACE( log, "geo='" << m_geometry_url <<
                    "', symbol='" << symbol <<
                    "', target='" << target_id << '\'' );

    MaterialBinding b;
    b.m_symbol = symbol;
    b.m_target_id = target_id;

    m_material_bindings[ symbol ] = b;
}

const std::string&
InstanceGeometry::materialBindingTargetId( const std::string& symbol ) const
{
    auto it = m_material_bindings.find( symbol );
    if( it != m_material_bindings.end() ) {
        return it->second.m_target_id;
    }
    else {
        static const string none;
        return none;
    }
}

const vector<Bind>&
InstanceGeometry::materialBindingBind( const std::string& symbol ) const
{
    auto it = m_material_bindings.find( symbol );
    if( it != m_material_bindings.end() ) {
        return it->second.m_bind;
    }
    else {
        static const vector<Bind> none;
        return none;
    }

}


void
InstanceGeometry::addMaterialBindingBind( const std::string& symbol, const Bind& bind )
{
    auto it = m_material_bindings.find( symbol );
    if( it != m_material_bindings.end() ) {
        it->second.m_bind.push_back( bind );
    }
    else {
        Logger log = getLogger( "Scene.InstanceGeometry.addMaterialBindingBind" );
        SCENELOG_ERROR( log, "Unknown symbol '" << symbol << "', ignoring bind" );
    }
}

std::list<std::string>
InstanceGeometry::materialBindingSymbols() const
{
    std::list<std::string> symbols;
    for( auto it = m_material_bindings.begin(); it!=m_material_bindings.end(); ++it ) {
        symbols.push_back( it->first );
    }
    return symbols;
}


const std::string&
InstanceGeometry::geometryId() const
{
    return m_geometry_url;
}









} // of namespace Scene
