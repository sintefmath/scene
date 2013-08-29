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
    
    void fillTexData( float* col, float* data )
{
    size_t size = 512 * 512 * 4;
    for( auto i = 0u; i < size-4; i+=4 ){
        data[i] =   col[0];
        data[i+1] = col[1];
        data[i+2] = col[2];
        data[i+3] = col[3];
    }
}    

GLuint createProceduralTexture()
{
    GLuint cbTex;
    glGenTextures( 1, &cbTex );
    //glBindTexture( GL_TEXTURE_2D, cbTex );
    glBindTexture( GL_TEXTURE_CUBE_MAP, cbTex );

    float col[4] = {1.0, 0.0, 0.0, 1.0};
    float* data = new float[512*512*4];
    fillTexData( col, data);
    // glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_FLOAT, (void*)data);
    glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_FLOAT, (void*)data);
    col[0] = 0.0f;
    col[1] = 1.0f;
    fillTexData( col, data );
    glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_FLOAT, (void*)data);
    col[1] = 0.0f;
    col[2] = 1.0f;
    fillTexData( col, data );
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_FLOAT, (void*)data);
    col[1] = 1.0f;
    fillTexData( col, data );
    glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_FLOAT, (void*)data);
    col[0] = 1.0f;
    fillTexData( col, data );
    glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_FLOAT, (void*)data);
    col[2] = 0.0f;
    fillTexData( col, data );
    glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_FLOAT, (void*)data);
    
    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
    
    delete data;
    return cbTex;
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
    //m_texture = createProceduralTexture();
    CHECK_GL;
    m_shader_prog = createSkyboxShader();
    m_tex_location = glGetUniformLocation( m_shader_prog, "skyboxTexture" );

    CHECK_GL;
}

void
Skybox::render( float* mvp, float* p, const float* bbMin, const float* bbMax )
{
    //    glDisable(GL_DEPTH_TEST);
    glUseProgram( m_shader_prog );
    glBindVertexArray( m_vao );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, m_texture );
    CHECK_GL;
    float bbmin[3] = { -1.0, -1.0, -1.0 };
    float bbmax[3] = { 1.0, 1.0, 1.0 };
    glUniform3fv( 0, 1, bbmin );
    glUniform3fv( 1, 1, bbmax );
    glUniformMatrix4fv( skybox_mvp_location, 1, GL_FALSE, mvp );
    glUniformMatrix4fv( skybox_proj_location, 1, GL_FALSE, p );
    CHECK_GL;
    glDrawArrays( GL_TRIANGLES, 0, 36 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    CHECK_GL;
    //glActiveTexture( GL_TEXTURE0 );
    //glBindTexture( GL_TEXTURE_2D, 0 );
    //    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
    glUseProgram( 0 );
    glBindVertexArray( 0 );
    CHECK_GL;

}

void
Skybox::createCubeMap( )
{
    glGenTextures( 1, &m_texture);
    glBindTexture( GL_TEXTURE_CUBE_MAP, m_texture );
    glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    //    glBindTexture( GL_TEXTURE_CUBE_MAP, m_texture );
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
        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)temp );
        //        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, x, y, 0, GL_RGBA, GL_BYTE, (void*)temp );
	
    }
	
    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );

}

