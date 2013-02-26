#include <cstring>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <scene/Log.hpp>
#include <scene/SourceBuffer.hpp>
#include <scene/Image.hpp>
#include <scene/Pass.hpp>
#include <scene/Light.hpp>
#include <scene/Utils.hpp>
#include <scene/tinia/Bridge.hpp>
#include <tinia/renderlist/Buffer.hpp>
#include <tinia/renderlist/SetInputs.hpp>
#include <tinia/renderlist/SetUniforms.hpp>
#include <tinia/renderlist/SetViewCoordSys.hpp>
#include <tinia/renderlist/SetLocalCoordSys.hpp>
#include <tinia/renderlist/SetLight.hpp>
#include <tinia/renderlist/SetShader.hpp>
#include <tinia/renderlist/Draw.hpp>

using std::unordered_map;
namespace rl = tinia::renderlist;

namespace Scene {
    namespace Tinia {

static const std::string package = "Scene.Runtime.XMLRuntime";

Bridge::Bridge( const DataBase&     database,
                        ProfileType         profile,
                        const std::string&  platform )
    : m_database( database ),
      m_resolver( database, profile, platform ),
      m_renderlist( m_resolver ),
      m_transform_cache( database )
{
    m_last_update.invalidate();

}

Bridge::~Bridge()
{

}

bool
Bridge::build( const std::string& visual_scene )
{
//    Logger log = getLogger( package + ".build" );
    if( m_renderlist.build( visual_scene) || !m_last_update.asRecentAs( m_database.valueChanged() ) ) {
        push();
        m_last_update.touch();
        return true;
    }
    else {
        return false;
    }
}

Runtime::Resolver&
Bridge::resolver()
{
    return m_resolver;
}

const rl::DataBase&
Bridge::renderListDataBase() const
{
    return m_renderlist_db;
}

void
Bridge::push()
{
    Logger log = getLogger( package + ".push" );

    // Step 1, find items that needs updating
    unordered_map< Runtime::CacheKey<1>, const SourceBuffer* > source_buffers;
    unordered_map< Runtime::CacheKey<1>, const Image* > images;
    unordered_map< Runtime::CacheKey<1>, const Pass* > shaders;
    unordered_map< Runtime::CacheKey<1>, const Runtime::RenderAction* > actions;
    if( !m_last_update.asRecentAs( m_database.structureChanged() ) ) {
        m_transform_cache.purge();
    }

#ifdef SCENE_RL_CHUNKS
    for( size_t i=0; i<m_renderlist.items(); i++ ) {
        const Runtime::RenderList::Item* item = &m_renderlist.item(i);

        // tag vertex data for pulling
        for( size_t k=0; k<item->m_action_set_input->m_set_inputs.m_items.size(); k++ ) {
            const SourceBuffer* buf = item->m_action_set_input->m_set_inputs.m_items[k].m_source;
            Runtime::CacheKey<1> key( buf );
            auto it = source_buffers.find( key );
            if( it == source_buffers.end() ) {
                if( (m_renderlist_db.itemByName( buf->idString() ) == NULL ) || (!m_last_update.asRecentAs( buf->valueChanged() ) ) ) {
                    source_buffers[ key ] = buf;
                }
            }
        }

        // tag index data for pulling
        if( item->m_draw_indexed != NULL ) {
            const SourceBuffer* buf = item->m_draw_indexed->m_index_buffer;
            Runtime::CacheKey<1> key( buf );
            auto it = source_buffers.find( key );
            if( it == source_buffers.end() ) {
                if( (m_renderlist_db.itemByName( buf->idString() ) == NULL ) || (!m_last_update.asRecentAs( buf->valueChanged() ) ) ) {
                    source_buffers[ key ] = buf;
                }
            }
        }

        // tag shader for pulling
        if(1) {
            const Pass* pass = item->m_set_pass->m_pass;
            Runtime::CacheKey<1> key( pass );
            auto it = shaders.find( key );
            if( it == shaders.end() ) {
                if( (m_renderlist_db.itemByName( pass->idString() ) == NULL ) || (!m_last_update.asRecentAs( pass->valueChanged() ) ) ) {
                    shaders[ key ] = pass;
                }
            }
        }

        // tag textures for pulling
        if( item->m_action_set_samplers != NULL ) {
            for(size_t k=0; k<item->m_set_samplers->m_items.size(); k++ ) {
                const Image* image = item->m_set_samplers->m_items[k].m_image;
                Runtime::CacheKey<1> key( image );
                auto it = images.find( key );
                if( it == images.end() ) {
                    if( !m_last_update.asRecentAs( image->valueChanged() ) ) {
                        images[ key ] = image;
                    }
                }
            }
        }

        // tag render targets for pulling
        for( size_t k=0; k<item->m_set_render_targets->m_items.size(); k++ ) {
            const Image* image = item->m_set_render_targets->m_items[k].m_image;
            Runtime::CacheKey<1> key( image );
            auto it = images.find( key );
            if( it == images.end() ) {
                if( !m_last_update.asRecentAs( image->valueChanged() ) ) {
                    images[ key ] = image;
                }
            }
        }

        // tag matrices that are needed
        m_transform_cache.runtimeSemantic( RUNTIME_PROJECTION_MATRIX, NULL, item->m_set_view_coordsys, NULL );
        m_transform_cache.runtimeSemantic( RUNTIME_PROJECTION_INVERSE_MATRIX, NULL, item->m_set_view_coordsys, NULL );
        m_transform_cache.runtimeSemantic( RUNTIME_EYE_FROM_WORLD, NULL, item->m_set_view_coordsys, NULL );
        m_transform_cache.runtimeSemantic( RUNTIME_WORLD_FROM_EYE, NULL, item->m_set_view_coordsys, NULL );
        for( size_t k=0; k<SCENE_LIGHTS_MAX; k++ ) {
            if( item->m_set_view_coordsys->m_lights[k] != NULL ) {
                RuntimeSemantic lfw = (RuntimeSemantic)(RUNTIME_LIGHT0_EYE_FROM_WORLD + i);
                m_transform_cache.runtimeSemantic( lfw, NULL, item->m_set_view_coordsys, NULL );
                RuntimeSemantic wfl =  (RuntimeSemantic)(RUNTIME_WORLD_FROM_LIGHT0_EYE + i);
                m_transform_cache.runtimeSemantic( wfl, NULL, item->m_set_view_coordsys, NULL );
            }
        }
        m_transform_cache.runtimeSemantic( RUNTIME_WORLD_FROM_OBJECT, NULL, NULL, item->m_set_local_coordsys );
        m_transform_cache.runtimeSemantic( RUNTIME_OBJECT_FROM_WORLD, NULL, NULL, item->m_set_local_coordsys );
    }
    m_transform_cache.update( 1, 1 );

    // -- push buffer objects
    for( auto it=source_buffers.begin(); it!=source_buffers.end(); ++it ) {
        const SourceBuffer* bd = it->second;
        std::string id = bd->idString();
        rl::Buffer* bs = m_renderlist_db.castedItemByName<rl::Buffer*>( id );
        if( bs == NULL ) {
            bs = m_renderlist_db.createBuffer( id );
        }
        switch( bd->elementType() ) {
        case ELEMENT_INT:
            bs->set( bd->intData(), bd->elementCount() );
            SCENELOG_INFO( log, "buffer[" <<id<<"] (" << bd->id() << ") = int(..."<< bd->elementCount() << "...)" );
            break;
        case ELEMENT_FLOAT:
            bs->set( bd->floatData(), bd->elementCount() );
            SCENELOG_INFO( log, "buffer[" <<id<<"] (" << bd->id() << ") = float(..."<< bd->elementCount() << "...)" );
            break;
        }
    }

    // --- push images
    for( auto it=images.begin(); it!=images.end(); ++it ) {
        const Image* di = it->second;
        std::string id = di->idString();
        SCENELOG_DEBUG( log, "image[" << id << "] <- ignored" );
    }

    // --- push shaders
    for( auto it=shaders.begin(); it!=shaders.end(); ++it ) {
        const Pass* ss = it->second;
        rl::Shader* ls = m_renderlist_db.castedItemByName<rl::Shader*>( ss->idString() );
        if( ls == NULL ) {
            ls = m_renderlist_db.createShader( ss->idString() );
        }
        if( !ss->shaderSource( STAGE_VERTEX ).empty() ) {
            ls->setVertexStage( ss->shaderSource( STAGE_VERTEX ) );
            SCENELOG_INFO( log, "+- vertex shader" );
        }
        if( !ss->shaderSource( STAGE_TESSELLATION_CONTROL ).empty() ) {
            ls->setTessCtrlStage( ss->shaderSource( STAGE_TESSELLATION_CONTROL ) );
            SCENELOG_INFO( log, "+- tessellation control shader" );
        }
        if( !ss->shaderSource( STAGE_TESSELLATION_EVALUATION ).empty() ) {
            ls->setTessEvalStage( ss->shaderSource( STAGE_TESSELLATION_EVALUATION ) );
            SCENELOG_INFO( log, "+- tessellation evaluation shader" );
        }
        if( !ss->shaderSource( STAGE_GEOMETRY ).empty() ) {
            ls->setGeometryStage( ss->shaderSource( STAGE_GEOMETRY ) );
            SCENELOG_INFO( log, "+- geometry shader" );
        }
        if( !ss->shaderSource( STAGE_FRAGMENT ).empty() ) {
            ls->setFragmentStage( ss->shaderSource( STAGE_FRAGMENT ) );
            SCENELOG_INFO( log, "+- fragment shader" );
        }
    }

    Runtime::RenderList::Item dummy;
    memset( &dummy, 0, sizeof(Runtime::RenderList::Item) );

    const Runtime::RenderList::Item* prev_item = &dummy;

    m_renderlist_db.drawOrderClear();
    for( size_t i=0; i<m_renderlist.items(); i++ ) {
        const Runtime::RenderList::Item* item = &m_renderlist.item(i);
        if( prev_item->m_set_render_targets != item->m_set_render_targets ) {
            const std::string name = item->m_set_render_targets->idString();

            rl::SetFramebuffer* a = m_renderlist_db.castedItemByName<rl::SetFramebuffer*>( name );
            if( a == NULL ) {
                a = m_renderlist_db.createAction<rl::SetFramebuffer>( name );
            }
            m_renderlist_db.drawOrderAdd( name );
        }

        if( prev_item->m_set_view_coordsys != item->m_set_view_coordsys ) {
            const std::string name = item->m_set_view_coordsys->idString();
            rl::SetViewCoordSys* a = m_renderlist_db.castedItemByName<rl::SetViewCoordSys*>( name );
            if( a == NULL ) {
                a = m_renderlist_db.createAction<rl::SetViewCoordSys>( name );
            }
            a->setProjection( m_transform_cache.runtimeSemantic( RUNTIME_PROJECTION_MATRIX,
                                                                 NULL, item->m_set_view_coordsys, NULL )->floatData(),
                              m_transform_cache.runtimeSemantic( RUNTIME_PROJECTION_INVERSE_MATRIX,
                                                                 NULL, item->m_set_view_coordsys, NULL )->floatData() );
            a->setOrientation( m_transform_cache.runtimeSemantic( RUNTIME_EYE_FROM_WORLD,
                                                                  NULL, item->m_set_view_coordsys, NULL )->floatData(),
                               m_transform_cache.runtimeSemantic( RUNTIME_EYE_FROM_WORLD,
                                                                  NULL, item->m_set_view_coordsys, NULL )->floatData() );
            m_renderlist_db.drawOrderAdd( name );

            for(int k=0; k<SCENE_LIGHTS_MAX; k++) {
                const Light* l = item->m_set_view_coordsys->m_lights[k];
                if( l != NULL ) {
                    const std::string name = item->m_set_view_coordsys->idString() + "_" + l->idString();
                    rl::SetLight* a = m_renderlist_db.castedItemByName<rl::SetLight*>( name );
                    if( a == NULL ) {
                        a = m_renderlist_db.createAction<rl::SetLight>( name );
                    }
                    a->setIndex( k );
                    switch( l->type() ) {
                    case Light::LIGHT_NONE:         a->setType( rl::LIGHT_AMBIENT ); break;
                    case Light::LIGHT_AMBIENT:      a->setType( rl::LIGHT_AMBIENT ); break;
                    case Light::LIGHT_DIRECTIONAL:  a->setType( rl::LIGHT_DIRECTIONAL ); break;
                    case Light::LIGHT_POINT:        a->setType( rl::LIGHT_POINT ); break;
                    case Light::LIGHT_SPOT:         a->setType( rl::LIGHT_SPOT ); break;
                    }
                    a->setColor( l->color()->floatData()[0],
                                 l->color()->floatData()[1],
                                 l->color()->floatData()[2] );
                    a->setAttenuation( l->constantAttenuation()->floatData()[0],
                                       l->linearAttenuation()->floatData()[0],
                                       l->quadraticAttenuation()->floatData()[0] );
                    a->setFalloff( l->falloffAngle()->floatData()[0],
                                   l->falloffExponent()->floatData()[0] );
                    a->setOrientation( m_transform_cache.runtimeSemantic( (RuntimeSemantic)(RUNTIME_LIGHT0_EYE_FROM_WORLD + k),
                                                                          NULL, item->m_set_view_coordsys, NULL )->floatData(),
                                       m_transform_cache.runtimeSemantic( (RuntimeSemantic)(RUNTIME_WORLD_FROM_LIGHT0_EYE + k),
                                                                          NULL, item->m_set_view_coordsys, NULL )->floatData() );
                    m_renderlist_db.drawOrderAdd( name );
                }
            }
        }

        if( prev_item->m_set_local_coordsys != item->m_set_local_coordsys ) {
            const std::string name = item->m_set_local_coordsys->idString();
            rl::SetLocalCoordSys* a = m_renderlist_db.castedItemByName<rl::SetLocalCoordSys*>( name );
            if( a == NULL ) {
                a = m_renderlist_db.createAction<rl::SetLocalCoordSys>( name );
            }
            a->setOrientation( m_transform_cache.runtimeSemantic( RUNTIME_OBJECT_FROM_WORLD,
                                                                  NULL, NULL, item->m_set_local_coordsys )->floatData(),
                               m_transform_cache.runtimeSemantic( RUNTIME_WORLD_FROM_OBJECT,
                                                                  NULL, NULL, item->m_set_local_coordsys )->floatData() );
            m_renderlist_db.drawOrderAdd( name );
        }

        if( prev_item->m_set_pass != item->m_set_pass ) {
            const std::string name = item->m_set_pass->idString();
            rl::SetShader* rl_action = m_renderlist_db.castedItemByName<rl::SetShader*>( name );
            if( rl_action == NULL ) {
                rl_action = m_renderlist_db.createAction<rl::SetShader>( name );
            }
            rl_action->setShader( item->m_set_pass->m_pass->idString() );
            m_renderlist_db.drawOrderAdd( name );
        }

        if( prev_item->m_set_inputs != item->m_set_inputs ) {
            const std::string name = item->m_set_inputs->idString();
            rl::SetInputs* si = m_renderlist_db.castedItemByName<rl::SetInputs*>( name );
            if( si == NULL ) {
                si = m_renderlist_db.createAction<rl::SetInputs>( name );
            }
            si->setShader( item->m_set_inputs->m_pass->idString() );

            si->clearInputs();
            for( size_t k=0; k<item->m_set_inputs->m_items.size(); k++ ) {
                si->setInput( item->m_set_inputs->m_pass->attributeSymbol(k),
                              item->m_set_inputs->m_items[k].m_source->idString(),
                              item->m_set_inputs->m_items[k].m_components,
                              item->m_set_inputs->m_items[k].m_offset,
                              item->m_set_inputs->m_items[k].m_stride );
            }
            m_renderlist_db.drawOrderAdd( name );
        }

        if(1) {
            const std::string name = item->m_set_uniforms->idString();
            rl::SetUniforms* su = m_renderlist_db.castedItemByName<rl::SetUniforms*>( name );
            if( su == NULL ) {
                su = m_renderlist_db.createAction<rl::SetUniforms>( name );
            }
            su->setShader( item->m_set_uniforms->m_pass->idString() );
            su->clear();

            for( size_t j=0; j<item->m_set_uniforms->m_items.size(); j++ ) {
                const Runtime::SetUniforms::Item& m = item->m_set_uniforms->m_items[j];
                const std::string& sym = item->m_set_uniforms->m_pass->uniformSymbol(j);
                if( m.m_semantic != RUNTIME_SEMANTIC_N ) {
                    rl::UniformSemantic semantic;
                    switch( m.m_semantic ) {
                    case RUNTIME_MODELVIEW_PROJECTION_MATRIX:
                        semantic = rl::SEMANTIC_MODELVIEW_PROJECTION_MATRIX;
                        break;
                    case RUNTIME_NORMAL_MATRIX:
                        semantic = rl::SEMANTIC_NORMAL_MATRIX;
                        break;
                    default:
                        SCENELOG_ERROR( log, "Symbol '" << sym << "' has unsupported semantic " << m.m_semantic );
                        continue;
                    }
                    su->setSemantic( sym, semantic );
                }
                else {
                    switch( m.m_value->type() ) {
                    case VALUE_TYPE_INT:      su->setInt1( sym, m.m_value->intData()[0] ); break;
                    case VALUE_TYPE_FLOAT:    su->setFloat1v( sym, m.m_value->floatData() ); break;
                    case VALUE_TYPE_FLOAT2:   su->setFloat2v( sym, m.m_value->floatData() ); break;
                    case VALUE_TYPE_FLOAT3:   su->setFloat3v( sym, m.m_value->floatData() ); break;
                    case VALUE_TYPE_FLOAT4:   su->setFloat4v( sym, m.m_value->floatData() ); break;
                    case VALUE_TYPE_FLOAT3X3: su->setFloat3x3v( sym, m.m_value->floatData() ); break;
                    case VALUE_TYPE_FLOAT4X4: su->setFloat4x4v( sym, m.m_value->floatData() ); break;
                    case VALUE_TYPE_BOOL:
                    case VALUE_TYPE_SAMPLER1D:
                    case VALUE_TYPE_SAMPLER2D:
                    case VALUE_TYPE_SAMPLER3D:
                    case VALUE_TYPE_SAMPLERCUBE:
                    case VALUE_TYPE_SAMPLERDEPTH: su->setInt1( sym, m.m_value->intData()[0] ); break;
                    default:
                        break;
                    }
                }
            }
            m_renderlist_db.drawOrderAdd( name );
        }

        if( prev_item->m_set_samplers != item->m_set_samplers) {
        }

        if( prev_item->m_set_fb_ctrl != item->m_set_fb_ctrl ) {
        }

        if( prev_item->m_set_pixel_ops != item->m_set_pixel_ops ) {
        }

        if( prev_item->m_set_raster != item->m_set_raster ) {
        }

        if( item->m_draw != NULL ) {
            const std::string name = item->m_draw->idString();
            rl::Draw* rl_action = m_renderlist_db.castedItemByName<rl::Draw*>( name );
            if( rl_action == NULL ) {
                rl_action = m_renderlist_db.createAction<rl::Draw>( name );
            }
            rl::PrimitiveType type = rl::PRIMITIVE_POINTS;
            switch( item->m_draw->m_mode ) {
            case GL_POINTS:         type = rl::PRIMITIVE_POINTS; break;
            case GL_LINES:          type = rl::PRIMITIVE_LINES; break;
            case GL_LINE_STRIP:     type = rl::PRIMITIVE_LINE_STRIP; break;
            case GL_LINE_LOOP:      type = rl::PRIMITIVE_LINE_LOOP; break;
            case GL_TRIANGLES:      type = rl::PRIMITIVE_TRIANGLES; break;
            case GL_TRIANGLE_STRIP: type = rl::PRIMITIVE_TRIANGLE_STRIP; break;
            case GL_TRIANGLE_FAN:   type = rl::PRIMITIVE_TRIANGLE_FAN; break;
            case GL_QUADS:          type = rl::PRIMITIVE_QUADS; break;
            case GL_QUAD_STRIP:     type = rl::PRIMITIVE_QUAD_STRIP; break;
            }
            rl_action->setNonIndexed( type,
                                      item->m_draw->m_first,
                                      item->m_draw->m_count );
            m_renderlist_db.drawOrderAdd( name );
        }

         if( item->m_draw_indexed != NULL ) {
            rl::Buffer* rl_buffer = m_renderlist_db.castedItemByName<rl::Buffer*>( item->m_draw_indexed->m_index_buffer->idString() );
            if( rl_buffer == NULL ) {
                SCENELOG_ERROR( log, "renderlist buffer '" <<  item->m_draw_indexed->m_index_buffer->idString() << "' doesn't exist!" );
            }
            else {
                const std::string name = item->m_draw_indexed->idString();
                rl::Draw* rl_action = m_renderlist_db.castedItemByName<rl::Draw*>( name );
                if( rl_action == NULL ) {
                    rl_action = m_renderlist_db.createAction<rl::Draw>( name );
                }
                rl::PrimitiveType type = rl::PRIMITIVE_POINTS;
                switch( item->m_draw_indexed->m_mode ) {
                case GL_POINTS:         type = rl::PRIMITIVE_POINTS; break;
                case GL_LINES:          type = rl::PRIMITIVE_LINES; break;
                case GL_LINE_STRIP:     type = rl::PRIMITIVE_LINE_STRIP; break;
                case GL_LINE_LOOP:      type = rl::PRIMITIVE_LINE_LOOP; break;
                case GL_TRIANGLES:      type = rl::PRIMITIVE_TRIANGLES; break;
                case GL_TRIANGLE_STRIP: type = rl::PRIMITIVE_TRIANGLE_STRIP; break;
                case GL_TRIANGLE_FAN:   type = rl::PRIMITIVE_TRIANGLE_FAN; break;
                case GL_QUADS:          type = rl::PRIMITIVE_QUADS; break;
                case GL_QUAD_STRIP:     type = rl::PRIMITIVE_QUAD_STRIP; break;
                }

                size_t first = 0;
                switch( item->m_draw_indexed->m_type ) {
                case GL_UNSIGNED_BYTE:
                    first = (size_t)item->m_draw_indexed->m_offset/sizeof(GLubyte);
                    break;
                case GL_UNSIGNED_SHORT:
                    first = (size_t)item->m_draw_indexed->m_offset/sizeof(GLshort);
                    break;
                case GL_UNSIGNED_INT:
                    first = (size_t)item->m_draw_indexed->m_offset/sizeof(GLint);
                    break;
                }
                rl_action->setIndexed( type,
                                       rl_buffer->id(),
                                       first,
                                       item->m_draw_indexed->m_count );
                m_renderlist_db.drawOrderAdd( name );
            }
         }

         prev_item = item;
    }


#else
    for( size_t i=0; i<m_renderlist.size(); i++ ) {
        const RenderAction* action = m_renderlist[i];

        if( true || !m_last_update.asRecentAs( action->m_timestamp )  ) {
            CacheKey<1> key( action );
            auto it = actions.find( key );
            if( it == actions.end() ) {
                actions[ key ] = action;
                // Make sure that assets are updated as well
                // Iterate over all set_buffers and make sure that the clients
                // gets updated source buffers
                if( action->m_type == RenderAction::ACTION_SET_INPUTS ) {
                    for( size_t j=0; j<action->m_set_inputs.m_items.size(); j++ ) {
                        const SourceBuffer* buf = action->m_set_inputs.m_items[j].m_source;
                        if( (m_renderlist_db.itemByName( boost::lexical_cast<std::string>(buf) ) == NULL ) ||
                            (!m_last_update.asRecentAs( buf->valueChanged() ) ) )
                        {
                            CacheKey<1> buf_key( buf );
                            auto it = source_buffers.find( buf_key );
                            if( it == source_buffers.end() ) {
                                source_buffers[ buf_key ] = buf;
                            }
                        }
                    }
                }
                // Make sure that client has updated buffer with index data
                else if( action->m_type == RenderAction::ACTION_DRAW_INDEXED ) {
                    const SourceBuffer* buf = action->m_draw_indexed.m_index_buffer;
                    if( (m_renderlist_db.itemByName( boost::lexical_cast<std::string>(buf) ) == NULL ) ||
                        (!m_last_update.asRecentAs( buf->valueChanged() ) ) )
                    {
                        CacheKey<1> buf_key( buf );
                        auto it = source_buffers.find( buf_key );
                        if( it == source_buffers.end() ) {
                            source_buffers[ buf_key ] = buf;
                        }
                    }
                }

                else if( action->m_type == RenderAction::ACTION_SET_PASS ) {
                    const Pass* pass = action->m_set_pass.m_pass;
                    if( (m_renderlist_db.itemByName( boost::lexical_cast<std::string>(pass) ) == NULL ) ||
                        (!m_last_update.asRecentAs( pass->valueChanged() ) ) )
                    {
                        CacheKey<1> shader_key( pass );
                        auto it = shaders.find( shader_key );
                        if( it == shaders.end() ) {
                            shaders[ shader_key ] = pass;
                        }
                    }
                }

                else if( action->m_type == RenderAction::ACTION_SET_SAMPLERS ) {
                    for( size_t j=0; j<action->m_set_samplers.m_items.size(); j++ ) {
                        const Image* image = action->m_set_samplers.m_items[j].m_image;
                        if( !m_last_update.asRecentAs( image->valueChanged() ) ) {
                            CacheKey<1> image_key( image );
                            auto it = images.find( image_key );
                            if( it == images.end() ) {
                                images[ image_key ] = image;
                            }
                        }
                    }
                }

                else if( action->m_type == RenderAction::ACTION_SET_FRAMEBUFFER ) {
                    for( size_t j=0; j<action->m_set_render_targets.m_items.size(); j++ ) {
                        const Image* image = action->m_set_samplers.m_items[j].m_image;
                        if( !m_last_update.asRecentAs( image->valueChanged() ) ) {
                            CacheKey<1> image_key( image );
                            auto it = images.find( image_key );
                            if( it == images.end() ) {
                                images[ image_key ] = image;
                            }
                        }
                    }
                }

                else if( action->m_type == RenderAction::ACTION_SET_VIEW_COORDSYS ) {
                    // Tag matrices that are needed so they will be calculated
                    // when we do update
                    m_transform_cache.runtimeSemantic( RUNTIME_PROJECTION_MATRIX,
                                                       NULL,
                                                       &action->m_set_view,
                                                       NULL );
                    m_transform_cache.runtimeSemantic( RUNTIME_PROJECTION_INVERSE_MATRIX,
                                                       NULL,
                                                       &action->m_set_view,
                                                       NULL );
                    m_transform_cache.runtimeSemantic( RUNTIME_EYE_FROM_WORLD,
                                                       NULL,
                                                       &action->m_set_view,
                                                       NULL );
                    m_transform_cache.runtimeSemantic( RUNTIME_WORLD_FROM_EYE,
                                                       NULL,
                                                       &action->m_set_view,
                                                       NULL );
                    for( int i=0; i<SCENE_LIGHTS_MAX; i++ ) {
                        if( action->m_set_view.m_lights[i] != NULL ) {
                            m_transform_cache.runtimeSemantic( (RuntimeSemantic)(RUNTIME_LIGHT0_EYE_FROM_WORLD + i),
                                                               NULL,
                                                               &action->m_set_view,
                                                               NULL );
                            m_transform_cache.runtimeSemantic( (RuntimeSemantic)(RUNTIME_WORLD_FROM_LIGHT0_EYE + i),
                                                               NULL,
                                                               &action->m_set_view,
                                                               NULL );

                        }
                    }
                }

                else if( action->m_type == RenderAction::ACTION_SET_LOCAL_COORDSYS ) {
                    // Tag matrices that are needed so they will be calculated
                    // when we do update
                    m_transform_cache.runtimeSemantic( RUNTIME_WORLD_FROM_OBJECT,
                                                       NULL,
                                                       NULL,
                                                       &action->m_set_local );
                    m_transform_cache.runtimeSemantic( RUNTIME_OBJECT_FROM_WORLD,
                                                       NULL,
                                                       NULL,
                                                       &action->m_set_local );
                }
            }
        }
    }
    m_transform_cache.update( 1, 1 );


    // -- push buffer objects
    for( auto it=source_buffers.begin(); it!=source_buffers.end(); ++it ) {
        const SourceBuffer* bd = it->second;
        std::string id = boost::lexical_cast<std::string>( bd );
        rl::Buffer* bs = m_renderlist_db.castedItemByName<rl::Buffer*>( id );
        if( bs == NULL ) {
            bs = m_renderlist_db.createBuffer( id );
        }
        switch( bd->elementType() ) {
        case ELEMENT_INT:
            bs->set( bd->intData(), bd->elementCount() );
            SCENELOG_INFO( log, "buffer[" <<id<<"] (" << bd->id() << ") = int(..."<< bd->elementCount() << "...)" );
            break;
        case ELEMENT_FLOAT:
            bs->set( bd->floatData(), bd->elementCount() );
            SCENELOG_INFO( log, "buffer[" <<id<<"] (" << bd->id() << ") = float(..."<< bd->elementCount() << "...)" );
            break;
        }
    }

    // --- push images
    for( auto it=images.begin(); it!=images.end(); ++it ) {
        const Image* di = it->second;
        std::string id = boost::lexical_cast<std::string>( di );
        SCENELOG_DEBUG( log, "image[" << id << "] <- ignored" );
    }

    // --- push shaders
    for( auto it=shaders.begin(); it!=shaders.end(); ++it ) {
        const Pass* ss = it->second;
        std::string id = boost::lexical_cast<std::string>( ss );
        rl::Shader* ls = m_renderlist_db.castedItemByName<rl::Shader*>( id );
        SCENELOG_INFO( log, "shader[" << id << "]" );
        if( ls == NULL ) {
            ls = m_renderlist_db.createShader( id );
        }
        if( !ss->shaderSource( STAGE_VERTEX ).empty() ) {
            ls->setVertexStage( ss->shaderSource( STAGE_VERTEX ) );
            SCENELOG_INFO( log, "+- vertex shader" );
        }
        if( !ss->shaderSource( STAGE_TESSELLATION_CONTROL ).empty() ) {
            ls->setTessCtrlStage( ss->shaderSource( STAGE_TESSELLATION_CONTROL ) );
            SCENELOG_INFO( log, "+- tessellation control shader" );
        }
        if( !ss->shaderSource( STAGE_TESSELLATION_EVALUATION ).empty() ) {
            ls->setTessEvalStage( ss->shaderSource( STAGE_TESSELLATION_EVALUATION ) );
            SCENELOG_INFO( log, "+- tessellation evaluation shader" );
        }
        if( !ss->shaderSource( STAGE_GEOMETRY ).empty() ) {
            ls->setGeometryStage( ss->shaderSource( STAGE_GEOMETRY ) );
            SCENELOG_INFO( log, "+- geometry shader" );
        }
        if( !ss->shaderSource( STAGE_FRAGMENT ).empty() ) {
            ls->setFragmentStage( ss->shaderSource( STAGE_FRAGMENT ) );
            SCENELOG_INFO( log, "+- fragment shader" );
        }
    }

    // --- push actions
    for( auto it=actions.begin(); it!=actions.end(); ++it ) {
        const RenderAction* sc_action = it->second;
        std::string id = boost::lexical_cast<std::string>( sc_action->m_serial_no );

        // --- SetViewCoordSys -------------------------------------------------
        if( sc_action->m_type == RenderAction::ACTION_SET_VIEW_COORDSYS ) {
            rl::SetViewCoordSys* la = m_renderlist_db.castedItemByName<rl::SetViewCoordSys*>( id );
            if( la == NULL ) {
                la = m_renderlist_db.createAction<rl::SetViewCoordSys>( id );
            }
            la->setProjection( m_transform_cache.runtimeSemantic( RUNTIME_PROJECTION_MATRIX,
                                                                  NULL,
                                                                  &sc_action->m_set_view,
                                                                  NULL )->floatData(),
                               m_transform_cache.runtimeSemantic( RUNTIME_PROJECTION_INVERSE_MATRIX,
                                                                  NULL,
                                                                  &sc_action->m_set_view,
                                                                  NULL )->floatData() );
            la->setOrientation( m_transform_cache.runtimeSemantic( RUNTIME_EYE_FROM_WORLD,
                                                                   NULL,
                                                                   &sc_action->m_set_view,
                                                                   NULL )->floatData(),
                                m_transform_cache.runtimeSemantic( RUNTIME_EYE_FROM_WORLD,
                                                                   NULL,
                                                                   &sc_action->m_set_view,
                                                                   NULL )->floatData() );

            SCENELOG_INFO( log, "setViewCoordSys[" << id << "]" );
            for(int i=0; i<SCENE_LIGHTS_MAX; i++) {
                if( sc_action->m_set_view.m_lights[i] != NULL ) {
                    const Light* l = sc_action->m_set_view.m_lights[i];

                    const std::string lid = id + '_' + boost::lexical_cast<std::string>( i );
                    rl::SetLight* sl = m_renderlist_db.castedItemByName<rl::SetLight*>( lid );
                    if( sl == NULL ) {
                        sl = m_renderlist_db.createAction<rl::SetLight>( lid );
                    }
                    sl->setIndex( i );
                    switch( l->type() ) {
                    case Light::LIGHT_NONE:         sl->setType( rl::LIGHT_AMBIENT ); break;
                    case Light::LIGHT_AMBIENT:      sl->setType( rl::LIGHT_AMBIENT ); break;
                    case Light::LIGHT_DIRECTIONAL:  sl->setType( rl::LIGHT_DIRECTIONAL ); break;
                    case Light::LIGHT_POINT:        sl->setType( rl::LIGHT_POINT ); break;
                    case Light::LIGHT_SPOT:         sl->setType( rl::LIGHT_SPOT ); break;
                    }
                    sl->setColor( l->color()->floatData()[0],
                                  l->color()->floatData()[1],
                                  l->color()->floatData()[2] );
                    sl->setAttenuation( l->constantAttenuation()->floatData()[0],
                                        l->linearAttenuation()->floatData()[0],
                                        l->quadraticAttenuation()->floatData()[0] );
                    sl->setFalloff( l->falloffAngle()->floatData()[0],
                                    l->falloffExponent()->floatData()[0] );
                    sl->setOrientation( m_transform_cache.runtimeSemantic( (RuntimeSemantic)(RUNTIME_LIGHT0_EYE_FROM_WORLD + i),
                                                                           NULL,
                                                                           &sc_action->m_set_view,
                                                                           NULL )->floatData(),
                                        m_transform_cache.runtimeSemantic( (RuntimeSemantic)(RUNTIME_WORLD_FROM_LIGHT0_EYE + i),
                                                                           NULL,
                                                                           &sc_action->m_set_view,
                                                                           NULL )->floatData() );
                    SCENELOG_INFO( log, "SetLight[" << lid << "]" );
                }
            }
        }
        // --- SetLocalCoordSys ------------------------------------------------
        if( sc_action->m_type == RenderAction::ACTION_SET_LOCAL_COORDSYS ) {
            rl::SetLocalCoordSys* rl_action = m_renderlist_db.castedItemByName<rl::SetLocalCoordSys*>( id );
            if( rl_action == NULL ) {
                rl_action = m_renderlist_db.createAction<rl::SetLocalCoordSys>( id );
            }
            rl_action->setOrientation( m_transform_cache.runtimeSemantic( RUNTIME_OBJECT_FROM_WORLD,
                                                                          NULL,
                                                                          NULL,
                                                                          &sc_action->m_set_local )->floatData(),
                                       m_transform_cache.runtimeSemantic( RUNTIME_WORLD_FROM_OBJECT,
                                                                          NULL,
                                                                          NULL,
                                                                          &sc_action->m_set_local )->floatData() );

            SCENELOG_INFO( log, "setLocalCoordSys[" << id << "]" );
        }
        // --- SetShader -------------------------------------------------------
        if( sc_action->m_type == RenderAction::ACTION_SET_PASS ) {
            rl::SetShader* rl_action = m_renderlist_db.castedItemByName<rl::SetShader*>( id );
            if( rl_action == NULL ) {
                rl_action = m_renderlist_db.createAction<rl::SetShader>( id );
            }

            const std::string shader = boost::lexical_cast<std::string>( sc_action->m_set_pass.m_pass );
            rl_action->setShader( shader );
            SCENELOG_INFO( log, "setShader[" << id << "]" );
            SCENELOG_INFO( log, "+- shader = " << shader );
        }
        if( sc_action->m_type == RenderAction::ACTION_SET_INPUTS ) {
            SCENELOG_INFO( log, "setInputs[" << id << "]" );
            rl::SetInputs* si = m_renderlist_db.castedItemByName<rl::SetInputs*>( id );
            if( si == NULL ) {
                si = m_renderlist_db.createAction<rl::SetInputs>( id );
            }
            const std::string sh_id = boost::lexical_cast<std::string>( sc_action->m_set_inputs.m_pass );
            SCENELOG_INFO( log, "+- shader = " << sh_id );
            si->setShader( sh_id );

            si->clearInputs();
            for( size_t j=0; j<sc_action->m_set_inputs.m_items.size(); j++ ) {
                const SetInputs::Item& m = sc_action->m_set_inputs.m_items[j];
                const std::string buf_id = boost::lexical_cast<std::string>( sc_action->m_set_inputs.m_items[j].m_source );
                si->setInput( sc_action->m_set_inputs.m_pass->attributeSymbol(j),
                              buf_id,
                              m.m_components,
                              m.m_offset,
                              m.m_stride );
                SCENELOG_INFO( log, "+- input[" << sc_action->m_set_inputs.m_pass->attributeSymbol(j)
                               << "] = " << buf_id << ":" <<
                               m.m_components << ":" << m.m_offset << ":" << m.m_stride );
            }

        }
        if( sc_action->m_type == RenderAction::ACTION_SET_FRAMEBUFFER ) {
            SCENELOG_INFO( log, "setFramebuffer[" << id << "]" );
        }
        if( sc_action->m_type == RenderAction::ACTION_SET_UNIFORMS ) {
            SCENELOG_INFO( log, "setUniforms[" << id << "]" );
            rl::SetUniforms* su = m_renderlist_db.castedItemByName<rl::SetUniforms*>( id );
            if( su == NULL ) {
                su = m_renderlist_db.createAction<rl::SetUniforms>( id );
            }
            const std::string sh_id = boost::lexical_cast<std::string>( sc_action->m_set_uniforms.m_pass );
            SCENELOG_INFO( log, "+- shader = " << sh_id );
            su->setShader( sh_id );
            su->clear();
            for( size_t j=0; j<sc_action->m_set_uniforms.m_items.size(); j++ ) {
                const SetUniforms::Item& m = sc_action->m_set_uniforms.m_items[j];

                const std::string& sym = sc_action->m_set_uniforms.m_pass->uniformSymbol(j);
                if( m.m_semantic != RUNTIME_SEMANTIC_N ) {

                    rl::UniformSemantic semantic;
                    switch( m.m_semantic ) {
                    case RUNTIME_MODELVIEW_PROJECTION_MATRIX:
                        semantic = rl::SEMANTIC_MODELVIEW_PROJECTION_MATRIX;
                        break;
                    case RUNTIME_NORMAL_MATRIX:
                        semantic = rl::SEMANTIC_NORMAL_MATRIX;
                        break;
                    default:
                        SCENELOG_ERROR( log, "Symbol '" << sym << "' has unsupported semantic " << m.m_semantic );
                        continue;
                        //semantic = rl::SEMANTIC_MODELVIEW_PROJECTION_MATRIX;
                    }
                    su->setSemantic( sym, semantic );
                    SCENELOG_INFO( log, "+- " << sym << " = semantic " << semantic );
                }
                else {
                    switch( m.m_value->type() ) {
                    case VALUE_TYPE_INT:
                        su->setInt1( sym, m.m_value->intData()[0] );
                        SCENELOG_INFO( log, "+- " << sym << " int" );
                        break;
                    case VALUE_TYPE_FLOAT:
                        su->setFloat1v( sym, m.m_value->floatData() );
                        SCENELOG_INFO( log, "+- " << sym << " float" );
                        break;
                    case VALUE_TYPE_FLOAT2:
                        su->setFloat2v( sym, m.m_value->floatData() );
                        SCENELOG_INFO( log, "+- " << sym << " float2" );
                        break;
                    case VALUE_TYPE_FLOAT3:
                        su->setFloat3v( sym, m.m_value->floatData() );
                        SCENELOG_INFO( log, "+- " << sym << " float3" );
                        break;
                    case VALUE_TYPE_FLOAT4:
                        su->setFloat4v( sym, m.m_value->floatData() );
                        SCENELOG_INFO( log, "+- " << sym << " float4" );
                        break;
                    case VALUE_TYPE_FLOAT3X3:
                        su->setFloat3x3v( sym, m.m_value->floatData() );
                        SCENELOG_INFO( log, "+- " << sym << " float3x3" );
                        break;
                    case VALUE_TYPE_FLOAT4X4:
                        su->setFloat4x4v( sym, m.m_value->floatData() );
                        SCENELOG_INFO( log, "+- " << sym << " float4x4" );
                        break;
                    case VALUE_TYPE_BOOL:
                    case VALUE_TYPE_SAMPLER1D:
                    case VALUE_TYPE_SAMPLER2D:
                    case VALUE_TYPE_SAMPLER3D:
                    case VALUE_TYPE_SAMPLERCUBE:
                    case VALUE_TYPE_SAMPLERDEPTH:
                        su->setInt1( sym, m.m_value->intData()[0] );
                        SCENELOG_INFO( log, "+- " << sym << " int" );
                        break;
                    default:
                        break;
                    }

                }




                /*
                SCENELOG_DEBUG( log, "  uniform " << i << ": symbol='" << m_set_uniforms.m_pass->uniformSymbol(i) <<
                                    "', sem=" << m.m_semantic <<
                                    ", value=" << m.m_value->debugString() );
                */
            }

        }
        if( sc_action->m_type == RenderAction::ACTION_SET_SAMPLERS ) {
            SCENELOG_INFO( log, "setSamplers[" << id << "]" );
        }
        if( sc_action->m_type == RenderAction::ACTION_SET_FB_CTRL ) {
            SCENELOG_INFO( log, "setFBCtrl[" << id << "]" );
        }
        if( sc_action->m_type == RenderAction::ACTION_SET_PIXEL_OPS ) {
            SCENELOG_INFO( log, "setPixelOps[" << id << "]" );
        }
        if( sc_action->m_type == RenderAction::ACTION_SET_RASTER ) {
            SCENELOG_INFO( log, "setRaster[" << id << "]" );
        }
        if( sc_action->m_type == RenderAction::ACTION_DRAW ) {
            rl::Draw* rl_action = m_renderlist_db.castedItemByName<rl::Draw*>( id );
            if( rl_action == NULL ) {
                rl_action = m_renderlist_db.createAction<rl::Draw>( id );
            }
            rl::PrimitiveType type = rl::PRIMITIVE_POINTS;
            switch( sc_action->m_draw.m_mode ) {
            case GL_POINTS:         type = rl::PRIMITIVE_POINTS; break;
            case GL_LINES:          type = rl::PRIMITIVE_LINES; break;
            case GL_LINE_STRIP:     type = rl::PRIMITIVE_LINE_STRIP; break;
            case GL_LINE_LOOP:      type = rl::PRIMITIVE_LINE_LOOP; break;
            case GL_TRIANGLES:      type = rl::PRIMITIVE_TRIANGLES; break;
            case GL_TRIANGLE_STRIP: type = rl::PRIMITIVE_TRIANGLE_STRIP; break;
            case GL_TRIANGLE_FAN:   type = rl::PRIMITIVE_TRIANGLE_FAN; break;
            case GL_QUADS:          type = rl::PRIMITIVE_QUADS; break;
            case GL_QUAD_STRIP:     type = rl::PRIMITIVE_QUAD_STRIP; break;
            }
            size_t first = sc_action->m_draw.m_first;
            size_t count = sc_action->m_draw.m_count;

            rl_action->setNonIndexed( type, first, count );
            SCENELOG_INFO( log, "draw[" << id <<
                           "] type=" << type <<
                           ", first=" << first <<
                           ", count=" << count );
        }
        if( sc_action->m_type == RenderAction::ACTION_DRAW_INDEXED ) {

            const std::string buffer = boost::lexical_cast<std::string>( sc_action->m_draw_indexed.m_index_buffer );
            rl::Buffer* rl_buffer = m_renderlist_db.castedItemByName<rl::Buffer*>( buffer );
            if( rl_buffer == NULL ) {
                SCENELOG_ERROR( log, "renderlist buffer '" << buffer << "' doesn't exist!" );
            }
            else {
                rl::Draw* rl_action = m_renderlist_db.castedItemByName<rl::Draw*>( id );
                if( rl_action == NULL ) {
                    rl_action = m_renderlist_db.createAction<rl::Draw>( id );
                }
                rl::PrimitiveType type = rl::PRIMITIVE_POINTS;
                switch( sc_action->m_draw_indexed.m_mode ) {
                case GL_POINTS:         type = rl::PRIMITIVE_POINTS; break;
                case GL_LINES:          type = rl::PRIMITIVE_LINES; break;
                case GL_LINE_STRIP:     type = rl::PRIMITIVE_LINE_STRIP; break;
                case GL_LINE_LOOP:      type = rl::PRIMITIVE_LINE_LOOP; break;
                case GL_TRIANGLES:      type = rl::PRIMITIVE_TRIANGLES; break;
                case GL_TRIANGLE_STRIP: type = rl::PRIMITIVE_TRIANGLE_STRIP; break;
                case GL_TRIANGLE_FAN:   type = rl::PRIMITIVE_TRIANGLE_FAN; break;
                case GL_QUADS:          type = rl::PRIMITIVE_QUADS; break;
                case GL_QUAD_STRIP:     type = rl::PRIMITIVE_QUAD_STRIP; break;
                }

                size_t first = 0;
                switch( sc_action->m_draw_indexed.m_type ) {
                case GL_UNSIGNED_BYTE:
                    first = (size_t)sc_action->m_draw_indexed.m_offset/sizeof(GLubyte);
                    break;
                case GL_UNSIGNED_SHORT:
                    first = (size_t)sc_action->m_draw_indexed.m_offset/sizeof(GLshort);
                    break;
                case GL_UNSIGNED_INT:
                    first = (size_t)sc_action->m_draw_indexed.m_offset/sizeof(GLint);
                    break;
                }
                size_t count = sc_action->m_draw_indexed.m_count;
                rl_action->setIndexed( type, rl_buffer->id(), first, count );
                SCENELOG_INFO( log, "draw(indexed)[" << id <<
                               "] type=" << type <<
                               ", buffer=" << buffer <<
                               ", first=" << first <<
                               ", count=" << count );
            }

        }

    }


    // --- update draw order
    m_renderlist_db.drawOrderClear();
    for( size_t i=0; i<m_renderlist.size(); i++ ) {
        const RenderAction* action = m_renderlist[i];
        const std::string id = boost::lexical_cast<std::string>( action->m_serial_no );
        m_renderlist_db.drawOrderAdd( id );
    }
#endif
}

    } // of namespace Runtime
} // of namespace Scene
