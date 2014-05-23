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

#include <string>
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;


const string Importer::m_vertex_semantics[ VERTEX_SEMANTIC_N ] =
{
    "POSITION",
    "COLOR",
    "NORMAL",
    "TEXCOORD",
    "TANGENT",
    "BINORMAL",
    "UV"
};

const string Exporter::m_vertex_semantics[ VERTEX_SEMANTIC_N ] =
{
    "POSITION",
    "COLOR",
    "NORMAL",
    "TEXCOORD",
    "TANGENT",
    "BINORMAL",
    "UV"
};



    }
}
