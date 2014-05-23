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

#include <sstream>
#include <scene/SeqPos.hpp>

namespace Scene {

size_t SeqPos::m_global_next_pos = 0u;

const std::string
SeqPos::string() const
{
    std::stringstream o;
    o << m_pos;
    return o.str();
}

const std::string
SeqPos::debugString() const
{
    std::stringstream o;
    o << "SeqPos[" << m_pos << ']';
    return o.str();
}


Identifiable::Id Identifiable::m_identity_pool = 1u;

const std::string
Identifiable::idString() const
{
    std::stringstream o;
    o << "Id[" << m_identity << ']';
    return o.str();
}

} // of namespace Scene
