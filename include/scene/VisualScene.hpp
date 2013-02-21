#pragma once

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include "scene/Asset.hpp"
#include "scene/Bind.hpp"
#include "scene/Node.hpp"
#include "scene/Scene.hpp"
#include "scene/EvaluateScene.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {

/**
  * A visual scene contains a node tree that is used for rendering. A node can
  * contain instance_node attributes, which includes any node with an id and
  * its children.
  *
  * Rendering of the visual scene is described using EvaluateScene objects,
  * which contains a set of Render items.
  */
class VisualScene : public StructureValueSequences
{
    friend class Library<VisualScene>;
public:

    ~VisualScene();

    const Asset&
    asset() const { return m_asset; }

    void
    setAsset( const Asset& asset ) { m_asset = asset; }

    const std::string&
    id() const { return m_id; }


    /** Returns the number of evaluate scenes for this scene. */
    size_t
    evaluateScenes() const { return m_evaluate_scene.size(); }

    /** Get the evaluate scene item of a particular index. */
    EvaluateScene*
    evaluateScene( size_t ix ) { return m_evaluate_scene[ix]; }

    /** Get the evaluate scene item of a particular index. */
    const EvaluateScene*
    evaluateScene( const size_t ix ) const { return m_evaluate_scene[ix]; }

    /** Add a new evaluate scene item. */
    EvaluateScene*
    addEvaluateScene();

    /** Get the id of the node in library<Node> that holds the nodes of this visual scene. */
    const std::string&
    nodesId() const { return m_nodes_id; }

    /** Set the id of the node in library<Node> that holds the nodes of this visual scene. */
    void
    setNodesId( const std::string& nodes_id ) { m_nodes_id = nodes_id; }

protected:
    std::string                     m_id;
    Asset                           m_asset;
    std::string                     m_nodes_id;
    std::vector<EvaluateScene*>     m_evaluate_scene;

    VisualScene( Library<VisualScene>* library_visual_scenes, const std::string& id );

};


} // of namespace Scene
