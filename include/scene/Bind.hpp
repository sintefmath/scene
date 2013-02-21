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
