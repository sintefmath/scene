#include "Skybox.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>

#include "stb_image.h"

namespace {
    static const char* skybox_fs = "#version 420\n\
\n                                                \
in vec3 texCoords;\n                              \
\n                                                \
uniform samplerCube skyboxCube;\n                 \
\n                                                \
out vec4 colour;\n                                \
\n                                                \
void main(void)\n                                 \
{\n                                               \
colour = texture( skyboxCube, texCoords );\n      \
colour.r = 1.0;\n                                 \
colour.w = 1.0;\n                                 \
}\n";

    static const char* skybox_vs = "#version 420\n\
\n                                                \
layout(location=0) in vec3 inPosition;\n          \
\n                                                \
uniform mat4 modelView;\n                         \
uniform mat4 projection;\n                        \
\n                                                \
out vec3 texCoords;\n                             \
\n                                                \
void main(void){\n                                              \
vec3 wPos = ( modelView * vec4( inPosition, 0.0 ) ).xyz;\n      \
gl_Position = projection * modelView * vec4( inPosition, 1.0 );\n       \
\n                                                              \
texCoords = inPosition;\n                                       \
}\n";

 static const float skybox_vertices[144] = { -1.0f, -1.0f, -1.0f, 0.0f, //front
                                            1.0f, -1.0f, -1.0f, 0.0f,
                                            1.0f, 1.0f, -1.0f, 0.0f,
                                            1.0f, 1.0f, -1.0f, 0.0f,
                                            -1.0f, 1.0f, -1.0f, 0.0f,
                                            -1.0f, -1.0f, -1.0f, 0.0f,

                                            -1.0f, -1.0f, 1.0f, 0.0f, //back
                                            1.0f, -1.0f, 1.0f, 0.0f,
                                            1.0f, 1.0f, 1.0f, 0.0f,
                                            1.0f, 1.0f, 1.0f, 0.0f,
                                            -1.0f, 1.0f, 1.0f, 0.0f,
                                            -1.0f, -1.0f, 1.0f, 0.0f,

                                            -1.0f, -1.0f, -1.0f, 0.0f, //bottom
                                            1.0f, -1.0f, -1.0f, 0.0f,
                                            1.0f, -1.0f, 1.0f, 0.0f,
                                            1.0f, -1.0f, 1.0f, 0.0f,
                                            -1.0f, -1.0f, 1.0f, 0.0f,
                                            -1.0f, -1.0f, -1.0f, 0.0f,

                                            -1.0f, 1.0f, -1.0f, 0.0f, //top
                                            1.0f, 1.0f, -1.0f, 0.0f,
                                            1.0f, 1.0f, 1.0f, 0.0f,
                                            1.0f, 1.0f, 1.0f, 0.0f,
                                            -1.0f, 1.0f, 1.0f, 0.0f,
                                            -1.0f, 1.0f, -1.0f, 0.0f,

                                            -1.0f, -1.0f, -1.0f, 0.0f, //left
                                            -1.0f, 1.0f, -1.0f, 0.0f,
                                            -1.0f, 1.0f, 1.0f, 0.0f,
                                            -1.0f, 1.0f, 1.0f, 0.0f,
                                            -1.0f, -1.0f, 1.0f, 0.0f,
                                            -1.0f, -1.0f, -1.0f, 0.0f,
                                            
                                            1.0f, -1.0f, -1.0f, 0.0f, //right
                                            1.0f, 1.0f, -1.0f, 0.0f,
                                            1.0f, 1.0f, 1.0f, 0.0f,
                                            1.0f, 1.0f, 1.0f, 0.0f,
                                            1.0f, -1.0f, 1.0f, 0.0f,
                                            1.0f, -1.0f, -1.0f, 0.0f                                            
    };
  
}


Skybox::Skybox( const char* texture_base_path )
{
    if( texture_base_path != nullptr ){
        m_tex_base_path = texture_base_path;
    }else{
        m_tex_base_path = "/tmp/skybox/";
    }
    
}

Skybox::~Skybox()
{

}

void
Skybox::initSkybox()
{

    //setup vao
    glGenVertexArrays( 1, &m_vao );
    glBindVertexArray( m_vao );
    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float) * 144, skybox_vertices, GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 4, GL_FLOAT, false, 0, nullptr );
    glBindVertexArray( 0 );

    //read image files

    //setup cubemap
    loadSkyboxImages( );
}

void
Skybox::renderSkybox()
{

}

void
Skybox::createCubeMap( )
{
    glGenTextures( 1, &m_cubemap);
    glBindTexture( GL_TEXTURE_CUBE_MAP, m_cubemap );
    //hardcoding location of skybox images for now.
    //would be nice to just embed them in .exe or something, but will not add qtResource for it now.
    //will need image loading lib for this, using stb_image  

    //names need to be skybox_[side].png, with side= [east, west, up, down, south, north]
    //ordering corresponding to GL_TEXTURE_CUBE_MAP_[side] ordering.
    int x,y,n;
    std::vector< std::string > img_names;
    img_names.push_back( std::string(m_tex_base_path) + " skybox_east.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + " skybox_west.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + " skybox_up.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + " skybox_down.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + " skybox_south.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + " skybox_north.jpg" );

    unsigned char* temp;
    int format;
    for(auto i = 0u; i < img_names.size(); ++i )
    {
        temp = stbi_load( img_names[i].c_str(), &x, &y, &n, 4 );
        format = GL_RGBA; //n == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, x, y, 0, GL_RGBA, GL_FLOAT, (void*)temp );

    }

    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );    
}

