#define _USE_MATH_DEFINES
#ifdef SCENE_USE_THREADS
#include <atomic>
#endif

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <algorithm>
#include "scene/Log.hpp"
#include "scene/Camera.hpp"
#include "scene/Node.hpp"
#include "scene/Light.hpp"
#include "scene/DataBase.hpp"
#include <scene/Utils.hpp>
#include "scene/runtime/TransformCache.hpp"
#include <scene/runtime/TransformCompute.hpp>


namespace Scene {
    namespace Runtime {
        using std::string;

static const string package = "Scene.Runtime.TransformCache";




TransformCache::TransformCache(const DataBase &database, const bool use_threadpool)
    : m_database( database ),
      m_use_threadpool( use_threadpool ),
      m_has_dumped( false )
{
    m_bias_matrix = Value::createFloat4x4( 0.5f, 0.0f, 0.0f, 0.0f,
                                           0.0f, 0.5f, 0.0f, 0.0f,
                                           0.0f, 0.0f, 0.5f, 0.0f,
                                           0.5f, 0.5f, 0.5f, 1.0f );


    m_default_fbo_size = Value::createFloat2( 1.f, 1.f );
#ifdef SCENE_USE_THREADS
    if( m_use_threadpool ) {
        m_thread_pool.m_state = ThreadPool::THROTTLE;
        for(size_t i=0; i<4; i++ ) {
            m_thread_pool.m_workers.push_back( std::thread( ThreadPool::worker, this ) );
        }
        Logger log = getLogger( package + "TransformCache" );
        SCENELOG_DEBUG( log, "Created threadpool with " << m_thread_pool.m_workers.size() << " threads" );
    }
#endif
}

TransformCache::~TransformCache()
{
#ifdef SCENE_USE_THREADS
    if( m_use_threadpool ) {
        std::unique_lock<std::mutex> lock( m_thread_pool.m_mutex );
        m_thread_pool.m_state = ThreadPool::DIE;
        m_thread_pool.m_worker_cond.notify_all();
        lock.unlock();
        for(auto it=m_thread_pool.m_workers.begin(); it!=m_thread_pool.m_workers.end(); ++it ) {
            it->join();
        }
        Logger log = getLogger( package + "~TransformCache" );
        SCENELOG_DEBUG( log, "Joined " << m_thread_pool.m_workers.size() << " threads in threadpool" );
    }
#endif
    purge();
}

#ifdef SCENE_USE_THREADS
void
TransformCache::ThreadPool::worker( TransformCache* that )
{
    while(1) {
        std::unique_lock<std::mutex> lock( that->m_thread_pool.m_mutex );
        while( that->m_thread_pool.m_state == ThreadPool::THROTTLE ) {
            that->m_thread_pool.m_worker_cond.wait( lock );
        }
        if( that->m_thread_pool.m_state == ThreadPool::DIE ) {
            break;
        }
        else if( that->m_thread_pool.m_iteration_next < that->m_thread_pool.m_iteration_total ) {
            size_t i = that->m_thread_pool.m_iteration_next;
            size_t n = std::min( (size_t)100u,
                                 that->m_thread_pool.m_iteration_total - i );
            that->m_thread_pool.m_iteration_next += n;
            PoolState s = that->m_thread_pool.m_state;
            lock.unlock();

            switch( s ) {
            case COMPUTE_PASS1:
                for( size_t k=0; k<n; k++ ) {
                    TransformCache::CacheItem<1>& item = that->m_pass1_values[i+k];
                    switch( item.m_action ) {
                    case PASS1_DEDUCE_COSINE_OF_RADIAN_ANGLE:
                        TransformCompute::cosine( item.m_value, item.m_source_values[0] );
                        break;
                    case PASS1_DEDUCE_RECIPROCAL_VEC2:
                        TransformCompute::reciprocal( item.m_value, item.m_source_values[0] );
                        break;
                    case PASS1_COMPUTE_CAMERA_PROJECTION:
                        TransformCompute::projection( item.m_value, item.m_source_camera );
                        break;
                    case PASS1_COMPUTE_CAMERA_PROJECTION_INVERSE:
                        TransformCompute::projectionInverse( item.m_value, item.m_source_camera );
                        break;
                    case PASS1_COMPUTE_NODE_TRANSFORM:
                        TransformCompute::nodeTransform( item.m_value, item.m_source_node );
                        break;
                    case PASS1_COMPUTE_NODE_TRANSFORM_INVERSE:
                        TransformCompute::nodeInverseTransform( item.m_value, item.m_source_node );
                        break;
                    default:
                        break;
                    }
                }
                break;
            case COMPUTE_BRANCH_TRANSFORM:
                for( size_t k=0; k<n; k++ ) {
                    TransformCache::CacheItem<SCENE_PATH_MAX>& item = that->m_branch_transform[i+k];
                    TransformCompute::multiplyMatrices( item.m_value, item.m_N, item.m_source_values );
                }
                break;
            case COMPUTE_PATH_TRANSFORM:
                for( size_t k=0; k<n; k++ ) {
                    TransformCache::CacheItem<SCENE_PATH_MAX>& item = that->m_path_transform[i+k];
                    TransformCompute::multiplyMatrices( item.m_value, item.m_N, item.m_source_values );
                }
                break;
            case COMPUTE_PASS5:
                for( size_t k=0; k<n; k++ ) {
                    TransformCache::CacheItem<SCENE_PATH_MAX>& item = that->m_pass4_values[i+k];
                    switch( item.m_action )
                    {
                    case PASS5_PRODUCT_UPPER3X3_TRANSPOSE:
                        TransformCompute::transposedUpper3x3( item.m_value, item.m_N, item.m_source_values );
                        break;
                    case PASS5_SUBSET_POSTMULTIPLY_ORIGIN:
                        TransformCompute::transformOrigin(  item.m_value, item.m_N, item.m_source_values );
                        break;
                    case PASS5_SUBSET_PREMULTIPLY_Z:
                        TransformCompute::transformZAxis( item.m_value, item.m_N, item.m_source_values );
                        break;
                    case PASS5_CHECK_BBOX_IN_FRUSTUM:
                        TransformCompute::boundingBoxTest( item.m_value, item.m_N, item.m_source_values );
                        break;
                    case MULTIPLY_MATRICES:
                        TransformCompute::multiplyMatrices( item.m_value, item.m_N, item.m_source_values );
                        break;
                    default:
                        break;
                    }
                }
                break;
            default:
                break;
            }

            lock.lock();
            that->m_thread_pool.m_iteration_done += n;
            if( that->m_thread_pool.m_iteration_done >= that->m_thread_pool.m_iteration_total ) {

                // Am I the last thread to finish this pass?
                that->m_thread_pool.m_iteration_next = 0;
                that->m_thread_pool.m_iteration_done = 0;
                switch( that->m_thread_pool.m_state ) {
                case COMPUTE_PASS1:
                    that->m_thread_pool.m_state = COMPUTE_BRANCH_TRANSFORM;
                    that->m_thread_pool.m_iteration_total = that->m_branch_transform.size();
                    that->m_thread_pool.m_worker_cond.notify_all();
                    break;

                case COMPUTE_BRANCH_TRANSFORM:
                    that->m_thread_pool.m_state = COMPUTE_PATH_TRANSFORM;
                    that->m_thread_pool.m_iteration_total = that->m_path_transform.size();
                    that->m_thread_pool.m_worker_cond.notify_all();
                    break;

                case COMPUTE_PATH_TRANSFORM:
                    that->m_thread_pool.m_state = COMPUTE_PASS5;
                    that->m_thread_pool.m_iteration_total = that->m_pass4_values.size();
                    that->m_thread_pool.m_worker_cond.notify_all();
                    break;
                case COMPUTE_PASS5:
                    that->m_thread_pool.m_state = THROTTLE;
                    that->m_thread_pool.m_iteration_total = 0;
                default:
                    that->m_thread_pool.m_master_cond.notify_one();
                }
            }
        }
    }
}

void
TransformCache::threadedUpdate()
{
    {
        // Set up a compute pass
        std::unique_lock<std::mutex> lock( m_thread_pool.m_mutex );
        m_thread_pool.m_state = ThreadPool::COMPUTE_PASS1;
        m_thread_pool.m_iteration_next = 0;
        m_thread_pool.m_iteration_total = m_pass1_values.size();
        m_thread_pool.m_iteration_done = 0;
        // Trigger workers in thread pool to start.
        m_thread_pool.m_worker_cond.notify_all();
        // wait for workers in thread pool to finish.
        while( m_thread_pool.m_state != ThreadPool::THROTTLE ) {
            m_thread_pool.m_master_cond.wait( lock );
        }

    }
}
#endif

void
TransformCache::update( unsigned int m_default_fbo_width,
                        unsigned int m_default_fbo_height )
{

    m_default_fbo_size = Value::createFloat2( m_default_fbo_width,
                                              m_default_fbo_height );
#ifdef SCENE_USE_THREADS
    if( m_use_threadpool ) {

        if( !m_has_dumped ) {
            Logger log = getLogger( package + ".update" );
            SCENELOG_ERROR( log, "pass 1 = " << m_pass1_values.size() << " items" );
            SCENELOG_ERROR( log, "pass 2 = " << m_branch_transform.size() << " items" );
            SCENELOG_ERROR( log, "pass 3 = " << m_path_transform.size() << " items" );
            SCENELOG_ERROR( log, "pass 4 = " << m_pass4_values.size() << " items" );
            m_has_dumped = true;
        }

        threadedUpdate();
        return;
    }
#endif

    for( auto it=m_pass1_values.begin(); it!=m_pass1_values.end(); ++it ) {
        TransformCache::CacheItem<1>& item = *it;
        switch( item.m_action ) {
        case PASS1_DEDUCE_COSINE_OF_RADIAN_ANGLE:
            TransformCompute::cosine( item.m_value, item.m_source_values[0] );
            break;
        case PASS1_DEDUCE_RECIPROCAL_VEC2:
            TransformCompute::reciprocal( item.m_value, item.m_source_values[0] );
            break;
        case PASS1_COMPUTE_CAMERA_PROJECTION:
            TransformCompute::projection( item.m_value, item.m_source_camera );
            break;
        case PASS1_COMPUTE_CAMERA_PROJECTION_INVERSE:
            TransformCompute::projectionInverse( item.m_value, item.m_source_camera );
            break;
        case PASS1_COMPUTE_NODE_TRANSFORM:
            TransformCompute::nodeTransform( item.m_value, item.m_source_node );
            break;
        case PASS1_COMPUTE_NODE_TRANSFORM_INVERSE:
            TransformCompute::nodeInverseTransform( item.m_value, item.m_source_node );
            break;
        default:
            break;
        }
    }
    for( auto it=m_branch_transform.begin(); it!=m_branch_transform.end(); ++it ) {
        TransformCache::CacheItem<SCENE_PATH_MAX>& item = *it;
        TransformCompute::multiplyMatrices( item.m_value, item.m_N, item.m_source_values );
    }
    for( auto it=m_path_transform.begin(); it!=m_path_transform.end(); ++it ) {
        TransformCache::CacheItem<SCENE_PATH_MAX>& item = *it;
        TransformCompute::multiplyMatrices( item.m_value, item.m_N, item.m_source_values );
    }
    for( auto it=m_pass4_values.begin(); it!=m_pass4_values.end(); ++it ) {
        TransformCache::CacheItem<SCENE_PATH_MAX>& item = *it;
        switch( item.m_action )
        {
        case PASS5_PRODUCT_UPPER3X3_TRANSPOSE:
            TransformCompute::transposedUpper3x3( item.m_value, item.m_N, item.m_source_values );
            break;
        case PASS5_SUBSET_POSTMULTIPLY_ORIGIN:
            TransformCompute::transformOrigin(  item.m_value, item.m_N, item.m_source_values );
            break;
        case PASS5_SUBSET_PREMULTIPLY_Z:
            TransformCompute::transformZAxis( item.m_value, item.m_N, item.m_source_values );
            break;
        case PASS5_CHECK_BBOX_IN_FRUSTUM:
            TransformCompute::boundingBoxTest( item.m_value, item.m_N, item.m_source_values );
            break;
        case MULTIPLY_MATRICES:
            TransformCompute::multiplyMatrices( item.m_value, item.m_N, item.m_source_values );
            break;
        default:
            break;
        }
    }
}

void
TransformCache::purge()
{
    Logger log = getLogger( package + ".purge" );
    SCENELOG_DEBUG( log, "Purging contents." );

    for( auto it=m_pass1_values.begin(); it!=m_pass1_values.end(); ++it ) {
#ifdef SCENE_CHECK_TYPES
        it->m_value->undefine();
#endif
        delete it->m_value;
    }
    m_pass1_values.clear();
    m_deduced_values_lut.clear();
    m_camera_projection_lut.clear();
    m_camera_inverse_projection_lut.clear();
    m_node_transform_lut.clear();
    m_node_inverse_transform_lut.clear();
    //m_deduced_values_cache.clear();
    //m_camera_projection_cache.clear();
    //m_camera_projection_inverse_cache.clear();
    //m_node_transform_cache.clear();
    //m_node_transform_inverse_cache.clear();

    for( auto it=m_branch_transform.begin(); it!=m_branch_transform.end(); ++it ) {
#ifdef SCENE_CHECK_TYPES
        it->m_value->undefine();
#endif
        delete it->m_value;
    }
    m_branch_transform.clear();
    m_branch_transform_lut.clear();
    m_branch_inverse_transform_lut.clear();
    //m_branch_transform_cache.clear();
    //m_branch_transform_inverse_cache.clear();

    for( auto it=m_path_transform.begin(); it!=m_path_transform.end(); ++it ) {
#ifdef SCENE_CHECK_TYPES
        it->m_value->undefine();
#endif
        delete it->m_value;
    }
    m_path_transform.clear();
    m_path_transform_lut.clear();
    m_path_inverse_transform_lut.clear();
    //m_path_transform_cache.clear();
    //m_path_transform_inverse_cache.clear();


    for( auto it=m_pass4_values.begin(); it!=m_pass4_values.end(); ++it ) {
#ifdef SCENE_CHECK_TYPES
        it->m_value->undefine();
#endif
        delete it->m_value;
    }
    m_pass4_values.clear();
    m_matrix_composition_lut.clear();
    m_transform_origin_lut.clear();
    m_premultiply_z_lut.clear();
    m_matrix_prod_3x3_transpose_lut.clear();
    m_bbox_check_lut.clear();
    //m_matrix_composition_cache.clear();
    //m_matrix_z_axis_cache.clear();
    //m_matrix_origin_cache.clear();
    //m_matrix_prod_3x3_transpose_cache.clear();
    //m_bbox_check_cache.clear();

    m_last_purge.touch();
    m_has_dumped = false;
}



// -----------------------------------------------------------------------------
// ---                                                                       ---
// --- Functions that fetch or create new cache entries                      ---
// ---                                                                       ---
// -----------------------------------------------------------------------------

const Value*
TransformCache::cosineOfRadianAngle( const Value* source )
{
    CacheLUT<2>::Key key( reinterpret_cast<const void*>( PASS1_DEDUCE_COSINE_OF_RADIAN_ANGLE ), source );
    size_t i = m_deduced_values_lut.find( key );
    if( i != CacheLUT<2>::none() ) {
        return m_pass1_values[ i ].m_value;
    }
    else {
        CacheItem<1> item;
        item.m_value = new Value( Value::createFloat(0.f) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS1_DEDUCE_COSINE_OF_RADIAN_ANGLE;
        item.m_source_values[0] = source;
        m_deduced_values_lut.insert( key, m_pass1_values.size() );
        m_pass1_values.push_back( item );
        return item.m_value;
    }
/*
    auto it = m_deduced_values_cache.find( key );
    if( it != m_deduced_values_cache.end() ) {
        return m_pass1_values[ it->second ].m_value;
    }
    else {
        CacheItem<1> item;
        item.m_value = new Value( Value::createFloat(0.f) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS1_DEDUCE_COSINE_OF_RADIAN_ANGLE;
        item.m_source_values[0] = source;
        m_deduced_values_cache[ key ] = m_pass1_values.size();
        m_pass1_values.push_back( item );
        return item.m_value;
    }
*/
}

const Value*
TransformCache::reciprocalOfVec2( const Value* source )
{
    CacheLUT<2>::Key key( reinterpret_cast<const void*>( PASS1_DEDUCE_RECIPROCAL_VEC2 ), source );
    size_t i = m_deduced_values_lut.find( key );
    if( i != CacheLUT<2>::none() ) {
        return m_pass1_values[ i ].m_value;
    }
    else {
        CacheItem<1> item;
        item.m_value = new Value( Value::createFloat2(0.f, 0.f) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS1_DEDUCE_RECIPROCAL_VEC2;
        item.m_source_values[0] = source;
        m_deduced_values_lut.insert( key, m_pass1_values.size() );
        m_pass1_values.push_back( item );
        return item.m_value;
    }
    /*
        auto it = m_deduced_values_cache.find( key );
    if( it != m_deduced_values_cache.end() ) {
        return m_pass1_values[ it->second ].m_value;
    }
    else {
        CacheItem<1> item;
        item.m_value = new Value( Value::createFloat2(0.f, 0.f) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS1_DEDUCE_RECIPROCAL_VEC2;
        item.m_source_values[0] = source;
        m_deduced_values_cache[ key ] = m_pass1_values.size();
        m_pass1_values.push_back( item );
        return item.m_value;
    }
*/
}


const Value*
TransformCache::cameraProjectionMatrix( const Camera* camera )
{
    CacheLUT<1>::Key key( camera );
    size_t i = m_camera_projection_lut.find( key );
    if( i != CacheLUT<1>::none() ) {
        return m_pass1_values[ i ].m_value;
    }
    else {
        CacheItem<1> item;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS1_COMPUTE_CAMERA_PROJECTION;
        item.m_source_camera = camera;
        m_camera_projection_lut.insert( key, m_pass1_values.size() );
        m_pass1_values.push_back( item );
        return item.m_value;
    }
/*
    CacheKey<1> key( camera );
    auto it = m_camera_projection_cache.find( key );
    if( it != m_camera_projection_cache.end() ) {
        return m_pass1_values[ it->second ].m_value;
    }
    else {
        CacheItem<1> item;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS1_COMPUTE_CAMERA_PROJECTION;
        item.m_source_camera = camera;
        m_camera_projection_cache[ key ] = m_pass1_values.size();
        m_pass1_values.push_back( item );
        return item.m_value;
    }
*/
}

const Value*
TransformCache::cameraProjectionMatrixInverse( const Camera* camera )
{
    CacheLUT<1>::Key key( camera );
    size_t i = m_camera_inverse_projection_lut.find( key );
    if( i != CacheLUT<1>::none() ) {
        return m_pass1_values[ i ].m_value;
    }
    else {
        CacheItem<1> item;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS1_COMPUTE_CAMERA_PROJECTION_INVERSE;
        item.m_source_camera = camera;
        m_camera_inverse_projection_lut.insert( key, m_pass1_values.size() );
        m_pass1_values.push_back( item );
        return item.m_value;
    }
/*
    CacheKey<1> key( camera );
    auto it = m_camera_projection_inverse_cache.find( key );
    if( it != m_camera_projection_inverse_cache.end() ) {
        return m_pass1_values[ it->second ].m_value;
    }
    else {
        CacheItem<1> item;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS1_COMPUTE_CAMERA_PROJECTION_INVERSE;
        item.m_source_camera = camera;
        m_camera_projection_inverse_cache[ key ] = m_pass1_values.size();
        m_pass1_values.push_back( item );
        return item.m_value;
    }
*/
}



const Value*
TransformCache::nodeTransformMatrix( const Node* node )
{
#ifdef DEBUG
    // Check that node is valid
    Logger log = getLogger( package + ".nodeTransformMatrix" );
    for( size_t i=0; i<node->transforms(); i++ ) {
        const TransformType type = node->transformType(i);
        switch( type ) {
        case TRANSFORM_LOOKAT:
        case TRANSFORM_MATRIX:
        case TRANSFORM_TRANSLATE:
        case TRANSFORM_SCALE:
        case TRANSFORM_ROTATE:
            break;
        default:
            SCENELOG_FATAL( log, "Node " << node->debugString() << " transform no " << i << " has illegal type " << type );
            // Generate segfault
            if( 0 ) {
                unsigned int *foo = (unsigned int*)0;
                *foo = 42;
            }
            break;
        }
    }
#endif
    CacheLUT<1>::Key key( node );
    size_t i = m_node_transform_lut.find( key );
    if( i != CacheLUT<1>::none() ) {
        return m_pass1_values[ i ].m_value;
    }
    else {
        CacheItem<1> item;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS1_COMPUTE_NODE_TRANSFORM;
        item.m_source_node = node;
        m_node_transform_lut.insert( key, m_pass1_values.size() );
        m_pass1_values.push_back( item );
        return item.m_value;
    }
/*
    CacheKey<1> key( node );
    auto it = m_node_transform_cache.find( key );
    if( it != m_node_transform_cache.end() ) {
        return m_pass1_values[ it->second ].m_value;
    }
    else {
        CacheItem<1> item;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS1_COMPUTE_NODE_TRANSFORM;
        item.m_source_node = node;
        m_node_transform_cache[ key ] = m_pass1_values.size();
        m_pass1_values.push_back( item );
        return item.m_value;
    }
*/
}


const Value*
TransformCache::nodeTransformInverseMatrix( const Node* node )
{
    CacheLUT<1>::Key key( node );
    size_t i = m_node_inverse_transform_lut.find( key );
    if( i != CacheLUT<1>::none() ) {
        return m_pass1_values[ i ].m_value;
    }
    else {
        CacheItem<1> item;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS1_COMPUTE_NODE_TRANSFORM_INVERSE;
        item.m_source_node = node;
        m_node_inverse_transform_lut.insert( key, m_pass1_values.size() );
        m_pass1_values.push_back( item );
        return item.m_value;
    }

    /*CacheKey<1> key( node );
    auto it = m_node_transform_inverse_cache.find( key );
    if( it != m_node_transform_inverse_cache.end() ) {
        return m_pass1_values[ it->second ].m_value;
    }
    else {
        CacheItem<1> item;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS1_COMPUTE_NODE_TRANSFORM_INVERSE;
        item.m_source_node = node;
        m_node_transform_inverse_cache[ key ] =  m_pass1_values.size();
        m_pass1_values.push_back( item );
        return item.m_value;
    }
*/
}

const Value*
TransformCache::branchTransformMatrix( const Node* root, const Node* leaf )
{
    CacheLUT<2>::Key key( root, leaf );
    size_t i = m_branch_transform_lut.find( key );
    if( i != CacheLUT<1>::none() ) {
        return m_branch_transform[ i ].m_value;
    }
    else {

        CacheItem<SCENE_PATH_MAX> item;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();

        // Branches are inclusive, and the root should never be NULL. We can
        // let the root be one step lower to simplify the loop
        root = root->parent();
        size_t N=0;
        const Value* path[ SCENE_PATH_MAX ];
        while( leaf != root ) {
            if( SCENE_PATH_MAX <= N ) {
                Logger log = getLogger( package + ".branchTransformMatrix" );
                SCENELOG_FATAL( log, "Branch length is " << N << ", and is larger than SCENE_PATH_MAX" );
                break;
            }
            path[ N++ ] = nodeTransformMatrix( leaf );
            leaf = leaf->parent();
        }
        // The sequence is retrieved backwards, so we reverse it so that
        // the update func can do right-multiply.
        for( size_t i=0; i<N; i++ ) {
            item.m_source_values[i] = path[ N-i-1 ];
        }
        item.m_N = N;
        m_branch_transform_lut.insert( key,  m_branch_transform.size() );
        m_branch_transform.push_back( item );
        return item.m_value;
    }


    /*CacheKey<2> key( root, leaf );
    auto it = m_branch_transform_cache.find( key );
    if( it != m_branch_transform_cache.end() ) {
        return m_branch_transform[ it->second ].m_value;
    }
    else {

        CacheItem<SCENE_PATH_MAX> item;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();

        // Branches are inclusive, and the root should never be NULL. We can
        // let the root be one step lower to simplify the loop
        root = root->parent();
        size_t N=0;
        const Value* path[ SCENE_PATH_MAX ];
        while( leaf != root ) {
            if( SCENE_PATH_MAX <= N ) {
                Logger log = getLogger( package + ".branchTransformMatrix" );
                SCENELOG_FATAL( log, "Branch length is " << N << ", and is larger than SCENE_PATH_MAX" );
                break;
            }
            path[ N++ ] = nodeTransformMatrix( leaf );
            leaf = leaf->parent();
        }
        // The sequence is retrieved backwards, so we reverse it so that
        // the update func can do right-multiply.
        for( size_t i=0; i<N; i++ ) {
            item.m_source_values[i] = path[ N-i-1 ];
        }
        item.m_N = N;



        m_branch_transform_cache[ key ] = m_branch_transform.size();
        m_branch_transform.push_back( item );
        return item.m_value;
    }*/
}


const Value*
TransformCache::branchTransformInverseMatrix( const Node* root, const Node* leaf )
{
    CacheLUT<2>::Key key( root, leaf );
    size_t i = m_branch_inverse_transform_lut.find( key );
    if( i != CacheLUT<1>::none() ) {
        return m_branch_transform[ i ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();

        // Branches are inclusive, and the root should never be NULL. We can
        // let the root be one step lower to simplify the loop
        root = root->parent();

        // The sequence is retrieved backwards, and since we want the inverse,
        // we don't need to reverse it

        size_t i=0;
        while( leaf != root ) {
            if( SCENE_PATH_MAX <= i ) {
                Logger log = getLogger( package + ".branchTransformInverseMatrix" );
                SCENELOG_FATAL( log, "Branch length is " << i << ", and is larger than SCENE_PATH_MAX" );
                break;
            }
            item.m_source_values[i++] = nodeTransformInverseMatrix( leaf );
            leaf = leaf->parent();
        }
        item.m_N = i;
        m_branch_inverse_transform_lut.insert( key, m_branch_transform.size() );
        m_branch_transform.push_back( item );
        return item.m_value;
    }
        /*
    CacheKey<2> key( root, leaf );
    auto it = m_branch_transform_inverse_cache.find( key );
    if( it != m_branch_transform_inverse_cache.end() ) {
        return m_branch_transform[ it->second ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();

        // Branches are inclusive, and the root should never be NULL. We can
        // let the root be one step lower to simplify the loop
        root = root->parent();

        // The sequence is retrieved backwards, and since we want the inverse,
        // we don't need to reverse it

        size_t i=0;
        while( leaf != root ) {
            if( SCENE_PATH_MAX <= i ) {
                Logger log = getLogger( package + ".branchTransformInverseMatrix" );
                SCENELOG_FATAL( log, "Branch length is " << i << ", and is larger than SCENE_PATH_MAX" );
                break;
            }
            item.m_source_values[i++] = nodeTransformInverseMatrix( leaf );
            leaf = leaf->parent();
        }
        item.m_N = i;

        m_branch_transform_inverse_cache[ key ] = m_branch_transform.size();
        m_branch_transform.push_back( item );
        return item.m_value;
    }
*/
}



const Value*
TransformCache::pathTransformMatrix( const Node* const (&path)[SCENE_PATH_MAX] )
{

    CacheLUT<SCENE_PATH_MAX>::Key key;
    for( size_t i=0; i<SCENE_PATH_MAX; i++ ) {
        key[i] = path[i];
    }
    size_t i = m_path_transform_lut.find( key );
    if( i != CacheLUT<SCENE_PATH_MAX>::none() ) {
        return m_path_transform[ i ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        size_t i=0;

        for( i=0; i<(SCENE_PATH_MAX/2) && path[2*i] != NULL; i++ ) {
            item.m_source_values[i] = branchTransformMatrix( path[2*i+0],
                                                             path[2*i+1] );
        }
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        item.m_N = i;
        m_path_transform_lut.insert( key, m_path_transform.size() );
        m_path_transform.push_back( item );
        return item.m_value;
    }
/*
        CacheKey<SCENE_PATH_MAX> key;
    for( size_t i=0; i<SCENE_PATH_MAX; i++ ) {
        key[i] = path[i];
    }
    auto it = m_path_transform_cache.find( key );
    if( it != m_path_transform_cache.end() ) {
        return m_path_transform[ it->second ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        size_t i=0;

        for( i=0; i<(SCENE_PATH_MAX/2) && path[2*i] != NULL; i++ ) {
            item.m_source_values[i] = branchTransformMatrix( path[2*i+0],
                                                             path[2*i+1] );
        }
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();


        item.m_N = i;



        m_path_transform_cache[ key ] = m_path_transform.size();
        m_path_transform.push_back( item );
        return item.m_value;
    }
*/
}


const Value*
TransformCache::pathTransformInverseMatrix( const Node* const (&path)[SCENE_PATH_MAX] )
{
    CacheLUT<SCENE_PATH_MAX>::Key key;
    for( size_t i=0; i<SCENE_PATH_MAX; i++ ) {
        key[i] = path[i];
    }
    size_t i = m_path_inverse_transform_lut.find( key );
    if( i != CacheLUT<SCENE_PATH_MAX>::none() ) {
        return m_path_transform[ i ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        size_t N=0;

        // Flip order since we want the inverse
        for(N=0; N<(SCENE_PATH_MAX/2) && path[2*N] != NULL; N++ );
        for(size_t i=0; i<N; i++ ) {
            item.m_source_values[N-i-1] = branchTransformInverseMatrix( path[2*i+0],
                                                                        path[2*i+1] );

        }
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        item.m_N = N;
        m_path_inverse_transform_lut.insert( key,  m_path_transform.size() );
        m_path_transform.push_back( item );
        return item.m_value;
    }

        /*
    CacheKey<SCENE_PATH_MAX> key;
    for( size_t i=0; i<SCENE_PATH_MAX; i++ ) {
        key[i] = path[i];
    }
    auto it = m_path_transform_inverse_cache.find( key );
    if( it != m_path_transform_inverse_cache.end() ) {
        return m_path_transform[ it->second ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        size_t N=0;

        // Flip order since we want the inverse
        for(N=0; N<(SCENE_PATH_MAX/2) && path[2*N] != NULL; N++ );
        for(size_t i=0; i<N; i++ ) {
            item.m_source_values[N-i-1] = branchTransformInverseMatrix( path[2*i+0],
                                                                        path[2*i+1] );

        }
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();


        item.m_N = N;



        m_path_transform_inverse_cache[ key ] = m_path_transform.size();
        m_path_transform.push_back( item );
        return item.m_value;
    }*/
}



const Value*
TransformCache::matrixComposition( const Value* M0, const Value* M1, const Value* M2, const Value* M3 )
{
    CacheLUT<4>::Key key( M0, M1, M2, M3 );
    size_t i = m_matrix_composition_lut.find( key );
    if( i != CacheLUT<4>::none() ) {
        return m_pass4_values[ i ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        item.m_action = MULTIPLY_MATRICES;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();

        unsigned int N = 0;
        item.m_source_values[N] = M0;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_source_values[N] = M1;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_source_values[N] = M2;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_source_values[N] = M3;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_N = N;
        m_matrix_composition_lut.insert( key, m_pass4_values.size() );
        m_pass4_values.push_back( item );
        return item.m_value;
    }

/*
    CacheKey<4> key( M0, M1, M2, M3 );
    auto it = m_matrix_composition_cache.find( key );
    if( it != m_matrix_composition_cache.end() ) {
        return m_pass4_values[ it->second ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        item.m_action = MULTIPLY_MATRICES;
        item.m_value = new Value( Value::createFloat4x4( 1.f, 0.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f, 0.f,
                                                         0.f, 0.f, 1.f, 0.f,
                                                         0.f, 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();

        unsigned int N = 0;
        item.m_source_values[N] = M0;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_source_values[N] = M1;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_source_values[N] = M2;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_source_values[N] = M3;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_N = N;
        m_matrix_composition_cache[ key ] = m_pass4_values.size();
        m_pass4_values.push_back( item );
        return item.m_value;
    }
    return NULL;
*/
}

const Value*
TransformCache::matrixProductUpper3x3Transpose( const Value* A, const Value* B )
{
    CacheLUT<2>::Key key( A, B );
    size_t i = m_matrix_prod_3x3_transpose_lut.find( key );
    if( i != CacheLUT<SCENE_PATH_MAX>::none() ) {
        return m_pass4_values[ i ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        item.m_action = PASS5_PRODUCT_UPPER3X3_TRANSPOSE;
        item.m_value = new Value( Value::createFloat3x3( 1.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f,
                                                         0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        unsigned int N=0;
        item.m_source_values[N] = A;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_source_values[N] = B;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_N = N;
        m_matrix_prod_3x3_transpose_lut.insert( key, m_pass4_values.size() );
        m_pass4_values.push_back( item );
        return item.m_value;
    }

    /*CacheKey<2> key( A, B );
    auto it = m_matrix_prod_3x3_transpose_cache.find( key );
    if( it != m_matrix_prod_3x3_transpose_cache.end() ) {
        return m_pass4_values[ it->second ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        item.m_action = PASS5_PRODUCT_UPPER3X3_TRANSPOSE;
        item.m_value = new Value( Value::createFloat3x3( 1.f, 0.f, 0.f,
                                                         0.f, 1.f, 0.f,
                                                         0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();

        unsigned int N=0;
        item.m_source_values[N] = A;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_source_values[N] = B;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_N = N;
        m_matrix_prod_3x3_transpose_cache[key ] = m_pass4_values.size();
        m_pass4_values.push_back( item );
        return item.m_value;
    }
*/
}


const Value*
TransformCache::matrixSubsetPostmultiplyOrigin( const Value* A, const Value* B )
{
    CacheLUT<2>::Key key( A, B );
    size_t i = m_transform_origin_lut.find( key );
    if( i != CacheLUT<2>::none() ) {
        return m_pass4_values[ i ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        item.m_action = PASS5_SUBSET_POSTMULTIPLY_ORIGIN;
        item.m_value = new Value( Value::createFloat3( 0.f, 0.f, 0.f ) );
        item.m_value->valueChanged().invalidate();
        unsigned int N=0;
        item.m_source_values[N] = A;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_source_values[N] = B;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_N = N;
        m_transform_origin_lut.insert( key, m_pass4_values.size() );
        m_pass4_values.push_back( item );
        return item.m_value;
    }

    /*CacheKey<2> key( A, B );
    auto it = m_matrix_origin_cache.find( key );
    if( it != m_matrix_origin_cache.end() ) {
        return m_pass4_values[ it->second ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        item.m_action = PASS5_SUBSET_POSTMULTIPLY_ORIGIN;
        item.m_value = new Value( Value::createFloat3( 0.f, 0.f, 0.f ) );
        item.m_value->valueChanged().invalidate();
        unsigned int N=0;
        item.m_source_values[N] = A;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_source_values[N] = B;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_N = N;
        m_matrix_origin_cache[ key ] = m_pass4_values.size();
        m_pass4_values.push_back( item );
        return item.m_value;
    }
*/
}


const Value*
TransformCache::matrixSubsetPremultiplyZ( const Value* A, const Value* B  )
{
    CacheLUT<2>::Key key( A, B );
    size_t i = m_premultiply_z_lut.find( key );
    if( i != CacheLUT<2>::none() ) {
        return m_pass4_values[ i ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        item.m_action = PASS5_SUBSET_PREMULTIPLY_Z;
        item.m_value = new Value( Value::createFloat3( 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        unsigned int N=0;
        item.m_source_values[N] = A;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_source_values[N] = B;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_N = N;
        m_premultiply_z_lut.insert( key, m_pass4_values.size() );
        m_pass4_values.push_back( item );
        return item.m_value;
    }

    /*CacheKey<2> key( A, B );
    auto it = m_matrix_z_axis_cache.find( key );
    if( it != m_matrix_z_axis_cache.end() ) {
        return m_pass4_values[ it->second ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        item.m_action = PASS5_SUBSET_PREMULTIPLY_Z;
        item.m_value = new Value( Value::createFloat3( 0.f, 0.f, 1.f ) );
        item.m_value->valueChanged().invalidate();
        unsigned int N=0;
        item.m_source_values[N] = A;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_source_values[N] = B;
        if( item.m_source_values[N] != NULL ) { N++; }
        item.m_N = N;

        m_matrix_z_axis_cache[ key ] = m_pass4_values.size();
        m_pass4_values.push_back( item );
        return item.m_value;
    }*/
}




const Value*
TransformCache::checkBoundingBox( const SetViewCoordSys*   view_coords,
                                  const SetLocalCoordSys*  local_coords,
                                  const Geometry*          geometry )
{
    Logger log = getLogger( package + ".checkBoundingBox" );
    SCENELOG_ASSERT( log, view_coords != NULL );
    SCENELOG_ASSERT( log, local_coords != NULL );
    SCENELOG_ASSERT( log, geometry != NULL );

    CacheLUT<3>::Key key( view_coords, local_coords, geometry );
    size_t i = m_bbox_check_lut.find( key );
    if( i != CacheLUT<3>::none() ) {
        return m_pass4_values[ i ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        item.m_value = new Value( Value::createBool( GL_TRUE ) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS5_CHECK_BBOX_IN_FRUSTUM;
        if( geometry->boundingBox( item.m_source_values[0],
                                   item.m_source_values[1] ) )
        {
            unsigned int k=2;
            item.m_source_values[k] = cameraProjectionMatrix( view_coords->m_camera );
            if( item.m_source_values[k] != NULL ) { k++; }
            item.m_source_values[k] = pathTransformInverseMatrix( view_coords->m_camera_path );
            if( item.m_source_values[k] != NULL ) { k++; }
            item.m_source_values[k] = pathTransformMatrix( local_coords->m_node_path );
            if( item.m_source_values[k] != NULL ) { k++; }
            item.m_N = k;
        }
        else {
            item.m_N = 0;
        }
        m_bbox_check_lut.insert( key, m_pass4_values.size() );
        m_pass4_values.push_back( item );
        return item.m_value;
    }

    /*CacheKey<3> key( view_coords, local_coords, geometry );
    auto it = m_bbox_check_cache.find( key );
    if( it != m_bbox_check_cache.end() ) {
        return m_pass4_values[ it->second ].m_value;
    }
    else {
        CacheItem<SCENE_PATH_MAX> item;
        item.m_value = new Value( Value::createBool( GL_TRUE ) );
        item.m_value->valueChanged().invalidate();
        item.m_action = PASS5_CHECK_BBOX_IN_FRUSTUM;
        geometry->boundingBox( item.m_source_values[0],
                               item.m_source_values[1] );
        unsigned int k=2;
        item.m_source_values[k] = cameraProjectionMatrix( view_coords->m_camera );
        if( item.m_source_values[k] != NULL ) { k++; }
        item.m_source_values[k] = pathTransformInverseMatrix( view_coords->m_camera_path );
        if( item.m_source_values[k] != NULL ) { k++; }
        item.m_source_values[k] = pathTransformMatrix( local_coords->m_node_path );
        if( item.m_source_values[k] != NULL ) { k++; }
        item.m_N = k;
        m_bbox_check_cache[ key ] = m_pass4_values.size();
        m_pass4_values.push_back( item );
        return item.m_value;
    }
    */
}



const Value*
TransformCache::runtimeSemantic( RuntimeSemantic          semantic,
                                 const SetRenderTargets*  render_targets,
                                 const SetViewCoordSys*   view_coords,
                                 const SetLocalCoordSys*  local_coords )
{
    Logger log = getLogger( package + ".runtimeSemantic" );

    const Value* t;

    size_t light;
    //std::cerr << "sem:" << Scene::runtimeSemantic( semantic ) << ": " << semantic << "\n";
    switch( semantic )
    {
    case RUNTIME_FRAMEBUFFER_SIZE:
        if( render_targets != NULL ) {
            SCENELOG_WARN( log, "Framebuffer size of offscreen render targets not yet handled." );
        }
        return &m_default_fbo_size;
        break;
    case RUNTIME_FRAMEBUFFER_SIZE_RECIPROCAL:
        if( render_targets != NULL ) {
            SCENELOG_WARN( log, "Framebuffer size of offscreen render targets not yet handled." );
        }
        return reciprocalOfVec2( &m_default_fbo_size );
        break;

    case RUNTIME_MODELVIEW_MATRIX:
        return matrixComposition( pathTransformInverseMatrix( view_coords->m_camera_path ),
                                  pathTransformMatrix( local_coords->m_node_path ) );
        break;
    case RUNTIME_PROJECTION_MATRIX:
        return cameraProjectionMatrix( view_coords->m_camera );
        break;
    case RUNTIME_PROJECTION_INVERSE_MATRIX:
        return cameraProjectionMatrixInverse( view_coords->m_camera );
        break;
    case RUNTIME_MODELVIEW_PROJECTION_MATRIX:
        return matrixComposition( cameraProjectionMatrix( view_coords->m_camera ),
                                  pathTransformInverseMatrix( view_coords->m_camera_path ),
                                  pathTransformMatrix( local_coords->m_node_path ) );
        break;
    case RUNTIME_NORMAL_MATRIX:
        return matrixProductUpper3x3Transpose( pathTransformInverseMatrix( local_coords->m_node_path ),
                                               pathTransformMatrix( view_coords->m_camera_path ) );
        break;
    case RUNTIME_WORLD_FROM_OBJECT:
        return pathTransformMatrix( local_coords->m_node_path );
        break;
    case RUNTIME_OBJECT_FROM_WORLD:
        return pathTransformInverseMatrix( local_coords->m_node_path );
        break;

    case RUNTIME_EYE_FROM_WORLD:
        return pathTransformInverseMatrix( view_coords->m_camera_path );
        break;
    case RUNTIME_WORLD_FROM_EYE:
        return pathTransformMatrix( view_coords->m_camera_path );
        break;
    case RUNTIME_WORLD_FROM_CLIP:
        return matrixComposition( pathTransformMatrix( view_coords->m_camera_path),
                                  cameraProjectionMatrixInverse( view_coords->m_camera ) );
        break;
    case RUNTIME_LIGHT0_COLOR:
    case RUNTIME_LIGHT1_COLOR:
    case RUNTIME_LIGHT2_COLOR:
    case RUNTIME_LIGHT3_COLOR:
        light = semantic - RUNTIME_LIGHT0_COLOR;
        if( view_coords->m_lights[ light ] == NULL ) {
            SCENELOG_ERROR( log, "RUNTIME_LIGHTx_COLOR: light " << light << " undefined." );
        }
        else {
            return view_coords->m_lights[ light ]->color();
        }
        break;
    case RUNTIME_LIGHT0_CONSTANT_ATT:
    case RUNTIME_LIGHT1_CONSTANT_ATT:
    case RUNTIME_LIGHT2_CONSTANT_ATT:
    case RUNTIME_LIGHT3_CONSTANT_ATT:
        light = semantic - RUNTIME_LIGHT0_CONSTANT_ATT;
        if( view_coords->m_lights[ light ] == NULL ) {
            SCENELOG_ERROR( log, "RUNTIME_LIGHTx_CONSTANT_ATT: light " << light << " undefined." );
        }
        else {
            return view_coords->m_lights[ light ]->constantAttenuation();
        }
        break;
    case RUNTIME_LIGHT0_LINEAR_ATT:
    case RUNTIME_LIGHT1_LINEAR_ATT:
    case RUNTIME_LIGHT2_LINEAR_ATT:
    case RUNTIME_LIGHT3_LINEAR_ATT:
        light = semantic - RUNTIME_LIGHT0_LINEAR_ATT;
        if( view_coords->m_lights[ light ] == NULL ) {
            SCENELOG_ERROR( log, "RUNTIME_LIGHT0_LINEAR_ATT: light " << light << " undefined." );
        }
        else {
            return view_coords->m_lights[ light ]->linearAttenuation();
        }
        break;
    case RUNTIME_LIGHT0_QUADRATIC_ATT:
    case RUNTIME_LIGHT1_QUADRATIC_ATT:
    case RUNTIME_LIGHT2_QUADRATIC_ATT:
    case RUNTIME_LIGHT3_QUADRATIC_ATT:
        light = semantic - RUNTIME_LIGHT0_QUADRATIC_ATT;
        if( view_coords->m_lights[ light ] == NULL ) {
            SCENELOG_ERROR( log, "RUNTIME_LIGHTx_QUADRATIC_ATT: light " << light << " undefined." );
        }
        else {
            return view_coords->m_lights[ light ]->quadraticAttenuation();
        }
        break;
    case RUNTIME_LIGHT0_FALLOFF_COS:
    case RUNTIME_LIGHT1_FALLOFF_COS:
    case RUNTIME_LIGHT2_FALLOFF_COS:
    case RUNTIME_LIGHT3_FALLOFF_COS:
        light = semantic - RUNTIME_LIGHT0_FALLOFF_COS;
        if( view_coords->m_lights[ light ] == NULL ) {
            SCENELOG_ERROR( log, "RUNTIME_LIGHTx_FALLOFF_COS: light " << light << " undefined." );
        }
        else {
            return cosineOfRadianAngle( view_coords->m_lights[ light ]->falloffAngle() );
        }
        break;
    case RUNTIME_LIGHT0_FALLOFF_EXPONENT:
    case RUNTIME_LIGHT1_FALLOFF_EXPONENT:
    case RUNTIME_LIGHT2_FALLOFF_EXPONENT:
    case RUNTIME_LIGHT3_FALLOFF_EXPONENT:
        light = semantic - RUNTIME_LIGHT0_FALLOFF_EXPONENT;
        if( view_coords->m_lights[ light ] == NULL ) {
            SCENELOG_ERROR( log, "RUNTIME_LIGHTx_FALLOFF_EXPONENT: light " << light << " undefined." );
        }
        else {
            return view_coords->m_lights[ light ]->falloffExponent();
        }
        break;
    case RUNTIME_LIGHT0_POS_OBJECT:
    case RUNTIME_LIGHT1_POS_OBJECT:
    case RUNTIME_LIGHT2_POS_OBJECT:
    case RUNTIME_LIGHT3_POS_OBJECT:
        light = semantic - RUNTIME_LIGHT0_POS_OBJECT;
        return matrixSubsetPostmultiplyOrigin( pathTransformInverseMatrix( local_coords->m_node_path ),
                                               pathTransformMatrix( view_coords->m_light_paths[ light ] ) );
        break;
    case RUNTIME_LIGHT0_POS_WORLD:
    case RUNTIME_LIGHT1_POS_WORLD:
    case RUNTIME_LIGHT2_POS_WORLD:
    case RUNTIME_LIGHT3_POS_WORLD:
        light = semantic - RUNTIME_LIGHT0_POS_WORLD;
        return matrixSubsetPostmultiplyOrigin( pathTransformMatrix( view_coords->m_light_paths[ light ] ),
                                               NULL );
        break;
    case RUNTIME_LIGHT0_POS_EYE:
    case RUNTIME_LIGHT1_POS_EYE:
    case RUNTIME_LIGHT2_POS_EYE:
    case RUNTIME_LIGHT3_POS_EYE:
        light = semantic - RUNTIME_LIGHT0_POS_EYE;
        return matrixSubsetPostmultiplyOrigin( pathTransformInverseMatrix( view_coords->m_camera_path ),
                                               pathTransformMatrix( view_coords->m_light_paths[ light ] ) );
        break;
    case RUNTIME_LIGHT0_Z_OBJECT:
    case RUNTIME_LIGHT1_Z_OBJECT:
    case RUNTIME_LIGHT2_Z_OBJECT:
    case RUNTIME_LIGHT3_Z_OBJECT:
        light = semantic - RUNTIME_LIGHT0_Z_OBJECT;
        return matrixSubsetPremultiplyZ( pathTransformInverseMatrix( view_coords->m_light_paths[ light ] ),
                                         pathTransformMatrix( local_coords->m_node_path ) );
        break;
    case RUNTIME_LIGHT0_Z_WORLD:
    case RUNTIME_LIGHT1_Z_WORLD:
    case RUNTIME_LIGHT2_Z_WORLD:
    case RUNTIME_LIGHT3_Z_WORLD:
        light = semantic - RUNTIME_LIGHT0_Z_WORLD;
        return matrixSubsetPremultiplyZ( pathTransformInverseMatrix( view_coords->m_light_paths[ light ] ),
                                         NULL );
        break;
    case RUNTIME_LIGHT0_Z_EYE:
    case RUNTIME_LIGHT1_Z_EYE:
    case RUNTIME_LIGHT2_Z_EYE:
    case RUNTIME_LIGHT3_Z_EYE:
        light = semantic - RUNTIME_LIGHT0_Z_EYE;
        return matrixSubsetPremultiplyZ( pathTransformInverseMatrix( view_coords->m_light_paths[ light ] ),
                                         pathTransformMatrix( view_coords->m_camera_path ) );
        break;
    case RUNTIME_LIGHT0_EYE_FROM_OBJECT:
    case RUNTIME_LIGHT1_EYE_FROM_OBJECT:
    case RUNTIME_LIGHT2_EYE_FROM_OBJECT:
    case RUNTIME_LIGHT3_EYE_FROM_OBJECT:
        light = semantic - RUNTIME_LIGHT0_EYE_FROM_OBJECT;
        return matrixComposition( pathTransformInverseMatrix( view_coords->m_light_paths[ light ] ),
                                  pathTransformMatrix( local_coords->m_node_path ) );
        break;
    case RUNTIME_LIGHT0_EYE_FROM_WORLD:
    case RUNTIME_LIGHT1_EYE_FROM_WORLD:
    case RUNTIME_LIGHT2_EYE_FROM_WORLD:
    case RUNTIME_LIGHT3_EYE_FROM_WORLD:
        light = semantic - RUNTIME_LIGHT0_EYE_FROM_WORLD;
        return pathTransformInverseMatrix( view_coords->m_light_paths[ light ] );
        break;
    case RUNTIME_WORLD_FROM_LIGHT0_EYE:
    case RUNTIME_WORLD_FROM_LIGHT1_EYE:
    case RUNTIME_WORLD_FROM_LIGHT2_EYE:
    case RUNTIME_WORLD_FROM_LIGHT3_EYE:
        light = semantic - RUNTIME_WORLD_FROM_LIGHT0_EYE;
        return pathTransformMatrix( view_coords->m_light_paths[ light ] );
        break;
    case RUNTIME_LIGHT0_EYE_FROM_EYE:
    case RUNTIME_LIGHT1_EYE_FROM_EYE:
    case RUNTIME_LIGHT2_EYE_FROM_EYE:
    case RUNTIME_LIGHT3_EYE_FROM_EYE:
        light = semantic - RUNTIME_LIGHT0_EYE_FROM_EYE;
        return matrixComposition( pathTransformInverseMatrix( view_coords->m_light_paths[ light ] ),
                                  pathTransformMatrix( view_coords->m_camera_path ) );
        break;
    case RUNTIME_LIGHT0_CLIP_FROM_OBJECT:
    case RUNTIME_LIGHT1_CLIP_FROM_OBJECT:
    case RUNTIME_LIGHT2_CLIP_FROM_OBJECT:
    case RUNTIME_LIGHT3_CLIP_FROM_OBJECT:
        light = semantic - RUNTIME_LIGHT0_CLIP_FROM_OBJECT;
        return matrixComposition( cameraProjectionMatrix( view_coords->m_light_projections[ light ] ),
                                  pathTransformInverseMatrix( view_coords->m_light_paths[ light ] ),
                                  pathTransformMatrix( local_coords->m_node_path ) );
        break;
    case RUNTIME_LIGHT0_CLIP_FROM_WORLD:
    case RUNTIME_LIGHT1_CLIP_FROM_WORLD:
    case RUNTIME_LIGHT2_CLIP_FROM_WORLD:
    case RUNTIME_LIGHT3_CLIP_FROM_WORLD:
        light = semantic - RUNTIME_LIGHT0_CLIP_FROM_WORLD;
        return matrixComposition( cameraProjectionMatrix( view_coords->m_light_projections[ light ] ),
                                  pathTransformInverseMatrix( view_coords->m_light_paths[ light ] ) );
        break;
    case RUNTIME_LIGHT0_CLIP_FROM_EYE:
    case RUNTIME_LIGHT1_CLIP_FROM_EYE:
    case RUNTIME_LIGHT2_CLIP_FROM_EYE:
    case RUNTIME_LIGHT3_CLIP_FROM_EYE:
        light = semantic - RUNTIME_LIGHT0_CLIP_FROM_EYE;
         return matrixComposition( cameraProjectionMatrix( view_coords->m_light_projections[ light ] ),
                                  pathTransformInverseMatrix( view_coords->m_light_paths[ light ] ),
                                  pathTransformMatrix( view_coords->m_camera_path ) );

         break;
    case RUNTIME_LIGHT0_CLIP_FROM_CLIP:
    case RUNTIME_LIGHT1_CLIP_FROM_CLIP:
    case RUNTIME_LIGHT2_CLIP_FROM_CLIP:
    case RUNTIME_LIGHT3_CLIP_FROM_CLIP:
        light = semantic - RUNTIME_LIGHT0_CLIP_FROM_CLIP;
         return matrixComposition( cameraProjectionMatrix( view_coords->m_light_projections[ light ] ),
                                   pathTransformInverseMatrix( view_coords->m_light_paths[ light ] ),
                                   pathTransformMatrix( view_coords->m_camera_path ),
                                   cameraProjectionMatrixInverse( view_coords->m_camera ));
         break;
    case RUNTIME_LIGHT0_TEX_FROM_OBJECT:
    case RUNTIME_LIGHT1_TEX_FROM_OBJECT:
    case RUNTIME_LIGHT2_TEX_FROM_OBJECT:
    case RUNTIME_LIGHT3_TEX_FROM_OBJECT:
        light = semantic - RUNTIME_LIGHT0_TEX_FROM_OBJECT;
        return matrixComposition( &m_bias_matrix,
                                  cameraProjectionMatrix( view_coords->m_light_projections[ light ] ),
                                  pathTransformInverseMatrix( view_coords->m_light_paths[ light ] ),
                                  pathTransformMatrix( local_coords->m_node_path ) );
        break;
    case RUNTIME_LIGHT0_TEX_FROM_WORLD:
    case RUNTIME_LIGHT1_TEX_FROM_WORLD:
    case RUNTIME_LIGHT2_TEX_FROM_WORLD:
    case RUNTIME_LIGHT3_TEX_FROM_WORLD:
        light = semantic - RUNTIME_LIGHT0_TEX_FROM_WORLD;
        return matrixComposition( &m_bias_matrix,
                                  cameraProjectionMatrix( view_coords->m_light_projections[ light ] ),
                                  pathTransformInverseMatrix( view_coords->m_light_paths[ light ] ) );
        break;
    case RUNTIME_LIGHT0_TEX_FROM_EYE:
    case RUNTIME_LIGHT1_TEX_FROM_EYE:
    case RUNTIME_LIGHT2_TEX_FROM_EYE:
    case RUNTIME_LIGHT3_TEX_FROM_EYE:
        light = semantic - RUNTIME_LIGHT0_TEX_FROM_EYE;
        return matrixComposition( &m_bias_matrix,
                                  cameraProjectionMatrix( view_coords->m_light_projections[ light ] ),
                                  pathTransformInverseMatrix( view_coords->m_light_paths[ light ] ),
                                  pathTransformMatrix( view_coords->m_camera_path ) );

        break;
    case RUNTIME_SEMANTIC_N:
        break;
    }
    return NULL;
}




    }
}
