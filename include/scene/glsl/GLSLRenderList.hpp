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

#pragma once

#include <scene/runtime/RenderList.hpp>
#include <scene/runtime/TransformCache.hpp>
#include <scene/glsl/GLSLRuntime.hpp>

namespace Scene {
    namespace Runtime {

class GLSLRenderList
{
public:
    GLSLRenderList( GLSLRuntime& runtime );


    /** Specifies the default render output target.
      *
      * Unless a specific render target is specified in the renderlist (which is
      * the case when e.g. doing render-to-texture), all rendering commands are
      * channelled to the default output.
      *
      * \param[in] framebuffer  The default output framebuffer (usually 0).
      * \param[in] x            The X offset of the default viewport.
      * \param[in] y            The Y offset of the default viewport.
      * \param[in] w            The width of the default viewport.
      * \param[in] h            The height of the default viewport.
      */
    void
    setDefaultOutput( GLuint framebuffer,
                      size_t x,
                      size_t y,
                      size_t w,
                      size_t h );

    /* Returns true if anything have changed. */
    bool
    build( const std::string& visual_scene = "" );


    /** Clears the contents of the render list and releases all references to GLSLRuntime items. */
    void
    clear();

    void
    render( );

    const RenderList&
    renderList() const
    { return m_renderlist; }

protected:
    struct GLSLItem
    {
        const Value*                m_bbox_test;
        std::vector<const Value*>   m_uniform_values;
        GLSLFrameBuffer*            m_glsl_framebuffer;
        GLSLShader*                 m_glsl_pass;
        GLSLVertexArray*            m_glsl_inputs;
        GLSLSamplers*               m_glsl_samplers;
        GLSLBuffer*                 m_glsl_indices;
    };
    std::vector<GLSLItem>           m_glsl_items;


    GLSLRuntime&                   m_runtime;
    RenderList                     m_renderlist;
    TransformCache                 m_transform_cache;
    GLuint                         m_default_framebuffer;
    size_t                         m_default_viewport_x;
    size_t                         m_default_viewport_y;
    size_t                         m_default_viewport_w;
    size_t                         m_default_viewport_h;
    bool                           m_valid;
    struct GLSLRenderAction
    {
        enum Type {
            GLSL_ACTION_EMPTY,
            GLSL_ACTION_SET_UNIFORMS
        }                          m_type;

        union {
            GLSLShader*            m_set_pass;
            GLSLVertexArray*       m_set_inputs;
            struct {
                const Value*       m_bbox_check;
                GLSLBuffer*        m_indices;
            }                      m_draw_indexed;
            struct {
                const Value*       m_bbox_check;
            }                      m_draw;
            GLSLSamplers*          m_set_samplers;
            GLSLFrameBuffer*       m_set_framebuffer;
            struct {
                const Value**      m_values;
                size_t             m_count;
            }                      m_set_uniforms;
        };
    };
    std::vector<GLSLRenderAction>  m_glsl_list;

    void
    minorUpdate();

    void
    majorUpdate();


    void
    calcuateTransforms( std::vector<Value>& runtime_semantics_value,
                        const RenderAction*       set_transforms );




};

    } // of namespace Runtime
} // of namespace Scene
