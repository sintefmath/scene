#include <sstream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ViewerJob.hpp"
#include <scene/glsl/GLSLRuntime.hpp>
#include <scene/glsl/GLSLRenderList.hpp>
#include <scene/collada/Importer.hpp>
#include <scene/tinia/Bridge.hpp>
#include <scene/tools/BBoxTool.hpp>
#include <scene/tools/ShaderGen.hpp>

#ifdef DEBUG
#include "GLDebugMessages.hpp"
#endif

#include "Skybox.hpp"

//#define skybox_texture_location "../data/skybox/pond/"
#define skybox_texture_location "/usr/var/trell/apps/bridgeData/skybox/pond/"

static const std::string visual_scenes_key      = "visual_scenes";
static const std::string camera_instances_key   = "camera_instances";

TiniaViewerJob::TiniaViewerJob( const std::list<std::string>& files )
    : m_files_to_read( files),
      m_scene_db( NULL ),
      m_glsl_runtime( NULL ),
      m_glsl_renderlist( NULL ),
      m_exporter_runtime( NULL ),
      m_app_camera( NULL ),
      m_app_camera_node( NULL )
{
    float identity[16] = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };

    const char* visual_scenes[] = { "[none]" };
    const char* cameras[] = { "[none]" };

    // fetch data that is needed to populate the policy
    tinia::model::Viewer viewer;
    viewer.height = 640;
    viewer.width = 360;
    for(unsigned int i=0; i<16; i++) {
        viewer.modelviewMatrix[i] = identity[i];
        viewer.projectionMatrix[i] = identity[i];
    }

    m_model->addElement( "viewer", viewer );
    m_model->addElement<std::string>( "boundingbox", "-0.1 -0.1 -0.1 1.1 1.1 1.1" );
    m_model->addElement<int>( "renderlist", 0 );


    m_model->addElementWithRestriction<std::string>( visual_scenes_key,
                                                     visual_scenes[0],
                                                     &visual_scenes[0],
                                                     &visual_scenes[1] );
    m_model->addAnnotation( visual_scenes_key, "Visual scenes in DB" );
    m_model->addStateListener( visual_scenes_key, this );

    m_model->addElementWithRestriction<std::string>( camera_instances_key,
                                                     cameras[0],
                                                     &cameras[0],
                                                     &cameras[1] );
    m_model->addAnnotation( camera_instances_key, "Camera instances in DB");
    m_model->addStateListener( camera_instances_key, this );

    // --- gui
    auto root = new tinia::model::gui::HorizontalLayout;


    auto canvas = new tinia::model::gui::Canvas( "viewer", "renderlist", "boundingbox" );
    canvas->boundingBoxKey( "boundingbox" );
    //canvas->setViewerType( std::string( "MouseClickResponder" ) );
    root->addChild( canvas );

    auto rightcol = new tinia::model::gui::VerticalLayout;
    root->addChild( rightcol );

    rightcol->addChild( new tinia::model::gui::Label( "camera_instances" ) );
    rightcol->addChild( new tinia::model::gui::ComboBox( "camera_instances" ) );
    rightcol->addChild( new tinia::model::gui::VerticalSpace );
    rightcol->addChild( new tinia::model::gui::Label( "visual_scenes" ) );
    rightcol->addChild( new tinia::model::gui::ComboBox( "visual_scenes" ) );
    rightcol->addChild( new tinia::model::gui::VerticalExpandingSpace );

    m_model->setGUILayout( root, tinia::model::gui::DESKTOP );

    m_skybox = new Skybox( skybox_texture_location );
    
}

TiniaViewerJob::~TiniaViewerJob()
{
    delete(m_skybox);
}

void
TiniaViewerJob::stateElementModified( tinia::model::StateElement *stateElement )
{
    const std::string key = stateElement->getKey();
    // handle visual scene selection
    if( key == visual_scenes_key ) {
        std::string value;
        stateElement->getValue( value );
        for(size_t i=0; i<m_visual_scenes.size(); i++ ) {
            if( m_visual_scenes[i] == value ) {
                switchToVisualScene( i );
                break;
            }
        }
    }
    // handle camera instance selection
    else if (key == camera_instances_key ) {
        std::string value;
        stateElement->getValue( value );
        for(size_t i=0; i<m_camera_instances.size(); i++ ) {
            if( m_camera_instances[i].m_label == value ) {
                switchToCameraInstance( i );
                break;
            }
        }
    }
}

bool
TiniaViewerJob::init()
{
    m_scene_db = new Scene::DataBase;
    readFiles();
    return true;
}

bool
TiniaViewerJob::initGL()
{
    glewInit();
#ifdef DEBUG
    siut3::gl_tools::GLDebugMessages::setupGLDebugMessages();
    siut3::gl_tools::GLDebugMessages::controlGLDebugMessages( GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);
#endif
    m_glsl_runtime = new Scene::Runtime::GLSLRuntime( *m_scene_db );
    m_glsl_renderlist = new Scene::Runtime::GLSLRenderList( *m_glsl_runtime );
    m_skybox->init();
    return true;
}

bool
TiniaViewerJob::renderFrame( const std::string&  session,
                  const std::string&  key,
                  unsigned int        fbo,
                  const size_t        width,
                  const size_t        height )
{
    if( m_visual_scenes.empty() || (m_glsl_runtime == NULL) || (m_glsl_renderlist == NULL) ) {
        return true;
    }
    tinia::model::Viewer viewer;
    m_model->getElementValue( "viewer", viewer );
    m_glsl_renderlist->setDefaultOutput( fbo, 0, 0, width, height );

    // Forward tinia projection matrix to app_camera
    if( m_app_camera != NULL ) {
        Scene::Value P = Scene::Value::createFloat4x4( viewer.projectionMatrix.data() );
        m_app_camera->setCustomMatrix( P );
    }

    // Forward tinia modelview matrix to app_camera_node
    glm::mat4 MVi; //want to use this for skybox, to get camera position
    if( m_app_camera_node != NULL ) {
        MVi = glm::inverse( glm::make_mat4x4( viewer.modelviewMatrix.data() ) );
        Scene::Value M = Scene::Value::createFloat4x4( glm::value_ptr( MVi ) );
        m_app_camera_node->transformSetMatrix( 0, M );
    }

    // Not all COLLADA files have buffer clearing set up
    glBindFramebuffer( GL_FRAMEBUFFER, fbo );
    glClearColor(0.2, 0.3f, 0.1f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    m_skybox->render(viewer.modelviewMatrix.data(), viewer.projectionMatrix.data() );
    m_glsl_renderlist->build( m_visual_scenes[ m_visual_scene ] );
    m_glsl_renderlist->render();
    return true;
}


const tinia::renderlist::DataBase*
TiniaViewerJob::getRenderList( const std::string& session, const std::string& key )
{
    if( m_scene_db == NULL ) {
        return NULL;
    }
    if( m_exporter_runtime == NULL ) {
        // First request for a renderlist, set up pipe.
        m_exporter_runtime = new Scene::Tinia::Bridge( *m_scene_db, Scene::PROFILE_GLES2, "WebGL" );
    }
    m_exporter_runtime->build( m_visual_scenes[ m_visual_scene ] );
    return &m_exporter_runtime->renderListDataBase();
}


void
TiniaViewerJob::readFiles()
{
    if( m_scene_db == NULL ) {
        return;
    }
    // --- Clear DB and create the app camera (which track the viewer cam) -----
    m_scene_db->clear();
    m_app_camera = m_scene_db->library<Scene::Camera>().add( "app_camera" );
    m_app_camera_node = m_scene_db->library<Scene::Node>().add( "app_camera_node" );
    m_app_camera_node->addInstanceCamera( "", "app_camera" );
    m_app_camera_node->transformAdd( "matrix" );
    m_app_camera_node->transformSetMatrix( 0, Scene::Value::createFloat4x4() );
    //if( m_stereo ) {
    //    m_app_camera_node->transformAdd( "translate" );
    //    m_app_camera_node->transformSetTranslate( 1, 0.f, 0.f, 0.f );
    //}

    // --- Import COLLADA files ------------------------------------------------
    Scene::Collada::Importer importer( *m_scene_db );
    for(auto it=m_files_to_read.begin(); it!=m_files_to_read.end(); ++it ) {
        if( it->length() > 4 &&
                ( (it->substr( it->length()-4 ) == ".xml") ||
                  (it->substr( it->length()-4 ) == ".XML") ||
                  (it->substr( it->length()-4 ) == ".dae") ||
                  (it->substr( it->length()-4 ) == ".DAE") ) )
        {
	  importer.parse( *it );
        }
    }
    m_files_to_read.clear();

    // --- Check if any file has shared inputs, and if so, flatten -------------
    for( size_t i=0; i<m_scene_db->library<Scene::Geometry>().size(); i++ ) {
        Scene::Geometry* g = m_scene_db->library<Scene::Geometry>().get( i );
        if( g->hasSharedInputs() ) {
            std::cerr << "Geometry '" << g->id() << "' has shared inputs, flattening\n";
            g->flatten();
        }
    }

    // --- Check if any effects have missing GLSL/GLES2-profiles ---------------
    Scene::Tools::generateShadersFromCommon( *m_scene_db,
		Scene::PROFILE_GLSL | 
		Scene::PROFILE_GLES2 );

    // --- Update bounding boxes of geometries ---------------------------------
    Scene::Tools::updateBoundingBoxes( *m_scene_db );

    // --- Update list of visual scenes ----------------------------------------
    m_visual_scenes.clear();
    m_visual_scene = 0;
    for( size_t i=0; i<m_scene_db->library<Scene::VisualScene>().size(); i++ ) {
        Scene::VisualScene* vs = m_scene_db->library<Scene::VisualScene>().get( i );

        if( vs->evaluateScenes() == 0 ) {
            // This scene does not have an evaluate block

            Scene::Node* root = m_scene_db->library<Scene::Node>().get( vs->nodesId() );
            root->addInstanceNode( "", "", "app_camera_node", "" );
            root->addInstanceLight( "my_light", "" );

            Scene::EvaluateScene* es = vs->addEvaluateScene();
            es->setEnabled( true );
            Scene::Render* ri = es->addRenderItem();
            ri->setCameraNodeId( "app_camera_node" );
        }
        m_visual_scenes.push_back( vs->id() );
    }
    if( m_visual_scenes.empty() ) {
        std::list<std::string> tmp;
        tmp.push_back("[none]" );
        m_model->updateRestrictions( "visual_scenes", tmp.front(), tmp );
    }
    else {
        m_model->updateRestrictions( "visual_scenes", m_visual_scenes.front(), m_visual_scenes );
    }
    switchToVisualScene( 0 );

    // --- Update list of camera instances -------------------------------------
    CameraInstance ci;
    m_camera_instances.clear();
    m_camera_instance = 0;
    for( size_t j=0; j<m_scene_db->library<Scene::Node>().size(); j++ ) {
        ci.m_node = m_scene_db->library<Scene::Node>().get( j );
        for( size_t i=0; i<ci.m_node->instanceCameras(); i++ ) {
            ci.m_camera = m_scene_db->library<Scene::Camera>().get( ci.m_node->instanceCameraURL(i) );
            if( ci.m_camera != NULL ) {
                std::stringstream o;
                o << ci.m_camera->id() << '@';
                if( ci.m_node->id().empty() ) {
                    o << ci.m_node->debugString();
                }
                else {
                    o << ci.m_node->id();
                }
                ci.m_label = o.str();
                m_camera_instances.push_back( ci );
            }
        }
    }
    std::list<std::string> tmp;
    if( m_camera_instances.empty() ) {
        tmp.push_back( "[none]" );
    }
    else {
        for(size_t i=0; i<m_camera_instances.size(); i++ ) {
            tmp.push_back( m_camera_instances[i].m_label );
        }
    }
    m_model->updateRestrictions( "camera_instances", tmp.front(), tmp );
    switchToCameraInstance( 0 );

	
}

void
TiniaViewerJob::switchToVisualScene( size_t ix )
{
    if( ix >= m_visual_scenes.size() ) {
        return;
    }
    m_visual_scene = ix;

    // --- We need a runtime & renderlist to determine what to render ----------
    Scene::Runtime::Resolver resolver( *m_scene_db, Scene::PROFILE_GLSL, "" );
    Scene::Runtime::RenderList renderlist( resolver );
    renderlist.build( m_visual_scenes[ m_visual_scene ] );

    Scene::Value bb_min, bb_max;
    if( Scene::Tools::visualSceneExtents( bb_min, bb_max, renderlist ) ) {
        std::stringstream o;
        o << bb_min.floatData()[0] << ' '
                                   << bb_min.floatData()[1] << ' '
                                   << bb_min.floatData()[2] << ' '
                                   << bb_max.floatData()[0] << ' '
                                   << bb_max.floatData()[1] << ' '
                                   << bb_max.floatData()[2];

        m_model->updateElement( "boundingbox", o.str() );
        std::cerr << o.str() << "\n";
    }
    std::cerr << "Switched to visual scene '" <<  m_visual_scenes[ m_visual_scene ] << "'.\n";
}

void
TiniaViewerJob::switchToCameraInstance( size_t ix )
{
    if( (m_scene_db == NULL)
            || (ix >= m_camera_instances.size())
            || (m_visual_scene >= m_visual_scenes.size()) )
    {
        return;
    }
    m_camera_instance = ix;

    // Current scene we're viewing
    Scene::VisualScene* scene = m_scene_db->library<Scene::VisualScene>().get( m_visual_scenes[ m_visual_scene ] );
    if( scene == NULL ) {
        std::cerr << "No such scene '" << m_visual_scenes[ m_visual_scene ] << "'\n";
        return;
    }
    // Node hierarchy of the scene we're viewing
    Scene::Node* root = m_scene_db->library<Scene::Node>().get ( scene->nodesId() );
    if( root == NULL ) {
        std::cerr << "Scene '" << scene->id() << "' has no root node.\n";
        return;
    }

    // We then need to find the camera in the node hierarcy of the scene, and
    // for this we need a resolver.
    std::list<const Scene::Node*> path;
    Scene::Runtime::Resolver resolver( *m_scene_db, Scene::PROFILE_GLSL, "" );
    if( !resolver.findNodePath( path, root, m_camera_instances[ ix ].m_node ) ) {
        std::cerr << "Couldn't find node path\n";
        return;
    }

    // And then, we need the actual transform matrices, and for this we need a
    // transform cache.
    Scene::Runtime::TransformCache tc( *m_scene_db );

    // Projection matrix
    const Scene::Value* P = tc.cameraProjectionMatrix( m_camera_instances[ ix ].m_camera );

    // Modelview matrix is found by creating a path from the root to the node
    // that contains the camera.
    const Scene::Node* cam_path[ SCENE_PATH_MAX ];
    size_t i=0;
    for( auto it=path.begin(); it!=path.end(); ++it ) {
        cam_path[i++] = *it;
    }
    for( ; i<SCENE_PATH_MAX; i++ ) {
        cam_path[i] = NULL;
    }
    const Scene::Value* M = tc.pathTransformInverseMatrix( cam_path );

    // And make sure that the values are up-to-date and we get the right
    // aspect ratio.
    tinia::model::Viewer viewer;
    m_model->getElementValue( "viewer", viewer );
    tc.update( viewer.width, viewer.height );

    // Then, we push the resulting matrices to the exposed model of Tinia.
    std::copy_n( P->floatData(), 16, viewer.projectionMatrix.data() );
    std::copy_n( M->floatData(), 16, viewer.modelviewMatrix.data() );
    m_model->updateElement( "viewer", viewer );

    std::cerr << "Switched to camera instance '" << m_camera_instances[ ix ].m_label << "'.\n";
}


