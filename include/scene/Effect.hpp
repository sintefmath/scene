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
#include <string>
#include <unordered_map>

#include "scene/Asset.hpp"
#include "scene/Scene.hpp"
#include "scene/Parameter.hpp"
#include "scene/Technique.hpp"
#include "scene/Pass.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {

class Effect : public StructureValueSequences
{
    friend class Library<Effect>;

    friend class Scene::Collada::Importer;

public:
    Effect( DataBase&  db, const std::string& id );

    ~Effect();

    /** Returns the COLLADA-related id. */
    const std::string&
    id() const { return m_id; }

    /** Returns an unique id that is suitable for use as a hash-key. */
    const std::string&
    key() const;


    const Profile*
    profile( ProfileType type ) const;

    Profile*
    profile( ProfileType type );
    
    
    Profile*
    createProfile( ProfileType );


    const Parameter*
    parameter( const size_t index ) const;
    
    const Parameter*
    parameter( const std::string& sid ) const;

    const size_t
    parameters() const { return m_parameters.size(); }

    void
    addParameter( const Parameter& p );


    const Asset&
    asset() const { return m_asset; }

    void
    setAsset( const Asset& asset ) { m_asset = asset; }


protected:
    DataBase&                m_db;

    /** The unique id of the effect, required. */
    const std::string        m_id;
    Asset                    m_asset;
    std::string              m_name;
    std::vector<Parameter*>  m_parameters;

    Profile*                 m_profile_common;
    Profile*                 m_profile_glsl;
    Profile*                 m_profile_gles;
    Profile*                 m_profile_gles2;

    Effect( Library<Effect>* library_effects, const std::string& id );


};

} // of namespace Scene
