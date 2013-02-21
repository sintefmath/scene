#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <string>
#include <vector>
#include "scene/Asset.hpp"
#include "scene/Scene.hpp"
#include "scene/Value.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {



class Material : public StructureValueSequences
{
    friend class Library<Material>;
public:

    Material( DataBase& db, const std::string& id );

    ~Material();

    const std::string&
    id() const { return m_id; }

    const Asset&
    asset() const { return m_asset; }

    void
    setParam( const std::string&  reference,
              const Value&        value );


    const std::string&
    effectId() const;

    void
    setEffectId( const std::string& effect_id );

    const size_t
    setParams() const { return m_set_params.size(); }


    const std::string&
    setParamReference( const size_t ix ) const;

    const Value*
    setParamValue( const size_t ix ) const;


    void
    setAsset( const Asset& asset ) { m_asset = asset; }

    /** Try to resolve effect using technique hints. */
    const std::string
    techniqueHint( const ProfileType   profile,
                   const std::string&  platform ) const;


    /** Return the number of technique hints set. */
    size_t
    techniqueHints() const;

    /** Return a given technique hint. */
    void
    techniqueHint( ProfileType& profile,
                   std::string& platform,
                   std::string& ref,
                   const size_t ix ) const;

    void
    addTechniqueHint( const ProfileType   profile,
                      const std::string&  platform,
                      const std::string&  ref );

protected:
    DataBase&                 m_db;
    Asset                     m_asset;
    std::string               m_id;
    std::string               m_name;
    std::string               m_effect_id;
    struct TechniqueHint {
        ProfileType           m_profile;
        std::string           m_platform;
        std::string           m_ref;
    };
    std::vector<TechniqueHint> m_technique_hints;
    struct SetParam
    {
        std::string    m_reference;
        Value*         m_value;
    };

    std::vector<SetParam>     m_set_params;

    Material( Library<Material>* library_materials, const std::string& id );

};


} // of namespace Scene
#endif // MATERIAL_HPP
