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
    uniform samplerCube skyboxTexture;\n                             \
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
bool isTop(void){\n                                                     \
    return ((gl_VertexID >= 2 && gl_VertexID <=4) || \n                 \
            (gl_VertexID >= 8 && gl_VertexID <=10) ||\n                 \
            (gl_VertexID >= 13 && gl_VertexID <= 15) ||\n               \
            (gl_VertexID >= 20 && gl_VertexID <= 22)\n                  \
            );\n                                                        \
}\n                                                                     \
\n                                                                      \
bool isRight(void){\n                                                   \
    return ((gl_VertexID >= 1 && gl_VertexID <= 3) || \n                \
            (gl_VertexID >= 6 && gl_VertexID <= 13) ||\n                \
            gl_VertexID == 17\n                                         \
            );\n                                                        \
}\n                                                                     \
bool isFront(void){\n                                                   \
    return ((gl_VertexID >= 0 && gl_VertexID <= 6) || \n                \
            (gl_VertexID == 10 || gl_VertexID == 11) ||\n               \
            (gl_VertexID >= 10 && gl_VertexID <= 21)\n                  \
            );\n                                                        \
}\n                                                                     \
\n                                                                      \
void main(void){\n                                                      \
    float x = isRight() ? 1.0f : -1.0f;\n                          \
    float y = isTop() ?   1.0f : -1.0f;\n                            \
    float z = isFront() ? 1.0f : -1.0f;\n                          \
\n                                                                      \
    mat4 mvI = modelView;                       \n                      \
    mvI[3][0] = 0.0f;                            \n                      \
    mvI[3][1] = 0.0f;                            \n                      \
    mvI[3][2] = 0.0f;                            \n                      \
    mvI = inverse(mvI);                         \n                      \
    vec4 pos = vec4( x, y, z, 1.0f );           \n                      \
    texCoords = (mvI * inverse( projection ) * pos).xyz;            \n  \
    gl_Position = pos;\n                                                \
}\n                                                                     \
";

