#pragma once


const GLint skybox_position_location = 0;
const GLint skybox_cube_map_location = 5;
const GLint skybox_mvp_location = 7;
const GLint skybox_proj_location = 6;
  
const char* skybox_fs = "#version 330\n\
    \n#extension GL_ARB_explicit_uniform_location : require\n        \
    \n                                                \
    in vec3 texCoords;\n                              \
    \n                                                \
    layout(location=5) uniform samplerCube skyboxCube;\n                 \
    \n                                                \
    out vec4 colour;\n                                \
    \n                                                \
    void main(void)\n                                 \
    {\n                                               \
//    colour = texture( skyboxCube, texCoords );\n      \
    colour.r = 1.0f;\n                            \
    colour.g = 0.25f;\n                            \
    colour.b = 1.0f;\n                            \
    colour.w = 1.0f;\n                                 \
    }\n";
/*
    const char* skybox_vs = "\
#version 330\n                                          \
layout(location=0) in vec4 inPosition;                  \
void main(void)\n                                       \
{\n                                                     \
  //drawing single triangle to cover entire screen\n    \
  float z = inPosition.z;                               \
  if( gl_VertexID == 0 ) {\n                            \
    gl_Position = vec4( -1.0, -1.0, z, 1.0 );\n       \
  }\n                                                   \
  else if( gl_VertexID == 1) {\n                        \
    gl_Position = vec4( 1.0, -1.0, z, 1.0 );\n        \
  }\n                                                   \
  else if( gl_VertexID == 2) {\n                        \
    gl_Position = vec4( -1.0, 1.0, z, 1.0 );\n        \
  }else if( gl_VertexID == 3) {\n                       \
    gl_Position = vec4( 1.0, 1.0, z, 1.0 );\n         \
  }\n                                                   \
}\n                                                     \
";
*/

    /*
    const char* skybox_vs = "\
#version 330\n                                          \
void main(void)\n                                       \
{\n                                                     \
  //drawing single triangle to cover entire screen\n    \
  if( gl_VertexID == 0 ) {\n                            \
    gl_Position = vec4( -1.0, -1.0, 0.0, 1.0 );\n       \
  }\n                                                   \
  else if( gl_VertexID == 1) {\n                        \
    gl_Position = vec4( 1.0, -1.0, 0.0, 1.0 );\n        \
  }\n                                                   \
  else if( gl_VertexID == 2) {\n                        \
    gl_Position = vec4( -1.0, 1.0, 0.0, 1.0 );\n        \
  }else if( gl_VertexID == 3) {\n                       \
    gl_Position = vec4( 1.0, 1.0, 0.0, 1.0 );\n         \
  }\n                                                   \
}\n                                                     \
";
*/

    
const char* skybox_vs = "#version 330\n\
    \n#extension GL_ARB_explicit_uniform_location : require\n    \
    \n                                                \
    layout(location=0) in vec4 inPosition;\n          \
    \n                                                \
    layout(location=7) uniform mat4 modelView;\n                         \
    layout(location=6) uniform mat4 projection;\n                        \
    \n                                                \
    out vec3 texCoords;\n                             \
    \n                                                \
    void main(void){\n                                              \
    vec3 wPos = ( modelView * vec4(inPosition.xyz, 0.0f) ).xyz;\n      \
//gl_Position =  projection  * modelView * vec4(inPosition.xyz, 1.0 );\n       \
    gl_Position = vec4(inPosition.xyz, 1.0 );\n       \
    \n                                                              \
    texCoords = wPos;\n                                       \
    }\n";
        
