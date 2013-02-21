
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;

bool
Importer::parseWrapMode( GLenum& mode, xmlNodePtr n )
{
    Logger log = getLogger( "Scene.XML.Importer.parseWrapMode" );

    bool success = true;

    xmlChar* p = xmlNodeGetContent( n );

    if( xmlStrEqual( p, BAD_CAST "WRAP" ) ) {
        mode = GL_REPEAT;
    }
    else if( xmlStrEqual( p, BAD_CAST "MIRROR" ) ) {
        mode = GL_MIRRORED_REPEAT;
    }
    else if( xmlStrEqual( p, BAD_CAST "CLAMP" ) ) {
        mode = GL_CLAMP_TO_EDGE;
    }
    else if( xmlStrEqual( p, BAD_CAST "BORDER"  ) ) {
        mode = GL_CLAMP_TO_BORDER;
    }
    else if( xmlStrEqual( p, BAD_CAST "MIRROR_ONCE"  ) ) {
        SCENELOG_ERROR( log, "MIRROR_ONCE not handled");
        success = false;
    }
    else {
        SCENELOG_ERROR( log, "wrap mode '" << reinterpret_cast<const char*>(p) << "' not recognized." );
        success = false;
    }
    xmlFree(p);
    return success;
}

bool
Importer::parseFilterMode( GLenum& mode,
                           xmlNodePtr n,
                           bool accept_none,
                           bool accept_nearest,
                           bool accept_linear,
                           bool accept_anisotropic )
{
    Logger log = getLogger( "Scene.XML.Importer.parseFilterMode" );
    bool success = true;
    xmlChar* p = xmlNodeGetContent( n );

    if( accept_none && xmlStrEqual( p, BAD_CAST "NONE" ) ) {
        mode = GL_NONE;
    }
    else if( accept_nearest && xmlStrEqual( p, BAD_CAST "NEAREST" ) ) {
        mode = GL_NEAREST;
    }
    else if( accept_linear && xmlStrEqual( p, BAD_CAST "LINEAR" ) ) {
        mode = GL_LINEAR;
    }
    else if( accept_linear && xmlStrEqual( p, BAD_CAST "LINEAR_MIPMAP_LINEAR" ) ) {
        SCENELOG_WARN( log, "COLLADA 1.4-ism: LINEAR_MIPMAP_LINEAR deprecated in 1.5, using LINEAR instead" );
        mode = GL_LINEAR;
    }
    else if( accept_anisotropic && xmlStrEqual( p, BAD_CAST "ANISOTROPIC" ) ) {
        SCENELOG_ERROR( log, "ANISOTROPIC not handled");
        success = false;
    }
    else {
        SCENELOG_ERROR( log, "filter mode '" << reinterpret_cast<const char*>(p) << "' not recognized." );
        success = false;
    }
    xmlFree(p);
    return success;
}


// Mapping between COLLADA types and Scene types. COLLADA accepts types and
// various forms of typenames in different context. The matrix below maps
// the typenames to scene types, and uses VALUE_TYPE_N where either the type
// is not supported or not accepted. See the COLLADA spec, Chapter 11, Section
// 'parameter-type elements' for details.

struct TypeMap
{
    std::string  m_token;
    ValueType    m_glsl_group;
    ValueType    m_gles_group;
    ValueType    m_gles2_group;
    ValueType    m_fx_newparam_group;
    ValueType    m_fx_setparam_group;
};
static const TypeMap typemap[42] =
{
    // token          GLSL                     GLES                     GLES2                    NEWPARAM                 SETPARAM
    { "bool",         VALUE_TYPE_BOOL,         VALUE_TYPE_BOOL,         VALUE_TYPE_BOOL,         VALUE_TYPE_BOOL,         VALUE_TYPE_BOOL },
    { "bool2",        VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "bool3",        VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "bool4",        VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "bvec2",        VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "bvec3",        VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "bvec4",        VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "int",          VALUE_TYPE_INT,          VALUE_TYPE_INT,          VALUE_TYPE_INT,          VALUE_TYPE_INT,          VALUE_TYPE_INT },
    { "int2",         VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "int3",         VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "int4",         VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "ivec2",        VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "ivec3",        VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "ivec4",        VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "float1x2",     VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "float1x3",     VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "float1x4",     VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "float2x2",     VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "float2x3",     VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "float2x4",     VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "float3x2",     VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "float3x3",     VALUE_TYPE_FLOAT3X3,     VALUE_TYPE_N,            VALUE_TYPE_FLOAT3X3,     VALUE_TYPE_FLOAT3X3,     VALUE_TYPE_FLOAT3X3 },
    { "float3x4",     VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "float4x2",     VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "float4x3",     VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "float4x4",     VALUE_TYPE_FLOAT4X4,     VALUE_TYPE_N,            VALUE_TYPE_FLOAT4X4,     VALUE_TYPE_FLOAT4X4,     VALUE_TYPE_FLOAT4X4 },
    { "float",        VALUE_TYPE_FLOAT,        VALUE_TYPE_FLOAT,        VALUE_TYPE_FLOAT,        VALUE_TYPE_FLOAT,        VALUE_TYPE_FLOAT },
    { "float2",       VALUE_TYPE_FLOAT2,       VALUE_TYPE_N,            VALUE_TYPE_FLOAT2,       VALUE_TYPE_FLOAT2,       VALUE_TYPE_FLOAT2 },
    { "float3",       VALUE_TYPE_FLOAT3,       VALUE_TYPE_N,            VALUE_TYPE_FLOAT3,       VALUE_TYPE_FLOAT3,       VALUE_TYPE_FLOAT3 },
    { "float4",       VALUE_TYPE_FLOAT4,       VALUE_TYPE_N,            VALUE_TYPE_FLOAT4,       VALUE_TYPE_FLOAT4,       VALUE_TYPE_FLOAT4 },
    { "vec2",         VALUE_TYPE_N,            VALUE_TYPE_FLOAT2,       VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "vec3",         VALUE_TYPE_N,            VALUE_TYPE_FLOAT3,       VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "vec4",         VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "mat2",         VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "mat3",         VALUE_TYPE_N,            VALUE_TYPE_FLOAT3X3,     VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "mat4",         VALUE_TYPE_N,            VALUE_TYPE_FLOAT4X4,     VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_N },
    { "sampler1D",    VALUE_TYPE_SAMPLER1D,    VALUE_TYPE_N,            VALUE_TYPE_N,            VALUE_TYPE_SAMPLER1D,    VALUE_TYPE_SAMPLER1D },
    { "sampler2D",    VALUE_TYPE_SAMPLER2D,    VALUE_TYPE_SAMPLER2D,    VALUE_TYPE_SAMPLER2D,    VALUE_TYPE_SAMPLER2D,    VALUE_TYPE_SAMPLER2D },
    { "sampler3D",    VALUE_TYPE_SAMPLER3D,    VALUE_TYPE_N,            VALUE_TYPE_SAMPLER3D,    VALUE_TYPE_SAMPLER3D,    VALUE_TYPE_SAMPLER3D },
    { "samplerCUBE",  VALUE_TYPE_SAMPLERCUBE,  VALUE_TYPE_N,            VALUE_TYPE_SAMPLERCUBE,  VALUE_TYPE_SAMPLERCUBE,  VALUE_TYPE_SAMPLERCUBE },
    { "samplerDEPTH", VALUE_TYPE_SAMPLERDEPTH, VALUE_TYPE_N,            VALUE_TYPE_SAMPLERDEPTH, VALUE_TYPE_SAMPLERDEPTH, VALUE_TYPE_SAMPLERDEPTH },
    { "enum",         VALUE_TYPE_ENUM,         VALUE_TYPE_N,            VALUE_TYPE_ENUM,         VALUE_TYPE_ENUM,         VALUE_TYPE_ENUM }
};




bool
Importer::parseValue( Scene::Value*       value,
                      xmlNodePtr          value_node,
                      const ValueContext  context )
{
    Logger log = getLogger( "Scene.XML.Importer.parseValue" );


    ValueType type = VALUE_TYPE_N;

    bool match = false;
    for(size_t i=0; i<42; i++) {
        if( strEq( value_node->name, typemap[i].m_token ) ) {
            if( context == VALUE_CONTEXT_GLSL_GROUP ) {
                type = typemap[i].m_glsl_group;
            }
            else if( context == VALUE_CONTEXT_GLES_GROUP ) {
                type = typemap[i].m_gles_group;
            }
            else if( context == VALUE_CONTEXT_GLES2_GROUP ) {
                type = typemap[i].m_gles2_group;
            }
            else if( context == VALUE_CONTEXT_FX_NEWPARAM ) {
                type = typemap[i].m_fx_newparam_group;
            }
            else if( context == VALUE_CONTEXT_FX_SETPARAM ) {
                type = typemap[i].m_fx_setparam_group;
            }
            match = true;
        }
    }
    if( !match ) {
        if( strEq( value_node->name, "surface" ) ) {
            nagAboutParseError( log, value_node, "node type removed in COLLADA 1.5.0, ignoring." );
        }
        else {
            nagAboutParseError( log, value_node, "unrecognized type" );
        }
        return false;
    }
    if( type == VALUE_TYPE_N ) {
        SCENELOG_ERROR( log, "Token '" << reinterpret_cast<const char*>( value_node->name )  << "' not accepted in current context." );
        return false;
    }
    else if( type == VALUE_TYPE_INT ) {
        std::vector<int> result;
        if( !parseBodyAsInts( result, value_node, 1 ) ) {
            SCENELOG_ERROR( log, "Failed to parse int" );
            return false;
        }
        *value = Value::createInt( result[0] );
        return true;
    }
    else if( type == VALUE_TYPE_FLOAT ) {
        std::vector<float> result;
        if( !parseBodyAsFloats( result, value_node, 1 ) ) {
            SCENELOG_ERROR( log, "Failed to parse float" );
            return false;
        }
        *value = Value::createFloat( result[0] );
        return true;
    }
    else if( type == VALUE_TYPE_FLOAT2 ) {
        std::vector<float> result;
        if( !parseBodyAsFloats( result, value_node, 2 ) ) {
            SCENELOG_ERROR( log, "Failed to parse float2" );
            return false;
        }
        *value = Value::createFloat2( result[0], result[1] );
        return true;
    }
    else if( type == VALUE_TYPE_FLOAT3 ) {
        std::vector<float> result;
        if( !parseBodyAsFloats( result, value_node, 3 ) ) {
            SCENELOG_ERROR( log, "Failed to parse float3" );
            return false;
        }
        *value = Value::createFloat3( result[0], result[1], result[2] );
        return true;
    }
    else if( type == VALUE_TYPE_FLOAT4 ) {
        std::vector<float> result;
        if( !parseBodyAsFloats( result, value_node, 4 ) ) {
            SCENELOG_ERROR( log, "Failed to parse float4" );
            return false;
        }
        *value = Value::createFloat4( result[0], result[1], result[2], result[3] );
        return true;
    }
    else if( type == VALUE_TYPE_FLOAT3X3 ) {
        std::vector<float> result;
        if( !parseBodyAsFloats( result, value_node, 9 ) ) {
            SCENELOG_ERROR( log, "Failed to parse float9x9" );
            return false;
        }
        *value = Value::createFloat3x3( &result[0] );
        return true;
    }
    else if( type == VALUE_TYPE_FLOAT4X4 ) {
        std::vector<float> result;
        if( !parseBodyAsFloats( result, value_node, 16 ) ) {
            SCENELOG_ERROR( log, "Failed to parse float4x4" );
            return false;
        }
        *value = Value::createFloat4x4( &result[0] );
        return true;
    }
    else if( (type == VALUE_TYPE_SAMPLER1D) ||
             (type == VALUE_TYPE_SAMPLER2D) ||
             (type == VALUE_TYPE_SAMPLER3D) ||
             (type == VALUE_TYPE_SAMPLERCUBE) ||
             (type == VALUE_TYPE_SAMPLERDEPTH) )
    {
        xmlNodePtr n = value_node->children;

        string instance_image;

        GLenum wrap_s = GL_REPEAT;
        GLenum wrap_t = GL_REPEAT;
        GLenum wrap_p = GL_REPEAT;

        GLenum min_filter = GL_LINEAR;
        GLenum mag_filter = GL_LINEAR;
        GLenum mip_filter = GL_LINEAR;

        //float mip_bias = 0.0f;
        //int mip_max_level = 0;
        //int mip_min_level = 0;
        //uint max_anisotropy = 1;

        if( checkNode( n, "instance_image" ) ) {
            instance_image = attribute( n, "url" );
            n = n->next;
        }
        else if( checkNode( n, "source" ) ) {
            SCENELOG_WARN( log, "COLLADA 1.4-ism: sampler2D/source construction deprecated in 1.5, trying to search for newparam/surface" );
            const std::string sid = getBody( n );
            if( sid.empty() ) {
                SCENELOG_ERROR( log, "Empty reference" );
                return false;
            }
            xmlNodePtr np = searchBySid( n, sid );
            if( np == NULL ) {
                SCENELOG_ERROR( log, "Failed to resolve sid='" << sid << "'" );
                return false;
            }
            xmlNodePtr s = np->children;
            if( !checkNode( s, "surface" ) ) {
                SCENELOG_ERROR( log, "sid='" << sid << "' doesn't refer to a newparam/surface" );
                return false;
            }

            for( xmlNodePtr m=s->children; m!=NULL; m=m->next ) {
                if( checkNode( m, "init_from" ) ) {
                    instance_image = getBody( m );
                }
            }
            n = n->next;
        }

        if( instance_image.empty() ) {
            SCENELOG_ERROR( log, "Required instance_image attribute 'url' empty." );
            return false;
        }


        if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "texcoord" ) ) {
            SCENELOG_WARN( log, "Ignoring <texcoord>." );
            n = n->next;
        }
        if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "wrap_s" ) ) {
            if(!parseWrapMode( wrap_s, n ) ) {
                SCENELOG_ERROR( log, "Failed to parse <wrap_s>" );
                return false;
            }
            n = n->next;
        }
        if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "wrap_t" ) ) {
            if(!parseWrapMode( wrap_t, n ) ) {
                SCENELOG_ERROR( log, "Failed to parse <wrap_t>" );
                return false;
            }
            n = n->next;
        }
        if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "wrap_p" ) ) {
            if(!parseWrapMode( wrap_p, n ) ) {
                SCENELOG_ERROR( log, "Failed to parse <wrap_p>" );
                return false;
            }
            n = n->next;
        }
        if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "minfilter" ) ) {
            if(!parseFilterMode( min_filter, n, false, true, true, false ) ) {
                SCENELOG_ERROR( log, "Failed to parse <minfilter>" );
                return false;
            }
            n = n->next;
        }
        if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "magfilter" ) ) {
            if(!parseFilterMode( mag_filter, n, false, true, true, false ) ) {
                SCENELOG_ERROR( log, "Failed to parse <magfilter>" );
                return false;
            }
            n = n->next;
        }
        if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "mipfilter" ) ) {
            if(!parseFilterMode( mip_filter, n, false, true, true, false ) ) {
                SCENELOG_ERROR( log, "Failed to parse <mipfilter>" );
                return false;
            }
            n = n->next;
        }
        if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "border_color" ) ) {
            SCENELOG_WARN( log, "ignoring <border_color>" );
            n = n->next;
        }
        if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "mip_max_level" ) ) {
            SCENELOG_WARN( log, "ignoring <mip_max_level>" );
            n = n->next;
        }
        if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "mip_min_level" ) ) {
            SCENELOG_WARN( log, "ignoring <mip_min_level>" );
            n = n->next;
        }
        if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "mip_bias" ) ) {
            SCENELOG_WARN( log, "ignoring <mip_bias>" );
            n = n->next;
        }
        if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "max_anisotropy" ) ) {
            SCENELOG_WARN( log, "ignoring <max_anisotropy>" );
            n = n->next;
        }
        ignoreExtraNodes( log, n );
        nagAboutRemainingNodes( log, n );

        if( min_filter == GL_NEAREST && mip_filter == GL_NEAREST ) {
            min_filter = GL_NEAREST_MIPMAP_NEAREST;
        }
        else if( min_filter == GL_NEAREST && mip_filter == GL_LINEAR ) {
            min_filter = GL_NEAREST_MIPMAP_LINEAR;
        }
        else if( min_filter == GL_LINEAR && mip_filter == GL_NEAREST ) {
            min_filter = GL_LINEAR_MIPMAP_NEAREST;
        }
        else if( min_filter == GL_LINEAR && mip_filter == GL_LINEAR ) {
            min_filter = GL_LINEAR_MIPMAP_LINEAR;
        }


        if( type == VALUE_TYPE_SAMPLER2D ) {
            *value = Value::createSampler2D( instance_image,
                                             wrap_s,
                                             wrap_t,
                                             min_filter,
                                             mag_filter );
            return true;
        }
        else if( type == VALUE_TYPE_SAMPLER3D ) {
            *value = Value::createSampler3D( instance_image,
                                             wrap_s,
                                             wrap_t,
                                             wrap_p,
                                             min_filter,
                                             mag_filter );
            return true;
        }
        else if( type == VALUE_TYPE_SAMPLERCUBE ) {
            *value = Value::createSamplerCUBE( instance_image,
                                               wrap_s,
                                               wrap_t,
                                               min_filter,
                                               mag_filter );
            return true;
        }

        else {
            SCENELOG_ERROR( log, "Missing code " << __FILE__ << "@" << __LINE__ );
            return false;
        }
    }
    else {
        SCENELOG_FATAL( log, "This should never happen." );
        return false;
    }
}


xmlNodePtr
Exporter::createValue( const Scene::Value*  value,
                       const ValueContext   context ) const
{
    ValueType type = value->type();

    xmlNodePtr val_node = NULL;
    switch( context ) {
    case VALUE_CONTEXT_GLSL_GROUP:
        for( size_t i=0; i<42; i++ ) {
            if( type == typemap[i].m_glsl_group ) {
                val_node = newNode( NULL, typemap[i].m_token );
                break;
            }
        }
        break;
    case VALUE_CONTEXT_GLES_GROUP:
        for( size_t i=0; i<42; i++ ) {
            if( type == typemap[i].m_gles_group ) {
                val_node = newNode( NULL, typemap[i].m_token );
                break;
            }
        }
        break;
    case VALUE_CONTEXT_GLES2_GROUP:
        for( size_t i=0; i<42; i++ ) {
            if( type == typemap[i].m_gles2_group ) {
                val_node = newNode( NULL, typemap[i].m_token );
                break;
            }
        }
        break;
    case VALUE_CONTEXT_FX_NEWPARAM:
        for( size_t i=0; i<42; i++ ) {
            if( type == typemap[i].m_fx_newparam_group ) {
                val_node = newNode( NULL, typemap[i].m_token );
                break;
            }
        }
        break;
    case VALUE_CONTEXT_FX_SETPARAM:
        for( size_t i=0; i<42; i++ ) {
            if( type == typemap[i].m_fx_setparam_group ) {
                val_node = newNode( NULL, typemap[i].m_token );
                break;
            }
        }
        break;
    default:
        break;
    }
    if( val_node == NULL ) {
        return NULL;
    }

    bool image = false;
    bool wrap_s = false;
    bool wrap_t = false;
    bool wrap_p = false;
    bool minfilter = false;
    bool magfilter = false;

    switch( type ) {
    case VALUE_TYPE_INT:
        setBody( val_node, value->intData(), 1 );
        break;
    case VALUE_TYPE_FLOAT:
        setBody( val_node, value->floatData(), 1 );
        break;
    case VALUE_TYPE_FLOAT2:
        setBody( val_node, value->floatData(), 2 );
        break;
    case VALUE_TYPE_FLOAT3:
        setBody( val_node, value->floatData(), 3 );
        break;
    case VALUE_TYPE_FLOAT4:
        setBody( val_node, value->floatData(), 4 );
        break;
    case VALUE_TYPE_FLOAT3X3:
        setBody( val_node, value->floatData(), 9 );
        break;
    case VALUE_TYPE_FLOAT4X4:
        setBody( val_node, value->floatData(), 16 );
        break;
    case VALUE_TYPE_BOOL:
        break;
    case VALUE_TYPE_ENUM:
        break;
    case VALUE_TYPE_ENUM2:
        break;
    case VALUE_TYPE_SAMPLER1D:
        image = true;
        wrap_s = true;
        minfilter = true;
        magfilter = true;
        break;
    case VALUE_TYPE_SAMPLER2D:
        image = true;
        wrap_s = true;
        wrap_t = true;
        minfilter = true;
        magfilter = true;
        break;
    case VALUE_TYPE_SAMPLER3D:
        image = true;
        wrap_s = true;
        wrap_t = true;
        wrap_p = true;
        minfilter = true;
        magfilter = true;
        break;

    case VALUE_TYPE_SAMPLERCUBE:
        image = true;
        wrap_s = true;
        wrap_t = true;
        minfilter = true;
        magfilter = true;
        break;
    case VALUE_TYPE_SAMPLERDEPTH:
        image = true;
        wrap_s = true;
        wrap_t = true;
        minfilter = true;
        magfilter = true;
        break;
    default:
        break;
    }
    if( image && !value->samplerInstanceImage().empty() ) {
        xmlNodePtr ii_node = newChild( val_node, NULL, "instance_image" );
        addProperty( ii_node, "url", "#" + value->samplerInstanceImage() );
    }
    if( wrap_s ) {
        switch( value->samplerWrapS() ) {
        case GL_REPEAT:          newChild( val_node, NULL, "wrap_s", "WRAP" ); break;
        case GL_MIRRORED_REPEAT: newChild( val_node, NULL, "wrap_s", "MIRROR" ); break;
        case GL_CLAMP_TO_EDGE:   newChild( val_node, NULL, "wrap_s", "CLAMP" ); break;
        case GL_CLAMP_TO_BORDER: newChild( val_node, NULL, "wrap_s", "BORDER" ); break;
        default:
            break;
        }
    }
    if( wrap_t ) {
        switch( value->samplerWrapT() ) {
        case GL_REPEAT:          newChild( val_node, NULL, "wrap_t", "WRAP" ); break;
        case GL_MIRRORED_REPEAT: newChild( val_node, NULL, "wrap_t", "MIRROR" ); break;
        case GL_CLAMP_TO_EDGE:   newChild( val_node, NULL, "wrap_t", "CLAMP" ); break;
        case GL_CLAMP_TO_BORDER: newChild( val_node, NULL, "wrap_t", "BORDER" ); break;
        default:
            break;
        }
    }
    if( wrap_p ) {
        switch( value->samplerWrapP() ) {
        case GL_REPEAT:          newChild( val_node, NULL, "wrap_p", "WRAP" ); break;
        case GL_MIRRORED_REPEAT: newChild( val_node, NULL, "wrap_p", "MIRROR" ); break;
        case GL_CLAMP_TO_EDGE:   newChild( val_node, NULL, "wrap_p", "CLAMP" ); break;
        case GL_CLAMP_TO_BORDER: newChild( val_node, NULL, "wrap_p", "BORDER" ); break;
        default:
            break;
        }
    }
    if( minfilter ) {
        switch( value->samplerMinFilter() ) {
        case GL_NONE:    newChild( val_node, NULL, "minfilter", "NONE" ); break;
        case GL_NEAREST: newChild( val_node, NULL, "minfilter", "NEAREST" ); break;
        case GL_LINEAR:  newChild( val_node, NULL, "minfilter", "LINEAR" ); break;
        default:
            break;
        }
    }
    if( magfilter ) {
        switch( value->samplerMagFilter() ) {
        case GL_NONE:    newChild( val_node, NULL, "magfilter", "NONE" ); break;
        case GL_NEAREST: newChild( val_node, NULL, "magfilter", "NEAREST" ); break;
        case GL_LINEAR:  newChild( val_node, NULL, "magfilter", "LINEAR" ); break;
        default:
            break;
        }
    }

    return val_node;
}

    } // of namespace XML
} // of namespace Scene
