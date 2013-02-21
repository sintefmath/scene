#pragma once
#include <GL/glew.h>
#include <unordered_map>
#include <scene/Log.hpp>
#include <scene/SeqPos.hpp>
#include "scene/DataBase.hpp"
#include "scene/runtime/Resolver.hpp"

namespace Scene {
    namespace Runtime {

class GLSLCachedObject
{
public:
    const SeqPos&
    timeStamp() const { return m_timestamp; }

protected:
    SeqPos  m_timestamp;


};



class GLSLBuffer : public GLSLCachedObject
{
public:
    GLSLBuffer();

    ~GLSLBuffer();

    void
    pull( const SourceBuffer* buffer );

    GLuint
    buffer() const { return m_buffer; }

    GLenum
    elementType() const { return m_element_type; }

protected:
    GLuint     m_buffer;
    GLenum     m_element_type;
    GLsizei    m_element_size;
    GLsizei    m_element_count;
    GLsizei    m_buffer_size;
};


class GLSLShader : public GLSLCachedObject
{
public:
    GLSLShader();

    ~GLSLShader();

    GLuint
    program() const { return m_program; }

    bool
    pull( const Pass* pass );

    GLint
    attribLocation( size_t ix ) const { return m_attributes[ix].m_location; }

    GLint
    attribComponents( size_t ix ) const { return m_attributes[ix].m_components; }

    GLenum
    attribElementType( size_t ix ) const { return m_attributes[ix].m_element_type; }

    GLsizei
    attribElementSize( size_t ix ) const { return m_attributes[ix].m_element_size; }

    GLint
    uniformLocation( size_t ix ) const { return m_uniforms[ix].m_location; }

    ValueType
    uniformType( size_t ix ) const { return m_uniforms[ix].m_type; }

    /** Returns the primitive type that the shader expects
      *
      * \returns GL_PATCHES if the shader contains a tessellation shader,
      * otherwise the primitive type specified in the input layout if the shader
      * contains a geometry shader, or GL_ALWAYS to indicate that all primitives
      * are accepted. */
    GLenum
    expectedInputPrimitiveType() const { return m_expected_input_primitive_type; }


protected:
    GLuint                  m_program;
    GLuint                  m_shader_vertex;
    GLuint                  m_shader_geometry;
    GLuint                  m_shader_tess_ctrl;
    GLuint                  m_shader_tess_eval;
    GLuint                  m_shader_fragment;
    struct Attribute {
        GLint               m_location;
        GLenum              m_element_type;
        GLsizei             m_element_size;
        GLint               m_components;
    };
    std::vector<Attribute>  m_attributes;
    struct Uniform {
        GLint               m_location;
        ValueType           m_type;
    };
    std::vector<Uniform>    m_uniforms;
    GLenum     m_expected_input_primitive_type;


    bool
    compileShader( GLuint shader, const std::string& source );

    bool
    linkProgram();

    void
    retrieveAttributeInfo( const Pass* pass );

    void
    retrieveUniformInfo( const Pass* pass );

    void
    release();

};

class GLSLVertexArray : public GLSLCachedObject
{
public:
    GLSLVertexArray();

    ~GLSLVertexArray();

    GLuint
    vertexArray() const { return m_vertex_array; }

    void
    pull( const std::vector<const GLSLBuffer*> sources,
          const GLSLShader*                    shader,
          const std::vector<SetInputs::Item>&  items );


protected:
    GLuint     m_vertex_array;

    void
    release();

};

class GLSLTexture : public GLSLCachedObject
{
public:

    GLSLTexture();

    ~GLSLTexture();

    void
    pull( const Image* image );

    /** Notify that texture contents has changed and mipmaps must be rebuilt. */
    void
    taint() { m_tainted = true; }

    /** Returns true if mipmaps must be built. */
    bool
    doBuildMipMap();

    GLenum
    target() const { return m_target; }

    GLuint
    texture() const { return m_texture; }

    GLsizei
    width() const { return m_width; }

    GLsizei
    height() const { return m_height;}

    GLsizei
    depth() const { return m_depth; }

    const std::string&
    debugName( ) const { return m_debug_name; }


protected:
    bool       m_build_mipmap;
    bool       m_tainted;
    GLenum     m_target;
    GLuint     m_texture;
    GLsizei    m_width;
    GLsizei    m_height;
    GLsizei    m_depth;

    std::string     m_debug_name;

    void
    release();
};

class GLSLFrameBuffer : public GLSLCachedObject
{
public:
    GLSLFrameBuffer();

    ~GLSLFrameBuffer();

    bool
    pull( const std::vector<GLSLTexture*>& textures,
          const SetRenderTargets&          set_fb );

    /** Returns the GL fbo id.
      *
      * Note, this taints the textures bound to this fbo so that the mipmap
      * pyramid will be rebuilt.
      */
    GLuint
    fbo() const;

    GLsizei
    width() const { return m_width; }

    GLsizei
    height() const { return m_height; }

protected:
    GLuint                     m_fbo;
    GLsizei                    m_width;
    GLsizei                    m_height;
    std::vector<GLSLTexture*>  m_textures;

    void
    release();

};


class GLSLSamplers : public GLSLCachedObject
{
public:
    GLSLSamplers();

    ~GLSLSamplers();

    void
    pull( const std::vector<GLSLTexture*>& textures,
          const SetSamplers&                     setsamplers );

    const size_t
    items() const { return m_samplers.size(); }

    GLuint
    textureName( const size_t ix ) const { return m_textures_name[ix]; }

    GLuint
    sampler( const size_t ix ) const { return m_samplers[ix]; }

    GLenum
    target( const size_t ix ) const { return m_targets[ix]; }

    GLSLTexture*
    texture( const size_t ix ) { return m_textures[ix]; }

protected:
    std::vector<GLSLTexture*>  m_textures;
    std::vector<GLenum>        m_targets;
    std::vector<GLuint>        m_textures_name;
    std::vector<GLuint>        m_samplers;

    void
    release();

};


class GLSLRuntime
{
public:
    GLSLRuntime( const Scene::DataBase& database );

    ~GLSLRuntime();

    /** Size used to set size of images that have size relative to the viewport.
      *
      * Note, this might trigger re-creation of textures.
      */
    void
    setReferenceSize( size_t w, size_t h );


    /** Clear all cached data. */
    void
    clear();

    /** Get a reference to the associated resolver. */
    Resolver&
    resolver() { return m_resolver; }

    /** Get the cached GL buffer object corresponding to the source buffer. */
    GLSLBuffer*
    buffer( const SourceBuffer* buffer );

    GLSLShader*
    shader( const Pass* pass );

    GLSLVertexArray*
    vbo( const RenderAction* set_inputs );

    GLSLSamplers*
    samplers( const RenderAction* set_samplers );

    GLSLFrameBuffer*
    frameBuffer( const RenderAction* set_framebuffer );

    GLSLTexture*
    texture( const Image* image );

    /** Checks OpenGL for errors
      *
      * Fetches errors from OpenGL and logs them.
      *
      * \param[in] log      The logger object to use (usually the logger object
      *                     of the caller which is likely to have provoked the
      *                     error).
      * \param[in] message  An optional message.
      * \returns True if no errors, false otherwise.
      */
    static bool
    checkGL( Logger& log, const std::string& message = "" );


protected:
    const DataBase&   m_database;
    Resolver          m_resolver;

    size_t                                            m_ref_viewport_width;
    size_t                                            m_ref_viewport_heigth;
    SeqPos                                            m_ref_viewport_timestamp;

    std::unordered_map<std::string,GLSLFrameBuffer*>  m_framebuffer_cache;
    std::unordered_map<std::string,GLSLShader*>       m_shader_cache;
    std::unordered_map<std::string,GLSLBuffer*>       m_buffer_cache;
    std::unordered_map<std::string,GLSLVertexArray*>  m_vbo_cache;
    std::unordered_map<std::string,GLSLSamplers*>     m_samplers_cache;
    std::unordered_map<std::string,GLSLTexture*>      m_texture_cache;

};


    } // of namespace Runtime
} // of namespace Scene
