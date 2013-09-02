#include "Skybox.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "stb_image.h"

namespace {

#include "Shaders.hpp"

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

    glDeleteShader( vs );
    glDeleteShader( fs );
    return shader_prog;
}
    
}//end anonymous namespace


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

    //setup empty vao
    glGenVertexArrays( 1, &m_vao );
    glBindVertexArray( m_vao );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    //setup cubemap
    createCubeMap();

    m_shader_prog = createSkyboxShader();
    fprintf(stderr, "finished skybox init");
}

void
Skybox::render( float* mv, float* p )
{
    glDisable( GL_DEPTH_TEST);
    glUseProgram( m_shader_prog );
    glBindVertexArray( m_vao );
    glUniform1i( skybox_texture_location, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, m_texture );

    glUniformMatrix4fv( skybox_mvp_location, 1, GL_FALSE, mv );
    glUniformMatrix4fv( skybox_proj_location, 1, GL_FALSE, p );
    glDrawArrays( GL_TRIANGLES, 0, 3 );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
    glUseProgram( 0 );
    glBindVertexArray( 0 );
    glEnable( GL_DEPTH_TEST );
}

void
Skybox::createCubeMap( )
{

    glGenTextures( 1, &m_texture);
    fprintf(stderr, "cube: tex name: %u\n", m_texture);
    glBindTexture( GL_TEXTURE_CUBE_MAP, m_texture );
    glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    //names need to be "[pos||neg][xyz].jpg", in order x, y, z
    //ordering corresponding to GL_TEXTURE_CUBE_MAP_[side] ordering.
    int x,y,n;
    std::vector< std::string > img_names;

    img_names.push_back( std::string(m_tex_base_path) + "posx.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + "negx.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + "posy.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + "negy.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + "posz.jpg" );
    img_names.push_back( std::string(m_tex_base_path) + "negz.jpg" );
	
    unsigned char* temp;
    int format;
    for(auto i = 0u; i < img_names.size(); ++i )
    {
        temp = stbi_load( img_names[i].c_str(), &x, &y, &n, 4 ); //forced to 4 components
        if(temp == NULL)
          fprintf(stderr, "temp couldn't load input %s, reason %s\n", img_names[i].c_str(), stbi_failure_reason() );
        format = GL_RGBA;
        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)temp );
    }
	
    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
    fprintf(stderr, "finished loading cubemap\n");
}

