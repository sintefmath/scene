#pragma once
#ifdef SCENE_USE_THREADS
#include <list>
#include <thread>
#include <mutex>
#endif
//#include <condition_variable>
#include <unordered_map>
#include "scene/Scene.hpp"
#include "scene/Value.hpp"
#include <scene/SeqPos.hpp>
#include <scene/runtime/CacheKey.hpp>
#include <scene/runtime/CacheLUT.hpp>
#include <scene/runtime/RenderAction.hpp>

namespace Scene {
    namespace Runtime {

/** Caches a set of transforms.
  *
  * By calling purge, the cache is completely cleaned and ready for a new
  * render list. This should be done each time the render list changes.
  *
  * The public access functions (e.g. cameraProjectionMatrix) fetches or creates
  * new entries in the cache. The returned pointer is persisent, and can be used
  * until purge is called. Initially, these values are undefined.
  *
  * The value's are populated once per frame by calling the update function,
  * which runs through all cache entries in a dependency-aware order and assigns
  * actual values to the values provided by the public access functions.
  *
  *
  */
class TransformCache
{
public:
    /** Create a new TransformCache attached to a specified database. */
    TransformCache( const DataBase& database, const bool use_threadpool = false );

    ~TransformCache();

    /** Remove all cached entries.
      *
      * This operation voids all pointers that have been returned by the public
      * methods.
      */
    void
    purge();

    /** Update all cache entries with fresh values from the scene graph.
      *
      * Should be called when a minor update has happened (usually once every
      * frame).
      *
      * Default FBO width and height are the width and height of the onscreen
      * framebuffer (and used when FBO == NULL).
      *
      */
    void
    update( unsigned int m_default_fbo_width,
            unsigned int m_default_fbo_height );

    /** Checks if the bounding box of a geometry intersects the current view frustum.
      *
      * \returns A value of type VALUE_TYPE_BOOL.
      */
    const Value*
    checkBoundingBox( const SetViewCoordSys*   view_coords,
                      const SetLocalCoordSys*  local_coords,
                      const Geometry*          geometry );

    const Value*
    runtimeSemantic( RuntimeSemantic          semantic,
                     const SetRenderTargets*  render_targets,
                     const SetViewCoordSys*   view_coords,
                     const SetLocalCoordSys*  local_coords );

    /** Get a pointer to a matrix describing the projection matrix of the camera.
      *
      * The pointer is valid until the next time purge is invoked.
      */
    const Value*
    cameraProjectionMatrix( const Camera* camera );

    /** Get a pointer to the inverse of the matrix describing the projection of the camera.
      *
      * The pointer is valid until the next time purge is invoked.
      */
    const Value*
    cameraProjectionMatrixInverse( const Camera* camera );


    /** Get a pointer to a matrix describing all transforms of a given node.
      *
      * The pointer is valid until the next time purge is invoked.
      */
    const Value*
    nodeTransformMatrix( const Node* node );

    /** Get a pointer to the inverse matrix describing all transforms of a given node.
      *
      * The pointer is valid until the next time purge is invoked.
      */
    const Value*
    nodeTransformInverseMatrix( const Node* node );


    /** Get a pointer to the matrix describing the path from root to leaf.
      *
      * Root and leaf can be arbitrary nodes as long as leaf is a descendant of
      * root.
      * The pointer is valid until the next time purge is invoked.
      */
    const Value*
    branchTransformMatrix( const Node* root, const Node* leaf );

    /** Get a pointer to the inverse matrix describing the path from root to leaf.
      *
      * Root and leaf can be arbitrary nodes as long as leaf is a descendant of
      * root.
      * The pointer is valid until the next time purge is invoked.
      */
    const Value*
    branchTransformInverseMatrix( const Node* root, const Node* leaf );

    /** Get a pointer to the matrix describing a sequence of paths.
      *
      * Two successive entries in the path list are the root and leaf nodes of
      * a branch, and the path is the composition of those branches.
      *
      * The pointer is valid until the next time purge is invoked.
      */
    const Value*
    pathTransformMatrix( const Node* const (&path)[SCENE_PATH_MAX] );


    /** Get a pointer to the inverse of a matrix describing a sequence of paths.
      *
      * Two successive entries in the path list are the root and leaf nodes of
      * a branch, and the path is the composition of those branches.
      *
      * The pointer is valid until the next time purge is invoked.
      */
    const Value*
    pathTransformInverseMatrix( const Node* const (&path)[SCENE_PATH_MAX] );

    const Value*
    matrixComposition( const Value* M0, const Value* M1, const Value* M2 = NULL, const Value* M3 = NULL );





protected:
//    0 4 8 12
//    1 5 9 13
//    2 6 10 14
//    3 7 11 15




    const DataBase&                                                           m_database;
    const bool                                              m_use_threadpool;
    SeqPos                                                                 m_last_purge;
    bool m_has_dumped;

#ifdef SCENE_USE_THREADS
    struct ThreadPool {
        enum PoolState {
            THROTTLE,
            COMPUTE_PASS1,
            COMPUTE_BRANCH_TRANSFORM,
            COMPUTE_PATH_TRANSFORM,
            COMPUTE_PASS5,
            DIE
        };
        std::list<std::thread>                              m_workers;
        std::mutex                                          m_mutex;
        std::condition_variable                             m_worker_cond;
        std::condition_variable                             m_master_cond;
        volatile PoolState                                  m_state;
        volatile size_t                                     m_iteration_next;
        volatile size_t                                     m_iteration_done;
        volatile size_t                                     m_iteration_total;

        static void worker( TransformCache* that );
    }                                                       m_thread_pool;

    void
    threadedUpdate( );
#endif
    Value                                                                     m_bias_matrix; // Converts clip space to light space
    Value                                                                     m_default_fbo_size;

    enum ComputeAction {
        PASS1_DEDUCE_COSINE_OF_RADIAN_ANGLE =0,
        PASS1_DEDUCE_RECIPROCAL_VEC2,
        PASS1_COMPUTE_CAMERA_PROJECTION,
        PASS1_COMPUTE_CAMERA_PROJECTION_INVERSE,
        PASS1_COMPUTE_NODE_TRANSFORM,
        PASS1_COMPUTE_NODE_TRANSFORM_INVERSE,


//        PASS5_SUBSET_UPPER3X3_TRANSPOSE = 0,
        PASS5_PRODUCT_UPPER3X3_TRANSPOSE = 0,
        PASS5_SUBSET_POSTMULTIPLY_ORIGIN,
        PASS5_SUBSET_PREMULTIPLY_Z,
        PASS5_CHECK_BBOX_IN_FRUSTUM,
        MULTIPLY_MATRICES
    };

    template<size_t N>
    struct CacheItem
    {
        Value*             m_value;                 ///< Result of this computation.
        ComputeAction      m_action;                ///< What to compute.
        size_t             m_N;                     ///< Number of source values.
        union {
            const Camera*     m_source_camera;      ///< Camera source
            const Node*       m_source_node;        ///< Node source
            const Value*      m_source_values[ N ]; ///< Value sources
        };
    };


    // Pass 1: Deduce values from scene, transforms, and projections
    std::vector< CacheItem<1> >                 m_pass1_values;
    CacheLUT<2>                                 m_deduced_values_lut;
    CacheLUT<1>                                 m_camera_projection_lut;
    CacheLUT<1>                                 m_camera_inverse_projection_lut;
    CacheLUT<1>                                 m_node_transform_lut;
    CacheLUT<1>                                 m_node_inverse_transform_lut;
    //std::unordered_map<CacheKey<2>, size_t >                m_deduced_values_cache;
    //std::unordered_map<CacheKey<1>, size_t >                m_camera_projection_cache;
    //std::unordered_map<CacheKey<1>, size_t >                m_camera_projection_inverse_cache;
    //std::unordered_map<CacheKey<1>, size_t >                m_node_transform_cache;
    //std::unordered_map<CacheKey<1>, size_t >                m_node_transform_inverse_cache;

    // Pass 2: Compositions of node hierarchies
    std::vector< CacheItem<SCENE_PATH_MAX> >    m_branch_transform;
    CacheLUT<2>                                 m_branch_transform_lut;
    CacheLUT<2>                                 m_branch_inverse_transform_lut;
    //std::unordered_map<CacheKey<2>, size_t >                m_branch_transform_cache;
    //std::unordered_map<CacheKey<2>, size_t >                m_branch_transform_inverse_cache;

    // Pass 3: Compositions of instance hierarchies
    std::vector< CacheItem<SCENE_PATH_MAX> >    m_path_transform;
    CacheLUT<SCENE_PATH_MAX>                    m_path_transform_lut;
    CacheLUT<SCENE_PATH_MAX>                    m_path_inverse_transform_lut;
    //std::unordered_map<CacheKey<SCENE_PATH_MAX>, size_t >   m_path_transform_cache;
    //std::unordered_map<CacheKey<SCENE_PATH_MAX>, size_t >   m_path_transform_inverse_cache;

    // Pass 4: Extracting or using results of passes 1, 2, and 3.
    std::vector< CacheItem<SCENE_PATH_MAX> >    m_pass4_values;
    CacheLUT<4>                                 m_matrix_composition_lut;
    CacheLUT<2>                                 m_transform_origin_lut;
    CacheLUT<2>                                 m_premultiply_z_lut;
    CacheLUT<2>                                 m_matrix_prod_3x3_transpose_lut;
    CacheLUT<3>                                 m_bbox_check_lut;
    //std::unordered_map<CacheKey<4>, size_t >                m_matrix_composition_cache;
    //std::unordered_map<CacheKey<2>, size_t >                m_matrix_z_axis_cache;
    //std::unordered_map<CacheKey<2>, size_t >                m_matrix_origin_cache;
    //std::unordered_map<CacheKey<2>, size_t >                m_matrix_prod_3x3_transpose_cache;
    //std::unordered_map<CacheKey<3>, size_t >                m_bbox_check_cache;


    const Value*
    matrixProductUpper3x3Transpose( const Value* A, const Value* B );

    const Value*
    matrixSubsetPostmultiplyOrigin( const Value* A, const Value* B );

    const Value*
    matrixSubsetPremultiplyZ( const Value* A, const Value* B );

    const Value*
    cosineOfRadianAngle( const Value* source );

    const Value*
    reciprocalOfVec2( const Value* source );


    TransformCache();

};


    }
}
