#pragma once
#include <scene/Scene.hpp>

namespace Scene {
    namespace Tools {

enum GenerateMode {
    GENERATE_ONE_TO_ONE
};

bool
generateShaderFromCommon( Effect*             effect,
                          const ProfileType   profile_mask,
                          const GenerateMode  generate_mode = GENERATE_ONE_TO_ONE );

void
generateShadersFromCommon( DataBase&           db,
                           const ProfileType   profile_mask,
                           const GenerateMode  generate_mode = GENERATE_ONE_TO_ONE );


    } // of namespace Tools
} // of namespace Scene
