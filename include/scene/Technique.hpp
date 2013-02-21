#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "scene/Asset.hpp"
#include "scene/DataBase.hpp"
#include "scene/CommonShadingModel.hpp"
#include "scene/Scene.hpp"
#include "scene/Pass.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {

class Technique : public StructureValueSequences
{
    friend class Profile;
public:

    const Profile*
    profile() const { return m_profile; }

    ~Technique();

    const std::string&
    sid() const { return m_sid; }

    /** Returns an unique id that is suitable for use as a hash-key. */
    const std::string
    key() const;

    /** Returns number of passes of this technique. */
    const size_t
    passes() const { return m_passes.size(); }

    /** Find a particular pass in this technique with the given sid. */
    const Pass*
    pass( const std::string& sid ) const;

    const Pass*
    pass( const size_t index ) const { return m_passes.at( index ); }

    CommonShadingModel*
    createCommonShadingModel( const ShadingModelType model );

    const CommonShadingModel*
    commonShadingModel() const { return m_common_shading_model; }

    Pass*
    createPass( const std::string& sid = "" );

    const Asset&
    asset() const { return m_asset; }

    void
    setAsset( const Asset& asset ) { m_asset = asset; }

protected:
    DataBase&                   m_db;
    Effect*                     m_effect;
    Profile*                    m_profile;
    //TimeStamp                   m_timestamp;
    Asset                       m_asset;

    /** Unique id of this technique, optional. */
    std::string                 m_id;


    /** Scoped id w.r.t. effect, required. */
    std::string                 m_sid;
    std::vector<Pass*>          m_passes;
    CommonShadingModel*         m_common_shading_model;

    Technique( DataBase&  db,
               Effect*    effect,
               Profile*   profile,
               const std::string& sid );

};



}
