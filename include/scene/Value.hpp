#pragma once
#include <vector>
#include <string>

#include <scene/Log.hpp>
#include <scene/Scene.hpp>
#include <scene/SeqPos.hpp>

namespace Scene {
    namespace Runtime {
        class TransformCache;
        class TransformCompute;
    }

/** Class to hold a value of any type. */
class Value
{
    friend class Runtime::TransformCache;
    friend class Runtime::TransformCompute;
public:
    Value() : m_type( VALUE_TYPE_N ) {}

    //Value( const Value& value );

    void
    undefine() { m_type = VALUE_TYPE_N; }

    SeqPos&
    valueChanged() { return m_value_changed; }

    const SeqPos&
    valueChanged() const { return m_value_changed; }

    static Value
    createInt( const int val );

    static Value
    createFloat( const float val );

    static Value
    createFloat2( const float float0, const float float1 );

    static Value
    createFloat3( const float float0, const float float1, const float float2 );

    static Value
    createFloat4( const float float0, const float float1, const float float2, const float float3 );

    static Value
    createFloat4x4( );


    /** Create a 3x3 matrix directly from a column-major array (OpenGL layout). */
    static Value
    createFloat3x3( );

    static Value
    createFloat3x3( const float* src );

    /** Create a 3x3 matrix from row-major arguments. */
    static Value
    createFloat3x3( const float e00, const float e01, const float e02,
                    const float e10, const float e11, const float e12,
                    const float e20, const float e21, const float e22 );

    /** Create a 4x4 matrix directly from a column-major array (OpenGL layout). */
    static Value
    createFloat4x4( const float* src );

    /** Create a 4x4 matrix from row-major arguments. */
    static Value
    createFloat4x4( const float e00, const float e01, const float e02, const float e03,
                    const float e10, const float e11, const float e12, const float e13,
                    const float e20, const float e21, const float e22, const float e23,
                    const float e30, const float e31, const float e32, const float e33 );

    static Value
    createBool( const GLboolean bool0 );

    static Value
    createEnum( const GLenum enum0 );

    static Value
    createEnum2( const GLenum enum0, const GLenum enum1 );

    static Value
    createSampler2D( const std::string instance_image,
                     const GLenum wrap_s,
                     const GLenum wrap_t,
                     const GLenum min_filter,
                     const GLenum mag_filter );

    static Value
    createSampler3D( const std::string instance_image,
                     const GLenum wrap_s,
                     const GLenum wrap_t,
                     const GLenum wrap_p,
                     const GLenum min_filter,
                     const GLenum mag_filter );

    static Value
    createSamplerCUBE( const std::string instance_image,
                       const GLenum wrap_s,
                       const GLenum wrap_t,
                       const GLenum min_filter,
                       const GLenum mag_filter );

    static Value
    createSamplerDEPTH( const std::string instance_image,
                        const GLenum wrap_s,
                        const GLenum wrap_t,
                        const GLenum min_filter,
                        const GLenum mag_filter );




    void
    set( const ValueType type );

    /** Returns true if this value has a type. */
    bool
    defined() const { return m_type != VALUE_TYPE_N; }

    /** Get the type of this value. */
    ValueType
    type() const { return m_type; }

    /** Directly access internal integer storage array.
      *
      * \note It is the applications resposibility to assert that this type is
      * an int-based type.
      */
    const int*
    intData() const
    {
#ifdef SCENE_CHECK_TYPES
        if(!( (m_type == VALUE_TYPE_INT) ) )
        {
            Logger log = getLogger( "Scene.Value.intData" );
            SCENELOG_FATAL( log, "Value has illegal type, " << debugString() );
        }
#endif
        return &m_payload.m_ints[0];
    }

    /** Directly access internal float storage array.
      *
      * \note It is the applications resposibility to assert that this type is
      * an float-based type.
      */
    const float*
    floatData() const
    {
#ifdef SCENE_CHECK_TYPES
        if(!( (m_type == VALUE_TYPE_FLOAT) ||
              (m_type == VALUE_TYPE_FLOAT2) ||
              (m_type == VALUE_TYPE_FLOAT3) ||
              (m_type == VALUE_TYPE_FLOAT4) ||
              (m_type == VALUE_TYPE_FLOAT3X3) ||
              (m_type == VALUE_TYPE_FLOAT4X4) ) )
        {
            Logger log = getLogger( "Scene.Value.floatData" );
            SCENELOG_FATAL( log, "Value has illegal type, " << debugString() );
            if(1) {
                unsigned int* foo = static_cast<unsigned int*>( 0x0 );
                *foo = 42;
            }
        }
#endif
        return &m_payload.m_floats[0];
    }

    /** Directly access internal bool storage array.
      *
      * \note It is the applications resposibility to assert that this type is
      * an bool-based type.
      */
    const GLboolean*
    boolData() const
    {
#ifdef SCENE_CHECK_TYPES
        if(!( (m_type == VALUE_TYPE_BOOL) ) )
        {
            Logger log = getLogger( "Scene.Value.boolData" );
            SCENELOG_FATAL( log, "Value has illegal type, " << debugString() );
        }
#endif
        return &m_payload.m_bools[0];
    }

    /** Directly access internal enum storage array.
      *
      * \note It is the applications resposibility to assert that this type is
      * an enum-based type.
      */
    const GLenum*
    enumData() const
    {
#ifdef SCENE_CHECK_TYPES
        if(!( (m_type == VALUE_TYPE_ENUM) ||
              (m_type == VALUE_TYPE_ENUM2) ) )
        {
            Logger log = getLogger( "Scene.Value.enumData" );
            SCENELOG_FATAL( log, "Value has illegal type, " << debugString() );
        }
#endif
        return &m_payload.m_enums[0];
    }

    /** Returns a pretty-print string of the contents, nice for debugging. */
    const std::string
    debugString() const;

    /** Returns true if value is a sampler type. */
    bool
    isSampler() const;

    const std::string&
    samplerInstanceImage() const { return m_instance_image; }

    const GLenum
    samplerWrapS() const { return m_payload.m_sampler.m_wrap_s; }

    const GLenum
    samplerWrapT() const { return m_payload.m_sampler.m_wrap_s; }

    const GLenum
    samplerWrapP() const { return m_payload.m_sampler.m_wrap_s; }

    const GLenum
    samplerMinFilter() const { return m_payload.m_sampler.m_min_filter; }

    const GLenum
    samplerMagFilter() const { return m_payload.m_sampler.m_mag_filter; }

protected:
    struct Sampler {
        GLenum      m_wrap_s;
        GLenum      m_wrap_t;
        GLenum      m_wrap_p;
        GLenum      m_min_filter;
        GLenum      m_mag_filter;
    };

    union {
#ifdef __SSE4_2__
#ifdef _WIN32
        __declspec(align(16)) float  m_floats[16];
#else
        float       m_floats[16] __attribute__((aligned(16)));
#endif
#else
        float       m_floats[16];
#endif
        GLenum      m_enums[4];
        GLboolean   m_bools[1];
        int         m_ints[1];
        Sampler     m_sampler;
    }               m_payload;
    ValueType       m_type;
    SeqPos          m_value_changed;

    std::string     m_instance_image;  // used by sampler. Has a constructor, so
                                     // it cannot reside inside an union.

    const std::string
    enums( size_t count ) const;

    const std::string
    ints( size_t count ) const;

    const std::string
    floats( size_t count, bool abbreviate=false ) const;

    const std::string
    samplerString( ) const;

    const std::string
    bools( size_t count ) const;

    bool
    setBools( const std::string& source, const size_t count );


};

} // of namespace Scene
