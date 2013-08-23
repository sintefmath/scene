#pragma once
#include <scene/DataBase.hpp>
#include <scene/glsl/GLSLRuntime.hpp>
#include <scene/glsl/GLSLRenderList.hpp>
#include <scene/Camera.hpp>
#include <scene/Node.hpp>
#include <tinia/jobcontroller/OpenGLJob.hpp>
#include <tinia/model/StateListener.hpp>
#include <tinia/renderlist/DataBase.hpp>
#include <scene/tinia/Bridge.hpp>

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
    /** List of files to read. */
    std::list<std::string>              m_files_to_read;

    /** Scene DB, independent on runtime. */
    Scene::DataBase*                    m_scene_db;

    /** Runtime for onscreen-rendering. */
    Scene::Runtime::GLSLRuntime*        m_glsl_runtime;

    /** Renderlist for onscreen-rendering. */
    Scene::Runtime::GLSLRenderList*     m_glsl_renderlist;

    /** Bridge for renderlist export. */
    Scene::Tinia::Bridge*               m_exporter_runtime;

    /** Application controlled camera. */
    Scene::Camera*                      m_app_camera;

    /** Application controlled node that instances the app camera. */
    Scene::Node*                        m_app_camera_node;

    /** List of visual scenes in Scene DB. */
    std::vector<std::string>            m_visual_scenes;

    /** Current visual scene index. */
    size_t                              m_visual_scene;

    /** Struct to hold info related to a camera instance. */
    struct CameraInstance {
        Scene::Node*                    m_node;
        Scene::Camera*                  m_camera;
        std::string                     m_label;
    };

    /** List of camera instances in Scene DB. */
    std::vector<CameraInstance>         m_camera_instances;

    /** Current camera instance index. */
    size_t                              m_camera_instance;

    /** Clear DB, setup app camera, and read all files in m_files_to_read. */
    void
    readFiles( );

    void
    switchToVisualScene( size_t ix );

    void
    switchToCameraInstance( size_t ix );

};
