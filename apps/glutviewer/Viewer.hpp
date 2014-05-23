#pragma once

#include <scene/Scene.hpp>
#include <scene/glsl/GLSLRuntime.hpp>
#include <scene/glsl/GLSLRenderList.hpp>
#ifdef SCENE_TINIA
#include <scene/tinia/Bridge.hpp>
#endif
#include "ViewManipulator.hpp"

class ViewerApp
{
public:
    ViewerApp( const std::vector<std::string>& files,
               bool stereo,
               bool torture,
               bool auto_shader,
               bool auto_flatten,
               float scale );

    void
    keyboard( unsigned char key );

    ViewManipulator&
    viewer();

    /** Run through nodes and find instances of cameras. */
    void
    updateCameraInstances();

    /** Set the viewer camera to that specifiaction of a camera instance. */
    void
    moveToCameraInstance( size_t index );

    void
    setup( );
    
    void
    updateBoundingBox();

    void
    reshape( int w, int h );

    /** Purge database and reload all COLLADA from disc. */
    void
    reload();

    /** Switch to viewing the next visual scene. */    
    void
    nextVisualScene();

#ifdef SCENE_TINIA
    /** Create a render list for the current view and dump the XML to cout. */    
    void
    dumpCurrentRenderlistXML();
#endif
    
    void
    render();

protected:
    Scene::DataBase                     m_db;
    Scene::Runtime::GLSLRuntime         m_runtime;
    Scene::Runtime::GLSLRenderList      m_renderlist;
    ViewManipulator                     m_viewer;
#ifdef SCENE_TINIA
    Scene::Tinia::Bridge                m_exporter_runtime;
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

};
