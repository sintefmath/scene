#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <libxml/tree.h>
#include "scene/Log.hpp"
#include "scene/Scene.hpp"
#include "scene/Geometry.hpp"

namespace Scene {
    namespace Collada {

class Importer
{
public:
    /** Create a new builder class for importing a COLLADA file.
      *
      * \param[in] database   The database to add assets.
      * \param[in] base_path  The base path to use when resolving relative
      *                       paths.
      */
    Importer( Scene::DataBase& database, const std::string base_path = "" );

    bool
    parse( const std::string& url );

    bool
    parseMemory( const char* buffer );

    /** Parse a \<COLLADA\> node.
      *
      * Recognizes the following elements:
      * - \<asset\>,
      *   see Scene::XML::Importer::parseAsset
      * - \<library_images\>,
      *   see Scene::XML::Importer::parseLibraryImages
      * - \<library_effects\>,
      *   see Scene::XML::Importer::parseLibraryEffects
      * - \<library_materials\>,
      *   see Scene::XML::Importer::parseLibraryMaterials
      * - \<library_geometries\>,
      *   see Scene::XML::Importer::parseLibraryGeometries
      * - \<library_nodes\>,
      *   see Scene::XML::Importer::parseLibraryNodes
      * - \<library_visual_scenes\>,
      *   see Scene::XML::Importer::parseLibraryVisualScenes
      * - \<library_lights\>,
      *   see Scene::XML::Importer::parseLibraryLights
      * - \<extra\>, the following extensions are recognized inside
      *   <technique profile="scene">:
      *   - \<include file="..." /\>, recursively imports the file into the
      *     database.
      *
      * \param[in] collada_node  The node of the COLLADA subtree to parse.
      * \returns True on success, false if any error occured.
      */
    bool
    parseCollada( xmlNodePtr collada_node );

protected:
    struct Context {
        enum {
            X_UP,   ///< Right is neg y, up is pos x, in is pos z.
            Y_UP,   ///< Right is pos x, up is pos y, in is pos z.
            Z_UP    ///< Right is pos x, up is pos z, in is neg y.
        }       m_up_axis;  ///< Coordinate system to interpret file in.
        float   m_unit;     ///< Units (in meters) to interpret file in.
    };


    Scene::DataBase&          m_database;
    std::string               m_base_path;
    const std::string         m_namespace;
    static const std::string  m_vertex_semantics[ VERTEX_SEMANTIC_N ];

    Importer();

    const std::string
    resolvePath( const std::string path );

    /** Removes some uneccessary nodes from the tree, to simplify parsing. */
    void
    clean( xmlNodePtr xml_node );

    static const std::string
    cleanRef( const std::string& ref );

    static const std::string
    attribute( xmlNodePtr, const std::string& attribute );

    static bool
    attribute( unsigned int& result, xmlNodePtr node, const std::string& attribute );

    static bool
    attribute( float& result, xmlNodePtr node, const std::string& attribute );

    static int
    strEq( const xmlChar* str1, const std::string& str2);


    xmlNodePtr
    searchBySid( xmlNodePtr start, const std::string& sid );

    bool
    retrieveTextFile( std::string& result, const std::string& url );

    bool
    retrieveBinaryFile( std::vector<char>& result, const std::string& url );

    const std::vector<unsigned char>
    parseHex( xmlNodePtr n );

    bool
    parseWrapMode( GLenum& mode, xmlNodePtr n );

    bool
    parseFilterMode( GLenum& mode,
                     xmlNodePtr n,
                     bool accept_none,
                     bool accept_nearest,
                     bool accept_linear,
                     bool accept_anistropic );

    bool
    assertNode( xmlNodePtr node, const std::string& name );

    bool
    assertChild( xmlNodePtr parent, xmlNodePtr child, const std::string& name );

    bool
    checkNode( xmlNodePtr node, const std::string& name );

    void
    skipNode( xmlNodePtr parent, xmlNodePtr& n, const std::string& name );

    void
    skipNodes( xmlNodePtr parent, xmlNodePtr& n, const std::string& name );



    void
    ignoreExtraNodes( Logger log, xmlNodePtr& n );

    void
    nagAboutRemainingNodes( Logger log, xmlNodePtr& n );

    void
    nagAboutParseError( Logger log, xmlNodePtr n, const std::string& why="" );



    bool
    parseBodyAsFloats( std::vector<float>& result, xmlNodePtr node, size_t expected );


    bool
    parseSemantic( RuntimeSemantic& semantic, xmlNodePtr semantic_node );


    bool
    parseBool( const std::string value, bool default_value );

    bool
    parseBind( Bind&       bind,
               xmlNodePtr  bind_node );


    bool
    parseBindMaterial( InstanceGeometry*  instgeo,
                       xmlNodePtr         bind_material_node );


    const std::string
    getBody( xmlNodePtr node );

    bool
    parseBodyAsInts( std::vector<int>& result, xmlNodePtr node, size_t expected, size_t offset=0, size_t stride=1 );

    bool
    parseEvaluate( Pass* pass,
                   xmlNodePtr  evaluate_node );

    bool
    parseStates( Pass* pass,
                 xmlNodePtr  states_node );

    bool
    parseProgram( Pass* pass,
                  const std::unordered_map<std::string,std::string>& code_blocks,
                  xmlNodePtr program_node );

    bool
    parsePass( Technique* technique,
               const std::unordered_map<std::string,std::string>& code_blocks,
               xmlNodePtr pass_node );

    bool
    parseCommonShadingModel( Technique* technique,
                             const ShadingModelType shading_model,
                             xmlNodePtr model_node );

    bool
    parseTechnique( Profile* profile,
                    const std::unordered_map<std::string,std::string>& code_blocks,
                    xmlNodePtr technique_node );


    bool
    parseEffect( const Asset&  asset_parent,
                 xmlNodePtr    effect_node );

    bool
    parseNewParam( Parameter&          parameter,
                   xmlNodePtr          newparam_node,
                   const ValueContext  context );


    bool
    parseValue( Scene::Value*       value,
                xmlNodePtr          value_node,
                const ValueContext  context );

    bool
    parseLibraryImages( const Asset& asset_parent,
                        xmlNodePtr lib_images_node );

    bool
    parseFormat( GLenum&  iformat,
                 GLenum&  format,
                 GLenum&  type,
                 xmlNodePtr format_node );

    bool
    parseLibraryNodes( Context      context,
                       const Asset& asset_parent,
                       xmlNodePtr   lib_nodes_node );

    bool
    parseProfile( Effect*     effect,
                  xmlNodePtr  profile_node );


    /** Parse mipmap-spec of images. */
    bool
    parseMips( size_t& mips, bool& auto_generate, xmlNodePtr mips_node );

    /** Parse image array length. */
    bool
    parseArray( size_t& array_length, xmlNodePtr array_node );

    /** Parse size of images. */
    bool
    parseSize( size_t* width, size_t* height, size_t* depth, xmlNodePtr n );

    /** Parse init_from within a create_x node */
    bool
    parseInitFrom( Image* image, xmlNodePtr init_from_node );

    bool
    parseImage( const Asset& asset_parent,
                xmlNodePtr image_node );

    bool
    parseInstanceGeometry( Node*       node,
                           xmlNodePtr  instance_geometry_node );

    bool
    parseCamera( const Asset&  asset_parent,
                 xmlNodePtr    camera_node );

    bool
    parseLibraryCameras( const Asset& asset_parent,
                         xmlNodePtr lib_cameras_node );

    bool
    parseLibraryVisualScenes( Context context,
                              const Asset& asset_parent,
                              xmlNodePtr lib_vis_scene_node );

    /** Parse a \<visual_scene\> node
      *
      * Recognizes the following elements:
      * - 0..1 \<asset\>, see Scene::XML::Importer::parseAsset
      * - 0..n \<node\>. Creates a root node in the node library and adds the
      *   nodes of this scene. See also Scene::XML::Importer::parseNode.
      * - 0..n \<evaluate_scene\>:
      *   - 0..1 \<asset\>,  currently ignored.
      *   - 0..n \<render\>:
      *     - 0..n \<layer\>. Adds that layer for inclusion in this render
      *       operation.
      *     - 0..1 \<instance_material\> enables instance_material pipeline:
      *       - 0..1 \<technique_override\>
      *       - 0..n \<bind\>, currently ignored.
      *       - 0..n \<extra\>, currently ignored.
      *   - 0..n \<extra\>:
      *     - 0..n <technique profile="scene">, scene specific extensions:
      *       - 0..n \<light_node index="N" ref="REF" /\>. Specifies that the
      *          light source in node REF should be used as light N
      *
      * \param[in] asset_parent       The asset to copy if no asset is present.
      * \param[in] visual_scene_node  The node to parse.
      * \returns True on success, false if any error occured.
      */
    bool
    parseVisualScene( Context       context,
                      const Asset&  asset_parent,
                      xmlNodePtr    visual_scene_node );

    bool
    parseLibraryLights( const Asset& parent_asset,
                        xmlNodePtr library_lights_node );

    bool
    parseLibraryGeometries( xmlNodePtr library_geometries_node );

    bool
    parseRender( VisualScene* visual_scene,
                 xmlNodePtr   render_node );

    bool
    parseNode( Node*           parent_node,
               const Asset&    parent_asset,
               xmlNodePtr      node_node );


    bool
    parseLibraryMaterials( xmlNodePtr library_materials_node );

    bool
    parseMaterial( const Asset& asset_parent,
                   xmlNodePtr material_node );

    bool
    parseInstanceEffect( Material* material,
                         xmlNodePtr instance_effect_node );

    bool
    parseLibraryEffects( xmlNodePtr library_effects_node );

    bool
    parseGeometry( Scene::Geometry* geometry,
                   xmlNodePtr  geometry_node );

    /** Parses a mesh node, adding vertex inputs and primitives sets to geometry. */
    bool
    parseMesh( Scene::Geometry*  geometry,
               xmlNodePtr        mesh_node );

    /** Parses <source> node and populates a vertex input, creating required source buffers. */
    bool
    parseSource( Scene::Geometry::VertexInput&  input,
                 xmlNodePtr                     source_node );

    /** Parses <float_array> node populating the passed source buffer. */
    bool
    parseFloatArray( Scene::SourceBuffer* source_buffer,
                     xmlNodePtr           float_array_node );

    /** Parses <accessor> populating a vertex input struct. */
    bool
    parseAccessor( Scene::Geometry::VertexInput&  input,
                   xmlNodePtr                     accessor_node );

    /** Parses a <vertices> node and sets the vertex inputs according to vertex semantic. */
    bool
    parseVertices( Geometry*  geo,
                   const std::unordered_map<std::string,Geometry::VertexInput>& inputs,
                   xmlNodePtr vertices_node );

    /** Parses directly supported primitive node */
    bool
    parseSimplePrimitives( Geometry* geometry,
                           const std::unordered_map<std::string,Geometry::VertexInput>& inputs,
                           xmlNodePtr                      polylist_node );


    bool
    parsePolylist( Geometry*                    geometry,
                   const std::unordered_map<std::string,Geometry::VertexInput>& inputs,
                   xmlNodePtr                      polylist_node );

    bool
    parseInputShared( Primitives& prim_set,
                      const std::unordered_map<std::string,Geometry::VertexInput>& inputs,
                      xmlNodePtr input_node );

    bool
    parseAsset( Context&    context,
                Asset&      asset,
                xmlNodePtr  asset_node );

    bool
    parseAsset( Asset&      asset,
                xmlNodePtr  asset_node,
                float*      scope_unit = NULL );


};

    } // of namespace Scene
} // of namespace Scene
