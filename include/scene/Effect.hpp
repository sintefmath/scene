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
    createProfile( ProfileType );


    /** Generate shaders from COMMON profile. */
    void
    generate( ProfileType profile );

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
