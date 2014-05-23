/* Copyright STIFTELSEN SINTEF 2014
 * 
 * This file is part of Scene.
 * 
 * Scene is free software: you can redistribute it and/or modifyit under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * Scene is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more
 * details.
 *  
 * You should have received a copy of the GNU Affero General Public License
 * along with the Scene.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#warning "This file contains only documentation"

/** \mainpage Scene Library
  *
  * The Scene library is a rendering asset manager and renderer, following the
  * COLLADA specification to some extent. A key design choice is that the
  * database can easily be populated by a compliant COLLADA document, as well
  * as that the database can similarily be exported as a compliant COLLADA
  * document. The rationale for this choice is:
  * - Allow easy import and export of assets from other tools.
  * - Easily manage rendering assets for multiple targets (GLSL, GLES, WebGL)
  * - Avoid inventing the wheel yet another time, as the COLLADA working group
  *   has considered these issues heavily.
  *
  * A divergence from the COLLADA specification is that references are allowed
  * to pull in other documents, e.g., a visual scene in one document may
  * instantiate geometry in another document. Scene uses a 'push' semantic,
  * which means that all documents that can provide assets has to be explicitly
  * pushed into the database. In principle, the database represents a single
  * document, sharing a namespace, and pushing multiple documents into the
  * database is effectively to concatenate the documents. The rationales for
  * this are:
  * - The ability to freely pull in content over the internet is usually not
  *   required.
  * - To maintain control over what is included.
  * - To be able to split a set of assets into separate files and allowing these
  *   assets to be trivially replaced without updating a set of references.
  * - To simplify the implementation.
  *
  * Other design guidelines are:
  * - Efficient rendering on workstations
  * - Export of baked render lists for relatively thin remote clients using
  *   WebGL or OpenGL ES.
  *
  * \section overview Library overview
  *
  * The library consists of three logical parts: The database, the import and
  * export modules, and the runtime modules.
  *
  * The Scene::DataBase is, as the name implies, a database of assets. The assets are
  * organized based on type into a set of libraries. These libraries follows the
  * COLLADA specification, with libraries for geometry, images, cameras,
  * effects, materials, scene graph nodes, and visual scenes. In addition, there
  * is a library for source buffers, which has no direct COLLADA counterpart.
  * The source buffers is basically an OpenGL buffer object, holding geometry
  * and index data. Keeping these buffers as separate entities allowing sharing
  * of buffers between geometric objects. COLLADA allows this by letting a
  * geometry to reference buffers of another geometry. In Scene, the import and
  * export libraries handles this mapping.
  *
  * The class Scene::XML::Importer and Scene::XML::Exporter are builder classes
  * that populates and extract
  * information from the database to and from COLLADA files. The import and
  * export modules have no special access rights beyond the public API of the
  * database, so all that can be specified in a COLLADA file can be specified
  * at runtime as well.
  *
  * The runtime modules handles complex queries of the database such as
  * resolving references and deducing render lists. A key issue is that creating
  * a render list can be an involved task, so rebuilding must be minimized. To
  * this end, the database differentiates between major and minor changes
  * - A minor change is a change in a material parameter, change of positioning
  *   of scene graph nodes, etc. Such values are pulled directly from the
  *   database each frame (as pointers to these values are stored and not the
  *   values themselves), avoiding rebuilding the renderlist. This is the
  *   reason for the numerous methods that returns const Value* pointers.
  * - A major change is a change where the structure of the scene is changed,
  *   like adding or removing assets, or reorganzining the scene graph. In such
  *   cases, the renderlist has to be rebuilt.
  *
  * The runtime currently consists of the following major parts:
  * - The Scene::Runtime::Resolver class, which resolves and caches references between assets.
  * - The Scene::Runtime::RenderList class, which is an API-agnoistic (it is not tied to
  *   specific profiles as GLSL or GLES). It takes a visual scene as input and
  *   extracts the actions required to render this scene.
  * - The Scene::Runtime::TransformCache class, which computes transformation matrices from the
  *   SetViewCoordSys and SetLocalCoordSys actions of a render list. It is
  *   designed such that the required values are pulled directly from the
  *   database each time the update method is invoked, not requirering the
  *   deduction of node paths each frame.
  * - The Scene::Runtime::GLSLRuntime class is used when Scene should render,
  *   and manages OpenGL objects like textures, shaders, and buffers. It
  *   basically clones a subset of the items in the Scene::DataBase in GPU mem,
  *   pulling and pushing data as needed.
  * - The Scene::Runtime::GLSLRenderList class sits on top of a
  *   Scene::Runtime::RenderList, and adds the specific actions required to
  *   perform rendering using OpenGL.
  * - The Scene::Runtime::ExporterRenderList sits on on top of a
  *   Scene::Runtime::RenderList, and creates an XML document describing the
  *   render list. It pulls data from the Scene::DataBase for a particular
  *   profile, such that the document is a self-contained rendering description
  *   for a particular visual scene.
  *
  * \section value Values
  *
  * The class Scene::Value is Scene's dynamically typed variables. For the most
  * part, Scene doesn't care about the actual contents of a value, just that it
  * is a defined value. Just using a single value class simplifies the API
  * (minimizes the number of methods) and makes the system flexible.
  *
  * It is quite common for Scene objects to return const pointers to values.
  * These pointer point directly into the internals of the objects, and thus,
  * these internal values can be pulled directly from the database without
  * resolving names etc. This is heavily used in the render lists to get
  * decent per-frame performance.
  *
  * \section geometry Specifying geometric shapes
  *
  * The method used to describe geometric shapes is closely tied to how OpenGL
  * renders geometry.
  *
  * \subsection geometry_data Step 1: The input data
  * The first step is to specify the input data (positons,
  * texcoords, etc) for each vertex. In COLLADA this is specified as float
  * arrays in sources,
  * \code
  * <library_geometries>
  *   <geometry id="my_geometry">
  *     <source id="my_geometry_positions">
  *       <float_array id="my_geometry_positions_array" count="...">
  *          1.0 0.0 0.0
  *          0.0 1.0 0.0 ...
  * \endcode
  * The count element is the number of values in the array (i.e., a 3-component
  * position has three elements etc.).
  *
  * As mentioned in \ref overview, Scene introduces Scene::SourceBuffer to
  * handle these buffers,
  * \code
  * std::vector<float> contents; // put data in this
  * Scene::SourceBuffer* buffer = m_db.library<Scene::SourceBuffer>().add( "my_geometry_positions_array" );
  * buffer->contents( contents );
  * \endcode
  *
  * \subsection geometry_accessors Step 2: Define the vertex stream
  *
  * The next step is to specify how the data in the buffers are accessed. This
  * is done through accessors,
  * \code
  *          ... 0.0 1.0 1.0
  *       </float_array>
  *       <technique_common>
  *         <accessor source="#my_geometry_positions_array" count="...">
  *           <param name="X" type="float" />
  *           <param name="Y" type="float" />
  *           <param name="Z" type="float" />
  *         </accessor>
  *       </technique_common>
  *     </source>
  * \endcode
  * Here, the number of <param> elements give the dimension of the elements,
  * in this case, it is 3D positions. The name attribute is not required, but
  * may give hints to the appliaction.
  *
  * In the case we have interleaved buffers, we define one source per logical
  * set of input data, but only one has an input buffer. For example, assume
  * we have interleaved 3-component positions, a 3-component normal vector, and
  * a 4-component color. In this case, we use the offset and stride attributes
  * and reference the same buffer,
  * \code
  *   <geometry id="my_geometry">
  *     <source id="my_geometry_positions">
  *       <float_array id="my_geometry_positions_array" count="..."> ... </float_array>
  *         <accessor source="#my_geometry_positions_array" count="..." offset="0" stride="10">
  *           <param name="X" type="float" />
  *           <param name="Y" type="float" />
  *           <param name="Z" type="float" />
  *         </accessor>
  *       </technique_common>
  *     </source>
  *     <source id="my_geometry_normals">
  *         <accessor source="#my_geometry_positions_array" count="..." offset="3" stride="10">
  *           <param name="X" type="float" />
  *           <param name="Y" type="float" />
  *           <param name="Z" type="float" />
  *         </accessor>
  *       </technique_common>
  *     </source>
  *     <source id="my_geometry_colors">
  *         <accessor source="#my_geometry_positions_array" count="..." offset="6" stride="10">
  *           <param name="R" type="float" />
  *           <param name="G" type="float" />
  *           <param name="B" type="float" />
  *           <param name="A" type="float" />
  *         </accessor>
  *       </technique_common>
  *     </source>
  * \endcode
  *
  * Given a set of accessors, we specify how these should be grouped into
  * vertices
  * \code
  *     <vertices>
  *       <input semantic="NORMAL" source="#my_geometry_normals"/>
  *       <input semantic="POSITION" source="#my_geometry_positions"/>
  *       <input semantic="COLOR" source="#my_geometry_colors"/>
  *     </vertices>
  * \endcode
  * The semantic attributes are used to match inputs to vertex attributes of
  * shaders.
  *
  * At runtime, we can directly set the accessor and the semantic in one go,
  * \code
  * Scene::Geometry* geo = m_db.library<Scene::Geometry>().add( "my_geometry" );
  * geo->setVertexSource( Scene::VERTEX_POSITION, // semantic
  *                       "my_positions_array",   // name of source buffer
  *                       3,                      // components
  *                       42,                     // count
  *                       0,                      // offset
  *                       10                      // stride
  * );
  * geo->setVertexSource( Scene::VERTEX_NORMAL,
  *                       "my_positions_array",
  *                       3, 42, 3, 10 );
  * geo->setVertexSource( Scene::VERTEX_COLOR,
  *                       "my_positions_array",
  *                       4, 42, 6, 10 );
  * \endcode
  *
  * \subsection geometry_primitives Step 3: Define primitives
  *
  * In the previous steps, we have set up a vertex stream where each vertex has
  * one to many elements, each with its own semantic label.
  *
  * The next step is to add primitives that connects these vertices. In its
  * simplest form, we directly draw from these arrays without any indexing. To
  * add a set of primitives in COLLADA, we add the appropriate tag in the mesh
  * tag,
  * \code
  *   <geometry ..>
  *     <mesh>
  *       <!-- set up vertex stream -->
  *       ...
  *       <!-- specify primitives -->
  *       <triangles count="10" material="foo" />
  *     </mesh>
  *   </geometry>
  * \endcode
  * At runtime, this amounts to
  * \code
  * geo->addPrimitiveSet( Scene::PRIMITIVE_TRIANGLES,
  *                       10,
  *                       "foo" );
  * \endcode
  * Currently is <points>, <lines>, <pathces>, <polylist>, and <triangles>
  * recognized. <patches> is not part of the COLLADA specification, and polylist
  * splits all polygons into triangles and quads.
  *
  * The material ('foo' in the example) is just a material symbol and not a
  * reference to a material. The material symbols are bound to an actual
  * material when the geometry is instantiated in a node, see \ref scene_graph_instancing .
  *
  * Primitives can also be indexed. In COLLADA, this is handled by adding a <p>
  * child to the primitive,
  * \code
  *       <triangles count="10" material="foo" />
  *         <p>0 1 2  0 4 1  0 2 6 ... </p>
  *       </triangles>
  * \endcode
  * Scene uses source buffers to hold index data, so the corresponding code is
  * \code
  * std::vector<int> index_data; // put indices into this
  * Scene::SourceBuffer* indices = m_db.library<Scene::SourceBuffer>().add( "my_indices" );
  * indices->contents( index_data );
  * geo->addPrimitiveSet( Scene::PRIMITIVE_TRIANGLES,
  *                       10,
  *                       "my_indices",
  *                       "foo" );
  * \endcode
  *
  * \section effects Effects and materials
  *
  * An effect describes a set of rendering state, and exports a set of
  * parameters (for example, diffuse color). A material is an instance of an
  * effect where the parameters can be set to particular values. Thus, one can
  * make one 'phong' effect and multiple materials like 'phong_red', 'phong_blue',
  * and so on.
  *
  * \subsection effect_overview Effect, profiles, techniques, and passes.
  *
  * An effect has a set of implementations for different rendering systems.
  * These implementations are called 'profiles'. For example, <profile_GLSL>
  * describes the GLSL implementation, while <profile_GLSL2> describes the
  * GLSL2 implementation.
  *
  * Each effect has one or more 'techniques'. A technique is just a variant of
  * an effect, for example, a high-detail and low-detail version of an effect,
  * or a GL 2.0 and a GL 4.0 version of the effect.
  *
  * Each technique contains a set of one or more 'passes'. The passes are
  * executed in order, allowing multi-pass effects.
  *
  * The skeleton for an effect looks like:
  * \code
  * <library_effects>
  *   <effect id="my_effect">
  *     <profile_GLSL>
  *       <technique sid="opengl2_0">
  *         <pass> ... </pass>
  *         <pass> ... </pass>
  *       </technique>
  *       <technique sid="opengl4_0">
  *         <pass> ... </pass>
  *       </technique>
  *     </profile_GLSL>
  *     <profile_GLES2>
  *       <technique sid="webgl">
  *         <pass> ... </pass>
  *       </technique>
  *       <technique sid="ios">
  *         <pass> ... </pass>
  *         <pass> ... </pass>
  *         <pass> ... </pass>
  *       </technique>
  *     </profile_GLES2>
  *     <profile_GLES>
  *       <technique sid="android">
  *         <pass> ... </pass>
  *         <pass> ... </pass>
  *       </technique>
  *     </profile_GLES>
  *   </effect>
  * </library_effects>
  * \endcode
  * Choosing a particular technique is handled by technique_hint in material,
  * see \ref materials .
  *
  * \subsection effect_parameters Parameters
  *
  * An effect define a set of parameters, which defines its interface to the
  * outside world. Parameters have a default value that can be overridden by
  * materials. In addition, a parameter can have a runtime semantic, which is
  * that the actual value for that parameter provided by the runtime system.
  * Runtime semantics are values that are usually not known until the render
  * list is processed, like the various transform matrices.
  *
  * An effect can define parameters at at two levels, 'globally' at the effect
  * level to provide a parameter for all profiles, and 'locally' inside the
  * profile tag to provide profile-specific parameters,
  * \code
  *   <effect>
  *     <newparam sid="modelview_matrix">                     <!-- Available on all profiles -->
  *       <semantic>MODELVIEW_MATRIX</semantic>               <!-- Pull modelview matrix from runtime system -->
  *       <float4x4>1 0 0 0 0 1 0 0 0 0 1 0 0 0 1</float4x4>  <!-- Type and default value -->
  *     </newparam>
  *     <profile_GLSL>
  *       <newparam sid="quux">                               <!-- Only available on GLSL profile -->
  *         <int>42</int>                                     <!-- Type and default value, no semantics -->
  *       </newparam>
  * \endcode
  *
  * \subsection effect_pass Passes
  *
  * A pass describes a set of GL state (e.g. enable depth testing), the shader
  * program to be employed, and optionally a render target. To actually use
  * the render target of a material, the effect must be invoked through the
  * instance_material pipeline (see \ref rendering).
  *
  * \subsubsection effect_pass_states States
  *
  * States describe the rendering state, corresponding to glEnable etc. when
  * programming directly against the GL API. A common use of multipass rendering
  * and states is hidden line rendering. First, the geometry is rendered using
  * filled primitives, with a slight z-offset, followed by rendering the
  * wireframe outline. This amounts to
  * \code
  *   <techniqe sid="hidden_line">
  *     <pass>
  *       <states>
  *         <depth_test_enable value="TRUE" />
  *         <polygon_mode value="FRONT_AND_BACK FILL" />
  *         <polygon_offset_fill_enable value="TRUE" />
  *         <polygon_offset value="1.0 1.0" />
  *       </states>
  *       <program> ... </program>
  *     </pass>
  *     <pass>
  *       <states>
  *         <depth_test_enable value="TRUE" />
  *         <polygon_mode value="FRONT_AND_BACK LINE" />
  *       </states>
  *       <program> ... </program>
  *     </pass>
  *   </technique>
  * \endcode
  *
  * If a value for a state is not given, the default value is used.
  *
  * \note The default value for depth_test_enable is FALSE, hence it is quite
  * common to at least need one item under the states tag.
  *
  * Scene does currently only support a subset of the state items, see Scene::StateType.
  *
  * \subsubsection effect_pass_program Shader programs
  *
  * Each profile that supports shaders can have a set of snippets of shader
  * source code defined common to all techniques. These are defined directly
  * below the profile tag, and has an sid-attribute which is used when setting
  * up the sources for a particular shader program. An example,
  * \code
  *   <effect id="foo">
  *     <profile_GLSL>
  *       <code sid="common_vertex_shader_source">                          <!-- inlined source code -->
  * attribute vec3 position;
  * uniform mat4 projection_modelview;
  * void
  * main()
  * {
  *     gl_Position = projection_modelview * vec4( position, 1.0 );
  * }
  *       </code>
  *       <include sid="fragment_shader_1_source" url="file://foo.glsl" />  <!-- shader source fetched from file -->
  *       <include sid="fragment_shader_2_source" url="file://bar.glsl" />  <!-- shader source fetched from file -->
  * \endcode
  * Combining these shader snippets into a shader for a shader stage is done
  * under the program tag,
  * \code
  *       <technique sid="default">
  *         <pass>
  *           <states> .. </states>
  *           <program>
  *             <shader stage="VERTEX">
  *               <sources>
  *                 <inline>#define FOOBAR 42</inline>                        <!-- Inline source code -->
  *                 <import ref="common_vertex_shader_source" />              <!-- Fetch from snippets defined in profile -->
  *               </sources>
  *             </shader>
  *             <shader stage="FRAGMENT">
  *               <sources>
  *                 <inline />                                                <!-- Empty inline (required, see text below) -->
  *                 <import ref="fragment_shader_1_source" />
  *               </sources>
  *             </shader>
  * \endcode
  * \note According to the COLLADA spec, all shader source listings must start with
  * at least one inline tag, so we add an empty tag even when it is not
  * necessary.
  *
  * A shader program can have a shader associated with each shader stage
  * (VERTEX, TESSELLATION_CONTROL, TESSELLATION_EVALUATION, GEOMETRY, and
  * FRAGMENT).
  *
  * The next step is to associate semantic values to the vertex input, so the
  * input from the geometry vertex stream is set up correctly,
  * \code
  *             ...
  *             </shader>
  *             <bind_attribute symbol="position"><semantic>POSITION</semantic></bind_attribute>
  * \endcode
  * If we have multiple sources in the vertex stream, we use multiple bind
  * attribute tags. The attribute 'symbol' refers to the attribute symbol in the
  * shader source code.
  *
  * The final step is to set the uniform values or bind them to parameters,
  * \code
  *             <bind_uniform symbol="foo">
  *                <float3>1 0 0</float>            <!-- directly set uniform value -->
  *             </bind_uniform>
  *             <bind_uniform symbol="projection_modelview">
  *               <param ref="some_parameter">      <!-- pull from parameter -->
  *             </bind_uniform>
  * \endcode
  *
  * \subsection materials Materials
  *
  * Materials are defined in COLLADA as
  * \code
  * <library_materials>
  *   <material id="phong_red">
  *     <instance_effect url="#phong_effect">
  *       <setparam ref="diffuse"><float3>1 0 0</float3></setparam>
  *     </instance_effect>
  *   </material>
  * </library_materials>
  * \endcode
  * The profile is usually given by the runtime system. In addition, the
  * runtime may provide a platform string, that specifies a subtype of the
  * profile. To match profile and platform to technique, we use the
  * technique_hint mechanism,
  * \code
  *     <instance_effect url="#phong_effect">
  *       <technique_hint profile="GL" platform="4.0" ref="openg4_0" />
  *       <technique_hint profile="GLES2" platform="webgl" ref="webgl" />
  *       <technique_hint platform="ios" ref="ios" />
  *       ...
  * \endcode
  *
  * \section scene_graph Setting up a scene graph
  *
  * Nodes are used to position entities in the scene. In a COLLADA file, nodes
  * exists in two places: In the node library and in the visual scenes. In
  * Scene, the nodes of a visual scene actually resides in the node library as
  * well, and the scene has a reference to the corresponding node.
  *
  * A visual scene can contain a set of nodes,
  * \code
  * <visual_scene>
  *   <node id="foo"> ... </node>
  *   <node id="bar"> ... </node>
  * </visual_scene>
  * \endcode
  * In this example, the nodes 'foo' and 'bar' reside in the root node of the
  * visual scene. This graph is used for all rendering defined in the visual
  * scene.
  *
  * \subsection scene_graph_transforms Position and orientation
  *
  * A node contains an arbitrary number of transforms (lookat, matrix, rotate,
  * scale, skew, and translate) in arbitrary order. These transforms are applied
  * to all children and instanced elements of the node.
  *
  * \code
  * <node id="mynode">
  *   <translate sid="mytranslate">0 0 1</translate>
  *   <scale sid="myscale">0.5 0.5 0.5</scale>
  *   <translate sid="anothertranslate">5 0 0</translate>
  *   <instance_geometry>...</instance_geometry>
  *   <node>...</node>
  * </node>
  * \endcode
  * In the runtime, each transform has an index which can be retrieved by the
  * scoped identifier,
  * \code
  * Scene::Node* node = m_db.library<Scene::Node>().get( "mynode" );
  * size_t ix = node->transformIndexBySid( "mytranslate ); // = 0.
  * \endcode
  * A transform can be set to any other type of transform directly from the
  * runtime,
  * \code
  * node->transformSetLookAt( ix,
  *                           eye_x, eye_y, eye_z,
  *                           coi_x, coi_y, coi_z,
  *                           up_x, up_y, up_z );
  * \endcode
  *
  * \subsection scene_graph_instancing Populating the scene graph
  *
  * Nodes can instance elements like geometry and cameras, which places the
  * elements inside the node (inheriting the position and orientation of the
  * node). That is, neither geometries, cameras, lights, etc. have any
  * position or orientation beside the one given by the node that instances
  * that element.
  *
  * Instancing is just a reference to the id of that element, and elements can
  * be instanced any number of times inside the scene graph.
  *
  * Instancing of geometry also binds specific materials to the material symbols
  * defined when specifying the geometry, handled by the bind material tag.
  * Thus, a geometric shape can be instanced multiple times with different
  * materials.
  *
  * Nodes can also instance other nodes, and that is where the node library
  * comes into play. For example, the scene viewer creates a node-tree that
  * contains the application camera, and the visual scene can include this tree
  * in its scene-graph by instancing the root node. This basically opens up a
  * can of worms as a node might e.g. instance its instancee, creating a loop.
  * In such cases, the result is undefined (and scene might hang if the loop is
  * not catched).
  *
  * An example,
  * \code
  * <library_nodes>
  *   <node id="mycube">
  *     <translate sid="translate_000">0.0 0.0 1.0</translate>
  *     <instance_geometry url="cube">
  *       <bind_material>
  *         <technique_common>
  *           <instance_material symbol="default" target="phong_red" />
  *         </technique_common>
  *       </bind_material>
  *     </instance_geometry>
  *   </node>
  * </library_nodes>
  * <library_visual_scenes>
  *
  * </library_visual_scenes>
  * \endcode
  *
  * \subsection scene_graph_layers Layers
  *
  * A node can belong to one or more layers, specified by a space separated
  * string
  * \code
  * <node id="some_node" layer="background shadow" />
  * \endcode
  * If the layer attribute is empty, the node is considered to belong to all
  * layers.
  *
  * Render items of the evaluate scene item specify a filter that is used to
  * only process the nodes of a particular set of layers, e.g.,
  * \code
  * <visual_scene>
  *   ... nodes ...
  *   <evaluate_scene>
  *     <render>
  *       <layer>background</layer>
  *     </render>
  *     <render>
  *       <layer>foreground</layer>
  *       <layer>gizmos</layer>
  *     </render>
  *   </evaluate_scene>
  * \endcode
  * specifies that first, only the nodes that belong to the 'background ' layer
  * are rendered, then all nodes that either belong to the 'foreground' layer or
  * the 'gizmos' layer are rendered. If no layer filter is specified, all layers
  * are rendered.
  *
  * The rule used when traversing is as follows: If the node has no layers, it
  * is processed. If it does not match the filter, it and its subtree are
  * skipped. If it does match, the node and the full sub-tree (disregarding
  * which layers nodes in the subtree belongs to) is processed.
  *
  * For example,
  * \code
  * <visual_scene>
  *   <node id="foo" layer="background">
  *     <node id="bar" />
  *   </node>
  *   <node id="baz" layer="foreground">
  *     <node id="quux layer="background">
  *   </node>
  * \endcode
  * rendering the 'background' layer will render the nodes 'foo' and 'bar'.
  * Node 'foo' will match, and thus 'foo' and all children (='bar') will be
  * processed. Also, rendering 'foreground' will process 'baz' and 'quux',
  * even though 'quux' belongs to background. This is because 'baz' belongs to
  * foreground and 'quux' is in the subtree of 'baz'.
  *
  * \note This part of the COLLADA specification was slightly unclear, and this
  * behaviour might change if it conflicts with the intention of the COLLADA
  * specification.
  *
  * \note This part of the code is not particularly heavily tested, so be
  * warned. Report inconsistensies to maintainer.
  *
  * \section rendering Controlling the rendering
  *
  * TODO.
  *
  *
  * \section extensions Extensions
  *
  * \subsection include_file Including other COLLADA documents.
  *
  * COLLADA uses URLs to specify locations, and ids are fragments in documents.
  * To avoid including a http-client in scene, scene does not resolve locations
  * as urls, all locations references ids of appropriate type in the current
  * database.
  *
  * To allow splitting a COLLADA document into multiple documents easily, scene
  * includes an extension to automatically import a list of other documents.
  *
  * Example:
  * \code
  * <COLLADA>
  *   ...
  *   <extra>
  *     <technique profile="Scene">
  *       <include file="my_cool_effect.xml" />
  *       <include file="some_geometry.xml" />
  *     </technique>
  *   </extra>
  * </COLLADA>
  * \endcode
  * In this example, the files 'my_cool_effect.xml' and 'some_geometry.xml' are
  * automatically included when the document is parsed.
  *
  * \subsection primitive_override Overriding the default primitive type in a pass.
  *
  * This extension allows using a different primitive type in a rendering pass.
  * Example:
  * \code
  * <pass>
  *   ...
  *   <extra>
  *     <technique profile="Scene">
  *       <primitive_override source="TRIANGLES" target="PATCHES" vertices="3" />
  *     </technique>
  *    </extra>
  *  </pass>
  * \endcode
  * In this rendering pass, all triangles are replaced with the patch primitive
  * type. The vertices attribute is used to specify the number of vertices in
  * the patch primitive, it is not required for other primitive types.
  *
  * Another example:
  * \code
  * <pass>
  *   ...
  *   <extra>
  *     <technique profile="Scene">
  *       <primitive_override source="TRIANGLES" target="POINTS" numerator="3" />
  *     </technique>
  *   </extra>
  * </pass>
  * \endcode
  * In this rendering pass, all triangles are replaced with points. Since each
  * triangle produces three points, we must change the primitive count. This is
  * accomplised by using a fraction which is specified through 'numerator' and
  * 'denominator' attributes, which both defaults to one. So, in this example,
  * the triangle primitive count is multiplied with 3/1 to produce the point
  * count.
  *
  * \subsection node_profile Including a node only in specific profiles
  *
  * Sometimes it is useful to include a node subtree only in when evaluating the
  * scene using a GLSL profile. To do this, use the following scene-specific
  * extension:
  * \code
  * <node id="foobar">
  *   ...
  *   <extra>
  *     <technique profile="scene">
  *       <profile>GLSL</profile>
  *       <profile>GLES2</profile>
  *     </technique>
  *   </extra>
  * </node>
  * \endcode
  * In this example, the node 'foobar' is only evaluated when using either the
  * GLSL or GLES2 profile. It will not be evaluted for e.g. the GLES profile.
  *
  * The default action is to include the node in all profiles.
  *
  */
