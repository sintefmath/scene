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

#define CHECK_GL do { printGLError( __FILE__, __LINE__ ); } while(0)

namespace {

const GLint skybox_position_location = 8;
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
    colour = texture( skyboxCube, texCoords );\n      \
    colour.r = 1.0f;\n                            \
    colour.w = 1.0f;\n                                 \
    }\n";
    
const char* skybox_vs = "#version 330\n\
    \n#extension GL_ARB_explicit_uniform_location : require\n    \
    \n                                                \
    layout(location=8) in vec4 inPosition;\n          \
    \n                                                \
    layout(location=7) uniform mat4 modelView;\n                         \
    layout(location=6) uniform mat4 projection;\n                        \
    \n                                                \
    out vec3 texCoords;\n                             \
    \n                                                \
    void main(void){\n                                              \
    vec3 wPos = ( modelView * vec4(inPosition.xyz, 0.0f) ).xyz;\n      \
    gl_Position =  projection  * modelView * vec4(inPosition.xyz, 1.0 );\n       \
//    gl_Position = vec4(inPosition.xyz, 1.0 );\n       \
    \n                                                              \
    texCoords = wPos;\n                                       \
    }\n";
    

    /*
const float skybox_vertices[18] = {
    -100.0, -100.0, -100.0,
    100.0, -100.0, -50.0,
    100.0, 100.0, 100.0,

    100.0, 100.0, 100.0,
    -100.0, 100.0, -50.0,
    -100.0, -100.0, -100.0
};
    */

const float skybox_vertices[144] = { -1.0f, -1.0f, -1.0f, 0.0f, //front
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



bool checkShader( GLuint shaderID )
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

void checkLinkerStatus( GLuint programID )
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

void dumpSourceWithLineNumbers(const std::string &orig, std::ostream &output = std::cerr)
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

GLuint createSkyboxShader()
{

    GLuint vs, fs;
    GLuint shader_prog = glCreateProgram();
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

    glAttachShader( shader_prog, vs );
    glAttachShader( shader_prog, fs );

    glLinkProgram( shader_prog );
    checkLinkerStatus( shader_prog );

    //hopefully safe to flag for deletion now, since they are attached and linked.
    //spec says its safe, drivers not always complying. 
    glDeleteShader( vs );
    glDeleteShader( fs );
    return shader_prog;
}


inline void printGLError(std::string fname, int line)
{   
    size_t count = 0;
    GLenum error = glGetError();
    if( error != GL_NO_ERROR ) {
        std::stringstream s;
        s << fname << '@' << line << ": OpenGL error: "; 
        
        do {
            switch( error ) {
            case GL_INVALID_ENUM: s << "GL_INVALID_ENUM "; break;
            case GL_INVALID_VALUE: s << "GL_INVALID_VALUE "; break;
            case GL_INVALID_OPERATION: s << "GL_INVALID_OPERATION "; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: s << "GL_INVALID_FRAMEBUFFER_OPERATION "; break;
            case GL_OUT_OF_MEMORY: s << "GL_OUT_OF_MEMORY"; break;
            default:
                s << "0x" << std::hex << error << std::dec << " "; break;
            }
            error = glGetError();
            ++count;
        } while( error != GL_NO_ERROR && count < 10 );
        std::cerr <<  s.str() << std::endl;
    }
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
Skybox::init()
{

    //setup vao
    glGenVertexArrays( 1, &m_vao );
    glBindVertexArray( m_vao );
    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float) * 144, skybox_vertices, GL_STATIC_DRAW );
    glVertexAttribPointer( skybox_position_location, 4, GL_FLOAT, false, 0, nullptr );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );


    //setup cubemap
    createCubeMap();
    CHECK_GL;
    m_shader_prog = createSkyboxShader();
    CHECK_GL;
}

void
Skybox::render( float* mvp, float* p )
{
    glUseProgram( m_shader_prog );
    glBindVertexArray( m_vao );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, m_cubemap );
    CHECK_GL;
    glUniform1i( skybox_cube_map_location, 0 );
    glUniformMatrix4fv( skybox_mvp_location, 1, GL_FALSE, mvp );
    glUniformMatrix4fv( skybox_proj_location, 1, GL_FALSE, p );
    CHECK_GL;
    
    glDrawArrays( GL_TRIANGLES, 0, 6 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    CHECK_GL;
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
    glUseProgram( 0 );
    glBindVertexArray( 0 );
    CHECK_GL;
}

void
Skybox::createCubeMap( )
{
    glGenTextures( 1, &m_cubemap);
    glBindTexture( GL_TEXTURE_CUBE_MAP, m_cubemap );
    //    glBindTexture( GL_TEXTURE_CUBE_MAP, m_cubemap );
    //hardcoding location of skybox images for now.
    //would be nice to just embed them in .exe or something, but will not add qtResource for it now.
    //will need image loading lib for this, using stb_image  

    //names need to be skybox_[side].png, with side= [east, west, up, down, south, north]
    //ordering corresponding to GL_TEXTURE_CUBE_MAP_[side] ordering.
    int x,y,n;
    std::vector< std::string > img_names;
    img_names.push_back( std::string(m_tex_base_path) + "skybox_east.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + "skybox_west.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + "skybox_up.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + "skybox_down.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + "skybox_south.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + "skybox_north.jpg" );

    unsigned char* temp;
    int format;
    for(auto i = 0u; i < img_names.size(); ++i )
    {
        temp = stbi_load( img_names[i].c_str(), &x, &y, &n, 4 );
        format = GL_RGBA; //n == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, x, y, 0, GL_RGBA, GL_BYTE, (void*)temp );
        //        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, x, y, 0, GL_RGBA, GL_BYTE, (void*)temp );

    }

    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );

}

