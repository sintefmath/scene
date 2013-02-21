#pragma once
#include <string>
#include <list>
#include "scene/Geometry.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {
    namespace Runtime {


    struct ResolvedParams;


struct SetViewCoordSys : public Identifiable
{
    const Camera*                  m_camera;
    const Node*                    m_camera_path[ SCENE_PATH_MAX ];
    const Light*                   m_lights[ SCENE_LIGHTS_MAX ];
    const Camera*                  m_light_projections[ SCENE_LIGHTS_MAX ];
    const Node*                    m_light_paths[ SCENE_LIGHTS_MAX][ SCENE_PATH_MAX ];
};

struct SetLocalCoordSys : public Identifiable
{
    const Node*                    m_node_path[ SCENE_PATH_MAX ];
};

struct SetPass : public Identifiable
{
    const Pass*                    m_pass;
};



/** Set current vertex inputs (OpenGL vertex attributes).
  *
  * Inputs references a list of source buffers as well as accessor information.
  * This more or less matches directly to glVertexAttribPointer.
  */
struct SetInputs : public Identifiable
{
    const Pass*                    m_pass;
    const Primitives*              m_primitives;
    struct Item {
        const SourceBuffer*        m_source;
        ElementType                m_type;
        int                        m_components;
        int                        m_offset;
        int                        m_stride;
    };
    std::vector<Item>              m_items;
};


/** Set current render target.
  *
  * Specifies a render target to use. This is a list of image references, along
  * with the attachment point (color, stencil, depth etc.) and image face
  * (cube map face, mip level etc.).
  */
struct SetRenderTargets : public Identifiable
{
    struct Item {
        const Image*   m_image;
        GLenum         m_target;
        GLenum         m_attachment;
        GLenum         m_textarget;
        GLint          m_level;
        GLint          m_layer;
    };
    GLenum             m_clear_mask;
    std::vector<Item>  m_items;
};


/** Set the current set of samples to use.
  *
  * This is a list of references to images, along with the sampler state needed.
  * In addition, a Value is given with the sampler unit as an int, which is
  * used by setUniforms.
  */
struct SetSamplers : public Identifiable
{
    const Pass*                    m_pass;
    struct Item {
        const Image*               m_image;
        GLenum                     m_wrap_s;
        GLenum                     m_wrap_t;
        GLenum                     m_wrap_p;
        GLenum                     m_min_filter;
        GLenum                     m_mag_filter;
        Value                      m_uniform_value;
    };
    std::vector<Item>              m_items;
};

/** Triggers rendering of a non-indexed primitive batch.
  *
  * This corresponds roughly to glDrawArrays. Primitive type and count is given.
  * For patches, the number of vertices per primitive is provided, for other
  * primitive types, this is implictly given.
  */
struct Draw : public Identifiable
{
    const Geometry*    m_geometry;   ///< Scene geometry for this action.
    const Primitives*  m_primitives; ///< Scene geometry primitive set
    GLenum             m_mode;       ///< Primitive type (point, triangle, ... ).
    GLint              m_vertices;   ///< Vertices per primitive.
    GLint              m_first;      ///< First vertex to draw.
    GLsizei            m_count;      ///< Number of vertices to issue.
};

/** Triggers renderering of an indexed primitive batch.
  *
  * Similar to draw, but has also a pointer to a source buffer that contains
  * the indices. For patches, the number of vertices per primitive is provided,
  * for other primitive types, this is implictly given.
  */
struct DrawIndexed : public Identifiable
{
    const SourceBuffer*  m_index_buffer; ///< Buffer that contains indices.
    const Geometry*      m_geometry;     ///< Scene geometry for this action.
    const Primitives*    m_primitives;   ///< Scene geometry primitive set for this action.
    GLenum               m_mode;         ///< Primitive type (point, triangle, ... ).
    GLint                m_vertices;     ///< Vertices per primitive.
    GLenum               m_type;         ///< Indices data type (uint, ushort, ... ).
    GLvoid*              m_offset;       ///< Byte offset into buffer.
    GLsizei              m_count;        ///< Number of vertices to issue.
};


/** Set a set of uniform values.
  *
  * The RuntimeSemantic enum is used to replace the provided "standard" values
  * with various runtime values (modelview matrices etc. )
  */
struct SetUniforms : public Identifiable
{
    const Pass*                    m_pass;

    struct Item {
        RuntimeSemantic            m_semantic;
        const Value*               m_value;
    };
    std::vector<Item>              m_items;
};

/** Set pixel operation state. */
struct SetPixelOps : public Identifiable
{
    GLboolean   m_depth_test;
    GLenum      m_depth_func;
    GLboolean   m_blend;
    GLenum      m_blend_src_rgb;
    GLenum      m_blend_src_alpha;
    GLenum      m_blend_dst_rgb;
    GLenum      m_blend_dst_alpha;
};

/** Set framebuffer control state. */
struct SetFBCtrl : public Identifiable
{
    GLboolean   m_color_writemask[4];
    GLboolean   m_depth_writemask;
};


/** Set rasterization state. */
struct SetRaster : public Identifiable
{
    GLfloat     m_point_size;
    GLboolean   m_cull_face;
    GLenum      m_cull_face_mode;
    GLenum      m_polygon_mode_front;
    GLenum      m_polygon_mode_back;
    GLfloat     m_polygon_offset_factor;
    GLfloat     m_polygon_offset_units;
    GLboolean   m_polygon_offset_point;
    GLboolean   m_polygon_offset_line;
    GLboolean   m_polygon_offset_fill;
};

struct RenderAction
{
    enum Type {
        ACTION_SET_VIEW_COORDSYS,
        ACTION_SET_LOCAL_COORDSYS,
        ACTION_SET_PASS,
        ACTION_SET_INPUTS,
        ACTION_SET_FRAMEBUFFER,
        ACTION_SET_UNIFORMS,
        ACTION_SET_SAMPLERS,
        ACTION_SET_FB_CTRL,
        ACTION_SET_PIXEL_OPS,
        ACTION_SET_RASTER,
        ACTION_DRAW,
        ACTION_DRAW_INDEXED
    };

    const Type         m_type;
    const std::string  m_id;
    unsigned int        m_serial_no;
    static unsigned int m_serial_no_counter;
    SeqPos         m_timestamp;
    //union {
        SetViewCoordSys   m_set_view;
        SetLocalCoordSys  m_set_local;
        SetPass           m_set_pass;
        Draw              m_draw;
        DrawIndexed       m_draw_indexed;
        SetPixelOps       m_set_pixel_ops;
        SetRaster         m_set_rasterization;
        SetFBCtrl         m_set_fb_ctrl;
    //};
    // contains a constructor so it can't be in a union
    SetUniforms       m_set_uniforms;
    SetInputs         m_set_inputs;
    SetSamplers       m_set_samplers;
    SetRenderTargets  m_set_render_targets;

    RenderAction( const Type type, const std::string id );

    static RenderAction*
    createSetViewCoordSys( const Camera*                  camera,
                           const std::list<const Node*>&  camera_path,
                           const Light*                   (&lights)[SCENE_LIGHTS_MAX],
                           const Camera*                  (&light_projections)[SCENE_LIGHTS_MAX],
                           const std::list<const Node*>   (&light_paths)[SCENE_LIGHTS_MAX] );

    static RenderAction*
    createSetLocalCoordSys( const std::list<const Node*>&  node_path );

    static RenderAction*
    createSetPass( const std::string&  id,
                   const Pass*         pass );

    static RenderAction*
    createSetInputs( const DataBase&     database,
                     const std::string&  id,
                     const Pass*         pass,
                     const Geometry*     geometry,
                     const Primitives*   primitives );

    static RenderAction*
    createDraw( const std::string&  id,
                const Geometry*     geometry,
                const Primitives*   primitives,
                const Pass*         pass );

    static RenderAction*
    createDrawIndexed( const DataBase&     database,
                       const std::string&  id,
                       const Geometry*     geometry,
                       const Primitives*   primitives,
                       const Pass*         pass );

    static RenderAction*
    createSetRenderTarget( const DataBase&        database,
                           const std::string&     id,
                           const ResolvedParams*  params,
                           const Pass*            pass );


    static RenderAction*
    createSetUniforms( const std::string&     id,
                       const RenderAction*    set_samplers,
                       const ResolvedParams*  params,
                       const Pass*            pass );

    static RenderAction*
    createSetSamplers( const DataBase&       database,
                       const std::string&    id,
                       const ResolvedParams* params,
                       const Pass*           pass );

    static RenderAction*
    createSetPixelOps( const std::string&     id,
                       const ResolvedParams*  params,
                       const Pass*            pass );

    static RenderAction*
    createSetFBCtrl( const std::string&     id,
                     const ResolvedParams*  params,
                     const Pass*            pass );

    static RenderAction*
    createSetRaster( const std::string&     id,
                     const ResolvedParams*  params,
                     const Pass*            pass );

    void
    debugDump() const;

private:
    // No-one should use this one.
    RenderAction();


};


    } // of namespace Runtime
} // of namespace Scene
