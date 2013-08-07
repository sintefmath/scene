#include "scene/Profile.hpp"
#include "scene/Technique.hpp"
#include "scene/Utils.hpp"
#include "scene/Log.hpp"
#include <stdexcept>
#include <unordered_map>

namespace Scene {
    using std::string;
    using std::runtime_error;
    using std::unordered_map;


Profile::Profile( DataBase& db, Effect* effect, ProfileType profile_type )
    : m_db(db),
      m_effect( effect ),
      m_profile_type(profile_type)
{
    switch( m_profile_type ) {
    case PROFILE_BRIDGE:
        m_profile_name = "BRIDGE";
        break;
    case PROFILE_CG:
        m_profile_name = "CG";
        break;
    case PROFILE_COMMON:
        m_profile_name = "COMMON";
        break;
    case PROFILE_GLES:
        m_profile_name = "GLES";
        break;
    case PROFILE_GLES2:
        m_profile_name = "GLES2";
        break;
    case PROFILE_GLSL:
        m_profile_name = "GLSL";
        break;
    }
}

Profile::~Profile()
{
    for( auto it=m_techniques.begin(); it!=m_techniques.end(); ++it) {
        delete *it;
    }
}

const std::string
Profile::key() const
{
    if( m_id.empty() ) {
        return m_effect->key() + "/" + m_profile_name;
    }
    else {
        return m_id;
    }
}


Technique*
Profile::createTechnique( const std::string sid )
{
    Logger log = getLogger( "Scene.Profile.createTechnique" );
    if( sid.empty() ) {
        SCENELOG_ERROR( log, "non-empty sid required." );
        return NULL;
    }
    for(size_t i=0; i<m_techniques.size(); i++) {
        if( m_techniques[i]->sid() == sid ) {
            SCENELOG_ERROR( log, "Technique with sid='" << sid << "' already exists.");
            return NULL;
        }
    }


    Technique* technique = new Technique( m_db,
                                          m_effect,
                                          this,
                                          sid );
    m_techniques.push_back( technique );
    return technique;
}

void
Profile::deleteTechnique( const std::string& sid )
{
    for( auto it=m_techniques.begin(); it!=m_techniques.end(); ++it ) {
        if( (*it)->sid() == sid ) {
            m_techniques.erase( it );
            break;
        }
    }
}


const Technique*
Profile::technique( const std::string& sid ) const
{
    Logger log = getLogger( "Scene.Profile.technique" );

    if( m_techniques.empty() ) {
        SCENELOG_ERROR( log, "No techniques specified for profile '"<< m_id << '\'' );
        return NULL;
    }

    // Empty sid, return first technique
    if( sid.empty() ) {
        return m_techniques[0];
    }

    // Otherwise, search through sids.
    for( auto i=m_techniques.begin(); i!=m_techniques.end(); ++i ) {
        if( (*i)->sid() == sid ) {
            return *i;
        }
    }
    return NULL;
}


Technique*
Profile::technique( const std::string& sid )
{
    Logger log = getLogger( "Scene.Profile.technique" );

    if( m_techniques.empty() ) {
        SCENELOG_ERROR( log, "No techniques specified for profile '"<< m_id << '\'' );
        return NULL;
    }

    // Empty sid, return first technique
    if( sid.empty() ) {
        return m_techniques[0];
    }

    // Otherwise, search through sids.
    for( auto i=m_techniques.begin(); i!=m_techniques.end(); ++i ) {
        if( (*i)->sid() == sid ) {
            return *i;
        }
    }
    return NULL;
}


void
Profile::addParameter( const Parameter& p )
{
    Logger log = getLogger( "Scene.Profile.addParameter" );
    m_parameters.push_back( new Parameter(p) );

    SCENELOG_DEBUG( log, "Adding parameter " <<
                    "sid='" << m_parameters.back()->sid() <<
                    "', semantic=" << std::hex << m_parameters.back()->semantic() << std::dec <<
                    ", value=" << m_parameters.back()->value()->debugString() );

    touchStructureChanged();
    m_effect->moveForward( *this );
    m_db.library<Effect>().moveForward( *this );
    m_db.moveForward( *this );
}

const Parameter*
Profile::parameter( const std::string& sid ) const
{
    if( sid.empty() ) {
        return NULL;
    }
    for( auto it=m_parameters.begin(); it!=m_parameters.end(); ++it ) {
        if( (*it)->sid() == sid ) {
            return *it;
        }
    }
    return m_effect->parameter( sid );
}


}
