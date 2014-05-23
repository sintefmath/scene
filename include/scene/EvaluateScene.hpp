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

#include <string>
#include <vector>
#include "scene/Scene.hpp"
#include "scene/Render.hpp"

namespace Scene {

/** Describes one way of rendering a scene using a set of rendering passes.
  *
  * The member variable m_render is a sequence of render passes that should
  * be applied.
  */
class EvaluateScene
{
    friend class VisualScene;
public:
    EvaluateScene( VisualScene* visual_scene );

    ~EvaluateScene();

    /** Get the visual scene that holds this object. */
    VisualScene*
    visualScene();

    /** Get the visual scene that holds this object. */
    const VisualScene*
    visualScene() const;

    /** Get the enabled/disabled state. */
    bool
    enabled() const;

    /** Set the enabled/disabled state. */
    void
    setEnabled( bool enabled );

    /** Get the id. */
    const std::string&
    id() const;

    /** Set the id. */
    void
    setId( const std::string& id );

    /** Get the scoped identifier. */
    const std::string&
    sid() const;

    /** Set the scoped identifier. */
    void
    setSid( const std::string& sid );

    /** Get the number of render items. */
    size_t
    renderItems() const;

    /** Get the render item at a given index. */
    Render*
    renderItem( size_t ix );

    const Render*
    renderItem( size_t ix ) const;

    /** Add a new render item at the end. */
    Render*
    addRenderItem();

protected:

    VisualScene*            m_visual_scene;
    bool                    m_enabled;
    std::string             m_id;      ///< Optional.
    std::string             m_sid;     ///< Optional.
    std::vector<Render*>    m_render;  ///< Set of render operations.

private:
    EvaluateScene();

};



} // of namespace Scene
