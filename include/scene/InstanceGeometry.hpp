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

#pragma once

#include <vector>
#include <unordered_map>
#include <list>
#include "scene/Bind.hpp"
#include "scene/Scene.hpp"

namespace Scene {


/** In a Node, specify geometry to instantiate. */
class InstanceGeometry
{
public:

    InstanceGeometry( const std::string geometry_url );

    const std::string&
    geometryId() const;

    void
    addMaterialBinding( const std::string& symbol, const std::string& target );

    const std::string&
    materialBindingTargetId( const std::string& symbol ) const;

    void
    addMaterialBindingBind( const std::string& symbol, const Bind& bind );

    const std::vector<Bind>&
    materialBindingBind( const std::string& symbol ) const;

    std::list<std::string>
    materialBindingSymbols() const;

protected:
    std::string                                      m_geometry_url;
    struct MaterialBinding
    {
        std::string m_symbol;
        std::string m_target_id;
        std::vector<Bind>  m_bind;
    };
    std::unordered_map<std::string,MaterialBinding>  m_material_bindings;

};





};
