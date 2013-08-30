#pragma once


const GLint skybox_position_location = 0;
const GLint skybox_texture_location = 5;
const GLint skybox_mvp_location = 7;
const GLint skybox_proj_location = 6;

const char* skybox_fs = "#version 330\n\
    \n#extension GL_ARB_explicit_uniform_location : require\n        \
    \n                                                               \
    in vec3 texCoords;\n                                             \
    \n                                                               \
    layout(location=5) uniform samplerCube skyboxTexture;\n                             \
    \n                                                               \
    out vec4 colour;\n                                               \
    \n                                                               \
    void main(void)\n                                                \
    {\n                                                              \
       colour = texture( skyboxTexture, texCoords  );\n              \
    }\n";

const char* skybox_vs = "#version 330\n\
#extension GL_ARB_explicit_uniform_location : require\n                 \
\n                                                                      \
layout(location=7) uniform mat4 modelView;\n                            \
layout(location=6) uniform mat4 projection;\n                           \
\n                                                                      \
out vec3 texCoords;\n                                                   \
\n                                                                      \
void main(void){\n                                                      \
    float x = gl_VertexID == 0 ? -3.0f : 1.0f;\n                               \
    float y = gl_VertexID == 2 ? 3.0f : -1.0f;\n                               \
    float z = 1.0f;\n                                                   \
\n                                                                      \
    mat4 mvI = modelView;                       \n                      \
    mvI[3][0] = 0.0f;                            \n                      \
    mvI[3][1] = 0.0f;                            \n                      \
    mvI[3][2] = 0.0f;                            \n                      \
    mvI[3][3] = 1.0f;                            \n                      \
    mvI = inverse(mvI);                         \n                      \
    vec4 pos = vec4( x, y, z, 1.0f );           \n                      \
    texCoords = (mvI * inverse( projection ) * pos).xyz;            \n  \
    gl_Position = pos;\n                                                \
}\n                                                                     \
";

