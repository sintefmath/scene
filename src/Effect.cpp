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

#include <stdexcept>
#include <vector>
#include "scene/DataBase.hpp"
#include "scene/Primitives.hpp"
#include "scene/Utils.hpp"
#include "scene/Profile.hpp"
#include "scene/Log.hpp"

namespace Scene {
    using std::vector;
    using std::string;
    using std::runtime_error;
    using std::unordered_map;

static const std::string package = "Scene.Effect";

Effect::Effect( DataBase&  db, const string& id )
: m_db( db ),
  m_id( id ),
  m_profile_common( NULL ),
  m_profile_glsl( NULL ),
  m_profile_gles( NULL ),
  m_profile_gles2( NULL )
{
}

Effect::Effect( Library<Effect>* library_effects, const std::string& id )
    : m_db( *library_effects->dataBase() ),
      m_id( id ),
      m_profile_common( NULL ),
      m_profile_glsl( NULL ),
      m_profile_gles( NULL ),
      m_profile_gles2( NULL )
{
}




Effect::~Effect( )
{
    if( m_profile_common != NULL) {
        delete m_profile_common;
    }
    if( m_profile_glsl != NULL ) {
        delete m_profile_glsl;
    }
    if( m_profile_gles != NULL ) {
        delete m_profile_gles;
    }
    if( m_profile_gles2 != NULL ) {
        delete m_profile_gles2;
    }
    for(auto it=m_parameters.begin(); it!=m_parameters.end(); ++it ) {
        delete *it;
    }
}


const Parameter*
Effect::parameter( const size_t index ) const
{
    Logger log = getLogger( "Scene.Effect.parameter" );

    SCENELOG_TRACE( log, m_id << ": " << m_parameters[index]->value()->debugString() );


    return m_parameters[index];
}


void
Effect::addParameter( const Parameter& p )
{
    Logger log = getLogger( "Scene.Effect.addParameter" );
    if( !p.sid().empty() ) {
        for( auto it=m_parameters.begin(); it!=m_parameters.end(); ++it ) {
            if( (*it)->sid() == p.sid() ) {
                if( ((*it)->semantic() == p.semantic()) && (((*it)->value()->type() == p.value()->type() )) ) {
                    // do nothing
                    return;
                }
                else {
                    SCENELOG_WARN( log, "Parameter sid='"<<p.sid() <<"' with slightly different definition already exists in effect" );
                    return;
                }
            }
        }
    }
    m_parameters.push_back( new Parameter(p) );
    SCENELOG_DEBUG( log, "Adding parameter " <<
                    "sid='" << m_parameters.back()->sid() <<
                    "', semantic=" << std::hex << m_parameters.back()->semantic() << std::dec <<
                    ", value=" << m_parameters.back()->value()->debugString() );


}

const std::string&
Effect::key() const
{
    return m_id;
}

Profile*
Effect::createProfile( ProfileType type )
{
    Logger log = getLogger( "Scene.Effect.createProfile" );
    if( profile( type ) != NULL ) {
        SCENELOG_ERROR( log, "Profile "
                        << std::hex << type << std::dec <<
                        " is already defined." );
        return NULL;
    }
    if( type == PROFILE_COMMON ) {
        m_profile_common = new Profile( m_db, this, PROFILE_COMMON );
        return m_profile_common;
    }
    if( type == PROFILE_GLSL ) {
        m_profile_glsl = new Profile( m_db, this, PROFILE_GLSL );
        return m_profile_glsl;
    }
    else if( type == PROFILE_GLES ) {
        m_profile_gles = new Profile( m_db, this, PROFILE_GLES );
        return m_profile_gles;
    }
    else if( type == PROFILE_GLES2 ) {
        m_profile_gles2 = new Profile( m_db, this, PROFILE_GLES2 );
        return m_profile_gles2;
    }
    SCENELOG_ERROR( log, "Profile " << std::hex << type << std::dec << " not supported yet." );
    return NULL;
}

const Profile*
Effect::profile( ProfileType type ) const
{
    switch( type ) {
    case PROFILE_COMMON:
        return m_profile_common;
    case PROFILE_GLSL:
        return m_profile_glsl;
    case PROFILE_GLES:
        return m_profile_gles;
    case PROFILE_GLES2:
        return m_profile_gles2;
    default:
        return NULL;
    }
}

Profile*
Effect::profile( ProfileType type )
{
    switch( type ) {
    case PROFILE_COMMON:
        return m_profile_common;
    case PROFILE_GLSL:
        return m_profile_glsl;
    case PROFILE_GLES:
        return m_profile_gles;
    case PROFILE_GLES2:
        return m_profile_gles2;
    default:
        return NULL;
    }
}


const Parameter*
Effect::parameter( const std::string& sid ) const
{
    if( sid.empty() ) {
        return NULL;
    }
    for( auto it=m_parameters.begin(); it!=m_parameters.end(); ++it ) {
        if( (*it)->sid() == sid ) {
            return *it;
        }
    }
    return NULL;
}



} // of namespace Scene
