#include <iostream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "scene/Utils.hpp"
#include "scene/Log.hpp"
#include <unordered_map>

namespace Scene {
    using std::string;
    using std::stringstream;
    using std::unordered_map;
    using std::vector;

static const string package = "Scene";

// This list must match the enum in Scene.hpp
static const std::string runtime_semantic_names[ RUNTIME_SEMANTIC_N+1 ] =
{
    "FRAMEBUFFER_SIZE",
    "FRAMEBUFFER_SIZE_RECIPROCAL",
    "MODELVIEW_MATRIX",
    "PROJECTION_MATRIX",
    "PROJECTION_INVERSE_MATRIX",
    "MODELVIEW_PROJECTION_MATRIX",
    "NORMAL_MATRIX",
    "WORLD_FROM_OBJECT",
    "OBJECT_FROM_WORLD",
    "EYE_FROM_WORLD",
    "WORLD_FROM_EYE",
    "WORLD_FROM_CLIP",
    "LIGHT0_COLOR",
    "LIGHT1_COLOR",
    "LIGHT2_COLOR",
    "LIGHT3_COLOR",
    "LIGHT0_CONSTANT_ATT",
    "LIGHT1_CONSTANT_ATT",
    "LIGHT2_CONSTANT_ATT",
    "LIGHT3_CONSTANT_ATT",
    "LIGHT0_LINEAR_ATT",
    "LIGHT1_LINEAR_ATT",
    "LIGHT2_LINEAR_ATT",
    "LIGHT3_LINEAR_ATT",
    "LIGHT0_QUADRATIC_ATT",
    "LIGHT1_QUADRATIC_ATT",
    "LIGHT2_QUADRATIC_ATT",
    "LIGHT3_QUADRATIC_ATT",
    "LIGHT0_FALLOFF_COS",
    "LIGHT1_FALLOFF_COS",
    "LIGHT2_FALLOFF_COS",
    "LIGHT3_FALLOFF_COS",
    "LIGHT0_FALLOFF_EXPONENT",
    "LIGHT1_FALLOFF_EXPONENT",
    "LIGHT2_FALLOFF_EXPONENT",
    "LIGHT3_FALLOFF_EXPONENT",
    "LIGHT0_POS_OBJECT",
    "LIGHT1_POS_OBJECT",
    "LIGHT2_POS_OBJECT",
    "LIGHT3_POS_OBJECT",
    "LIGHT0_POS_WORLD",
    "LIGHT1_POS_WORLD",
    "LIGHT2_POS_WORLD",
    "LIGHT3_POS_WORLD",
    "LIGHT0_POS_EYE",
    "LIGHT1_POS_EYE",
    "LIGHT2_POS_EYE",
    "LIGHT3_POS_EYE",
    "LIGHT0_Z_OBJECT",
    "LIGHT1_Z_OBJECT",
    "LIGHT2_Z_OBJECT",
    "LIGHT3_Z_OBJECT",
    "LIGHT0_Z_WORLD",
    "LIGHT1_Z_WORLD",
    "LIGHT2_Z_WORLD",
    "LIGHT3_Z_WORLD",
    "LIGHT0_Z_EYE",
    "LIGHT1_Z_EYE",
    "LIGHT2_Z_EYE",
    "LIGHT3_Z_EYE",
    "LIGHT0_EYE_FROM_OBJECT",
    "LIGHT1_EYE_FROM_OBJECT",
    "LIGHT2_EYE_FROM_OBJECT",
    "LIGHT3_EYE_FROM_OBJECT",
    "LIGHT0_EYE_FROM_WORLD",
    "LIGHT1_EYE_FROM_WORLD",
    "LIGHT2_EYE_FROM_WORLD",
    "LIGHT3_EYE_FROM_WORLD",
    "WORLD_FROM_LIGHT0_EYE",
    "WORLD_FROM_LIGHT1_EYE",
    "WORLD_FROM_LIGHT2_EYE",
    "WORLD_FROM_LIGHT3_EYE",
    "LIGHT0_EYE_FROM_EYE",
    "LIGHT1_EYE_FROM_EYE",
    "LIGHT2_EYE_FROM_EYE",
    "LIGHT3_EYE_FROM_EYE",
    "LIGHT0_CLIP_FROM_OBJECT",
    "LIGHT1_CLIP_FROM_OBJECT",
    "LIGHT2_CLIP_FROM_OBJECT",
    "LIGHT3_CLIP_FROM_OBJECT",
    "LIGHT0_CLIP_FROM_WORLD",
    "LIGHT1_CLIP_FROM_WORLD",
    "LIGHT2_CLIP_FROM_WORLD",
    "LIGHT3_CLIP_FROM_WORLD",
    "LIGHT0_CLIP_FROM_EYE",
    "LIGHT1_CLIP_FROM_EYE",
    "LIGHT2_CLIP_FROM_EYE",
    "LIGHT3_CLIP_FROM_EYE",
    "LIGHT0_CLIP_FROM_CLIP",
    "LIGHT1_CLIP_FROM_CLIP",
    "LIGHT2_CLIP_FROM_CLIP",
    "LIGHT3_CLIP_FROM_CLIP",
    "LIGHT0_TEX_FROM_OBJECT",
    "LIGHT1_TEX_FROM_OBJECT",
    "LIGHT2_TEX_FROM_OBJECT",
    "LIGHT3_TEX_FROM_OBJECT",
    "LIGHT0_TEX_FROM_WORLD",
    "LIGHT1_TEX_FROM_WORLD",
    "LIGHT2_TEX_FROM_WORLD",
    "LIGHT3_TEX_FROM_WORLD",
    "LIGHT0_TEX_FROM_EYE",
    "LIGHT1_TEX_FROM_EYE",
    "LIGHT2_TEX_FROM_EYE",
    "LIGHT3_TEX_FROM_EYE",
    ""
};


const std::string
runtimeSemantic( const RuntimeSemantic semantic )
{
    if( semantic < RUNTIME_SEMANTIC_N+1 ) {
        return runtime_semantic_names[ semantic ];
    }
    else {
        Logger log = getLogger( package + ".runtimeSemantic" );
        SCENELOG_FATAL( log, "Illegal runtime semantic 0x" << std::hex << semantic << std::dec );
        return runtime_semantic_names[ RUNTIME_SEMANTIC_N ];
    }
}


RuntimeSemantic
runtimeSemantic( const std::string& semantic_string )
{
    static bool first_time = true;
    static unordered_map<string,RuntimeSemantic> map;
    if( first_time ) {
        for( int i=0; i<RUNTIME_SEMANTIC_N; i++) {
            map[ runtime_semantic_names[i] ] = (RuntimeSemantic)i;
        }
        first_time = false;
    }

    auto it = map.find( semantic_string );
    if( it != map.end() ) {
        return it->second;
    }
    else {
        Logger log = getLogger( package + ".runtimeSemantic" );
        SCENELOG_WARN( log, "Illegal runtime semantic name '" << semantic_string << "'." );
        return RUNTIME_SEMANTIC_N;
    }
}


const std::string
asString( int val )
{
    return boost::lexical_cast<std::string>( val );
}

const std::string
asString( unsigned int val )
{
    return boost::lexical_cast<std::string>( val );
}

const std::string
asString( unsigned long int  val )
{
    return boost::lexical_cast<std::string>( val );
}


const std::string
profileName( ProfileType profile )
{
    switch( profile ) {
    case PROFILE_COMMON:
        return "profile_common";
    case PROFILE_GLSL:
        return "profile_GLSL";
    case PROFILE_GLES2:
        return "profile_GLES2";
    default:
        return "<error>";
    }

}


const std::string
valueType( ValueType type )
{
    switch( type ) {
    case VALUE_TYPE_INT:
        return "int";
    case VALUE_TYPE_FLOAT:
        return "float";
    case VALUE_TYPE_FLOAT2:
        return "float2";
    case VALUE_TYPE_FLOAT3:
        return "float3";
    case VALUE_TYPE_FLOAT4:
        return "float4";
    case VALUE_TYPE_FLOAT3X3:
        return "float3x3";
    case VALUE_TYPE_FLOAT4X4:
        return "float4x4";
    case VALUE_TYPE_BOOL:
        return "bool";
    case VALUE_TYPE_ENUM:
        return "enum";
    case VALUE_TYPE_ENUM2:
        return "enum2";
    case VALUE_TYPE_SAMPLER1D:
        return "sampler1D";
    case VALUE_TYPE_SAMPLER2D:
        return "sampler2D";
    case VALUE_TYPE_SAMPLER3D:
        return "sampler3D";
    case VALUE_TYPE_SAMPLERCUBE:
        return "samplerCUBE";
    case VALUE_TYPE_SAMPLERDEPTH:
        return "samplerDEPTH";
    case VALUE_TYPE_N:
        return "undef";
    }
    return "error";
}



VertexSemantic
vertexSemantic( const std::string& semantic )
{




    if( semantic == "POSITION" ) {
        return VERTEX_POSITION;
    }
    else if( semantic == "NORMAL" ) {
        return VERTEX_NORMAL;
    }
    else if( semantic == "COLOR" ) {
        return VERTEX_COLOR;
    }
    else {
        return VERTEX_SEMANTIC_N;
    }
}

const std::string
vertexSemantic( VertexSemantic semantic )
{
    switch( semantic ) {
    case VERTEX_POSITION:
        return "POSITION"; break;
    case VERTEX_COLOR:
        return "COLOR"; break;
    case VERTEX_NORMAL:
        return "NORMAL"; break;
    case VERTEX_TEXCOORD:
        return "TEXCOORD"; break;
    case VERTEX_TEXTURE:
        return "TEXTURE"; break;
    case VERTEX_TANGENT:
        return "TANGENT"; break;
    case VERTEX_BINORMAL:
        return "BINORMAL"; break;
    case VERTEX_UV:
        return "UV"; break;
    case VERTEX_SEMANTIC_N:
        return "undef"; break;
    }
    return "error";
}

const std::string
stateTypeString( const StateType type )
{
    switch( type ) {
    case STATE_POINT_SIZE:
        return "POINT_SIZE";
    case STATE_POLYGON_OFFSET:
        return "POLYGON_OFFSET";
    case STATE_POLYGON_OFFSET_FILL_ENABLE:
        return "POLYGON_OFFSET_FILL_ENABLE";
    case STATE_BLEND_ENABLE:
        return "BLEND_ENABLE";
    case STATE_BLEND_FUNC:
        return "BLEND";
    case STATE_CULL_FACE:
        return "CULL_FACE";
    case STATE_CULL_FACE_ENABLE:
        return "CULL_FACE_ENABLE";
    case STATE_DEPTH_TEST_ENABLE:
        return "DEPTH_TEST";
    case STATE_DEPTH_MASK:
        return "DEPTH_MASK";
    case STATE_POLYGON_MODE:
        return "POLYGON_MODE";
    case STATE_N:
    default:
        return "UNDEFINED_STATE";
    }

}




const std::string
uniformSemantic( RuntimeSemantic semantic )
{
    switch( semantic ) {
    case RUNTIME_MODELVIEW_MATRIX:
        return "MODELVIEW_MATRIX";
    case RUNTIME_PROJECTION_MATRIX:
        return "PROJECTION_MATRIX";
    case RUNTIME_MODELVIEW_PROJECTION_MATRIX:
        return "MODELVIEW_PROJECTION_MATRIX";
    case RUNTIME_NORMAL_MATRIX:
        return "NORMAL_MATRIX";
    case RUNTIME_SEMANTIC_N:
        return "UNDEF";
    default:
        return "ERROR";
    }

}

const std::string
elementType( ElementType element_type )
{
    switch( element_type ) {
    case ELEMENT_FLOAT:
        return "float";
    case ELEMENT_INT:
        return "int";
    default:
        return "error";
    }
}



ShaderStage
shaderStage( const std::string& stage )
{
    if( stage == "VERTEX" ) {
        return STAGE_VERTEX;
    }
    else if( stage == "GEOMETRY" ) {
        return STAGE_GEOMETRY;
    }
    else if( stage == "FRAGMENT" ) {
        return STAGE_FRAGMENT;
    }
    else {
        return STAGE_N;
    }
}



const std::string
shaderStage( ShaderStage stage )
{
    switch( stage ) {
    case STAGE_VERTEX:
        return "VERTEX";
    case STAGE_GEOMETRY:
        return "GEOMETRY";

/*    case STAGE_TESSELLATION_CONTROL:
        return "TESSELLATION_CONTROL";
    case STAGE_TESSELLATION_EVALUATION:
        return "TESSELLATION_EVALUATION";
*/    case STAGE_FRAGMENT:
        return "FRAGMENT";
    default:
        return "<error>";
    }

}


namespace GL {

    const GLboolean
    booleanValue( const std::string& value )
    {
        if( value == "TRUE" ) {
            return GL_TRUE;
        }
        else {
            return GL_FALSE;
        }
    }

    const std::string
    booleanString( GLboolean boolean )
    {
        return boolean == GL_TRUE ? "TRUE" : "FALSE";
    }

    GLenum
    blendFuncValue( const std::string& value )
    {
        if( value == "ZERO" ) {
            return GL_ZERO;
        }
        else if( value == "ONE" ) {
            return GL_ONE;
        }
        else if( value == "SRC_COLOR" ) {
            return GL_SRC_COLOR;
        }
        else if( value == "ONE_MINUS_SRC_COLOR" ) {
            return GL_ONE_MINUS_SRC_COLOR;
        }
        else if( value == "SRC_ALPHA" ) {
            return GL_SRC_ALPHA;
        }
        else if( value == "SRC_ALPHA_SATURATE" ) {
            return GL_SRC_ALPHA_SATURATE;
        }
        else if( value == "ONE_MINUS_SRC_ALPHA" ) {
            return GL_ONE_MINUS_SRC_ALPHA;
        }
        else if( value == "DST_COLOR" ) {
            return GL_DST_COLOR;
        }
        else if( value == "ONE_MINUS_DST_COLOR" ) {
            return GL_ONE_MINUS_DST_COLOR;
        }
        else if( value == "DST_ALPHA" ) {
            return GL_DST_ALPHA;
        }
        else if( value == "ONE_MINUS_DST_ALPHA" ) {
            return GL_ONE_MINUS_DST_ALPHA;
        }
        else {
            return GL_NONE;
        }
    }


    const std::string
    blendFuncString( GLenum blend_func )
    {
        switch( blend_func ) {
        case GL_ZERO:
            return "ZERO";
        case GL_ONE:
            return "ONE";
        case GL_SRC_COLOR:
            return "SRC_COLOR";
        case GL_ONE_MINUS_SRC_COLOR:
            return "ONE_MINUS_SRC_COLOR";
        case GL_SRC_ALPHA:
            return "SRC_ALPHA";
        case GL_SRC_ALPHA_SATURATE:
            return "SRC_ALPHA_SATURATE";
        case GL_ONE_MINUS_SRC_ALPHA:
            return "ONE_MINUS_SRC_ALPHA";
        case GL_DST_COLOR:
            return "DST_COLOR";
        case GL_ONE_MINUS_DST_COLOR:
            return "ONE_MINUS_DST_COLOR";
        case GL_DST_ALPHA:
            return "DST_ALPHA";
        case GL_ONE_MINUS_DST_ALPHA:
            return "ONE_MINUS_DST_ALPHA";
        default:
            return "";
        }
    }

    GLenum
    faceValue( const std::string& value )
    {
        if( value == "FRONT" ) {
            return GL_FRONT;
        }
        else if( value == "BACK" ) {
            return GL_BACK;
        }
        else if( value == "FRONT_AND_BACK" ) {
            return GL_FRONT_AND_BACK;
        }
        else {
            return GL_NONE;
        }
    }

    const std::string
    faceString( GLenum face )
    {
        switch( face ) {
        case GL_FRONT:
            return "FRONT";
        case GL_BACK:
            return "BACK";
        case GL_FRONT_AND_BACK:
            return "FRONT_AND_BACK";
        default:
            return "";
        }
    }

    GLenum
    polygonModeValue( const std::string& value )
    {
        if( value == "POINT" ) {
            return GL_POINT;
        }
        else if( value == "LINE" ) {
            return GL_LINE;
        }
        else if( value == "FILL" ) {
            return GL_FILL;
        }
        else {
            return GL_NONE;
        }
    }

    const std::string
    polygonModeString( GLenum mode )
    {
        switch( mode ) {
        case GL_POINT:
            return "POINT";
        case GL_LINE:
            return "LINE";
        case GL_FILL:
            return "FILL";
        default:
            return "";
        }
    }



GLenum
shaderStageGLenum( ShaderStage stage )
{
    switch( stage ) {
    case STAGE_VERTEX:
        return GL_VERTEX_SHADER;
    case STAGE_GEOMETRY:
        return GL_GEOMETRY_SHADER;
/*
    case STAGE_TESSELLATION_CONTROL:
        return GL_TESS_CONTROL_SHADER;
    case STAGE_TESSELLATION_EVALUATION:
        return GL_TESS_EVALUATION_SHADER;
*/
    case STAGE_FRAGMENT:
        return GL_FRAGMENT_SHADER;
    default:
        return GL_NONE;
    }
}

std::string
enumString( GLenum e )
{
    switch(e) {
    case GL_FLOAT:
        return "GL_FLOAT";
    case GL_INT:
        return "GL_INT";
    default:
        return "?";
    }
}


GLenum
parseGLenum(const std::string &text)
{
    Logger log = getLogger( "Scene.Util.parseGLenum" );

    const string prefix = "Scene::Utils::GL::parseGLenum: ";

    if( text == "SRC_ALPHA" ) {
        return GL_SRC_ALPHA;
    }
    else if( text == "ONE_MINUS_SRC_ALPHA" ) {
        return GL_ONE_MINUS_SRC_ALPHA;
    }
    else {
        SCENELOG_WARN( log, text + " not recognized" );
        return GL_NONE;
    }
}


} // of namespace GL



}
