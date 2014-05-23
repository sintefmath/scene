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

#ifndef _WIN32
#ifndef __GXX_EXPERIMENTAL_CXX0X__
#warning "enable CXX0X support!"
// defined just to make qt creator happy.
#define __GXX_EXPERIMENTAL_CXX0X__
#define _GLIBCXX_DEBUG
#define _GLIBCXX_PROFILE
#endif
#endif


#include <string>
#include <GL/glew.h>

#define SCENE_PATH_MAX 8
#define SCENE_LIGHTS_MAX 4

namespace Scene {
    typedef size_t index_t;
    static const index_t index_none = ~0;

    enum ValueContext {
        VALUE_CONTEXT_FX_SETPARAM,
        VALUE_CONTEXT_FX_NEWPARAM,
        VALUE_CONTEXT_COMMON_GROUP,
        VALUE_CONTEXT_GLSL_GROUP,
        VALUE_CONTEXT_GLES_GROUP,
        VALUE_CONTEXT_GLES2_GROUP
    };


    enum ValueType {
        VALUE_TYPE_INT,
        VALUE_TYPE_FLOAT,
        VALUE_TYPE_FLOAT2,
        VALUE_TYPE_FLOAT3,
        VALUE_TYPE_FLOAT4,
        VALUE_TYPE_FLOAT3X3,
        VALUE_TYPE_FLOAT4X4,
        VALUE_TYPE_BOOL,
        VALUE_TYPE_ENUM,
        VALUE_TYPE_ENUM2,
        VALUE_TYPE_SAMPLER1D,
        VALUE_TYPE_SAMPLER2D,
        VALUE_TYPE_SAMPLER3D,
        VALUE_TYPE_SAMPLERCUBE,
        VALUE_TYPE_SAMPLERDEPTH,
        VALUE_TYPE_N
    };

    enum ElementType {
        ELEMENT_INT,
        ELEMENT_FLOAT
    };

    enum ImageType {
        IMAGE_2D = 0,
        IMAGE_3D,
        IMAGE_CUBE,
        IMAGE_N
    };

    enum ShadingModelType
    {
        SHADING_BLINN,
        SHADING_CONSTANT,
        SHADING_LAMBERT,
        SHADING_PHONG
    };

    enum ShadingModelComponentType
    {
        SHADING_COMP_EMISSION = 0, ///< Color of light emitted, float4.
        SHADING_COMP_AMBIENT,      ///< Color of light ambiently emitted, float4.
        SHADING_COMP_DIFFUSE,      ///< Color of light diffusily reflected, float4 (note: inconsistency in COLLADA 1.5 spec).
        SHADING_COMP_SPECULAR,     ///< Color of specularly reflected color, float4.
        SHADING_COMP_SHININESS,    ///< Specularity/roughness of the specular reflection, float.
        SHADING_COMP_REFLECTIVE,   ///< Color for a perfect mirror reflection, float4.
        SHADING_COMP_REFLECTIVITY, ///< Amount of light perfectly reflected, float.
        SHADING_COMP_TRANSPARENT,  ///< Color of perfectly transmitted light, float4.
        SHADING_COMP_TRANSPARENCY, ///< Amount of transmitted light, float.
        SHADING_COMP_REFINDEX,     ///< Index of refraction, float.
        SHADING_COMP_COMP_N
    };


    enum VertexSemantic {
        VERTEX_POSITION = 0,
        VERTEX_COLOR,
        VERTEX_NORMAL,
        VERTEX_TEXCOORD,
        VERTEX_TEXTURE,
        VERTEX_TANGENT,
        VERTEX_BINORMAL,
        VERTEX_UV,
        VERTEX_SEMANTIC_N
    };

    enum RuntimeSemantic {
        /** Returns the width and height of the current framebuffer (float2) */
        RUNTIME_FRAMEBUFFER_SIZE,

        /** Returns 1/width and 1/height of the current framebuffer (float2) */
        RUNTIME_FRAMEBUFFER_SIZE_RECIPROCAL,

        /** Converts object coordinates to eye space coordinates (float4x4). */
        RUNTIME_MODELVIEW_MATRIX,

        /** Converts eye space coordinates to clip space coordines (float4x4). */
        RUNTIME_PROJECTION_MATRIX,

        /** Converts clip space coordinates to eye space coordinates (float4x4). */
        RUNTIME_PROJECTION_INVERSE_MATRIX,

        /** Converts object coordinates to clip space coordinates (float4x4). */
        RUNTIME_MODELVIEW_PROJECTION_MATRIX,

        /** The upper 3x4 of the inverse transpose of the modelview matrix (float3x3). */
        RUNTIME_NORMAL_MATRIX,

        /** Converts object coordinates to world coordinates (float4x4). */
        RUNTIME_WORLD_FROM_OBJECT,

        /** Converts world coordinates to object coordinates (float4x4). */
        RUNTIME_OBJECT_FROM_WORLD,

        /** Converts world coordinates to eye coordinates (float4x4). */
        RUNTIME_EYE_FROM_WORLD,

        /** Converts camera eye coordinates to world coordinates (float4x4). */
        RUNTIME_WORLD_FROM_EYE,
        /** Converts camera clip space coordinates to world coordinates (float4x4). */
        RUNTIME_WORLD_FROM_CLIP,

        /** The specified color of the light source (float3). */
        RUNTIME_LIGHT0_COLOR,
        /** \copydoc RUNTIME_LIGHT0_COLOR */
        RUNTIME_LIGHT1_COLOR,
        /** \copydoc RUNTIME_LIGHT0_COLOR */
        RUNTIME_LIGHT2_COLOR,
        /** \copydoc RUNTIME_LIGHT0_COLOR */
        RUNTIME_LIGHT3_COLOR,

        /** The constant attenuation factor of light source (float). */
        RUNTIME_LIGHT0_CONSTANT_ATT,
        /** \copydoc RUNTIME_LIGHT0_CONSTANT_ATT */
        RUNTIME_LIGHT1_CONSTANT_ATT,
        /** \copydoc RUNTIME_LIGHT0_CONSTANT_ATT */
        RUNTIME_LIGHT2_CONSTANT_ATT,
        /** \copydoc RUNTIME_LIGHT0_CONSTANT_ATT */
        RUNTIME_LIGHT3_CONSTANT_ATT,

        /** The linear attenuation factor of light source (float). */
        RUNTIME_LIGHT0_LINEAR_ATT,
        /** \copydoc RUNTIME_LIGHT0_LINEAR_ATT */
        RUNTIME_LIGHT1_LINEAR_ATT,
        /** \copydoc RUNTIME_LIGHT0_LINEAR_ATT */
        RUNTIME_LIGHT2_LINEAR_ATT,
        /** \copydoc RUNTIME_LIGHT0_LINEAR_ATT */
        RUNTIME_LIGHT3_LINEAR_ATT,

        /** The quadratic attenuation factor of light source (float). */
        RUNTIME_LIGHT0_QUADRATIC_ATT,
        /** \copydoc RUNTIME_LIGHT0_QUADRATIC_ATT */
        RUNTIME_LIGHT1_QUADRATIC_ATT,
        /** \copydoc RUNTIME_LIGHT0_QUADRATIC_ATT */
        RUNTIME_LIGHT2_QUADRATIC_ATT,
        /** \copydoc RUNTIME_LIGHT0_QUADRATIC_ATT */
        RUNTIME_LIGHT3_QUADRATIC_ATT,

        /** The cosine of the falloff angle of light source (float). */
        RUNTIME_LIGHT0_FALLOFF_COS,
        /** \copydoc RUNTIME_LIGHT0_FALLOFF_COS */
        RUNTIME_LIGHT1_FALLOFF_COS,
        /** \copydoc RUNTIME_LIGHT0_FALLOFF_COS */
        RUNTIME_LIGHT2_FALLOFF_COS,
        /** \copydoc RUNTIME_LIGHT0_FALLOFF_COS */
        RUNTIME_LIGHT3_FALLOFF_COS,

        /** The falloff exponent of light source (float). */
        RUNTIME_LIGHT0_FALLOFF_EXPONENT,
        /** \copydoc RUNTIME_LIGHT0_FALLOFF_EXPONENT */
        RUNTIME_LIGHT1_FALLOFF_EXPONENT,
        /** \copydoc RUNTIME_LIGHT0_FALLOFF_EXPONENT */
        RUNTIME_LIGHT2_FALLOFF_EXPONENT,
        /** \copydoc RUNTIME_LIGHT0_FALLOFF_EXPONENT */
        RUNTIME_LIGHT3_FALLOFF_EXPONENT,

        /** The position of light source 0 in object coordinates (float3). */
        RUNTIME_LIGHT0_POS_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_POS_OBJECT */
        RUNTIME_LIGHT1_POS_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_POS_OBJECT */
        RUNTIME_LIGHT2_POS_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_POS_OBJECT */
        RUNTIME_LIGHT3_POS_OBJECT,

        /** The position of light source in world coordinates (float3). */
        RUNTIME_LIGHT0_POS_WORLD,
        /** \copydoc RUNTIME_LIGHT0_POS_WORLD */
        RUNTIME_LIGHT1_POS_WORLD,
        /** \copydoc RUNTIME_LIGHT0_POS_WORLD */
        RUNTIME_LIGHT2_POS_WORLD,
        /** \copydoc RUNTIME_LIGHT0_POS_WORLD */
        RUNTIME_LIGHT3_POS_WORLD,

        /** The position of light source in eye coordinates (float3). */
        RUNTIME_LIGHT0_POS_EYE,
        /** \copydoc RUNTIME_LIGHT0_POS_EYE */
        RUNTIME_LIGHT1_POS_EYE,
        /** \copydoc RUNTIME_LIGHT0_POS_EYE */
        RUNTIME_LIGHT2_POS_EYE,
        /** \copydoc RUNTIME_LIGHT0_POS_EYE */
        RUNTIME_LIGHT3_POS_EYE,

        /** The direction of the z-axis of the light source in object coordinates (float3). */
        RUNTIME_LIGHT0_Z_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_Z_OBJECT */
        RUNTIME_LIGHT1_Z_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_Z_OBJECT */
        RUNTIME_LIGHT2_Z_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_Z_OBJECT */
        RUNTIME_LIGHT3_Z_OBJECT,

        /** The direction of the z-axis of the light source in world coordinates (float3). */
        RUNTIME_LIGHT0_Z_WORLD,
        /** \copydoc RUNTIME_LIGHT0_Z_WORLD */
        RUNTIME_LIGHT1_Z_WORLD,
        /** \copydoc RUNTIME_LIGHT0_Z_WORLD */
        RUNTIME_LIGHT2_Z_WORLD,
        /** \copydoc RUNTIME_LIGHT0_Z_WORLD */
        RUNTIME_LIGHT3_Z_WORLD,

        /** The direction of the z-axis of the light source in eye coordinates (float3). */
        RUNTIME_LIGHT0_Z_EYE,
        /** \copydoc RUNTIME_LIGHT0_Z_EYE */
        RUNTIME_LIGHT1_Z_EYE,
        /** \copydoc RUNTIME_LIGHT0_Z_EYE */
        RUNTIME_LIGHT2_Z_EYE,
        /** \copydoc RUNTIME_LIGHT0_Z_EYE */
        RUNTIME_LIGHT3_Z_EYE,

        /** Converts object coordinates to eye space coordinates of light (float4x4). */
        RUNTIME_LIGHT0_EYE_FROM_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_EYE_FROM_OBJECT */
        RUNTIME_LIGHT1_EYE_FROM_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_EYE_FROM_OBJECT */
        RUNTIME_LIGHT2_EYE_FROM_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_EYE_FROM_OBJECT */
        RUNTIME_LIGHT3_EYE_FROM_OBJECT,

        /** Converts world coordinates to eye space coordinates of light (float4x4). */
        RUNTIME_LIGHT0_EYE_FROM_WORLD,
        /** \copydoc RUNTIME_LIGHT0_EYE_FROM_WORLD */
        RUNTIME_LIGHT1_EYE_FROM_WORLD,
        /** \copydoc RUNTIME_LIGHT0_EYE_FROM_WORLD */
        RUNTIME_LIGHT2_EYE_FROM_WORLD,
        /** \copydoc RUNTIME_LIGHT0_EYE_FROM_WORLD */
        RUNTIME_LIGHT3_EYE_FROM_WORLD,

        /** Converts light eye space coordinates to world coordinates (float4x4). */
        RUNTIME_WORLD_FROM_LIGHT0_EYE,
        /** \copydoc RUNTIME_WORLD_FROM_LIGHT0_EYE */
        RUNTIME_WORLD_FROM_LIGHT1_EYE,
        /** \copydoc RUNTIME_WORLD_FROM_LIGHT0_EYE */
        RUNTIME_WORLD_FROM_LIGHT2_EYE,
        /** \copydoc RUNTIME_WORLD_FROM_LIGHT0_EYE */
        RUNTIME_WORLD_FROM_LIGHT3_EYE,

        /** Converts eye coordinates to eye space coordinates of light (float4x4). */
        RUNTIME_LIGHT0_EYE_FROM_EYE,
        /** \copydoc RUNTIME_LIGHT0_EYE_FROM_EYE */
        RUNTIME_LIGHT1_EYE_FROM_EYE,
        /** \copydoc RUNTIME_LIGHT0_EYE_FROM_EYE */
        RUNTIME_LIGHT2_EYE_FROM_EYE,
        /** \copydoc RUNTIME_LIGHT0_EYE_FROM_EYE */
        RUNTIME_LIGHT3_EYE_FROM_EYE,

        /** Converts object coordinates to clip space coordinates of light (float4x4). */
        RUNTIME_LIGHT0_CLIP_FROM_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_CLIP_FROM_EYE */
        RUNTIME_LIGHT1_CLIP_FROM_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_CLIP_FROM_EYE */
        RUNTIME_LIGHT2_CLIP_FROM_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_CLIP_FROM_EYE */
        RUNTIME_LIGHT3_CLIP_FROM_OBJECT,

        /** Converts world coordinates to clip space coordinates of light (float4x4). */
        RUNTIME_LIGHT0_CLIP_FROM_WORLD,
        /** \copydoc RUNTIME_LIGHT0_CLIP_FROM_WORLD */
        RUNTIME_LIGHT1_CLIP_FROM_WORLD,
        /** \copydoc RUNTIME_LIGHT0_CLIP_FROM_WORLD */
        RUNTIME_LIGHT2_CLIP_FROM_WORLD,
        /** \copydoc RUNTIME_LIGHT0_CLIP_FROM_WORLD */
        RUNTIME_LIGHT3_CLIP_FROM_WORLD,

        /** Converts eye coordinates to clip space coordinates of light (float4x4). */
        RUNTIME_LIGHT0_CLIP_FROM_EYE,
        /** \copydoc RUNTIME_LIGHT0_CLIP_FROM_EYE */
        RUNTIME_LIGHT1_CLIP_FROM_EYE,
        /** \copydoc RUNTIME_LIGHT0_CLIP_FROM_EYE */
        RUNTIME_LIGHT2_CLIP_FROM_EYE,
        /** \copydoc RUNTIME_LIGHT0_CLIP_FROM_EYE */
        RUNTIME_LIGHT3_CLIP_FROM_EYE,

        /** Coverts camera clip space coordinates to light clip space coordinates (float4x4).*/
        RUNTIME_LIGHT0_CLIP_FROM_CLIP,
        /** \copydoc RUNTIME_LIGHT0_CLIP_FROM_CLIP */
        RUNTIME_LIGHT1_CLIP_FROM_CLIP,
        /** \copydoc RUNTIME_LIGHT0_CLIP_FROM_CLIP */
        RUNTIME_LIGHT2_CLIP_FROM_CLIP,
        /** \copydoc RUNTIME_LIGHT0_CLIP_FROM_CLIP */
        RUNTIME_LIGHT3_CLIP_FROM_CLIP,

        /** Converts object coordinates to texture coordinates of light (float4x4). */
        RUNTIME_LIGHT0_TEX_FROM_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_TEX_FROM_OBJECT */
        RUNTIME_LIGHT1_TEX_FROM_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_TEX_FROM_OBJECT */
        RUNTIME_LIGHT2_TEX_FROM_OBJECT,
        /** \copydoc RUNTIME_LIGHT0_TEX_FROM_OBJECT */
        RUNTIME_LIGHT3_TEX_FROM_OBJECT,

        /** Converts world coordinates to texture coordinates of light (float4x4). */
        RUNTIME_LIGHT0_TEX_FROM_WORLD,
        /** \copydoc RUNTIME_LIGHT0_TEX_FROM_WORLD */
        RUNTIME_LIGHT1_TEX_FROM_WORLD,
        /** \copydoc RUNTIME_LIGHT0_TEX_FROM_WORLD */
        RUNTIME_LIGHT2_TEX_FROM_WORLD,
        /** \copydoc RUNTIME_LIGHT0_TEX_FROM_WORLD */
        RUNTIME_LIGHT3_TEX_FROM_WORLD,

        /** Converts eye coordinates to texture coordinates of light (float4x4). */
        RUNTIME_LIGHT0_TEX_FROM_EYE,
        /** \copydoc RUNTIME_LIGHT0_TEX_FROM_EYE */
        RUNTIME_LIGHT1_TEX_FROM_EYE,
        /** \copydoc RUNTIME_LIGHT0_TEX_FROM_EYE */
        RUNTIME_LIGHT2_TEX_FROM_EYE,
        /** \copydoc RUNTIME_LIGHT0_TEX_FROM_EYE */
        RUNTIME_LIGHT3_TEX_FROM_EYE,

        /** Placeholder for illegal runtime semantic. */
        RUNTIME_SEMANTIC_N
    };

    enum PrimitiveType {
        PRIMITIVE_POINTS,
        PRIMITIVE_LINES,
        PRIMITIVE_TRIANGLES,
        PRIMITIVE_QUADS,
        PRIMITIVE_PATCHES,
        PRIMITIVE_N
    };

    enum Usage {
        USAGE_HOST = 1,
        USAGE_DEVICE = 2,
        USAGE_HOST_DEVICE = 3
    };

    enum ShaderStage {
        STAGE_VERTEX = 0,
        STAGE_GEOMETRY,
        STAGE_TESSELLATION_CONTROL,
        STAGE_TESSELLATION_EVALUATION,
        STAGE_FRAGMENT,
        STAGE_N
    };

    enum ProfileType {
        PROFILE_BRIDGE = (1<<0),
        PROFILE_CG     = (1<<1),
        PROFILE_COMMON = (1<<2),
        PROFILE_GLES   = (1<<3),
        PROFILE_GLES2  = (1<<4),
        PROFILE_GLSL   = (1<<5)
    };

    enum SourceType {
        SOURCE_REF,
        SOURCE_INLINE
    };

    enum RenderTarget {
        RENDER_TARGET_COLOR,
        RENDER_TARGET_DEPTH,
        RENDER_TARGET_STENCIL
    };

    enum Draw {
        // Normal rendering
        DRAW_GEOMETRY,
        // Full scene using only this effect (e.g. shadow-buffering)
        DRAW_SCENE_GEOMETRY,
        // Draw normally into targets
        DRAW_SCENE_IMAGE,
        // Draw fsq instead of geometry
        DRAW_FULL_SCREEN_QUAD
    };

    enum TransformType {
        TRANSFORM_LOOKAT,
        TRANSFORM_MATRIX,
        TRANSFORM_ROTATE,
        TRANSFORM_SCALE,
        TRANSFORM_SKEW,
        TRANSFORM_TRANSLATE,
        TRANSFORM_N
    };

    enum CameraType {
        CAMERA_PERSPECTIVE,
        CAMERA_ORTHOGONAL,
        CAMERA_CUSTOM_MATRIX,
        CAMERA_N
    };

    enum StateType {
        /** Sets the point size, defaults to <float>1</float> */
        STATE_POINT_SIZE = 0,
        /** Sets the polygon offset, defaults to <float2>0 0</float2> */
        STATE_POLYGON_OFFSET,
        STATE_POLYGON_OFFSET_FILL_ENABLE,
        STATE_BLEND_ENABLE,
        STATE_BLEND_FUNC,
        STATE_CULL_FACE_ENABLE,
        STATE_CULL_FACE,
        STATE_DEPTH_TEST_ENABLE,
        STATE_DEPTH_MASK,
        STATE_POLYGON_MODE,
        STATE_N
    };

    enum InstanceType {
        INSTANCE_GEOMETRY = 0
    };

    class Asset;
    class Effect;
    class Camera;
    class DataBase;
    class Geometry;

    template<class T> class Library;

    class EvaluateScene;
    class Render;
    class Image;
    class Light;
    class Material;
    class Node;
    class NodeList;
    class Parameter;
    class Pass;
    class Profile;
    class SourceBuffer;
    class State_;
    class Technique;
    class Value;
    class InstanceGeometry;
    class VisualScene;
    class Primitives;
    class CommonShadingModel;
    struct Bind;

    template<class T>
    void
    deadBeef( T p )
    { p = reinterpret_cast<T>(0xDEADBEEF); }

#define DEADBEEF(a) deadBeef(a)

    namespace GLSL {
        class Runtime;
        class RenderList;
    } // of namespace GLSL

    namespace Collada {
        class Importer;
        class BuilderTest;
    } // of namespace XML

    namespace Kernels {
        class BoundingBox;

    } // of namespace Kernels

} // of namespace Scene
