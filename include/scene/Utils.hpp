#pragma once

#include <string>
#include <vector>
#include <libxml/tree.h>
#include "scene/Scene.hpp"

namespace Scene {

    const std::string
    asString( int );

    const std::string
    asString( unsigned int );

    const std::string
    asString( unsigned long int );

    const std::string
    profileName( ProfileType profile );

    const std::string
    uniformSemantic( RuntimeSemantic semantic );

    const std::string
    elementType( ElementType element_type );

    const std::string
    valueType( ValueType type );

    VertexSemantic
    vertexSemantic( const std::string& semantic );

    const std::string
    vertexSemantic( VertexSemantic semantic );

    const std::string
    stateTypeString( const StateType type );


    /** Converts a runtime semantic enum to a human readable string. */
    const std::string
    runtimeSemantic( const RuntimeSemantic semantic );

    /** Converts a human readable string to a runtime semantic enum. */
    RuntimeSemantic
    runtimeSemantic( const std::string& semantic_string );


    ShaderStage
    shaderStage( const std::string& stage );

    const std::string
    shaderStage( ShaderStage stage );


    namespace GL
    {

        const GLboolean
        booleanValue( const std::string& value );

        const std::string
        booleanString( GLboolean boolean );

        GLenum
        blendFuncValue( const std::string& value );

        const std::string
        blendFuncString( GLenum blend_func );

        GLenum
        faceValue( const std::string& value );

        const std::string
        faceString( GLenum face );

        GLenum
        polygonModeValue( const std::string& value );

        const std::string
        polygonModeString( GLenum mode );


        GLenum
        shaderStageGLenum( ShaderStage stage );

        GLenum
        parseGLenum( const std::string& text );

        std::string
        enumString( GLenum e );

    }



}
