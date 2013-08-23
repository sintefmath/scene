#pragma once

class Skybox 
{
public:
    Skybox( const char* texture_base_path = nullptr );

    void initSkybox();
    void renderSkybox();

    ~Skybox();

private:
    void loadSkyboxImages( unsigned int tex_id );
    unsigned int m_vao;
    unsigned int m_cubemap;
    const char* m_tex_base_path;

};
