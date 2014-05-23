scene
=====

COLLADA parser and runtime


CMake options
==============

SCENE_TINIA         Build Tinia bridge (export of renderlists), say YES if you
                    intend to use Scene with Tinia.
SCENE_LOG4CXX       Use log4cxx for logging, say YES if you use log4cxx in your
                    project, NO otherwise.
SCENE_OPTIMIZATION  Enable compiler optimizations, say YES if unsure.
SCENE_SSE4_2        Enable use of SSE4.2 intrinsics in transformcache, say YES
                    unless you want to run Scene on a non-SSE4.2 CPU (pre i7).
SCENE_DEBUG         Enable debug symbols, say YES if you intend to run anything
                    linked to Scene through a debugger.
SCENE_THREADS       Use a thread-pool in transformcache, currently say NO if
                    unsure.
SCENE_PROFILING     Enable compile-time profiling info, say NO if unsure.
SCENE_CHECK_TYPES   Enable runtime-checks of types, say NO unless you develop
                    Scene itself.
SCENE_RL_CHUNKS     Use new chunk-based renderlist building, say YES if unsure.
SCENE_UNITTEST      Build a small set of unit-tests


Requirements
============

General:
- libPNG                Required to read textures.
- libXml2               Required to parse COLLADA documents.
- boost 1.47 or newer
- GLEW
- GLM
- GLUT/FreeGLUT         Used by GLUT-based viewer.
- Tinia                 Used by Tinia-based viewer and Tinia-bridge.

Windows:
- zlib

Bounding-boxes
==============

COLLADA files do not have any intrinsic bounding-box information, which is
needed when determining the view frustum. Scene handles this using three
functions:

- Scene::Tools::updateBoundingBox( geometry )
  Updates the bounding box for a single geometry.

- Scene::Tools::updateBoundingBox( database )
  Updates bounding boxes for all geometries in a database.

- Scene::Tools::visualSceneExtents( bbmin, bbmax, renderlist )
  Get the composite world-space bounding box for all transformed geometry
  instances in a visual scene.

Shared inputs
=============

COLLADA allows indexed geometry to be specified using index-tuples, which is not
supported by OpenGL. Scene handles this using the following two functions:

- Scene::Geometry::hasSharedInputs
  Returns true if the geometry has shared inputs and cannot be directly
  rendered.

- Scene::Geometry::flatten
  Flattens multi-indices to single-indicies so that the geometry can be
  directly rendered.


Shader profiles
===============

COLLADA allows an effect to have different profiles. For on-screen rendering,
Scene requires that a GLSL profile is present. For webgl renderlist, the GLES2
profile is required to be present.

If a COMMON profile is present GLSL/GLES2 profiles can be generated:

- Scene::Effect::profile( profile )
  Returns the prescribed profile, or NULL if not present.

- Scene::Effect::generate( profile )
  Generate profile from COMMON profile.


