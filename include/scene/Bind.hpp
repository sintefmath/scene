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

#include "scene/Scene.hpp"

namespace Scene {


/** Bind maps a runtime semantic to a sid.
  *
  * It is used in two ways. First, it can be used to assign runtime semantics
  * to a shader parameter. Example here is that the parameter "light" of the
  * shader should be bound to the runtime semantic LIGHT0.
  *
  * Second, it can be used to bind a sid from the node tree to a semantic, e.g.
  * telling the runtime that the position of lightnode should be used as the
  * runtime semantic LIGHT0.
  *
  */
struct Bind {
    RuntimeSemantic  m_semantic;
    std::string      m_target;
};


} // of namespace Scene
