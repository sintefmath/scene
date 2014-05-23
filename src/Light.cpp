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

#include "scene/Log.hpp"
#include "scene/Light.hpp"
#include "scene/Library.hpp"
#include "scene/DataBase.hpp"

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif


namespace Scene {
    using std::string;

static const string package = "Scene.Light";

Light::Light(Library<Light>* library_lights, const std::string id)
    : m_library_lights( library_lights ),
      m_id( id ),
      m_type( LIGHT_NONE ),
      m_color( Value::createFloat3(1.f, 1.f, 1.f) ),
      m_constant_attenuation( Value::createFloat(1.0) ),
      m_linear_attenuation( Value::createFloat( 0.0 ) ),
      m_quadratic_attenuation( Value::createFloat( 0.0 ) ),
      m_falloff_angle( Value::createFloat( M_PI ) ),
      m_falloff_exponent( Value::createFloat( 0.0 ) )
{
    touchStructureChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}

Light::~Light()
{
    touchStructureChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}

void
Light::setAsset( const Asset& parent_asset )
{
    m_asset = parent_asset;
}

const Light::Type
Light::type() const
{
    return m_type;
}

void
Light::setType( Type type )
{
    m_type = type;
    touchStructureChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}

const Value*
Light::color() const
{
    if( m_type == LIGHT_NONE ) {
        Logger log = getLogger( package + ".color" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have color property." );
    }
    return &m_color;
}

void
Light::setColor( float red, float green, float blue )
{
    if( m_type == LIGHT_NONE ) {
        Logger log = getLogger( package + ".color" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the color property." );
    }
    m_color = Value::createFloat3( red, green, blue );
    touchValueChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}

const Value*
Light::constantAttenuation() const
{
    if( m_type != LIGHT_POINT && m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".constantAttenuation" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the constant attenuation property." );
    }
    return &m_constant_attenuation;
}

void
Light::setConstantAttenuation( float constant_attenuation )
{
    if( m_type != LIGHT_POINT && m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".setConstantAttenuation" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the constant attenuation property." );
    }
    m_constant_attenuation = Value::createFloat( constant_attenuation );
    touchValueChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}

const std::string&
Light::constantAttenuationSid() const
{
    if( m_type != LIGHT_POINT && m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".constantAttenuationSid" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the constant attenuation property." );
    }
    return m_constant_attenuation_sid;
}

void
Light::setConstantAttenuationSid( const std::string& sid )
{
    if( m_type != LIGHT_POINT && m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".setConstantAttenuationSid" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the constant attenuation property." );
    }
    m_constant_attenuation_sid = sid;
    touchStructureChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}


const Value*
Light::linearAttenuation() const
{
    if( m_type != LIGHT_POINT && m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".linearAttenuation" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the linear attenuation property." );
    }
    return &m_linear_attenuation;
}

void
Light::setLinearAttenuation( float linear_attenuation )
{
    if( m_type != LIGHT_POINT && m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".setLinearAttenuation" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the linear attenuation property." );
    }
    m_linear_attenuation = Value::createFloat( linear_attenuation );
    touchValueChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}


const std::string&
Light::linearAttenuationSid() const
{
    if( m_type != LIGHT_POINT && m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".linearAttenuationSid" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the linear attenuation property." );
    }
    return m_linear_attenuation_sid;
}

void
Light::setLinearAttenuationSid( const std::string& sid )
{
    if( m_type != LIGHT_POINT && m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".setLinearAttenuationSid" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the linear attenuation property." );
    }
    m_linear_attenuation_sid = sid;
    touchStructureChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}


const Value*
Light::quadraticAttenuation() const
{
    if( m_type != LIGHT_POINT && m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".quadraticAttenuation" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the quadratic attenuation property." );
    }
    return &m_quadratic_attenuation;
}

void
Light::setQuadraticAttenuation( float linear_attenuation )
{
    if( m_type != LIGHT_POINT && m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".setQuadraticAttenuation" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the quadratic attenuation property." );
    }
    m_quadratic_attenuation = Value::createFloat( linear_attenuation );
    touchValueChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}

const std::string&
Light::quadraticAttenuationSid() const
{
    if( m_type != LIGHT_POINT && m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".quadraticAttenuationSid" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the quadratic attenuation property." );
    }
    return m_quadratic_attenuation_sid;
}

void
Light::setQuadraticAttenuationSid( const std::string& sid )
{
    if( m_type != LIGHT_POINT && m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".setQuadraticAttenuationSid" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the quadratic attenuation property." );
    }
    m_quadratic_attenuation_sid = sid;
    touchStructureChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}

const Value*
Light::falloffAngle() const
{
    if( m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".falloffAngle" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the falloff angle property." );
    }
    return &m_falloff_angle;
}

void
Light::setFalloffAngle( float angle )
{
    if( m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".setFalloffAngle" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the falloff angle property." );
    }
    m_falloff_angle = Value::createFloat( angle );
    touchValueChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}

const std::string&
Light::falloffAngleSid( ) const
{
    if( m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".falloffAngleSid" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the falloff angle property." );
    }
    return m_falloff_angle_sid;
}

void
Light::setFalloffAngleSid( const std::string& sid )
{
    if( m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".setFalloffAngleSid" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the falloff angle property." );
    }
    m_falloff_angle_sid = sid;
    touchStructureChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}

const Value*
Light::falloffExponent() const
{
    if( m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".falloffExponent" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the falloff exponent property." );
    }
    return &m_falloff_exponent;
}

void
Light::setFalloffExponent( float exponent )
{
    if( m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".setFalloffExponent" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the falloff exponent property." );
    }
    m_falloff_exponent = Value::createFloat( exponent );
    touchValueChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}

const std::string&
Light::falloffExponentSid() const
{
    if( m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".falloffExponentSid" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the falloff exponent property." );
    }
    return m_falloff_exponent_sid;
}

void
Light::setFalloffExponentSid( const std::string& sid )
{
    if( m_type != LIGHT_SPOT ) {
        Logger log = getLogger( package + ".setFalloffExponentSid" );
        SCENELOG_WARN( log, "Light id='" << m_id << "'does not have the falloff exponent property." );
    }
    m_falloff_exponent_sid = sid;
    touchValueChanged();
    m_library_lights->moveForward( *this );
    m_library_lights->dataBase()->moveForward( *this );
}


} // of namespace Scene
