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
