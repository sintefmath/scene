#pragma once


const GLint skybox_position_location = 0;
const GLint skybox_texture_location = 5;
const GLint skybox_mvp_location = 7;
const GLint skybox_proj_location = 6;
  
const char* skybox_fs = "#version 330\n\
    \n#extension GL_ARB_explicit_uniform_location : require\n        \
    \n                                                \
    in vec3 texCoords;\n                              \
    \n                                                \
//    uniform sampler2D skyboxTexture;\n                 \
    uniform samplerCube skyboxTexture;\n                 \
    \n                                                \
    out vec4 colour;\n                                \
    \n                                                \
    void main(void)\n                                 \
    {\n                                               \
//    colour = texture( skyboxTexture, vec2(10.0, 10.0)  );\n      \
    colour = texture( skyboxTexture, texCoords  );\n      \
//    colour.r = 1.0f;\n                            \
//    colour.g = 0.25f;\n                            \
    //colour.b += 0.5f;\n                            \
    colour.w = 1.0f;\n                                 \
    }\n";

const char* skybox_vs = "#version 330\n\
#extension GL_ARB_explicit_uniform_location : require\n\
\n\
layout(location=0) uniform vec3 bbMin;\n\
layout(location=1) uniform vec3 bbMax;\n\
layout(location=7) uniform mat4 modelView;\n\
layout(location=6) uniform mat4 projection;\n\
\n\
out vec3 texCoords;\n\
\n\
bool isTop(void){\n\
    return ((gl_VertexID >= 2 && gl_VertexID <=4) || \n\
            (gl_VertexID >= 8 && gl_VertexID <=10) ||\n\
            (gl_VertexID >= 13 && gl_VertexID <= 15) ||\n\
            (gl_VertexID >= 20 && gl_VertexID <= 22)\n\
            );\n\
}\n\
\n\
bool isRight(void){\n\
    return ((gl_VertexID >= 1 && gl_VertexID <= 3) || \n\
            (gl_VertexID >= 6 && gl_VertexID <= 13) ||\n\
            gl_VertexID == 17\n\
            );\n\
}\n\
bool isFront(void){\n\
    return ((gl_VertexID >= 0 && gl_VertexID <= 6) || \n\
            (gl_VertexID == 10 || gl_VertexID == 11) ||\n\
            (gl_VertexID >= 10 && gl_VertexID <= 21)\n\
            );\n\
}\n\
\n\
void main(void){\n\
    float x = isRight() ? bbMax.x : bbMin.x;\n\
    float y = isTop() ? bbMax.y : bbMin.y;\n\
    float z = isFront() ? bbMax.z : bbMin.z;\n\
\n\
    texCoords = vec3((x/2.0f)+0.5f, (y/2.0f)+0.5f, (z/2.0f)+0.5f);\n\
    gl_Position = vec4( x, y, z, 1.0f) ;\n\
}\n\
";

