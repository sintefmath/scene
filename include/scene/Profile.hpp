#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "scene/Asset.hpp"
#include "scene/Scene.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {

class Profile : public StructureValueSequences
{
    friend class Effect;
public:
    const Effect*
    effect() const { return m_effect; }

    ~Profile();

    /** Returns info on what type of profile this is (GLSL, GLES, etc.) */
    const ProfileType
    type() const { return m_profile_type; }

    /** Returns an unique id that is suitable for use as a hash-key. */
    const std::string
    key() const;

    void
    deleteTechnique( const std::string& sid );

    Technique*
    createTechnique( const std::string sid );

    const size_t
    techniques() const { return m_techniques.size(); }

    Technique*
    technique( const std::string& sid = "" );

    const Technique*
    technique( const std::string& sid = "" ) const;
    
    Technique*
    technique( const size_t ix ) { return m_techniques[ix]; }

    const Technique*
    technique( const size_t ix ) const { return m_techniques[ix]; }

    const size_t
    parameters() const { return m_parameters.size(); }

    const Parameter*
    parameter(const size_t index ) const { return m_parameters[index]; }

    const Parameter*
    parameter( const std::string& sid ) const;
    
    void
    addParameter( const Parameter& p );

    const Asset&
    asset() const { return m_asset; }

    void
    setAsset( const Asset& asset ) { m_asset = asset; }

protected:
    DataBase&                m_db;

    /** Parent effect. */
    Effect*                  m_effect;
    ProfileType              m_profile_type;
    /** Verbose name of the profile */
    std::string              m_profile_name;

    /** Unique id of the profile, optional. */
    std::string              m_id;
    Asset                    m_asset;

    std::vector<Technique*>  m_techniques;
    std::vector<Parameter*>  m_parameters;

    Profile( DataBase& db,
             Effect*     effect,
             ProfileType profile_type );

};




} // of namespace Scene
