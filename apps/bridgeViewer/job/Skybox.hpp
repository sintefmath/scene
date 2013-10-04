#pragma once

class Skybox 
{
public:
    Skybox( const char* texture_base_path = nullptr );

    void init();
    void render( float* mvp, float* projection );

    ~Skybox();

private:
    void createCubeMap( );
    unsigned int m_vao;

    unsigned int m_texture;
    unsigned int m_shader_prog;

    const char* m_tex_base_path;

};
