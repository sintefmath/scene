
#include "scene/Log.hpp"
#include "scene/SourceBuffer.hpp"
#include "scene/DataBase.hpp"
#include "scene/Camera.hpp"
#include "scene/Light.hpp"
#include "scene/Node.hpp"
#include "scene/Image.hpp"
#include "scene/runtime/RenderAction.hpp"
#include "scene/runtime/Resolver.hpp"

namespace Scene {
    namespace Runtime {
        using std::string;

unsigned int RenderAction::m_serial_no_counter = 0;

static const string package = "Scene.Runtime.RenderAction";

RenderAction::RenderAction( const Type type, const std::string id )
    : m_type( type ),
      m_id( id ),
      m_serial_no( m_serial_no_counter++ )
{
    m_timestamp.touch();
}

RenderAction*
RenderAction::createSetLocalCoordSys( const std::list<const Node*>&  node_path )
{
    RenderAction* action = new RenderAction( RenderAction::ACTION_SET_LOCAL_COORDSYS, "" );

    size_t i=0;
    for( auto it=node_path.begin(); it!=node_path.end(); ++it ) {
        if( i < SCENE_PATH_MAX ) {
            action->m_set_local.m_node_path[ i++ ] = *it;
        }
        else {
            Logger log = getLogger( package + ".createSetLocalCoordSys" );
            SCENELOG_ERROR( log, "Node path larger than SCENE_PATH_MAX." );
            break;
        }
    }
    for( ; i< SCENE_PATH_MAX; i++ ) {
        action->m_set_local.m_node_path[ i ] = NULL;
    }

    return action;
}


RenderAction*
RenderAction::createSetViewCoordSys( const Camera*                  camera,
                                     const std::list<const Node*>&  camera_path,
                                     const Light*                   (&lights)[SCENE_LIGHTS_MAX],
                                     const Camera*                  (&light_projections)[SCENE_LIGHTS_MAX],
                                     const std::list<const Node*>   (&light_paths)[SCENE_LIGHTS_MAX] )
{
    Logger log = getLogger( "Scene.Runtime.RenderAction.createSetView" );


    RenderAction* action = new RenderAction( RenderAction::ACTION_SET_VIEW_COORDSYS, "" );

    action->m_set_view.m_camera = camera;
    size_t i=0;
    for(auto it=camera_path.begin(); it!=camera_path.end(); ++it ) {
        if( i < SCENE_PATH_MAX ) {
            action->m_set_view.m_camera_path[i++] = *it;
        }
        else {
            SCENELOG_ERROR( log, "Camera node path larger than SCENE_PATH_MAX" );
        }
    }
    for( ; i<SCENE_PATH_MAX; i++ ) {
        action->m_set_view.m_camera_path[i] = NULL;
    }

    for( size_t j=0; j<SCENE_LIGHTS_MAX; j++ ) {
        action->m_set_view.m_lights[j] = lights[j];
        action->m_set_view.m_light_projections[j] = light_projections[j];

        size_t i=0;
        for(auto it=light_paths[j].begin(); it!=light_paths[j].end(); ++it ) {
            if( i < SCENE_PATH_MAX ) {
                action->m_set_view.m_light_paths[j][i++] = *it;
            }
            else {
                SCENELOG_ERROR( log, "Light " << j << " node path larger than SCENE_PATH_MAX" );
            }
        }
        for( ; i<SCENE_PATH_MAX; i++ ) {
            action->m_set_view.m_light_paths[j][i] = NULL;
        }
    }

    return action;
}

RenderAction*
RenderAction::createDraw( const std::string&    id,
                          const Geometry*       geometry,
                          const Primitives*     primitives,
                          const Pass*           pass )
{
    Logger log = getLogger( "Scene.Runtime.RenderAction.createDraw" );

    RenderAction* action = new RenderAction( RenderAction::ACTION_DRAW, id );
    action->m_draw.m_geometry = geometry;
    action->m_draw.m_primitives = primitives;

    unsigned int vertices_per_primitive;
    PrimitiveType type = primitives->primitiveType();
    if( pass->primitiveOverride( type ) ) {
        vertices_per_primitive = pass->primitiveOverrideVertices( type );
        type = pass->primitiveOverrideType( type );
    }
    else {
        vertices_per_primitive = primitives->verticesPerPrimitive();
    }

    switch( type ) {
    case PRIMITIVE_POINTS:
        action->m_draw.m_mode = GL_POINTS;
        break;
    case PRIMITIVE_LINES:
        action->m_draw.m_mode = GL_LINES;
        break;
    case PRIMITIVE_TRIANGLES:
        action->m_draw.m_mode = GL_TRIANGLES;
        break;
    case PRIMITIVE_QUADS:
        action->m_draw.m_mode = GL_QUADS;
        break;
    case PRIMITIVE_PATCHES:
        action->m_draw.m_mode = GL_PATCHES;
        SCENELOG_DEBUG( log, action->m_draw.m_vertices );
        break;
    case PRIMITIVE_N:
        SCENELOG_ERROR( log, "Illegal primitive encountered." );
        delete action;
        return NULL;
    }
    action->m_draw.m_first    = 0;
    action->m_draw.m_vertices = vertices_per_primitive;
    action->m_draw.m_count    = primitives->vertexCount();
    SCENELOG_TRACE( log,
                    "first=" <<action->m_draw.m_first <<
                    ", verts pr prim=" << action->m_draw.m_vertices <<
                    ", count=" << action->m_draw.m_count );
    return action;
}

RenderAction*
RenderAction::createDrawIndexed( const DataBase&     database,
                                 const std::string&  id,
                                 const Geometry*     geometry,
                                 const Primitives*   primitives,
                                 const Pass*         pass )
{
    Logger log = getLogger( "Scene.Runtime.RenderAction.createDrawIndexd" );

    const SourceBuffer* index_buffer = database.library<SourceBuffer>().get( primitives->indexBufferId() );
    if( index_buffer == NULL ) {
        SCENELOG_ERROR( log, "Unable to retrieve index buffer '" << primitives->indexBufferId() );
        return NULL;
    }

    RenderAction* action = new RenderAction( RenderAction::ACTION_DRAW_INDEXED, id );
    action->m_draw_indexed.m_index_buffer = index_buffer;
    action->m_draw_indexed.m_geometry = geometry;
    action->m_draw_indexed.m_primitives = primitives;

    GLsizei size;
    switch( index_buffer->elementType() ) {
    case ELEMENT_FLOAT:
        SCENELOG_ERROR( log, "Unsupported index element type float." );
        delete action;
        return NULL;
    case ELEMENT_INT:
        action->m_draw_indexed.m_type = GL_UNSIGNED_INT;
        size = sizeof(GLuint);
        break;
    }

    unsigned int patch_vertices;
    PrimitiveType type = primitives->primitiveType();
    if( pass->primitiveOverride( type ) ) {
        patch_vertices = pass->primitiveOverrideVertices( type );
        type = pass->primitiveOverrideType( type );
    }
    else {
        patch_vertices = primitives->verticesPerPrimitive();
    }

    switch( type ) {
    case PRIMITIVE_POINTS:
        action->m_draw_indexed.m_mode = GL_POINTS;
        break;
    case PRIMITIVE_LINES:
        action->m_draw_indexed.m_mode = GL_LINES;
        break;
    case PRIMITIVE_TRIANGLES:
        action->m_draw_indexed.m_mode = GL_TRIANGLES;
        break;
    case PRIMITIVE_QUADS:
        action->m_draw_indexed.m_mode = GL_QUADS;
        break;
    case PRIMITIVE_PATCHES:
        action->m_draw_indexed.m_mode = GL_PATCHES;
        break;
    case PRIMITIVE_N:
        SCENELOG_ERROR( log, "Illegal primitive encountered." );
        delete action;
        return NULL;
    }
    action->m_draw_indexed.m_offset   = reinterpret_cast<GLvoid*>( size*primitives->indexOffset() );
    action->m_draw_indexed.m_vertices = patch_vertices;
    action->m_draw_indexed.m_count    = primitives->vertexCount();
    SCENELOG_TRACE( log,
                    "offset=" <<action->m_draw_indexed.m_offset <<
                    ", verts pr prim=" << action->m_draw_indexed.m_vertices <<
                    ", count=" << action->m_draw_indexed.m_count );
    return action;
}
RenderAction*
RenderAction::createSetPass( const std::string&  id,
                             const Pass*         pass )
{
    RenderAction* action = new RenderAction( RenderAction::ACTION_SET_PASS, id );
    action->m_set_pass.m_pass = pass;
    return action;
}

RenderAction*
RenderAction::createSetInputs( const DataBase&                database,
                               const std::string&             id,
                               const Pass*                    pass,
                               const Geometry*                geometry,
                               const Primitives*  primitives )
{
    Logger log = getLogger( "Scene.Runtime.RenderAction.createSetInputs" );
    RenderAction* action = new RenderAction( ACTION_SET_INPUTS, id );

    action->m_set_inputs.m_pass = pass;
    action->m_set_inputs.m_primitives = primitives;
    action->m_set_inputs.m_items.resize( pass->attributes() );
    for(size_t i=0; i<action->m_set_inputs.m_items.size(); i++) {
        const VertexSemantic semantic = pass->attributeSemantic( i );
        const Geometry::VertexInput& input = geometry->vertexInput( semantic );
        if( !input.m_enabled ) {
            SCENELOG_ERROR( log, "Required geometry input for semantic " << semantic << "missing." );
            return false;
        }
        const SourceBuffer* buffer = database.library<SourceBuffer>().get( input.m_source_buffer_id );
        if( buffer == NULL ) {
            SCENELOG_ERROR( log, "Unable to find buffer '" << input.m_source_buffer_id << '\'' );
            return false;
        }
        action->m_set_inputs.m_items[i].m_source = buffer;
        action->m_set_inputs.m_items[i].m_type = buffer->elementType();
        action->m_set_inputs.m_items[i].m_components = input.m_components;
        action->m_set_inputs.m_items[i].m_offset = input.m_offset;
        action->m_set_inputs.m_items[i].m_stride = input.m_stride;
    }

    return action;
}

RenderAction*
RenderAction::createSetSamplers( const DataBase&        database,
                                 const std::string&     id,
                                 const ResolvedParams*  params,
                                 const Pass*            passt )
{
    Logger log = getLogger( "Scene.Runtime.RenderAction.createSetSamplers" );
    SCENELOG_TRACE( log, "params=" << params );

    RenderAction* action = new RenderAction( RenderAction::ACTION_SET_SAMPLERS, id );

    const Pass* pass = params->m_pass;

    action->m_set_samplers.m_pass = pass;


    for( size_t i=0; i<pass->uniforms(); i++ ) {
        const Value* value = pass->uniformValue(i);
        const std::string& reference = pass->uniformParameterReference(i);
        RuntimeSemantic semantic = RUNTIME_SEMANTIC_N;
        if( !reference.empty() ) {
            // Override value using parameter reference
            auto it = params->m_map.find( reference );
            if( it == params->m_map.end() ) {
                SCENELOG_ERROR( log, "Cannot resolve parameter reference '" << reference << '\'' );
            }
            else {
                semantic = it->second.m_semantic;
                value = it->second.m_value;
            }
        }
        if( value == NULL ) {
            SCENELOG_ERROR( log, "No value given for uniform symbol '" << pass->uniformSymbol(i) << "' " );
            delete action;
            return NULL;
        }

        bool sampler = false;
        switch( value->type() ) {
        case VALUE_TYPE_SAMPLER1D:
        case VALUE_TYPE_SAMPLER2D:
        case VALUE_TYPE_SAMPLER3D:
        case VALUE_TYPE_SAMPLERCUBE:
        case VALUE_TYPE_SAMPLERDEPTH:
            sampler = true;
            break;
        default:
            break;
        }

        if( sampler ) {
            const string& image_id = value->samplerInstanceImage();
            const Image* image = database.library<Image>().get( image_id );
            if( image == NULL ) {
                SCENELOG_ERROR( log, "Unknown image reference '" << image_id << '\'' );
                continue;
            }
            if( (image->type() == IMAGE_2D   && value->type() != VALUE_TYPE_SAMPLER2D ) ||
                (image->type() == IMAGE_3D   && value->type() != VALUE_TYPE_SAMPLER3D ) ||
                (image->type() == IMAGE_CUBE && value->type() != VALUE_TYPE_SAMPLERCUBE ) )
            {
                SCENELOG_ERROR( log, "Image and sampler types unsupported." );
            }
            else {
                size_t sampler = action->m_set_samplers.m_items.size();
                action->m_set_samplers.m_items.resize( sampler + 1 );
                action->m_set_samplers.m_items.back().m_image = image;
                action->m_set_samplers.m_items.back().m_wrap_s = value->samplerWrapS();
                action->m_set_samplers.m_items.back().m_wrap_t = value->samplerWrapT();
                action->m_set_samplers.m_items.back().m_wrap_p = value->samplerWrapP();
                action->m_set_samplers.m_items.back().m_min_filter = value->samplerMinFilter();
                action->m_set_samplers.m_items.back().m_mag_filter = value->samplerMagFilter();
                action->m_set_samplers.m_items.back().m_uniform_value = Value::createInt( sampler );
            }
        }
    }

    if( action->m_set_samplers.m_items.empty() ) {
        delete action;
        return NULL;
    }
    else {
        return action;
    }
}

RenderAction*
RenderAction::createSetRenderTarget( const DataBase&        database,
                                     const std::string&     id,
                                     const ResolvedParams*  params,
                                     const Pass*            pass )
{
    Logger log = getLogger( "Scene.Runtime.RenderAction.createSetRenderTarget" );


    if( params == NULL || pass == NULL ) {
        RenderAction* action = new RenderAction( ACTION_SET_FRAMEBUFFER, id );
        return action;
    }

    std::vector<SetRenderTargets::Item> items;

    for(size_t i=0; i<pass->renderTargets(); i++ ) {
        const Image* image = NULL;
        const std::string& reference = pass->renderTargetParameterReference(i);
        if(!reference.empty()) {
            auto it = params->m_map.find( reference );
            if( it == params->m_map.end() ) {
                SCENELOG_ERROR( log, "Cannot resolve parameter reference '" << reference << '\'' );
            }
            else {
                const Value* value = it->second.m_value;
                bool sampler = false;
                switch( value->type() ) {
                case VALUE_TYPE_SAMPLER1D:
                case VALUE_TYPE_SAMPLER2D:
                case VALUE_TYPE_SAMPLER3D:
                case VALUE_TYPE_SAMPLERCUBE:
                case VALUE_TYPE_SAMPLERDEPTH:
                    sampler = true;
                    break;
                default:
                    break;
                }
                if( sampler ) {
                    image = database.library<Image>().get( value->samplerInstanceImage() );

                    // Is it any point in nagging about correct sampler type? Nah.
                }
                else {
                    SCENELOG_ERROR( log, "Parameter '" << reference << "' is not of sampler type." );
                }
            }
        }
        else {
            database.library<Image>().get( pass->renderTargetImageReference(i) );
        }

        if( image == NULL ) {
            SCENELOG_ERROR( log, "Failed to retreive image." );
            return NULL;
        }
        else {
            items.resize( items.size() + 1);
            items.back().m_image = image;
            items.back().m_target = GL_FRAMEBUFFER;
            items.back().m_level = pass->renderTargetMipLevel(i);
            switch( pass->renderTargetTarget(i) ) {
            case RENDER_TARGET_COLOR:
                items.back().m_attachment = GL_COLOR_ATTACHMENT0 + pass->renderTargetIndex(i);
                break;
            case RENDER_TARGET_DEPTH:
                if( pass->renderTargetIndex(i) != 0 ) {
                    SCENELOG_ERROR( log, "Depth target accepts only index 0" );
                    return NULL;
                }
                items.back().m_attachment = GL_DEPTH_ATTACHMENT;
                break;
            case RENDER_TARGET_STENCIL:
                if( pass->renderTargetIndex(i) != 0 ) {
                    SCENELOG_ERROR( log, "Stencil target accepts only index 0" );
                    return NULL;
                }
                items.back().m_attachment = GL_STENCIL_ATTACHMENT;
                break;
            }


            switch( image->type() ) {
            case IMAGE_2D:
                items.back().m_textarget = GL_TEXTURE_2D;
                break;
            case IMAGE_3D:
                items.back().m_textarget = GL_TEXTURE_3D;
                items.back().m_layer = pass->renderTargetSlice(i);
                break;
            case IMAGE_CUBE:
                items.back().m_textarget =
                        GL_TEXTURE_CUBE_MAP_POSITIVE_X +
                        pass->renderTargetFace(i);
                break;
            case IMAGE_N:
                SCENELOG_FATAL( log, "Code that should never be reached @" << __LINE__ );
                return NULL;
            }
        }
    }

    RenderAction* action = new RenderAction( ACTION_SET_FRAMEBUFFER, id );
    action->m_set_render_targets.m_items.swap( items );
    action->m_set_render_targets.m_clear_mask = GL_COLOR_BUFFER_BIT;
    return action;
}



RenderAction*
RenderAction::createSetUniforms( const std::string&                id,
                                 const RenderAction*               set_samplers,
                                 const ResolvedParams*             params,
                                 const Pass*                       pass )
{
    Logger log = getLogger( "Scene.Runtime.RenderAction.createSetUniforms" );
    RenderAction* action = new RenderAction( RenderAction::ACTION_SET_UNIFORMS, id );
    action->m_set_uniforms.m_pass = pass;
    action->m_set_uniforms.m_items.resize( pass->uniforms() );


    // Run through the uniforms of the pass program. They can either have a
    // value specified directly, or reference a parameter
    for( size_t i=0; i<pass->uniforms(); i++ ) {

        const Value* value = pass->uniformValue(i);
        const std::string& reference = pass->uniformParameterReference(i);


        RuntimeSemantic semantic = RUNTIME_SEMANTIC_N;
        if( !reference.empty() ) {
            // Override value using parameter reference
            auto it = params->m_map.find( reference );
            if( it == params->m_map.end() ) {
                SCENELOG_ERROR( log, "Cannot resolve parameter reference '" << reference << '\'' );
            }
            else {
                semantic = it->second.m_semantic;
                value = it->second.m_value;
            }
        }


        if( value == NULL ) {
            SCENELOG_ERROR( log, "No value given for uniform symbol '" << pass->uniformSymbol(i) << "' " );
            delete action;
            return NULL;
        }


        bool sampler_type = false;
        switch( value->type() ) {
        case VALUE_TYPE_INT:
            break;
        case VALUE_TYPE_FLOAT:
        case VALUE_TYPE_FLOAT2:
        case VALUE_TYPE_FLOAT3:
        case VALUE_TYPE_FLOAT4:
        case VALUE_TYPE_FLOAT3X3:
        case VALUE_TYPE_FLOAT4X4:
            break;
        case VALUE_TYPE_BOOL:
            break;
        case VALUE_TYPE_ENUM:
        case VALUE_TYPE_ENUM2:
            break;
        case VALUE_TYPE_SAMPLER1D:
        case VALUE_TYPE_SAMPLER2D:
        case VALUE_TYPE_SAMPLER3D:
        case VALUE_TYPE_SAMPLERCUBE:
        case VALUE_TYPE_SAMPLERDEPTH:
            sampler_type = true;
            break;
        case VALUE_TYPE_N:
            break;
        }
        if( sampler_type ) {
            if(set_samplers != NULL) {
                for(size_t k=0; k<set_samplers->m_set_samplers.m_items.size(); k++) {
                    const Image* image = set_samplers->m_set_samplers.m_items[k].m_image;
                    if( value->samplerInstanceImage() == image->id() ) {

                        value = &set_samplers->m_set_samplers.m_items[k].m_uniform_value;


                        break;
                    }
                }
            }
        }

        action->m_set_uniforms.m_items[i].m_semantic = semantic;
        action->m_set_uniforms.m_items[i].m_value = value;
        SCENELOG_TRACE( log, "uniform " << i << ", reference='" << reference << "', value=" << (value==NULL?"null":value->debugString()  ) );
    }

    return action;
}



RenderAction*
RenderAction::createSetFBCtrl( const std::string&     id,
                               const ResolvedParams*  params,
                               const Pass*            pass )
{
    Logger log = getLogger( "Scene.Runtime.RenderAction.createSetFBCtrl" );
    RenderAction* action = new RenderAction( ACTION_SET_FB_CTRL, id );

    action->m_set_fb_ctrl.m_color_writemask[0] = GL_TRUE;
    action->m_set_fb_ctrl.m_color_writemask[1] = GL_TRUE;
    action->m_set_fb_ctrl.m_color_writemask[2] = GL_TRUE;
    action->m_set_fb_ctrl.m_color_writemask[3] = GL_TRUE;
    action->m_set_fb_ctrl.m_depth_writemask = GL_TRUE;

    if( pass != NULL ) {
        bool touched = false;

        for(size_t i=0; i<pass->states(); i++ ) {
            const Value* value = pass->stateValue(i);
            if( pass->stateIsParameterReference(i) ) {
                if( params == NULL ) {
                    SCENELOG_FATAL( log, "params == NULL" );
                    continue;
                }
                const string& ref = pass->stateParameterReference( i );
                auto it = params->m_map.find( ref );
                if( it == params->m_map.end() ) {
                    SCENELOG_WARN( log, "Unable to resolve parameter reference '" << ref << "', ignoring." );
                    continue;
                }
                else {
                    value = it->second.m_value;
                }
            }

            switch( pass->stateType(i) ) {
            case STATE_DEPTH_MASK:
                if( value->type() != VALUE_TYPE_BOOL ) {
                    SCENELOG_WARN( log, "DEPTH_MASK expects BOOL, ignoring." );
                    continue;
                }
                action->m_set_fb_ctrl.m_depth_writemask = value->boolData()[0];
                touched = true;
                break;
            default:
                break;
            }
        }

        if( !touched ) {
            delete action;
            action = NULL;
        }
    }
    return action;
}

RenderAction*
RenderAction::createSetRaster( const std::string&     id,
                               const ResolvedParams*  params,
                               const Pass*            pass )
{
    Logger log = getLogger( "Scene.Runtime.RenderAction.createSetRaster" );
    RenderAction* action = new RenderAction( ACTION_SET_RASTER, id );

    action->m_set_rasterization.m_point_size = 1.f;
    action->m_set_rasterization.m_cull_face = GL_FALSE;
    action->m_set_rasterization.m_cull_face_mode = GL_BACK;
    action->m_set_rasterization.m_polygon_mode_front = GL_FILL;
    action->m_set_rasterization.m_polygon_mode_back = GL_FILL;
    action->m_set_rasterization.m_polygon_offset_factor = 0.f;
    action->m_set_rasterization.m_polygon_offset_units = 0.f;
    action->m_set_rasterization.m_polygon_offset_point = GL_FALSE;
    action->m_set_rasterization.m_polygon_offset_line = GL_FALSE;
    action->m_set_rasterization.m_polygon_offset_fill = GL_FALSE;

    if( pass != NULL ) {
        bool touched = false;

        for(size_t i=0; i<pass->states(); i++ ) {
            const Value* value = pass->stateValue(i);
            if( pass->stateIsParameterReference(i) ) {
                if( params == NULL ) {
                    SCENELOG_FATAL( log, "params == NULL" );
                    continue;
                }
                const string& ref = pass->stateParameterReference( i );
                auto it = params->m_map.find( ref );
                if( it == params->m_map.end() ) {
                    SCENELOG_WARN( log, "Unable to resolve parameter reference '" << ref << "', ignoring." );
                    continue;
                }
                else {
                    value = it->second.m_value;
                }
            }

            switch( pass->stateType(i) ) {
            case STATE_POINT_SIZE:
                if( value->type() != VALUE_TYPE_FLOAT ) {
                    SCENELOG_WARN( log, "POINT_SIZE expects FLOAT, ignoring." );
                    continue;
                }
                action->m_set_rasterization.m_point_size = value->floatData()[0];
                touched = true;
                break;

            case STATE_POLYGON_OFFSET:
                if( value->type() != VALUE_TYPE_FLOAT2 ) {
                    SCENELOG_WARN( log, "POLYGON_OFFSET expects FLOAT2, ignoring." );
                    continue;
                }
                action->m_set_rasterization.m_polygon_offset_factor = value->floatData()[0];
                action->m_set_rasterization.m_polygon_offset_units  = value->floatData()[1];
                touched = true;
                break;

            case STATE_POLYGON_OFFSET_FILL_ENABLE:
                if( value->type() != VALUE_TYPE_BOOL ) {
                    SCENELOG_WARN( log, "POLYGON_OFFSET_FILL expects BOOL, ignoring." );
                    continue;
                }
                action->m_set_rasterization.m_polygon_offset_fill = value->boolData()[0];
                touched = true;
                break;

            case STATE_CULL_FACE_ENABLE:
                if( value->type() != VALUE_TYPE_BOOL ) {
                    SCENELOG_WARN( log, "CULL_FACE_ENABLE expects BOOL, ignoring." );
                    continue;
                }
                action->m_set_rasterization.m_cull_face = value->boolData()[0];
                touched = true;
                break;

            case STATE_CULL_FACE:
                if( value->type() != VALUE_TYPE_ENUM ) {
                    SCENELOG_WARN( log, "CULL_FACE expects ENUM, ignoring." );
                    continue;
                }
                action->m_set_rasterization.m_cull_face_mode = value->enumData()[0];
                touched = true;
                break;

            case STATE_POLYGON_MODE:
                if( value->type() != VALUE_TYPE_ENUM2 ) {
                    SCENELOG_WARN( log, "POLYGON_MODE expects ENUM2, ignoring." );
                    continue;
                }
                switch( value->enumData()[0] ) {
                case GL_FRONT:
                    action->m_set_rasterization.m_polygon_mode_front = value->enumData()[1];
                    break;
                case GL_BACK:
                    action->m_set_rasterization.m_polygon_mode_back = value->enumData()[1];
                    break;
                case GL_FRONT_AND_BACK:
                    action->m_set_rasterization.m_polygon_mode_front = value->enumData()[1];
                    action->m_set_rasterization.m_polygon_mode_back = value->enumData()[1];
                    break;
                default:
                    SCENELOG_WARN( log, "Invalid POLYGON_MODE enum[0]");
                }
                touched = true;
                break;

            default:
                break;
            }
        }

        if( !touched ) {
            delete action;
            action = NULL;
        }
    }
    return action;
}


RenderAction*
RenderAction::createSetPixelOps( const std::string&     id,
                                 const ResolvedParams*  params,
                                 const Pass*            pass )
{
    Logger log = getLogger( "Scene.Runtime.RenderAction.createSetPixelOps" );

    RenderAction* action = new RenderAction( ACTION_SET_PIXEL_OPS, id );

    action->m_set_pixel_ops.m_depth_test = GL_FALSE;
    action->m_set_pixel_ops.m_depth_func = GL_LESS;
    action->m_set_pixel_ops.m_blend = GL_FALSE;
    action->m_set_pixel_ops.m_blend_src_rgb = GL_ONE;
    action->m_set_pixel_ops.m_blend_src_alpha = GL_ONE;
    action->m_set_pixel_ops.m_blend_dst_rgb = GL_ZERO;
    action->m_set_pixel_ops.m_blend_dst_alpha = GL_ZERO;

    if( pass != NULL ) {
        bool touched = false;
        for(size_t i=0; i<pass->states(); i++ ) {
            const Value* value = pass->stateValue(i);
            if( pass->stateIsParameterReference(i) ) {
                if( params == NULL ) {
                    SCENELOG_FATAL( log, "params == NULL" );
                    continue;
                }
                const string& ref = pass->stateParameterReference( i );
                auto it = params->m_map.find( ref );
                if( it == params->m_map.end() ) {
                    SCENELOG_WARN( log, "Unable to resolve parameter reference '" << ref << "', ignoring." );
                    continue;
                }
                else {
                    value = it->second.m_value;
                }
            }

            switch( pass->stateType(i) ) {
            case STATE_BLEND_ENABLE:
                if( value->type() != VALUE_TYPE_BOOL ) {
                    SCENELOG_WARN( log, "BLEND_ENABLE expects BOOL, ignoring." );
                    continue;
                }
                action->m_set_pixel_ops.m_blend = value->boolData()[0];
                touched = true;
                break;
            case STATE_BLEND_FUNC:
                if( value->type() != VALUE_TYPE_ENUM2 ) {
                    SCENELOG_WARN( log, "BLEND_FUNC expects ENUM2, ignoring." );
                    continue;
                }
                action->m_set_pixel_ops.m_blend_src_rgb   = value->enumData()[0];
                action->m_set_pixel_ops.m_blend_src_alpha = value->enumData()[0];
                action->m_set_pixel_ops.m_blend_dst_rgb   = value->enumData()[1];
                action->m_set_pixel_ops.m_blend_dst_alpha = value->enumData()[1];
                touched = true;
                break;
            case STATE_DEPTH_TEST_ENABLE:
                if( value->type() != VALUE_TYPE_BOOL ) {
                    SCENELOG_WARN( log, "DEPTH_TEST_ENABLE expects BOOL, ignoring." );
                    continue;
                }
                action->m_set_pixel_ops.m_depth_test = value->boolData()[0];
                touched = true;
                break;
            default:
                // ignore
                break;
            }
        }
        if( !touched ) {
            delete action;
            action = NULL;
        }
    }
    return action;
}



void
RenderAction::debugDump() const
{
    Logger log = getLogger( "Scene.Runtime.RenderAction.debugDump" );

    switch( m_type ) {
    case ACTION_SET_VIEW_COORDSYS:
        SCENELOG_DEBUG( log, "SET_VIEW_COORDSYS" );
        break;
    case ACTION_SET_LOCAL_COORDSYS:
        SCENELOG_DEBUG( log, "SET_LOCAL_COORDSYS" );
        break;

    case ACTION_SET_PASS:
        SCENELOG_DEBUG( log, "SET_PASS pass=" << m_set_pass.m_pass );
        break;

    case ACTION_SET_FRAMEBUFFER:
        SCENELOG_DEBUG( log, "SET_FRAMEBUFFER" );
        if( m_set_render_targets.m_items.empty() ) {
            SCENELOG_DEBUG( log, "  default" );
        }
        else {
            for(size_t i=0; i<m_set_render_targets.m_items.size(); i++ ) {
                SCENELOG_DEBUG( log, "  " << m_set_render_targets.m_items[i].m_image->id() );
            }
        }

        break;
    case ACTION_SET_INPUTS:
        SCENELOG_DEBUG( log, "SET_INPUTS " << m_id );
        for(size_t i=0; i<m_set_inputs.m_items.size(); i++ ) {
            const SetInputs::Item& m = m_set_inputs.m_items[i];
            SCENELOG_DEBUG( log,
                           "  attribute '" << m_set_inputs.m_pass->attributeSymbol(i) << "', " <<
                           "buffer='" << m.m_source->id() << "', " <<
                           "components="<< m.m_components << ", " <<
                           "offset=" << m.m_offset << ", " <<
                           "stride=" << m.m_stride );
        }
        break;
    case ACTION_SET_UNIFORMS:
        SCENELOG_DEBUG( log, "SET_UNIFORMS '" << m_id << '\'' );
        for( size_t i=0; i<m_set_uniforms.m_items.size(); i++ ) {
            const SetUniforms::Item& m = m_set_uniforms.m_items[i];
            SCENELOG_DEBUG( log, "  uniform " << i << ": symbol='" << m_set_uniforms.m_pass->uniformSymbol(i) <<
                                "', sem=" << m.m_semantic <<
                                ", value=" << m.m_value->debugString() );
        }
        break;
    case ACTION_SET_SAMPLERS:
        SCENELOG_DEBUG( log, "SET_SAMPLERS '" << m_id << '\'' );
        for( size_t i=0; i<m_set_samplers.m_items.size(); i++ ) {
            SCENELOG_DEBUG( log, "  " << m_set_samplers.m_items[i].m_image->id() );
        }
        break;

    case ACTION_SET_FB_CTRL:
        SCENELOG_DEBUG( log, "SET_FB_CTRL");
        break;

    case ACTION_SET_PIXEL_OPS:
        SCENELOG_DEBUG( log, "SET_PIXEL_OPS" );
        if( m_set_pixel_ops.m_depth_test == GL_TRUE ) {
            SCENELOG_DEBUG( log, "  depth_test=true, depth_func=" << reinterpret_cast<void*>( m_set_pixel_ops.m_depth_func ) );
        }
        else {
            SCENELOG_DEBUG( log, "  depth_test=false" );
        }
        if( m_set_pixel_ops.m_blend == GL_TRUE ) {
            SCENELOG_DEBUG( log, "  blend=true, " <<
                             "src_rgb=" << reinterpret_cast<void*>( m_set_pixel_ops.m_blend_src_rgb ) << ", " <<
                             "src_alpha=" << reinterpret_cast<void*>( m_set_pixel_ops.m_blend_src_alpha ) << ", " <<
                             "dst_rgb=" << reinterpret_cast<void*>( m_set_pixel_ops.m_blend_dst_rgb ) << ", " <<
                             "dst_alpha=" << reinterpret_cast<void*>( m_set_pixel_ops.m_blend_dst_alpha ) );
        }
        else {
            SCENELOG_DEBUG( log, "  blend=false" );
        }

        break;
    case ACTION_SET_RASTER:
        SCENELOG_DEBUG( log, "SET_RASTER");
        break;

    case ACTION_DRAW:
        SCENELOG_DEBUG( log, "DRAW " <<
                        ", mode=" << reinterpret_cast<void*>( m_draw.m_mode ) <<
                        ", vertices=" << m_draw.m_vertices <<
                        ", first=" << m_draw.m_first <<
                        ", count=" << m_draw.m_count );
        break;

    case ACTION_DRAW_INDEXED:
        SCENELOG_DEBUG( log, "DRAW_INDEXED " <<
                            ", indices='" << m_draw_indexed.m_index_buffer->id() << "' (" << m_draw_indexed.m_index_buffer << ")"
                            ", mode=" << reinterpret_cast<void*>( m_draw_indexed.m_mode ) <<
                            ", vertices=" << m_draw_indexed.m_vertices <<
                            ", offset=" << m_draw_indexed.m_offset <<
                            ", count=" << m_draw_indexed.m_count );
        break;
    }


}




    }
}
