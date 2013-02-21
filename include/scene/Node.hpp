#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include "scene/Value.hpp"
#include "scene/Asset.hpp"
#include "scene/Scene.hpp"
#include "scene/Parameter.hpp"
#include "scene/Library.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {

/** Scene graph node.
  *
  * COLLADA allows nodes to appear in both the node library and in visual scenes.
  * In scene, the node hierarchy of a visual scene is put in the node library,
  * and the visual scene has a ref to the id of this node hierarchy.
  *
  * A node forms a hierarchy in the traditional sense by having other nodes as
  * children. In addition, nodes can instance other node hierarchies. In this
  * way, common parts of node hierarchies can be factored out.
  *
  * A node has the following properties:
  * - An id
  * - A sid
  * - A set of layer memberships. A node (with its children) are rendered if
  *   either no layer membership is set or one of the membership matches the
  *   layer to be rendered.
  *
  * In addition, a node can contain the following, applied in the given order:
  * - A sequence of transformations. This defines the local coordinate system
  *   of the node with respect to the parent node. For root nodes, the parent
  *   coordinate system is the world coordinate system.
  * - A sequence of camera instances. Places cameras in this node.
  * - A sequence of geometry instances. Places geometries in this node.
  * - A sequence of node intances. Places a node hierarchy inside this node by
  *   by reference.
  * - A sequence of child nodes.
  */
class Node : public StructureValueSequences
{
    friend class Library<Node>;
public:

    Library<Node>*
    libraryNodes() { return m_library_nodes; }


    const std::string
    debugString() const;

    /** Get asset information. */
    const Asset&
    asset() const { return m_asset; }

    Asset&
    asset() { return m_asset; }

    const std::string&
    id() const { return m_id; }

    const std::string&
    sid() const { return m_sid; }

    void
    setSid(  const std::string& sid );

    /** Set asset information. */
    void
    setAsset( const Asset& asset ) { m_asset = asset; }

    /** Add a geometry instance to this node. */
    void
    add( InstanceGeometry* instance_geometry );

    /** Return the number of geometry instances of this node. */
    const size_t
    geometryInstances() const;

    /** Return a particular geometry instance of this node. */
    const InstanceGeometry*
    geometryInstance( const size_t index ) const;

    /** \name API for which layers this node belongs to. */
    /** @{ */

    /** Include this node in the specified layer. */
    void
    addToLayer( const std::string& layer );

    /** Exclude this node from this layer if it already is included. */
    void
    excludeFromLayer( const std::string& layer );

    /** Get the number of layers that this node belongs to. */
    size_t
    layers() const { return m_layers.size(); }

    /** Get the name of the ix'th layer this node belongs to. */
    const std::string&
    layer( size_t ix ) const { return m_layers[ix]; }

    /** @} */


    void
    addInstanceCamera( const std::string& sid,
                       const std::string& url );

    size_t
    instanceCameras() const { return m_instance_camera.size(); }

    const std::string&
    instanceCameraURL( size_t ix ) const { return m_instance_camera[ix]; }

    void
    addInstanceNode( const std::string& sid,
                     const std::string& name,
                     const std::string& url,
                     const std::string& proxy );

    /** \name API to instance lights in the node */
    /** @{ */

    /** Get the number of lights instanced in this node. */
    size_t
    instanceLights() const;

    /** Add intancing of a light in this node. */
    void
    addInstanceLight( const std::string& ref, const std::string& sid="" );

    /** Look up the index of a light instancing by sid. */
    size_t
    instanceLightIndexBySid( const std::string& sid ) const;

    /** Get the reference to a instanced light. */
    const std::string&
    instanceLightRef( size_t index ) const;

    const std::string&
    instanceLightSid( size_t index ) const;


    /** @} */


    /** \name API to manipulate node transform. */
    /** @{ */
    /** Return the number of transforms in this node. */
    size_t
    transforms() const;

    /** Return the type of a particular transform in this node. */
    TransformType
    transformType( size_t ix ) const
    {
        return m_transforms_[ix].m_type;
    }

    const std::string&
    transformSid( size_t ix ) const
    {
        return m_transforms_[ix].m_sid;
    }

    const Value&
    transformValue( size_t ix ) const
    {
        return m_transforms_[ix].m_value;
    }

    /** Looks up the index of a transform from the scoped identifier.
      *
      * \returns The index of the transform or ~0u on failure.
      */
    size_t
    transformIndexBySid( const std::string& sid ) const;

    /** Adds a new transform with the given scoped identifier.
      *
      * \returns The index of the transform.
      */
    size_t
    transformAdd( const std::string& sid = "" );

    /** Set a transform to be a translation operation. */
    void
    transformSetTranslate( size_t ix, float x, float y, float z );

    /** Set a transform to be a scale operation. */
    void
    transformSetScale( size_t ix, float x, float y, float z );

    /** Set a transform to be a look-at operation. */
    void
    transformSetLookAt( size_t ix,
                        float eye_x, float eye_y, float eye_z,
                        float coi_x, float coi_y, float coi_z,
                        float up_x, float up_y, float up_z );

    /** Set a transform by directly setting the homogenenous transform matrix. */
    void
    transformSetMatrix( size_t ix, const Value& matrix );

    void
    transformSetRotate( size_t ix,
                        float axis_x, float axis_y, float axis_z, float angle_rad );

    /** @} */

    size_t
    instanceNodes() const;

    const std::string&
    instanceNode( size_t ix ) const;


    /** \name API to include or exclude nodes from profiles (non-standard COLLADA). */
    /** @{ */

    /** Returns the profile mask for this node. */
    const unsigned int
    profileMask() const;

    /** Clear the profile mask, node is never included. */
    void
    includeInNoProfiles();

    /** Clear the profile mask, node is always included. */
    void
    includeInAllProfiles();

    /** Updates the profile mask, node is included in that profile. */
    void
    includeInProfile( ProfileType profile );

    /** Updates the profile mask, node is included in that profile. */
    void
    excludeFromProfile( ProfileType profile );


    /** @} */


    const Node*
    parent() const;

    bool
    setParent( Node* node );

    size_t
    children() const;

    const Node*
    child( const size_t index ) const;



protected:
    Library<Node>*                        m_library_nodes;
    Asset                                 m_asset;
    std::string                           m_id;
    std::string                           m_sid;
    Node*                                 m_parent;
    struct Transform_ {
        TransformType                     m_type;
        std::string                       m_sid;
        Value                             m_value;
    };
    std::vector<Transform_>               m_transforms_;
    struct InstanceLight {
        std::string                       m_sid;
        std::string                       m_ref;
    };
    std::vector<InstanceLight>            m_light_instances;

    std::vector<Node*>                    m_children;
    std::vector<InstanceGeometry*>        m_instance_geometry;
    std::vector<std::string>              m_instance_node;
    std::vector<std::string>              m_instance_camera;
    std::vector<std::string>              m_layers;

    unsigned int                          m_profile_mask;

    Node( Library<Node>* library_nodes, const std::string& id );

    ~Node();


};



} // of namespace Scene
