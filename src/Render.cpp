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

#include "scene/Render.hpp"

namespace Scene {
    using std::string;

Render::Render( EvaluateScene* evaluate_scene )
    : m_evaluate_scene( evaluate_scene )
{
}

const string&
Render::sid() const
{
    return m_sid;
}

void
Render::setSid( const string& sid )
{
    m_sid = sid;
}

const string&
Render::cameraNodeId() const
{
    return m_camera_node;
}

void
Render::setCameraNodeId( const string& id )
{
    m_camera_node = id;
}

size_t
Render::layers() const
{
    return m_layers.size();
}

const string&
Render::layer( size_t ix ) const
{
    return m_layers[ix];
}

void
Render::addLayer( const string& layer )
{
    m_layers.push_back( layer );
}

const string&
Render::instanceMaterialId() const
{
    return m_material_id;
}

void
Render::setInstanceMaterialId( const string& id )
{
    m_material_id = id;
}

const string&
Render::instanceMaterialTechniqueOverrideSid() const
{
    return m_tech_override_ref;
}

const string&
Render::instanceMaterialTechniqueOverridePassSid() const
{
    return m_tech_override_pass;
}

void
Render::setInstanceMaterialTechniqueOverride( const string& technique_sid,
                                              const string& pass_sid )
{
    m_tech_override_ref = technique_sid;
    m_tech_override_pass = pass_sid;
}

void
Render::setLightNodeId( size_t light_index, const string& id )
{
    m_light_nodes[ light_index ] = id;
}

const string&
Render::lightNodeId( size_t light_index ) const
{
    return m_light_nodes[ light_index ];
}


} // of namespace Scene
