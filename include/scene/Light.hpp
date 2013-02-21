#pragma once
#include <string>

#include <scene/Scene.hpp>
#include <scene/Asset.hpp>
#include <scene/Value.hpp>
#include <scene/SeqPos.hpp>

namespace Scene {


/** Class that specifies a light source.
  *
  * To position and orient a light source in a scene, use instance_light inside
  * a node element of the scene graph. The local coordinate system of the node
  * will be used for the light source in that context.
  *
  * To use the light source in a shader, use bind in
  *
  *
  */
class Light : public StructureValueSequences
{
    friend class Library<Light>;
public:
    enum Type {
        /** Unspecified light source. */
        LIGHT_NONE,
        /** Emits light equally in all directions. Position and direction is irrelevant. */
        LIGHT_AMBIENT,
        /** Emits light along the negative z-axis of the local coordinate system. Position is irrelevant. */
        LIGHT_DIRECTIONAL,
        /** Emits the light from the origin of the local coordinate system. Orientation is irrelevant. */
        LIGHT_POINT,
        /** Emits the light from the origin and along the negative z-axis of the local coordinate system. */
        LIGHT_SPOT
    };


    const std::string&
    id() const { return m_id; }

    const Asset&
    asset() const { return m_asset; }

    void
    setAsset( const Asset& parent_asset );

    /** Returns the type of the light source. */
    const Type
    type() const;

    /** Sets the type of the light source. */
    void
    setType( Type type );



    /** Returns a pointer to the color of this light.
      *
      * The color is a float3 value. Pointer is valid for the lifetime of the
      * light object.
      *
      * Valid for types LIGHT_AMBIENT, LIGHT_DIRECTIONAL, LIGHT_POINT, LIGHT_SPOT.
      */
    const Value*
    color() const;

    /** Sets the color of this light source. */
    void
    setColor( float red, float green, float blue );

    /** Returns a pointer to the constant attenuation factor of this light.
      *
      * Value is a float-value. Defaults to 1.0. Pointer is valid for the
      * lifetime of the light object.
      *
      * Valid for types LIGHT_POINT, LIGHT_SPOT.
      */
    const Value*
    constantAttenuation() const;

    /** Sets the constant attenuation of this light source. */
    void
    setConstantAttenuation( float constant_attenuation );

    /** Returns the optional SID for the constant attenuation factor. */
    const std::string&
    constantAttenuationSid() const;

    /** Sets the optional SID for the constant attenuation factor. */
    void
    setConstantAttenuationSid( const std::string& sid );

    /** Returns a pointer to the linear attenuation factor of this light.
      *
      * Value is a float value. Defaults to 0.0. Pointer is valid for the
      * lifetime of the light object.
      *
      * Valid for types LIGHT_POINT, LIGHT_SPOT.
      */
    const Value*
    linearAttenuation() const;

    /** Sets the linear attenuation of a light source. */
    void
    setLinearAttenuation( float linear_attenuation );

    /** Returns the optional SID for the linear attenuation factor. */
    const std::string&
    linearAttenuationSid() const;

    /** Sets the optional SID for the linear attenuation factor. */
    void
    setLinearAttenuationSid( const std::string& sid );


    /** Returns a pointer to the quadratic attenuation factor of this light.
      *
      * Value is a float value. Defaults to 0.0. Pointer is valid for the
      * lifetime of the light object.
      *
      * Valid for types LIGHT_POINT, LIGHT_SPOT.
      */
    const Value*
    quadraticAttenuation() const;

    /** Sets the quadratic attenuation of a light source. */
    void
    setQuadraticAttenuation( float linear_attenuation );

    /** Returns the optional SID for the quadratic attenuation factor. */
    const std::string&
    quadraticAttenuationSid() const;

    /** Sets the optional SID for the quadratic attenuation factor. */
    void
    setQuadraticAttenuationSid( const std::string& sid );


    /** Returns a pointer to the falloff angle of a light source.
      *
      * Value is a float value in radians. Defaults to PI. Pointer is valid for
      * the lifetime of the light object.
      *
      * Valid for types LIGHT_SPOT.
      */
    const Value*
    falloffAngle() const;

    /** Sets the falloff angle for the light source. */
    void
    setFalloffAngle( float angle );

    /** Returns the optional SID for the falloff angle. */
    const std::string&
    falloffAngleSid( ) const;

    /** Sets the optional SID for the falloff angle. */
    void
    setFalloffAngleSid( const std::string& sid );

    /** Returns a pointer to the falloff exponent of a light source.
      *
      * Value is a float value. Defaults to 0.0. Pointer is valid for the
      * lifetime of the light object.
      *
      * Valid for types LIGHT_SPOT.
      */
    const Value*
    falloffExponent() const;

    /** Sets the falloff exponent of the light source. */
    void
    setFalloffExponent( float exponent );

    /** Returns the optional SID for the falloff exponent. */
    const std::string&
    falloffExponentSid() const;

    /** Sets the optional SID for the falloff exponent. */
    void
    setFalloffExponentSid( const std::string& sid );




protected:
    Library<Light>*  m_library_lights;
    std::string      m_id;
    Type             m_type;
    Asset            m_asset;
    Value            m_color;
    Value            m_constant_attenuation;
    std::string      m_constant_attenuation_sid;
    Value            m_linear_attenuation;
    std::string      m_linear_attenuation_sid;
    Value            m_quadratic_attenuation;
    std::string      m_quadratic_attenuation_sid;
    Value            m_falloff_angle;
    std::string      m_falloff_angle_sid;
    Value            m_falloff_exponent;
    std::string      m_falloff_exponent_sid;

    // Only Library<Light> may create light objects
    Light( Library<Light>* library_lights, const std::string id );

    // Only Library<Light> may delete light objects
    ~Light();

};





} // of namespace Scene
