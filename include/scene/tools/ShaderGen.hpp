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

#include <scene/Scene.hpp>

namespace Scene {
    namespace Tools {

enum GenerateMode {
    GENERATE_ONE_TO_ONE
};


/*class ShaderGen
{
public:
    
    
    
private:
    ShaderGen();
    
    
};
*/
/** Generate shaders from profile_COMMON. */
bool
generateShaderFromCommon( Effect*             effect,
                          const int           profile_mask,
                          const GenerateMode  generate_mode = GENERATE_ONE_TO_ONE );

/** Generate shaders from profile_COMMON. */
void
generateShadersFromCommon( DataBase&           db,
                           const int           profile_mask,
                           const GenerateMode  generate_mode = GENERATE_ONE_TO_ONE );


    } // of namespace Tools
} // of namespace Scene
