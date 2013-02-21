#pragma once

#include <string>
#include <vector>
#include <libxml/tree.h>
#include <unordered_map>
#include "scene/Log.hpp"
#include "scene/Scene.hpp"
#include "scene/Geometry.hpp"



namespace Scene {
    namespace Collada {

class Exporter
{
public:
    Exporter( const Scene::DataBase& database );

    xmlNodePtr
    create( bool lib_geometry     = true,
            bool lib_image        = true,
            bool lib_camera       = true,
            bool lib_light        = true,
            bool lib_effect       = true,
            bool lib_material     = true,
            bool lib_nodes        = true,
            bool lib_visual_scene = true,
            int profile_mask      = ~0 );

protected:
    struct Context {
        bool                                  m_lib_geometry;
        bool                                  m_lib_image;
        bool                                  m_lib_camera;
        bool                                  m_lib_light;
        bool                                  m_lib_effect;
        bool                                  m_lib_material;
        bool                                  m_lib_nodes;
        bool                                  m_lib_visual_scene;
        unsigned int                          m_profile_mask;
        std::unordered_map<std::string, bool> m_exported_sources;
        std::unordered_map<std::string, bool> m_exported_source_buffers;
    };

    const Scene::DataBase&    m_database;
    bool                      m_lean_export;
    static const std::string  m_vertex_semantics[ VERTEX_SEMANTIC_N ];

    Exporter();

    xmlNodePtr
    newChild( xmlNodePtr parent, xmlNsPtr ns, const std::string& name, const std::string& content ) const;

    xmlNodePtr
    newChild( xmlNodePtr parent, xmlNsPtr ns, const std::string& name ) const;

    xmlNodePtr
    newNode( xmlNsPtr ns, const std::string& name ) const;

    void
    addProperty( xmlNodePtr node, const std::string& key, const std::string& value ) const;

    void
    addProperty( xmlNodePtr node, const std::string& key, const int value ) const;

    void
    setBody( xmlNodePtr node, const float* values, size_t count ) const;

    void
    setBody( xmlNodePtr node, const int* values, size_t count ) const;

    const std::string
    sourceId( const std::string& source_buffer_id,
              const unsigned int count,
              const unsigned int components,
              const unsigned int offset,
              const unsigned int stride ) const;

    xmlNodePtr
    createValue( const Scene::Value*  value,
                 const ValueContext   context ) const;

    xmlNodePtr
    createCollada( Context& context ) const;

    xmlNodePtr
    createNewParam( Context&            context,
                    const ValueContext  value_context,
                    const Parameter*    param ) const;

    xmlNodePtr
    createLibraryVisualScenes( Context& context ) const;

    xmlNodePtr
    createVisualScene( Context& context, const VisualScene* scene ) const;

    xmlNodePtr
    createRender( Context& context, const Render* render ) const;

    xmlNodePtr
    createLibraryCameras( Context& context ) const;

    xmlNodePtr
    createCamera( Context& context, const Camera* camera ) const;

    xmlNodePtr
    createLibraryLights( Context& context ) const;

    xmlNodePtr
    createLight( Context& context, const Light* light ) const;

    xmlNodePtr
    createLibraryNodes( Context& context ) const;

    xmlNodePtr
    createNode( Context& context, const Node* node ) const;

    xmlNodePtr
    createInstanceGeometry( Context& context, const InstanceGeometry* instance ) const;

    xmlNodePtr
    createBindMaterial( Context& context, const InstanceGeometry* instance ) const;

    xmlNodePtr
    createBind( Context& context, const Bind& bind ) const;


    xmlNodePtr
    createLibraryGeometries( Context& context ) const;

    xmlNodePtr
    createLibraryMaterials( Context& context ) const;

    xmlNodePtr
    createInstanceEffect( Context&  context,
                          const Scene::Material*    material ) const;

    xmlNodePtr
    createMaterial( Context&  context,
                    const Scene::Material*    material ) const;

    xmlNodePtr
    createLibraryEffects( Context&  context ) const;

    xmlNodePtr
    createEffect( Context& context, const Effect* effect ) const;

    xmlNodePtr
    createProfile( Context& context, const Profile* profile ) const;

    xmlNodePtr
    createTechnique( Context& context, const Technique* technique ) const;

    xmlNodePtr
    createCommonShadingModel( Context& context, const CommonShadingModel* sm ) const;

    xmlNodePtr
    createPass( Context& context, const Pass* pass ) const;

    xmlNodePtr
    createStates( Context& context, const Pass* pass ) const;

    xmlNodePtr
    createProgram( Context& context, const Pass* pass ) const;

    xmlNodePtr
    createEvaluate( Context& context, const Pass* pass ) const;

    xmlNodePtr
    createGeometry( Context& context,
                   const Scene::Geometry* geometry ) const;


    xmlNodePtr
    createMesh( Context& context,
               const Scene::Geometry* geometry ) const;

    xmlNodePtr
    createSource( Context& context,
                  const std::string& source_buffer_id,
                  const unsigned int count,
                  const unsigned int components,
                  const unsigned int offset,
                  const unsigned int stride  ) const;


    xmlNodePtr
    createAccessor( Context&           context,
                    const std::string& source_buffer_id,
                    const unsigned int count,
                    const unsigned int components,
                    const unsigned int offset,
                    const unsigned int stride ) const;
    xmlNodePtr
    createFloatArray( Context& context,
                     const Scene::SourceBuffer* source_buffer ) const;

    xmlNodePtr
    createVertices( const Scene::Geometry* geometry ) const;


    xmlNodePtr
    createTriangles( Context& context,
                     const Primitives& ps ) const;

    xmlNodePtr
    createAsset( const Asset& asset ) const;

    xmlNodePtr
    createAsset( Context& context, const Asset& parent, const Asset& asset ) const;

};

    } // of namespace Scene
} // of namespace Scene
