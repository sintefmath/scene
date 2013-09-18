#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#pragma comment( lib, "WSock32" )
#else
#include <sys/time.h>
#endif

#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include <stdexcept>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <siut2/dsrv/FreeglutWindow.h>

#include <scene/DataBase.hpp>
#include <scene/collada/Importer.hpp>
#include <scene/Material.hpp>
#include <scene/Camera.hpp>
#include <scene/Node.hpp>
#include <scene/Geometry.hpp>

#include <scene/Log.hpp>
#include <scene/runtime/RenderList.hpp>
#include <scene/glsl/GLSLRuntime.hpp>
#include <scene/glsl/GLSLRenderList.hpp>
#include <scene/runtime/TransformCache.hpp>
#include <scene/tools/BBoxTool.hpp>
#include <scene/tools/ShaderGen.hpp>

#ifdef SCENE_TINIA
#include <scene/tinia/Bridge.hpp>
#include <tinia/renderlist/XMLWriter.hpp>
#endif



using std::string;
using std::vector;
using std::runtime_error;

#define STEREO
//#define USE_MATRIX

class ViewerApp : public siut2::dsrv::FreeglutWindow
{
public:
    ViewerApp( int* argc, char** argv )
        : FreeglutWindow(), //FreeglutWindow( argc, argv, true, "Scene::Viewer" ),
          m_runtime( m_db ),
          m_renderlist( m_runtime ),
#ifdef SCENE_TINIA
          m_exporter_runtime( m_db, Scene::PROFILE_GLES2, "WebGL" ),
#endif
          m_stereo( false ),
          m_torture( false ),
          m_auto_shader( true ),
          m_auto_flatten( true )
    {
        glutInitWindowSize( 960, 512 );
        float scale = 1.0;
        for(int i=1; i<*argc; i++) {
            string param( argv[i] );
            if( param == "--stereo" ) {
                m_stereo = true;
            }
            else if( param == "--torture" ) {
                m_torture = true;
            }
            else if( param == "--auto-shader" ) {
                m_auto_shader = true;
            }
            else if( param == "--no-auto-shader" ) {
                m_auto_shader = false;
            }
            else if( param == "--auto-flatten" ) {
                m_auto_flatten = true;
            }
            else if( param == "--no-auto-flatten" ) {
                m_auto_flatten = false;
            }
            else if( param.length() > 7 && (param.substr( param.length()-7 ) == ".config" ) ) {
                // skip
            }
            else if( param.length() > 4 && (param.substr( param.length()-4 ) == ".xml" ) ) {
                m_source_files.push_back( argv[i] );
            }
            else if(param.length() > 4 && (param.substr( param.length()-4 ) == ".dae"  ) ) {
                m_source_files.push_back( argv[i] );
            }
            else if(param.length() > 4 && (param.substr( param.length()-4 ) == ".DAE"  ) ) {
                m_source_files.push_back( argv[i] );
            }
            else {
                scale = static_cast<float>( atof( param.c_str() ) );
                if( scale == 0.0f ) {
                    scale = 1.0f;
                }
            }
        }

        if( m_stereo ) {
            setUpMixedModeContext( argc, argv, "ViewerApp", GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STEREO );
        }
        else {
            setUpMixedModeContext( argc, argv, "ViewerApp", GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
        }
        glm::vec3 bb_min( -scale, -scale, -scale );
        glm::vec3 bb_max( scale, scale, scale );

        init( bb_min, bb_max );

        /*if( m_stereo ) {
            setUpContext( "Stereo rendering",
                         CONTEXT_SPEC_DISPLAY_MODE, GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STEREO,
                         CONTEXT_SPEC_END );
        }
        else {
            setUpContext( "Non-stereo rendering", CONTEXT_SPEC_END );
        }

        init( BBox3f( Vec3f( -scale, -scale, -scale ),
                      Vec3f(  scale,  scale,  scale ) ) );

*/
        m_onscreen_scene = 0;
        setup();

        setFpsVisibility( true );
    }

    void
    keyboard( unsigned char key )
    {
        if( key == 'r' ) {
            m_renderlist.clear();
            m_runtime.clear();
            m_db.clear();
            setup();
        }
        else if( ( '0' <= key) && ( key <= '9' ) ) {
            moveToCameraInstance( key - '0' );
        }
        else if( key == '0' ) {
            moveToCameraInstance( 0 );
        }
        else if( key == '0' ) {
            moveToCameraInstance( 0 );
        }
        else if( key == '0' ) {
            moveToCameraInstance( 0 );
        }
        else if( key == ' ' ) {
            if( !m_onscreen_visual_scenes.empty() ) {
                m_onscreen_scene = (m_onscreen_scene+1) % m_onscreen_visual_scenes.size();

                m_renderlist.build( m_onscreen_visual_scenes[ m_onscreen_scene ] );


                updateBoundingBox();

             }
        }
#ifdef SCENE_TINIA
        else if( key == 'e' ) {
            if( m_onscreen_visual_scenes.empty() ) {
                std::cerr << "No visual scenes." << std::endl;
            }
            else {
                m_exporter_runtime.build( m_onscreen_visual_scenes[ m_onscreen_scene ] );
                std::cerr << tinia::renderlist::getUpdateXML( &m_exporter_runtime.renderListDataBase(),
                                                              tinia::renderlist::ENCODING_PLAIN,
                                                              0 );
            }
        }
#endif
    }

protected:
    Scene::DataBase                     m_db;
    Scene::Runtime::GLSLRuntime         m_runtime;
    Scene::Runtime::GLSLRenderList      m_renderlist;

#ifdef SCENE_TINIA
    Scene::Tinia::Bridge          m_exporter_runtime;
#endif
    Scene::Camera*                      m_app_camera;
    Scene::Node*                        m_app_camera_node;
    Scene::Camera*                      m_slave_camera;
    Scene::Node*                        m_slave_camera_node;

    bool                                m_stereo;
    bool                                m_torture;
    bool                                m_auto_shader;
    bool                                m_auto_flatten;
    std::vector<std::string>            m_source_files;
    std::vector<std::string>            m_onscreen_visual_scenes;
    size_t                              m_onscreen_scene;

    struct CameraInstance {
        Scene::Node*                    m_node;
        Scene::Camera*                  m_camera;
    };
    std::vector<CameraInstance>         m_camera_instances;

    /** Run through nodes and find instances of cameras. */
    void
    updateCameraInstances()
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

    /** Set the viewer camera to that specifiaction of a camera instance. */
    void
    moveToCameraInstance( size_t index )
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
        tc.update( m_viewer->getWindowSize()[0],
                   m_viewer->getWindowSize()[1] );

        // push projection and orientation to viewer
        glm::mat4 GM, GP;
        std::copy_n( P->floatData(), 16, glm::value_ptr( GP ) );
        std::copy_n( M->floatData(), 16, glm::value_ptr( GM ) );
        m_viewer->setCamera( GM, GP, false );
    }


    void
    setup( )
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
    updateBoundingBox()
    {
        Scene::Value bb_min, bb_max;
        if( Scene::Tools::visualSceneExtents( bb_min, bb_max, m_renderlist.renderList() ) ) {
            m_viewer->updateViewVolume( glm::vec3( bb_min.floatData()[0],
                                                   bb_min.floatData()[1],
                                                   bb_min.floatData()[2] ),
                                        glm::vec3( bb_max.floatData()[0],
                                                   bb_max.floatData()[1],
                                                   bb_max.floatData()[2] ) );
            m_viewer->viewAll();
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
    reshape( int w, int h )
    {
        m_renderlist.setDefaultOutput( 0, 0, 0, w, h );
    }

    void
    render()
    {
        CHECK_GL;

        if( m_onscreen_visual_scenes.empty() ) {
            return;
        }

#ifdef _WIN32
        LARGE_INTEGER freq;
        LARGE_INTEGER time;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&time);
        double t = time.QuadPart/static_cast<double>(freq.QuadPart);
#else
        timeval rolex;
        gettimeofday( &rolex, NULL );

        double t = rolex.tv_sec + 1e-6*rolex.tv_usec;
#endif
        if( m_torture ) {
            m_db.structureChanged().touch();
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
            glm::vec2 near_far = m_viewer->getNearFarPlanes();
            float fov_y = m_viewer->getFieldOfViewY()*(M_PI/180.f);
            float fov_x = 2.0 * atanf( tan(0.5*fov_y)*m_viewer->getAspectRatio() );
            m_app_camera->setPerspective( fov_x,
                                          fov_y,
                                          -near_far.x,
                                          -near_far.y );
        }
        // The positioning and orientation of the camera in scene is done using
        // a node with a lookat transform.
        glm::vec3 up = glm::vec3( 0.f, 1.f, 0.f ) * m_viewer->getOrientation();
        glm::vec3 eye = m_viewer->getCurrentViewPoint();
        glm::vec3 coi = m_viewer->getCenterOfInterest();
        m_app_camera_node->transformSetLookAt( m_app_camera_node->transformIndexBySid( "lookat" ),
                                              eye.x, eye.y, eye.z,
                                              coi.x, coi.y, coi.z,
                                              up.x,  up.y,  up.z );

#endif
        Scene::Material* m = m_db.library<Scene::Material>().get( "pntri_phong", true );
        if( m != NULL ) {
            m->setParam( "diffuse",
                        Scene::Value::createFloat3( static_cast<float>( 0.5+0.5*sin(t) ),
                                                   static_cast<float> ( 0.5+0.5*cos(1.3*t) ),
                                                   static_cast<float>( 0.5+0.5*cos(0.7*t) ) ) );
        }

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
        CHECK_GL;
//        exit(0);
        glutPostRedisplay();
    }


};


int
main( int argc, char** argv )
{
    Scene::initLogger( &argc, argv );
    ViewerApp app( &argc, argv );
    app.run();
    return EXIT_SUCCESS;
}
