#include <algorithm>
#include <cstring>
#include <scene/Log.hpp>
#include <scene/Camera.hpp>
#include <scene/Image.hpp>
#include <scene/SourceBuffer.hpp>
#include <scene/glsl/GLSLRenderList.hpp>



namespace Scene {
    namespace Runtime {
        using std::string;
        using std::max;

static const string package = "Scene.Runtime.GLSLRenderList";

GLSLRenderList::GLSLRenderList( GLSLRuntime& runtime )
    : m_runtime( runtime ),
      m_renderlist( m_runtime.resolver() ),
      m_transform_cache( m_runtime.resolver().database(), true ),
      m_default_framebuffer(0),
      m_default_viewport_x(0),
      m_default_viewport_y(0),
      m_default_viewport_w(1),
      m_default_viewport_h(1),
      m_valid( false )
{
}

void
GLSLRenderList::setDefaultOutput( GLuint framebuffer, size_t x, size_t y, size_t w, size_t h )
{
    m_default_framebuffer = framebuffer;
    m_default_viewport_x = x;
    m_default_viewport_y = y;
    m_default_viewport_w = w;
    m_default_viewport_h = h;
}

bool
GLSLRenderList::build( const std::string& visual_scene )
{

    if( m_renderlist.build( visual_scene ) ) {
        majorUpdate();
    }
    else {
        minorUpdate();
    }
    return false;
}

void
GLSLRenderList::clear()
{
    Logger log = getLogger( package + ".clear" );
    SCENELOG_DEBUG( log, "Invoked." );
    for( auto it=m_glsl_list.begin(); it!=m_glsl_list.end(); ++it ) {
        if( it->m_type == GLSLRenderAction::GLSL_ACTION_SET_UNIFORMS ) {
            delete[] (it->m_set_uniforms.m_values);
        }
    }

    m_glsl_list.clear();
    m_renderlist.clear();
    m_valid = false;
}

void
GLSLRenderList::minorUpdate()
{
    Logger log = getLogger( "Scene.Runtime.GLSLRenderList.minorUpdate" );

    if( !m_valid ) {
        return;
    }

    for( size_t i=0; i<m_renderlist.size(); i++ ) {
        const RenderAction* action = m_renderlist[i];
        switch( action->m_type ) {

        case RenderAction::ACTION_SET_INPUTS:
            for( size_t j=0; j<action->m_set_inputs.m_items.size(); j++ ) {
//                SCENELOG_DEBUG( log, "Checking " << action->m_set_inputs.m_items[j].m_source )
                m_runtime.buffer( action->m_set_inputs.m_items[j].m_source );
            }
            break;
        default:
            break;
        }
    }
}

void
GLSLRenderList::majorUpdate()
{
    Logger log = getLogger( "Scene.Runtime.GLSLRenderList.majorUpdate" );
    SCENELOG_DEBUG( log, "Rebuilding GL assets." );

    m_transform_cache.purge();

    glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
#ifdef SCENE_RL_CHUNKS

    m_glsl_items.clear();
    m_glsl_items.resize( m_renderlist.items() );
    for( size_t i=0; i<m_renderlist.items(); i++ ) {
        const RenderList::Item& item = m_renderlist.item(i);
        GLSLItem& glsl_item = m_glsl_items[i];
        glsl_item.m_bbox_test = NULL;

        // --- framebuffer
        if( item.m_action_set_framebuffer->m_set_render_targets.m_items.empty() ) {
            // Default FBO
            glsl_item.m_glsl_framebuffer = NULL;
            SCENELOG_DEBUG( log, "  viewport=[" << m_default_viewport_x << ", " << m_default_viewport_y << ", " << m_default_viewport_w << ", " << m_default_viewport_h );
            SCENELOG_DEBUG( log, "  fbo = 0" );
        }
        else {
            glsl_item.m_glsl_framebuffer = m_runtime.frameBuffer( item.m_action_set_framebuffer );
            if( glsl_item.m_glsl_framebuffer == NULL ) {
                SCENELOG_ERROR( log, "Set framebuffer failed." );
                continue;
            }
            SCENELOG_DEBUG( log, "  viewport=[0, 0, " <<  m_glsl_list[i].m_set_framebuffer->width() << ", " <<  m_glsl_list[i].m_set_framebuffer->height() );
            SCENELOG_DEBUG( log, "   fbo = " << m_glsl_list[i].m_set_framebuffer->fbo() );
        }

        // --- transforms


        // --- shader
        SCENELOG_ASSERT( log, item.m_action_set_pass->m_type == RenderAction::ACTION_SET_PASS );
        glsl_item.m_glsl_pass = m_runtime.shader( item.m_action_set_pass->m_set_pass.m_pass );
        if( glsl_item.m_glsl_pass == NULL ) {
            SCENELOG_ERROR( log, "Set pass failed." );
            continue;
        }
        SCENELOG_ASSERT( log, item.m_action_set_input->m_type == RenderAction::ACTION_SET_INPUTS );
        glsl_item.m_glsl_inputs = m_runtime.vbo( item.m_action_set_input );
        if( glsl_item.m_glsl_inputs == NULL ) {
            SCENELOG_ERROR( log, "Set inputs failed" );
            continue;
        }

        if( item.m_action_set_samplers != NULL ) {
            SCENELOG_ASSERT( log, item.m_action_set_samplers->m_type == RenderAction::ACTION_SET_SAMPLERS );
            glsl_item.m_glsl_samplers = m_runtime.samplers( item.m_action_set_samplers );
            if( glsl_item.m_glsl_samplers == NULL ) {
                SCENELOG_ERROR( log, "Set samplers failed." );
                continue;
            }
            for( size_t k=0; k<item.m_action_set_samplers->m_set_samplers.m_items.size(); k++ ) {
                SCENELOG_DEBUG( log, "  set samper unit " << k
                                << ": img='" << item.m_action_set_samplers->m_set_samplers.m_items[k].m_image->id()
                                << "', tex=" << glsl_item.m_glsl_samplers->textureName(k)
                                << ", sampler=" << glsl_item.m_glsl_samplers->sampler(k) );
            }
        }

        SCENELOG_ASSERT( log, item.m_action_set_uniform->m_type == RenderAction::ACTION_SET_UNIFORMS );
        glsl_item.m_uniform_values.resize( item.m_action_set_uniform->m_set_uniforms.m_items.size() );
        for( size_t k=0; k<glsl_item.m_uniform_values.size(); k++ ) {
            const Value* value = NULL;
            if( 0 <= glsl_item.m_glsl_pass->uniformLocation(k) ) {
                const RuntimeSemantic semantic = item.m_action_set_uniform->m_set_uniforms.m_items[k].m_semantic;
                if( semantic == RUNTIME_SEMANTIC_N ) {
                    // Pull value from database
                    value = item.m_action_set_uniform->m_set_uniforms.m_items[k].m_value;
                }
                else {
                    // Pull value from transform cache
                    value = m_transform_cache.runtimeSemantic( semantic,
                                                               &item.m_action_set_framebuffer->m_set_render_targets,
                                                               item.m_set_view_coordsys,
                                                               item.m_set_local_coordsys );


                }
                if( value->type() != glsl_item.m_glsl_pass->uniformType(k) ) {
                    SCENELOG_WARN( log, "uniform " << k
                                   << ": mismatch, expected type " << glsl_item.m_glsl_pass->uniformType( k )
                                   << ", got " << value->type() );

                }
            }
            glsl_item.m_uniform_values[k] = value;
        }

        const Geometry* geometry = NULL;
        if( item.m_draw != NULL ) {
            GLenum draw_mode = item.m_draw->m_mode;
            GLenum pass_mode = glsl_item.m_glsl_pass->expectedInputPrimitiveType();
            if( pass_mode != GL_ALWAYS ) {
                if( draw_mode != pass_mode ) {
                    SCENELOG_ERROR( log,
                                   "Shader expects primitive type 0x" << std::hex << pass_mode << std::dec <<
                                   " but draw command provides primitives of type 0x" << std::hex << draw_mode << std::dec );
                    continue;
                }
            }
            glsl_item.m_glsl_indices = NULL;
            geometry = item.m_draw->m_geometry;
        }
        else if( item.m_draw_indexed != NULL ) {
            GLenum draw_mode = item.m_draw_indexed->m_mode;
            GLenum pass_mode = glsl_item.m_glsl_pass->expectedInputPrimitiveType();
            if( pass_mode != GL_ALWAYS ) {
                if( draw_mode != pass_mode ) {
                    SCENELOG_ERROR( log,
                                   "Shader expects primitive type 0x" << std::hex << pass_mode << std::dec <<
                                   " but draw command provides primitives of type 0x" << std::hex << draw_mode << std::dec );
                    continue;
                }
            }
            glsl_item.m_glsl_indices = m_runtime.buffer( item.m_draw_indexed->m_index_buffer );
            if( glsl_item.m_glsl_indices == NULL ) {
                SCENELOG_ERROR( log, "Failed to retrieve draw indices." );
                continue;
            }
            geometry = item.m_draw_indexed->m_geometry;
        }
        else {
            SCENELOG_FATAL( log, "neither draw nor draw_indexed, shouldn't happen" );
            continue;
        }

        // everything worked out, add conditional on this item
        glsl_item.m_bbox_test = m_transform_cache.checkBoundingBox( item.m_set_view_coordsys,
                                                                    item.m_set_local_coordsys,
                                                                    geometry );

    }
#endif

    const SetRenderTargets* current_fbo = NULL;
    const SetViewCoordSys*  current_view_coordsys = NULL;
    const SetLocalCoordSys* current_local_coordsys = NULL;
    const GLSLShader*       current_pass = NULL;

    m_valid = true;
    m_glsl_list.resize( m_renderlist.size() );
    SCENELOG_DEBUG( log, "BEGIN" );
    for(size_t i=0; i<m_renderlist.size(); i++ ) {
        const RenderAction* action = m_renderlist[i];
        GLSLRenderAction* glsl_action = &m_glsl_list[i];

        glsl_action->m_type = GLSLRenderAction::GLSL_ACTION_EMPTY;


        switch( action->m_type ) {
        case RenderAction::ACTION_SET_VIEW_COORDSYS:
            SCENELOG_DEBUG( log, "SET_VIEW" );
            current_view_coordsys = &action->m_set_view;
            break;

        case RenderAction::ACTION_SET_LOCAL_COORDSYS:
            SCENELOG_DEBUG( log, "SET_LOCAL" );
            current_local_coordsys = &action->m_set_local;
            break;

        case RenderAction::ACTION_SET_FRAMEBUFFER:
            SCENELOG_DEBUG( log, "SET_FRAMEBUFFER" );



            if( action->m_set_render_targets.m_items.empty() ) {
                // default FBO
                m_glsl_list[i].m_set_framebuffer = NULL;
                current_fbo = NULL;
                SCENELOG_DEBUG( log, "  viewport=[" << m_default_viewport_x << ", " << m_default_viewport_y << ", " << m_default_viewport_w << ", " << m_default_viewport_h );
                SCENELOG_DEBUG( log, "  fbo = 0" );

            }
            else {
                m_glsl_list[i].m_set_framebuffer = m_runtime.frameBuffer( action );
                if( m_glsl_list[i].m_set_framebuffer == NULL ) {
                    SCENELOG_ERROR( log, "OpenGL failed, invalidating list." );
                    m_valid = false;
                    return;
                }
                current_fbo = &action->m_set_render_targets;

                SCENELOG_DEBUG( log, "  viewport=[0, 0, " <<  m_glsl_list[i].m_set_framebuffer->width() << ", " <<  m_glsl_list[i].m_set_framebuffer->height() );
                SCENELOG_DEBUG( log, "   fbo = " << m_glsl_list[i].m_set_framebuffer->fbo() );

            }
            break;

        case RenderAction::ACTION_SET_PASS:
            SCENELOG_DEBUG( log, "SET_PASS" );
            glsl_action->m_set_pass = m_runtime.shader( action->m_set_pass.m_pass );
            current_pass = glsl_action->m_set_pass;
            if( current_pass == NULL ) {
                SCENELOG_ERROR( log, "OpenGL failed, invalidating list." );
                m_valid = false;
                return;
            }
            break;
        case RenderAction::ACTION_SET_INPUTS:
            SCENELOG_DEBUG( log, "SET_INPUTS" );
            m_glsl_list[i].m_set_inputs = m_runtime.vbo( action );
            if( m_glsl_list[i].m_set_inputs == NULL ) {
                SCENELOG_ERROR( log, "OpenGL failed, invalidating list." );
                m_valid = false;
                return;
            }
            break;
        case RenderAction::ACTION_SET_SAMPLERS:
            SCENELOG_DEBUG( log, "SET_SAMPLERS" );
            m_glsl_list[i].m_set_samplers = m_runtime.samplers( action );
            if( m_glsl_list[i].m_set_samplers == NULL ) {
                SCENELOG_ERROR( log, "OpenGL failed, invalidating list." );
                m_valid = false;
                return;
            }
            for(size_t j=0; j<action->m_set_samplers.m_items.size(); j++ ) {
                SCENELOG_DEBUG( log, "  unit " << j << ": img='"<< action->m_set_samplers.m_items[j].m_image->id() <<
                                "', tex=" << m_glsl_list[i].m_set_samplers->textureName(j) <<
                                ", sampler=" << m_glsl_list[i].m_set_samplers->sampler(j) );
            }


            break;
        case RenderAction::ACTION_SET_UNIFORMS:
            SCENELOG_DEBUG( log, "SET_UNIFORMS" );

            glsl_action->m_type = GLSLRenderAction::GLSL_ACTION_SET_UNIFORMS;
            glsl_action->m_set_uniforms.m_count = action->m_set_uniforms.m_items.size();
            glsl_action->m_set_uniforms.m_values = new const Value*[ glsl_action->m_set_uniforms.m_count ]; // Allocate array of Value pointers
            for( size_t j=0; j<glsl_action->m_set_uniforms.m_count; j++ ) {
                const Value* value = NULL;

                if( current_pass->uniformLocation(j) != -1 ) {
                    const RuntimeSemantic semantic = action->m_set_uniforms.m_items[j].m_semantic;
                    if( semantic == RUNTIME_SEMANTIC_N ) {
                        // Pull value from database
                        value = action->m_set_uniforms.m_items[j].m_value;
                    }
                    else {
                        // Pull value from runtime system
                        value = m_transform_cache.runtimeSemantic( semantic,
                                                                   current_fbo,
                                                                   current_view_coordsys,
                                                                   current_local_coordsys );
                    }
                    if( value->type() != current_pass->uniformType( j ) ) {
                        SCENELOG_WARN( log, "uniform " << j << ": mismatch, expected type " << current_pass->uniformType( j ) << ", got " << value->type() );
                        value = NULL;
                    }
                }
                glsl_action->m_set_uniforms.m_values[j] = value;
            }

            for( size_t j=0; j<action->m_set_uniforms.m_items.size(); j++ ) {
                GLint loc = current_pass->uniformLocation( j );
                SCENELOG_DEBUG( log, "  loc=" << loc <<
                                ", val=" << action->m_set_uniforms.m_items[j].m_value->debugString() );
            }


            break;
        case RenderAction::ACTION_SET_FB_CTRL:
            SCENELOG_DEBUG( log, "SET_FB_CTRL" );
            break;
        case RenderAction::ACTION_SET_PIXEL_OPS:
            SCENELOG_DEBUG( log, "SET_PIXEL_OPS" );
            break;
        case RenderAction::ACTION_SET_RASTER:
            SCENELOG_DEBUG( log, "SET_RASTER" );
            break;
        case RenderAction::ACTION_DRAW:
            SCENELOG_DEBUG( log, "DRAW" );
            if( current_pass != NULL && current_pass->expectedInputPrimitiveType() != GL_ALWAYS ) {
                if( action->m_draw.m_mode != current_pass->expectedInputPrimitiveType() ) {
                    SCENELOG_ERROR( log,
                                   "Shader expects primitive type 0x" << std::hex << current_pass->expectedInputPrimitiveType() << std::dec <<
                                   " but draw command provides primitives of type 0x" << std::hex << action->m_draw.m_mode << std::dec );
                    m_valid = false;
                    return;
                }
            }
            m_glsl_list[i].m_draw.m_bbox_check = m_transform_cache.checkBoundingBox( current_view_coordsys,
                                                                                     current_local_coordsys,
                                                                                     action->m_draw.m_geometry );
            break;
        case RenderAction::ACTION_DRAW_INDEXED:
            SCENELOG_DEBUG( log, "DRAW_INDEXED" );
            if( current_pass != NULL && current_pass->expectedInputPrimitiveType() != GL_ALWAYS ) {
                if( action->m_draw_indexed.m_mode != current_pass->expectedInputPrimitiveType() ) {
                    SCENELOG_ERROR( log,
                                   "Shader expects primitive type 0x" << std::hex << current_pass->expectedInputPrimitiveType() << std::dec <<
                                   " but draw command provides primitives of type 0x" << std::hex << action->m_draw_indexed.m_mode << std::dec );
                    m_valid = false;
                    return;
                }
            }
            m_glsl_list[i].m_draw_indexed.m_bbox_check =
                    m_transform_cache.checkBoundingBox( current_view_coordsys,
                                                        current_local_coordsys,
                                                        action->m_draw_indexed.m_geometry );
            m_glsl_list[i].m_draw_indexed.m_indices = m_runtime.buffer( action->m_draw_indexed.m_index_buffer );
            if( m_glsl_list[i].m_draw_indexed.m_indices == NULL ) {
                SCENELOG_ERROR( log, "OpenGL failed, invalidating list." );
                m_valid = false;
                return;
            }
            break;
        }
    }
    SCENELOG_DEBUG( log, "END" );
}


void
GLSLRenderList::render( )
{
    Logger log = getLogger( "Scene.Runtime.GLSLRendererList.render" );
    size_t max_texture_unit = 0;
    if( !GLSLRuntime::checkGL( log ) ) {
        SCENELOG_ERROR( log, "Entered render function with pending GL errors, ignoring." );
    }
    m_transform_cache.update( m_default_viewport_w,
                              m_default_viewport_h );

#ifdef SCENE_RL_CHUNKS
    RenderList::Item dummy;
    GLSLItem glsl_dummy;
    memset( &dummy, 0, sizeof(RenderList::Item) );
    memset( &glsl_dummy, 0, sizeof( GLSLItem ) );

    const RenderList::Item* prev_item = &dummy;
    const GLSLItem* prev_glsl_item = &glsl_dummy;

    size_t skipped = 0;
    glBindFramebuffer( GL_FRAMEBUFFER, m_default_framebuffer );
    glViewport( m_default_viewport_x, m_default_viewport_y, m_default_viewport_w, m_default_viewport_h );
    for( size_t i=0; i<m_glsl_items.size(); i++ ) {
        const GLSLItem* glsl_item = &m_glsl_items[i];
        if( (glsl_item->m_bbox_test == NULL) || (glsl_item->m_bbox_test->boolData()[0] != GL_TRUE ) ) {
            skipped ++;
            continue;
        }
        const RenderList::Item* item = &m_renderlist.item( i );

        if( prev_glsl_item->m_glsl_framebuffer != glsl_item->m_glsl_framebuffer ) {
            if( glsl_item->m_glsl_framebuffer == NULL ) {                       // bind fbo
                glBindFramebuffer( GL_FRAMEBUFFER, m_default_framebuffer );
                glViewport( m_default_viewport_x, m_default_viewport_y, m_default_viewport_w, m_default_viewport_h );
            }
            else {
                glBindFramebuffer( GL_FRAMEBUFFER, glsl_item->m_glsl_framebuffer->fbo() );
                glViewport( 0, 0,
                            glsl_item->m_glsl_framebuffer->width(),
                            glsl_item->m_glsl_framebuffer->height() );
            }
        }

        if( (prev_glsl_item->m_glsl_framebuffer != glsl_item->m_glsl_framebuffer) ||
                (prev_item->m_set_fb_ctrl != item->m_set_fb_ctrl ) )
        {
                glColorMask( item->m_set_fb_ctrl->m_color_writemask[0],          // write masks
                             item->m_set_fb_ctrl->m_color_writemask[1],
                             item->m_set_fb_ctrl->m_color_writemask[2],
                             item->m_set_fb_ctrl->m_color_writemask[3] );
                glDepthMask( item->m_set_fb_ctrl->m_depth_writemask );
        }

        if( (prev_glsl_item->m_glsl_framebuffer != glsl_item->m_glsl_framebuffer) ||
                (prev_item->m_set_pixel_ops != item->m_set_pixel_ops ) )
        {
            if( item->m_set_pixel_ops->m_blend == GL_TRUE ) {                    // blending
                glEnable( GL_BLEND );
                glBlendFuncSeparate( item->m_set_pixel_ops->m_blend_src_rgb,
                                     item->m_set_pixel_ops->m_blend_dst_rgb,
                                     item->m_set_pixel_ops->m_blend_src_alpha,
                                     item->m_set_pixel_ops->m_blend_dst_alpha );
            }
            else {
                glDisable( GL_BLEND );
            }
            if( item->m_set_pixel_ops->m_depth_test == GL_TRUE ) {               // depth test
                glEnable( GL_DEPTH_TEST );
                glDepthFunc( item->m_set_pixel_ops->m_depth_func );
            }
            else {
                glDisable( GL_DEPTH_TEST );
            }
        }

        if( (prev_glsl_item->m_glsl_framebuffer != glsl_item->m_glsl_framebuffer) ||
                (prev_item->m_set_raster != item->m_set_raster ) )
        {
            glPointSize( item->m_set_raster->m_point_size );
            if( item->m_set_raster->m_cull_face == GL_TRUE ) {
                glEnable( GL_CULL_FACE );
                glCullFace( item->m_set_raster->m_cull_face_mode );
            }
            else {
                glDisable( GL_CULL_FACE );
            }
            if( item->m_set_raster->m_polygon_mode_front == item->m_set_raster->m_polygon_mode_back ) {
                glPolygonMode( GL_FRONT_AND_BACK, item->m_set_raster->m_polygon_mode_front );
            }
            else {
                glPolygonMode( GL_FRONT, item->m_set_raster->m_polygon_mode_front );
                glPolygonMode( GL_BACK, item->m_set_raster->m_polygon_mode_back );
            }
            glPolygonOffset( item->m_set_raster->m_polygon_offset_factor,
                             item->m_set_raster->m_polygon_offset_units );
            if( item->m_set_raster->m_polygon_offset_fill == GL_TRUE ) {
                glEnable( GL_POLYGON_OFFSET_FILL );
            }
            else {
                glDisable( GL_POLYGON_OFFSET_FILL );
            }
        }

        if( prev_glsl_item->m_glsl_pass != glsl_item->m_glsl_pass ) {
            glUseProgram( glsl_item->m_glsl_pass->program() );                  // use shader program
        }

        if( prev_glsl_item->m_glsl_inputs != glsl_item->m_glsl_inputs ) {
            glBindVertexArray( glsl_item->m_glsl_inputs->vertexArray() );       // bind vertex array object
        }


        // bind samplers and textures
        if( prev_item->m_action_set_samplers != item->m_action_set_samplers ) {
            if( item->m_action_set_samplers != NULL ) {
                for( size_t k=0; k<glsl_item->m_glsl_samplers->items(); k++ ) {
                    glBindSampler( k, glsl_item->m_glsl_samplers->sampler(k) );
                    glActiveTexture( GL_TEXTURE0 + k );
                    glBindTexture( glsl_item->m_glsl_samplers->target(k),
                                   glsl_item->m_glsl_samplers->textureName(k) );
                    if( glsl_item->m_glsl_samplers->texture(k)->doBuildMipMap() ) {
                        glGenerateMipmap( glsl_item->m_glsl_samplers->target(k) );
                    }
                }
                max_texture_unit = max( max_texture_unit, glsl_item->m_glsl_samplers->items() );
            }
        }

        if(1) {
            for( size_t k=0; k<glsl_item->m_uniform_values.size(); k++ ) {
                const Value* value = glsl_item->m_uniform_values[k];
                if( value != NULL ) {
                    GLint loc = glsl_item->m_glsl_pass->uniformLocation(k);
                    switch( value->type() ) {
                    case VALUE_TYPE_INT:
                        glUniform1iv( loc, 1, value->intData() );
                        break;

                    case VALUE_TYPE_FLOAT:
                        glUniform1fv( loc, 1, value->floatData() );
                        break;

                    case VALUE_TYPE_FLOAT2:
                        glUniform2fv( loc, 1, value->floatData() );
                        break;

                    case VALUE_TYPE_FLOAT3:
                        glUniform3fv( loc, 1, value->floatData() );
                        break;

                    case VALUE_TYPE_FLOAT4:
                        glUniform4fv( loc, 1, value->floatData() );
                        break;

                    case VALUE_TYPE_FLOAT3X3:
                        glUniformMatrix3fv( loc, 1, GL_FALSE, value->floatData() );
                        break;

                    case VALUE_TYPE_FLOAT4X4:
                        glUniformMatrix4fv( loc, 1, GL_FALSE, value->floatData() );
                        break;

                    case VALUE_TYPE_BOOL:
                    case VALUE_TYPE_ENUM:
                    case VALUE_TYPE_ENUM2:
                    case VALUE_TYPE_SAMPLER1D:
                    case VALUE_TYPE_SAMPLER2D:
                    case VALUE_TYPE_SAMPLER3D:
                    case VALUE_TYPE_SAMPLERCUBE:
                    case VALUE_TYPE_SAMPLERDEPTH:
                    case VALUE_TYPE_N:
                        break;
                    }
                }
            }
        }

        if( item->m_draw != NULL ) {
            if( item->m_draw->m_mode == GL_PATCHES ) {
                glPatchParameteri( GL_PATCH_VERTICES, item->m_draw->m_vertices );
            }
            glDrawArrays( item->m_draw->m_mode, item->m_draw->m_first, item->m_draw->m_count );
        }
        if( item->m_draw_indexed != NULL ) {
            if( item->m_draw_indexed->m_mode == GL_PATCHES ) {
                glPatchParameteri( GL_PATCH_VERTICES, item->m_draw_indexed->m_vertices );
            }
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, glsl_item->m_glsl_indices->buffer() );
            glDrawElements( item->m_draw_indexed->m_mode,
                            item->m_draw_indexed->m_count,
                            item->m_draw_indexed->m_type,
                            //                            GL_UNSIGNED_INT, //m_glsl_list[i].m_draw_indexed->elementType(),
                            item->m_draw_indexed->m_offset );
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        }





        prev_item = item;
        prev_glsl_item = glsl_item;



        /*


        case RenderAction::ACTION_DRAW:

            break;

        case RenderAction::ACTION_DRAW_INDEXED:
            break;
        }
    }
*/

    }
    if( skipped != 0 ) {
//        SCENELOG_ERROR( log, "skipped " << (int)((100.f*skipped)/m_glsl_items.size()) << "% items." );
    }


#else


    if(!m_valid ) {
        SCENELOG_TRACE( log, "Invalid GL render list." );
        return;
    }



    size_t skipped = 0;

    const GLSLShader* current_pass = NULL;
    const GLSLVertexArray* current_vao = NULL;
    for(size_t i=0; i<m_renderlist.size(); i++ ) {
        const RenderAction* action = m_renderlist[i];

        switch( action->m_type ) {

        case RenderAction::ACTION_SET_VIEW_COORDSYS:
            break;

        case RenderAction::ACTION_SET_LOCAL_COORDSYS:
            break;

        case RenderAction::ACTION_SET_FRAMEBUFFER:
            if( m_glsl_list[i].m_set_framebuffer == NULL ) {
                glBindFramebuffer( GL_FRAMEBUFFER, m_default_framebuffer );
                glViewport( m_default_viewport_x, m_default_viewport_y, m_default_viewport_w, m_default_viewport_h );
            }
            else {
                glBindFramebuffer( GL_FRAMEBUFFER, m_glsl_list[i].m_set_framebuffer->fbo() );
                glViewport( 0, 0,
                            m_glsl_list[i].m_set_framebuffer->width(),
                            m_glsl_list[i].m_set_framebuffer->height() );
            }

            break;

        case RenderAction::ACTION_SET_PASS:
            current_pass = m_glsl_list[i].m_set_pass;
            glUseProgram( current_pass->program() );
            break;
        case RenderAction::ACTION_SET_INPUTS:
            current_vao = m_glsl_list[i].m_set_inputs;
            glBindVertexArray( current_vao->vertexArray() );
            break;

        case RenderAction::ACTION_SET_SAMPLERS:
            for( size_t k=0; k<m_glsl_list[i].m_set_samplers->items(); k++ ) {
                GLSLTexture* tex = m_glsl_list[i].m_set_samplers->texture(k);

                glBindSampler( k,m_glsl_list[i].m_set_samplers->sampler(k) );
                glActiveTexture( GL_TEXTURE0 + k );
                glBindTexture( m_glsl_list[i].m_set_samplers->target(k),
                               m_glsl_list[i].m_set_samplers->textureName(k) );
                if( tex->doBuildMipMap() ) {
                    glGenerateMipmap( tex->target() );
                }

            }
            max_texture_unit = max( max_texture_unit, m_glsl_list[i].m_set_samplers->items() );
            break;

        case RenderAction::ACTION_SET_UNIFORMS:
            for( size_t j=0; j<action->m_set_uniforms.m_items.size(); j++ ) {
                const Value* value = m_glsl_list[i].m_set_uniforms.m_values[j];
                if( value != NULL ) {
                    GLint loc = current_pass->uniformLocation( j );

                    switch( value->type() ) {
                    case VALUE_TYPE_INT:
                        glUniform1iv( loc, 1, value->intData() );
                        break;

                    case VALUE_TYPE_FLOAT:
                        glUniform1fv( loc, 1, value->floatData() );
                        break;

                    case VALUE_TYPE_FLOAT2:
                        glUniform2fv( loc, 1, value->floatData() );
                        break;

                    case VALUE_TYPE_FLOAT3:
                        glUniform3fv( loc, 1, value->floatData() );
                        break;

                    case VALUE_TYPE_FLOAT4:
                        glUniform4fv( loc, 1, value->floatData() );
                        break;

                    case VALUE_TYPE_FLOAT3X3:
                        glUniformMatrix3fv( loc, 1, GL_FALSE, value->floatData() );
                        break;

                    case VALUE_TYPE_FLOAT4X4:
                        glUniformMatrix4fv( loc, 1, GL_FALSE, value->floatData() );
                        break;

                    case VALUE_TYPE_BOOL:
                    case VALUE_TYPE_ENUM:
                    case VALUE_TYPE_ENUM2:
                    case VALUE_TYPE_SAMPLER1D:
                    case VALUE_TYPE_SAMPLER2D:
                    case VALUE_TYPE_SAMPLER3D:
                    case VALUE_TYPE_SAMPLERCUBE:
                    case VALUE_TYPE_SAMPLERDEPTH:
                    case VALUE_TYPE_N:
                        SCENELOG_DEBUG( log, "Ignoring uniform loc " << loc );
                        break;
                    }
                }
            }
            break;

        case RenderAction::ACTION_SET_FB_CTRL:
            glColorMask( action->m_set_fb_ctrl.m_color_writemask[0],
                         action->m_set_fb_ctrl.m_color_writemask[1],
                         action->m_set_fb_ctrl.m_color_writemask[2],
                         action->m_set_fb_ctrl.m_color_writemask[3] );
            glDepthMask( action->m_set_fb_ctrl.m_depth_writemask );
            break;

        case RenderAction::ACTION_SET_PIXEL_OPS:
            if( action->m_set_pixel_ops.m_blend == GL_TRUE ) {
                glEnable( GL_BLEND );
                glBlendFuncSeparate( action->m_set_pixel_ops.m_blend_src_rgb,
                                     action->m_set_pixel_ops.m_blend_dst_rgb,
                                     action->m_set_pixel_ops.m_blend_src_alpha,
                                     action->m_set_pixel_ops.m_blend_dst_alpha );
            }
            else {
                glDisable( GL_BLEND );
            }
            if( action->m_set_pixel_ops.m_depth_test == GL_TRUE ) {
                glEnable( GL_DEPTH_TEST );
                glDepthFunc( action->m_set_pixel_ops.m_depth_func );
            }
            else {
                glDisable( GL_DEPTH_TEST );
            }
            break;

        case RenderAction::ACTION_SET_RASTER:
            glPointSize( action->m_set_rasterization.m_point_size );
            if( action->m_set_rasterization.m_cull_face == GL_TRUE ) {
                glEnable( GL_CULL_FACE );
                glCullFace( action->m_set_rasterization.m_cull_face_mode );
            }
            else {
                glDisable( GL_CULL_FACE );
            }
            if( action->m_set_rasterization.m_polygon_mode_front == action->m_set_rasterization.m_polygon_mode_back ) {
                glPolygonMode( GL_FRONT_AND_BACK, action->m_set_rasterization.m_polygon_mode_front );
            }
            else {
                glPolygonMode( GL_FRONT, action->m_set_rasterization.m_polygon_mode_front );
                glPolygonMode( GL_BACK, action->m_set_rasterization.m_polygon_mode_back );
            }
            glPolygonOffset( action->m_set_rasterization.m_polygon_offset_factor,
                             action->m_set_rasterization.m_polygon_offset_units );
            if( action->m_set_rasterization.m_polygon_offset_fill == GL_TRUE ) {
                glEnable( GL_POLYGON_OFFSET_FILL );
            }
            else {
                glDisable( GL_POLYGON_OFFSET_FILL );
            }
            break;

        case RenderAction::ACTION_DRAW:

            if( m_glsl_list[i].m_draw.m_bbox_check->boolData()[0] == GL_TRUE ) {
                if( action->m_draw.m_mode == GL_PATCHES ) {
                    glPatchParameteri( GL_PATCH_VERTICES, action->m_draw.m_vertices );
                }
                glDrawArrays( action->m_draw.m_mode, action->m_draw.m_first, action->m_draw.m_count );
            }
            else {
                skipped++;
            }
            break;

        case RenderAction::ACTION_DRAW_INDEXED:
            if( m_glsl_list[i].m_draw_indexed.m_bbox_check->boolData()[0] == GL_TRUE ) {
                if( action->m_draw_indexed.m_mode == GL_PATCHES ) {
                    glPatchParameteri( GL_PATCH_VERTICES, action->m_draw_indexed.m_vertices );
                }
                glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_glsl_list[i].m_draw_indexed.m_indices->buffer() );
                glDrawElements( action->m_draw_indexed.m_mode,
                                action->m_draw_indexed.m_count,
                                action->m_draw_indexed.m_type,
                                //                            GL_UNSIGNED_INT, //m_glsl_list[i].m_draw_indexed->elementType(),
                                action->m_draw_indexed.m_offset );
                glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
            }
            else {
                skipped++;
            }
            break;
        }
    }

#endif
    glBindFramebuffer( GL_FRAMEBUFFER, m_default_framebuffer );
    for(size_t i=0; i<max_texture_unit; i++) {
        glBindSampler( i, 0 );
        glActiveTexture( GL_TEXTURE0 + i );
        glBindTexture( GL_TEXTURE_2D, 0 );
    }
    glActiveTexture( GL_TEXTURE0 );
    glUseProgram( 0 );
    glBindVertexArray( 0 );
    if( !GLSLRuntime::checkGL( log ) ) {
        SCENELOG_ERROR( log, "One or more OpenGL errors was produced by the "
                        "renderlist, invalidating list." );
        m_valid = false;
    }
    //if( skipped != 0 ) {
    //    SCENELOG_ERROR( log, "skipped=" << skipped );
    //}
}



    } // of namespace Runtime
} // of namespace Scene
