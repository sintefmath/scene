#include <sstream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ViewerJob.hpp"
#include <scene/glsl/GLSLRuntime.hpp>
#include <scene/glsl/GLSLRenderList.hpp>
#include <scene/collada/Importer.hpp>
#include <scene/tools/BBoxTool.hpp>

TiniaViewerJob::TiniaViewerJob( const std::list<std::string>& files )
    : m_files_to_read( files),
      m_scene_db( NULL ),
      m_runtime( NULL ),
      m_renderlist( NULL ),
      m_app_camera( NULL ),
      m_app_camera_node( NULL )
{
    float identity[16] = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };

    const char* visual_scenes[] { "[none]" };
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

    m_model->addElementWithRestriction<std::string>( "visual_scenes",
                                                     visual_scenes[0],
                                                     &visual_scenes[0],
                                                     &visual_scenes[1] );

    m_model->addElementWithRestriction<std::string>( "camera_instances",
                                                     cameras[0],
                                                     &cameras[0],
                                                     &cameras[1] );

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

}

TiniaViewerJob::~TiniaViewerJob()
{

}

void
TiniaViewerJob::stateElementModified( tinia::model::StateElement *stateElement )
{

}

bool
TiniaViewerJob::init()
{
    return true;
}

bool
TiniaViewerJob::initGL()
{
    glewInit();
    m_scene_db = new Scene::DataBase;
    m_runtime = new Scene::Runtime::GLSLRuntime( *m_scene_db );
    m_renderlist = new Scene::Runtime::GLSLRenderList( *m_runtime );

    readFiles();
    return true;
}

bool
TiniaViewerJob::renderFrame( const std::string&  session,
                  const std::string&  key,
                  unsigned int        fbo,
                  const size_t        width,
                  const size_t        height )
{
    if( m_visual_scenes.empty() ) {
        return true;
    }
    tinia::model::Viewer viewer;
    m_model->getElementValue( "viewer", viewer );

    // Forward tinia projection matrix to app_camera
    if( m_app_camera != NULL ) {
        Scene::Value P = Scene::Value::createFloat4x4( viewer.projectionMatrix.data() );
        m_app_camera->setCustomMatrix( P );
    }
    // Forward tinia
    if( m_app_camera_node != NULL ) {
        glm::mat4 MVi = glm::inverse( glm::make_mat4x4( viewer.modelviewMatrix.data() ) );
        Scene::Value M = Scene::Value::createFloat4x4( glm::value_ptr( MVi ) );
        m_app_camera_node->transformSetMatrix( 0, M );
    }

    m_renderlist->render();
    return true;
}

const tinia::renderlist::DataBase*
TiniaViewerJob::getRenderList( const std::string& session, const std::string& key )
{
    return NULL;
}



void
TiniaViewerJob::readFiles()
{
    if( m_scene_db == NULL ) {
        return;
    }
    m_scene_db->clear();
    m_app_camera = NULL;
    m_app_camera_node = NULL;


    if( !m_files_to_read.empty() ) {
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

        // Create application camera (which we manipulate)
        m_app_camera = m_scene_db->library<Scene::Camera>().add( "app_camera" );
        m_app_camera_node = m_scene_db->library<Scene::Node>().add( "app_camera_node" );
        m_app_camera_node->addInstanceCamera( "", "app_camera" );
        m_app_camera_node->transformAdd( "matrix" );
        m_app_camera_node->transformSetMatrix( 0, Scene::Value::createFloat4x4() );
        //if( m_stereo ) {
        //    m_app_camera_node->transformAdd( "translate" );
        //    m_app_camera_node->transformSetTranslate( 1, 0.f, 0.f, 0.f );
        //}

        Scene::Tools::updateBoundingBoxes( *m_scene_db );
        updateVisualScenes();
        updateCameraInstances();
    }
}

void
TiniaViewerJob::updateVisualScenes()
{
    // Determine the set of visual scenes in the file and update exposedmodel
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
        std::list<std::string> tmp = { "[none]" };
        m_model->updateRestrictions( "visual_scenes", tmp.front(), tmp );
    }
    else {
        m_model->updateRestrictions( "visual_scenes", m_visual_scenes.front(), m_visual_scenes );
    }
    switchToVisualScene( 0 );
}

void
TiniaViewerJob::switchToVisualScene( size_t ix )
{
    if( ix >= m_visual_scenes.size() ) {
        return;
    }
    m_visual_scene = ix;

    m_renderlist->build( m_visual_scenes[ m_visual_scene ] );

    Scene::Value bb_min, bb_max;
    if( Scene::Tools::visualSceneExtents( bb_min, bb_max, m_renderlist->renderList() ) ) {
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
TiniaViewerJob::updateCameraInstances()
{
    CameraInstance ci;
    m_camera_instances.clear();
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
TiniaViewerJob::switchToCameraInstance( size_t ix )
{
    if( ix >= m_camera_instances.size() ) {
        return;
    }
    if( m_visual_scene >= m_visual_scenes.size() ) {
        return;
    }

    Scene::VisualScene* scene = m_scene_db->library<Scene::VisualScene>().get( m_visual_scenes[ m_visual_scene ] );
    if( scene == NULL ) {
        std::cerr << "No such scene '" << m_visual_scenes[ m_visual_scene ] << "'\n";
        return;
    }
    Scene::Node* root = m_scene_db->library<Scene::Node>().get ( scene->nodesId() );
    if( root == NULL ) {
        std::cerr << "Scene '" << scene->id() << "' has no root node.\n";
        return;
    }
    std::list<const Scene::Node*> path;
    if( !m_runtime->resolver().findNodePath( path, root, m_camera_instances[ ix ].m_node ) ) {
        std::cerr << "Couldn't find node path\n";
        return;
    }

    // Use a transform cache to calculate the matrices
    Scene::Runtime::TransformCache tc( *m_scene_db );
    const Scene::Value* P = tc.cameraProjectionMatrix( m_camera_instances[ ix ].m_camera );
    const Scene::Node* cam_path[ SCENE_PATH_MAX ];
    size_t i=0;
    for( auto it=path.begin(); it!=path.end(); ++it ) {
        cam_path[i++] = *it;
    }
    for( ; i<SCENE_PATH_MAX; i++ ) {
        cam_path[i] = NULL;
    }
    const Scene::Value* M = tc.pathTransformInverseMatrix( cam_path );

    tinia::model::Viewer viewer;
    m_model->getElementValue( "viewer", viewer );

    tc.update( viewer.width, viewer.height );

    // push projection and orientation to viewer
    std::copy_n( P->floatData(), 16, viewer.projectionMatrix.data() );
    std::copy_n( M->floatData(), 16, viewer.modelviewMatrix.data() );
    m_model->updateElement( "viewer", viewer );

    std::cerr << "Switched to camera instance '" << m_camera_instances[ ix ].m_label << "'.\n";
}
