#include <string>
#include <stdexcept>
#include "scene/Material.hpp"
#include "scene/Utils.hpp"
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"

namespace Scene {
    using std::string;
    using std::runtime_error;


static const string package = "Scene.Material";


Material::Material( DataBase&           db,
                    const std::string&  id )
: m_db( db ),
  m_id( id )
{

}

Material::Material( Library<Material>* library_materials, const std::string& id )
    : m_db( *library_materials->dataBase() ),
      m_id( id )
{
}

Material::~Material()
{
    for(auto it=m_set_params.begin(); it!=m_set_params.end(); ++it ) {
        delete it->m_value;
        DEADBEEF( it->m_value );
    }
    m_set_params.clear();
}

const std::string&
Material::effectId() const
{
    return m_effect_id;
}

void
Material::setEffectId( const std::string& effect_id )
{
    m_effect_id = effect_id;
    touchStructureChanged();
    m_db.touchStructureChanged();
}

const std::string&
Material::setParamReference( const size_t ix ) const
{
    return m_set_params[ix].m_reference;
}

const Value*
Material::setParamValue( const size_t ix ) const
{
    return m_set_params[ix].m_value;
}


size_t
Material::techniqueHints() const
{
    return m_technique_hints.size();
}

void
Material::techniqueHint( ProfileType& profile,
                         std::string& platform,
                         std::string& ref,
                         const size_t ix ) const
{
    if( ix < m_technique_hints.size() ) {
        profile = m_technique_hints[ix].m_profile;
        platform = m_technique_hints[ix].m_platform;
        ref = m_technique_hints[ix].m_ref;
    }
}


const std::string
Material::techniqueHint( const ProfileType   profile,
                         const std::string&  platform ) const
{
    Logger log = getLogger( package + ".techniqueHint" );


    // search for exact match
    std::string next_best;

    for(auto i=m_technique_hints.begin(); i!=m_technique_hints.end(); ++i) {
        bool profile_match = (i->m_profile == profile);
        bool platform_match = i->m_platform.empty() || (i->m_platform == platform);
        if( profile_match && platform_match ) {
            return i->m_ref;
        }
        else if( profile_match && next_best.empty() ) {
            next_best = i->m_ref;
        }
    }
    return next_best;


    // search for profile general match
    for(auto i=m_technique_hints.begin(); i!=m_technique_hints.end(); ++i) {
        if( i->m_profile == profile && i->m_platform.empty() ) {
            return i->m_ref;
        }
    }

    static const string none;
    return none;
}

void
Material::addTechniqueHint( const ProfileType   profile,
                            const std::string&  platform,
                            const std::string&  ref )
{
    Logger log = getLogger( "Scene.Material.addTechniqueHint" );
    if( ref.empty() ) {
        SCENELOG_ERROR( log, "Techinque hint with empty reference, ignoring." );
        return;
    }

    TechniqueHint hint;
    hint.m_profile = profile;
    hint.m_platform = platform;
    hint.m_ref = ref;
    m_technique_hints.push_back( hint );
}



void
Material::setParam(const std::string &reference, const Value &value)
{
    Logger log = getLogger( "Scene.Material.setParam" );
    if( !value.defined() ) {
        SCENELOG_ERROR( log, "Value is undefined!" );
        return;
    }

    for(auto p=m_set_params.begin(); p!=m_set_params.end(); ++p) {
        if( p->m_reference == reference ) {
            SCENELOG_TRACE( log, "Updated parameter " << reference <<
                            "of material " << m_id <<
                            ", val=" << value.debugString() );
            // Update value in-place to avoid tainting pointers.
            if( p->m_value->isSampler() || value.isSampler() ) {
                // If it is a sampler, we need to redo the render list
                *p->m_value = value;
                touchStructureChanged();
                m_db.touchStructureChanged();
            }
            else {
                // Otherwise, the render list should pull the data straight
                // from the pointer.
                *p->m_value = value;
                touchValueChanged();
            }
            return;
        }
    }

    m_set_params.resize( m_set_params.size() +1 );
    m_set_params.back().m_reference = reference;
    m_set_params.back().m_value = new Value( value );
    SCENELOG_TRACE( log, "Created parameter " << reference <<
                    " of material " << m_id <<
                    ", val=" << value.debugString() );

    touchStructureChanged();
    m_db.touchStructureChanged();
}


}
