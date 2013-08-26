#include "Skybox.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

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


    static bool checkShader( GLuint shaderID )
    {
        GLint llength;
        GLint status;

        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &llength);
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
        if( status != GL_TRUE) {
            std::string infol;
            if(llength > 1) {
                GLchar* infoLog = new GLchar[llength];
                glGetShaderInfoLog(shaderID, llength, NULL, infoLog);
                infol = std::string( infoLog );
                delete[] infoLog;
            }
            else {
                infol = "no log.";
            }

            if( status != GL_TRUE ) {
                std::cerr << "Compilation returned errors." << std::endl;
            }
            std::cerr << infol << std::endl;

            return false;
        }
        return true;
    }

    static void checkLinkerStatus( GLuint programID )
    {
        GLint linkStatus;
        glGetProgramiv( programID, GL_LINK_STATUS, &linkStatus );
        if( linkStatus != GL_TRUE ) {

            std::string log;
	
            GLint logSize;
            glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &logSize );
            if( logSize > 0 ) {
                std::vector<GLchar> infoLog( logSize+1 );
                glGetProgramInfoLog( programID, logSize, NULL, &infoLog[0] );
                log = std::string( infoLog.begin(), infoLog.end() );
            }
            else {
                log = "Empty log.";
            }
            std::cerr << "Error linking program: " << std::endl;
            std::cerr << log << std::endl;
        }
    }

    static void dumpSourceWithLineNumbers(const std::string &orig, std::ostream &output = std::cerr)
    {
        std::stringstream input(orig);
        std::string temp;
        size_t lCounter = 1;
        while(std::getline(input, temp))
            {
                output << std::setw(3) << lCounter++ << ": ";
                output << temp << std::endl;
            }
        output.flush();
    }

    
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
    createCubeMap();
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

    GLuint vs, fs;
    m_shader_prog = glCreateProgram();
    vs = glCreateShader( GL_VERTEX_SHADER );
    fs = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( vs, 1, &skybox_vs, NULL );
    glShaderSource( fs, 1, &skybox_fs, NULL );

    glCompileShader( vs );
    glCompileShader( fs );
    if( !checkShader( vs ) ) {
        dumpSourceWithLineNumbers( skybox_vs );
    }
    
    if( !checkShader( fs ) ) {
        dumpSourceWithLineNumbers( skybox_fs );
    }

    glLinkProgram( m_shader_prog );
    checkLinkerStatus( m_shader_prog );

}

