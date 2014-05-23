#include <iostream>
#include <scene/Camera.hpp>
#include <scene/tools/BBoxTool.hpp>
#include <scene/tools/ShaderGen.hpp>
#include <scene/collada/Importer.hpp>
#ifdef SCENE_TINIA
#include <tinia/renderlist/XMLWriter.hpp>
#endif
#include "Viewer.hpp"

//#define USE_MATRIX

ViewerApp::ViewerApp( const std::vector<std::string>& files,
                      bool stereo,
                      bool torture,
                      bool auto_shader,
                      bool auto_flatten,
                      float scale )
    : m_runtime( m_db ),
      m_renderlist( m_runtime ),
      m_viewer( glm::vec3(-scale), glm::vec3(scale) ),
      #ifdef SCENE_TINIA
      m_exporter_runtime( m_db, Scene::PROFILE_GLES2, "WebGL" ),
      #endif
      m_stereo( stereo ),
      m_torture( torture ),
      m_auto_shader( auto_shader ),
      m_auto_flatten( auto_flatten ),
      m_source_files( files ),
      m_onscreen_scene( 0 )
{
    setup();
}

void
ViewerApp::reload()
{
    m_renderlist.clear();
    m_runtime.clear();
    m_db.clear();
    setup();
}

void
ViewerApp::nextVisualScene()
{
    if( !m_onscreen_visual_scenes.empty() ) {
        m_onscreen_scene = (m_onscreen_scene+1) % m_onscreen_visual_scenes.size();
        m_renderlist.build( m_onscreen_visual_scenes[ m_onscreen_scene ] );
        updateBoundingBox();
    }   
}

#ifdef SCENE_TINIA
void
ViewerApp::dumpCurrentRenderlistXML()
{
    if( m_onscreen_visual_scenes.empty() ) {
        std::cout << "No visual scenes." << std::endl;
    }
    else {
        m_exporter_runtime.build( m_onscreen_visual_scenes[ m_onscreen_scene ] );
        std::cout << tinia::renderlist::getUpdateXML( &m_exporter_runtime.renderListDataBase(),
                                                      tinia::renderlist::ENCODING_PLAIN,
                                                      0 );
    }
}
#endif

ViewManipulator&
ViewerApp::viewer()
{
    return m_viewer;
}

void
ViewerApp::updateCameraInstances()
{
    CameraInstance ci;
    m_camera_instances.clear();
    for( size_t j=0; j<m_db.library<Scene::Node>().size(); j++ ) {
        ci.m_node = m_db.library<Scene::Node>().get( j );
        for( size_t i=0; i<ci.m_node->instanceCameras(); i++ ) {
            ci.m_camera = m_db.library<Scene::Camera>().get( ci.m_node->instanceCameraURL(i) );
            if( ci.m_camera != NULL ) {
                std::cerr << m_camera_instances.size() << ": " <<
                             " camera '" << ci.m_camera->id() << "'" <<
                             " instanced in node '" << ci.m_node->debugString() << "\n";
                m_camera_instances.push_back( ci );
            }
        }
    }
}

void
ViewerApp::moveToCameraInstance( size_t index )
{
    if( m_camera_instances.size() <= index ) {
        return; // illegal index
    }
    std::cerr << "Moving to camera " << index << "\n";
    
    Scene::VisualScene* scene = m_db.library<Scene::VisualScene>().get( m_onscreen_visual_scenes[ m_onscreen_scene ] );
    if( scene == NULL ) {
        std::cerr << "No such scene '" << m_onscreen_visual_scenes[ m_onscreen_scene ] << "'\n";
        return;
    }
    Scene::Node* root = m_db.library<Scene::Node>().get ( scene->nodesId() );
    if( root == NULL ) {
        std::cerr << "Scene '" << scene->id() << "' has no root node.\n";
        return;
    }
    
    // Find the path from the scene root to the camera node
    std::list<const Scene::Node*> path;
    if( !m_runtime.resolver().findNodePath( path, root, m_camera_instances[ index ].m_node ) ) {
        std::cerr << "Couldn't find node path\n";
        return;
    }
    
    // Use a transform cache to calculate the matrices
    Scene::Runtime::TransformCache tc( m_db );
    const Scene::Value* P = tc.cameraProjectionMatrix( m_camera_instances[ index ].m_camera );
    const Scene::Node* cam_path[ SCENE_PATH_MAX ];
    size_t i=0;
    for( auto it=path.begin(); it!=path.end(); ++it ) {
        cam_path[i++] = *it;
    }
    for( ; i<SCENE_PATH_MAX; i++ ) {
        cam_path[i] = NULL;
    }
    const Scene::Value* M = tc.pathTransformInverseMatrix( cam_path );
    tc.update( m_viewer.getWindowSize()[0],
            m_viewer.getWindowSize()[1] );
    
    // push projection and orientation to viewer
    glm::mat4 GM, GP;
    std::copy_n( P->floatData(), 16, glm::value_ptr( GP ) );
    std::copy_n( M->floatData(), 16, glm::value_ptr( GM ) );
    m_viewer.setCamera( GM, GP, false );
}


    void
    ViewerApp::setup( )
    {
        // Read files
        Scene::Collada::Importer builder( m_db );
        for(auto it=m_source_files.begin(); it!=m_source_files.end(); ++it) {
            std::cerr << "Parsing '" << (*it) << '\'' << std::endl;
            if( !builder.parse( *it ) ) {
                std::cerr << "Error parsing '" << (*it) << "'" << std::endl;
            }
        }

        if( m_auto_shader ) {
            Scene::Tools::generateShadersFromCommon( m_db,
                                                     Scene::PROFILE_GLSL | 
                                                     Scene::PROFILE_GLES2 );
            
/*
            for( size_t i=0; i<m_db.library<Scene::Effect>().size(); i++ ) {
                Scene::Effect* e = m_db.library<Scene::Effect>().get( i );
                std::cerr << "Generating GLSL and GLES2 profiles for '" << e->id() << '\'' << std::endl;
                e->generate( Scene::PROFILE_GLSL );
                e->generate( Scene::PROFILE_GLES2 );
            }
*/
        }
        if( m_auto_flatten ) {
            for( size_t i=0; i<m_db.library<Scene::Geometry>().size(); i++ ) {
                Scene::Geometry* g = m_db.library<Scene::Geometry>().get( i );
                if( g->hasSharedInputs() ) {
                    std::cerr << "Flattening '" << g->id() << '\'' << std::endl;
                    g->flatten();
                }
            }
        }
        Scene::Tools::updateBoundingBoxes( m_db );

        std::cerr << "Import done." << std::endl;

        updateCameraInstances();

        // Create application camera
        m_app_camera = m_db.library<Scene::Camera>().add( "app_camera" );
        if( m_app_camera == NULL ) {
            std::cerr << "Failed to create application camera.\n";
        }
        else {
            m_app_camera_node = m_db.library<Scene::Node>().add( "app_camera_node" );
            if( m_app_camera_node == NULL ) {
                std::cerr << "Failed to create application camera node.\n";
            }
            else {
                m_app_camera_node->addInstanceCamera( "", "app_camera" );
                m_app_camera_node->addInstanceCamera( "", "app_camera" );
#ifdef USE_MATRIX
                m_app_camera_node->transformAdd( "matrix" );
#else
                m_app_camera_node->transformAdd( "lookat" );
#endif
                if( m_stereo ) {
                    m_app_camera_node->transformAdd( "translate" );
                }
            }
        }


        m_slave_camera = m_db.library<Scene::Camera>().add( "slave_camera" );
        m_slave_camera_node = m_db.library<Scene::Node>().add( "slave_camera_node" );
        m_slave_camera_node->addInstanceCamera( "", "slave_camera" );
        m_slave_camera_node->transformAdd( "lookat" );

        // Browse through visual scenes. All visual scenes with a 'setup'-prefix
        // is rendered, and all visual scenes with a 'onscreen'-prefix is added
        // to the 'render mode' list.

        m_onscreen_visual_scenes.clear();
        for(size_t i=0; i<m_db.library<Scene::VisualScene>().size(); i++ ) {
            Scene::VisualScene* vs = m_db.library<Scene::VisualScene>().get( i );
            std::cerr << vs->id() << "\n";
            if( vs->evaluateScenes() == 0 ) {
                std::cerr << "Visual scene '" << vs->id() << "' doesn't have an evaluate block, adding a default.\n";

                Scene::Node* root = m_db.library<Scene::Node>().get( vs->nodesId() );
                root->addInstanceNode( "", "", "app_camera_node", "" );
                root->addInstanceLight( "my_light", "" );

                Scene::EvaluateScene* es = vs->addEvaluateScene();
                es->setEnabled( true );
                Scene::Render* ri = es->addRenderItem();
                ri->setCameraNodeId( "app_camera_node" );
            }


            std::string scene_id = m_db.library<Scene::VisualScene>().get(i)->id();
            if( scene_id.substr( 0, 5 ) == "setup" ) {
                m_renderlist.build( scene_id );
                m_renderlist.render( );
            }
            else /*if( scene_id.substr( 0, 8 ) == "onscreen" )*/ {
                m_onscreen_visual_scenes.push_back( scene_id );
            }
        }
        if( !m_onscreen_visual_scenes.empty() ) {
            m_onscreen_scene = m_onscreen_scene % m_onscreen_visual_scenes.size();
            m_renderlist.build( m_onscreen_visual_scenes[ m_onscreen_scene ] );
        }
        else {
            m_onscreen_scene = 0;
        }
        updateBoundingBox();
       
    }

    
    void
    ViewerApp::updateBoundingBox()
    {
        Scene::Value bb_min, bb_max;
        if( Scene::Tools::visualSceneExtents( bb_min, bb_max, m_renderlist.renderList() ) ) {
            m_viewer.updateViewVolume( glm::vec3( bb_min.floatData()[0],
                                                   bb_min.floatData()[1],
                                                   bb_min.floatData()[2] ),
                                        glm::vec3( bb_max.floatData()[0],
                                                   bb_max.floatData()[1],
                                                   bb_max.floatData()[2] ) );
            m_viewer.viewAll();
            std::cerr << "Bounding box: ["
                      << bb_min.floatData()[0] << ", "
                      << bb_min.floatData()[1] << ", "
                      << bb_min.floatData()[2] << "], ["
                      << bb_max.floatData()[0] << ", "
                      << bb_max.floatData()[1] << ", "
                      << bb_max.floatData()[2] << "]\n";
        }
        else {
            std::cerr << "Failed to determine bounding box.\n";
        }
        
    }
    

    void
    ViewerApp::reshape( int w, int h )
    {
        m_renderlist.setDefaultOutput( 0, 0, 0, w, h );
    }

    void
    ViewerApp::render()
    {
        // make sure that we have no errors on entering.
        while( glGetError() != GL_NO_ERROR ) {}
        
        if( m_onscreen_visual_scenes.empty() ) {
            return;
        }

        // Forward the projection setup from DSRV to the application-created
        // camera in scene
#ifdef USE_MATRIX
        Scene::Value P = Scene::Value::createFloat4x4( glm::value_ptr( m_viewer->getProjectionMatrix() ) );
        Scene::Value M = Scene::Value::createFloat4x4( glm::value_ptr( m_viewer->getModelviewInverseMatrix() ) );
        if( m_app_camera != NULL ) {
            m_app_camera->setCustomMatrix( P );
        }
        m_app_camera_node->transformSetMatrix( m_app_camera_node->transformIndexBySid( "matrix" ), M );
#else
        if( m_app_camera != NULL ) {
            glm::vec2 near_far = m_viewer.getNearFarPlanes();
            float fov_y = m_viewer.getFieldOfViewY()*(M_PI/180.f);
            float fov_x = 2.0 * atanf( tan(0.5*fov_y)*m_viewer.getAspectRatio() );
            m_app_camera->setPerspective( fov_x,
                                          fov_y,
                                          -near_far.x,
                                          -near_far.y );
        }
        // The positioning and orientation of the camera in scene is done using
        // a node with a lookat transform.
        glm::vec3 up = glm::vec3( 0.f, 1.f, 0.f ) * m_viewer.getOrientation();
        glm::vec3 eye = m_viewer.getCurrentViewPoint();
        glm::vec3 coi = m_viewer.getCenterOfInterest();
        m_app_camera_node->transformSetLookAt( m_app_camera_node->transformIndexBySid( "lookat" ),
                                              eye.x, eye.y, eye.z,
                                              coi.x, coi.y, coi.z,
                                              up.x,  up.y,  up.z );

#endif
        glClearColor( 0.2, 0.3, 0.4, 0.0 );

        m_renderlist.build( m_onscreen_visual_scenes[ m_onscreen_scene ] );
        if( m_stereo ) {
            m_app_camera_node->transformSetTranslate( m_app_camera_node->transformIndexBySid( "translate" ),
                                                      0.08f, 0.f, 0.f );
            glDrawBuffer( GL_BACK_LEFT );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            m_renderlist.render( );

            m_app_camera_node->transformSetTranslate( m_app_camera_node->transformIndexBySid( "translate" ),
                                                      -0.08f, 0.f, 0.f );
            glDrawBuffer( GL_BACK_RIGHT );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            m_renderlist.render( );
        }
        else {
            m_renderlist.render( );
        }

        GLenum error = glGetError();
        while( error != GL_NO_ERROR ) {
            std::cerr << "At end of display, got GL error " << std::hex << error << std::endl;
        }

    }
