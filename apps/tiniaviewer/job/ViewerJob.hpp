#pragma once
#include <scene/DataBase.hpp>
#include <scene/glsl/GLSLRuntime.hpp>
#include <scene/glsl/GLSLRenderList.hpp>
#include <scene/Camera.hpp>
#include <scene/Node.hpp>
#include <tinia/jobcontroller/OpenGLJob.hpp>
#include <tinia/model/StateListener.hpp>
#include <tinia/renderlist/DataBase.hpp>

class TiniaViewerJob
        : public tinia::jobcontroller::OpenGLJob,
          public tinia::model::StateListener
{
public:
    TiniaViewerJob( const std::list<std::string>& files );

    ~TiniaViewerJob();

    void
    stateElementModified( tinia::model::StateElement *stateElement );

    bool
    init();

    bool
    initGL();


    bool
    renderFrame( const std::string&  session,
                 const std::string&  key,
                 unsigned int        fbo,
                 const size_t        width,
                 const size_t        height );

    const tinia::renderlist::DataBase*
    getRenderList( const std::string& session, const std::string& key );


protected:
    std::list<std::string>              m_files_to_read;
    Scene::DataBase*                    m_scene_db;
    Scene::Runtime::GLSLRuntime*        m_runtime;
    Scene::Runtime::GLSLRenderList*     m_renderlist;
//    Scene::Runtime::XMLRuntime*         m_rl_runtime;
    Scene::Camera*                      m_app_camera;
    Scene::Node*                        m_app_camera_node;
    std::vector<std::string>            m_visual_scenes;
    size_t                              m_visual_scene;
    struct CameraInstance {
        Scene::Node*                    m_node;
        Scene::Camera*                  m_camera;
        std::string                     m_label;
    };
    std::vector<CameraInstance>         m_camera_instances;


    void
    readFiles( );

    void
    updateVisualScenes();

    void
    updateCameraInstances();

    void
    switchToVisualScene( size_t ix );

    void
    switchToCameraInstance( size_t ix );


};
