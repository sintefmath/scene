#include "scene/Render.hpp"

namespace Scene {
    using std::string;

Render::Render( EvaluateScene* evaluate_scene )
    : m_evaluate_scene( evaluate_scene )
{
}

const string&
Render::sid() const
{
    return m_sid;
}

void
Render::setSid( const string& sid )
{
    m_sid = sid;
}

const string&
Render::cameraNodeId() const
{
    return m_camera_node;
}

void
Render::setCameraNodeId( const string& id )
{
    m_camera_node = id;
}

size_t
Render::layers() const
{
    return m_layers.size();
}

const string&
Render::layer( size_t ix ) const
{
    return m_layers[ix];
}

void
Render::addLayer( const string& layer )
{
    m_layers.push_back( layer );
}

const string&
Render::instanceMaterialId() const
{
    return m_material_id;
}

void
Render::setInstanceMaterialId( const string& id )
{
    m_material_id = id;
}

const string&
Render::instanceMaterialTechniqueOverrideSid() const
{
    return m_tech_override_ref;
}

const string&
Render::instanceMaterialTechniqueOverridePassSid() const
{
    return m_tech_override_pass;
}

void
Render::setInstanceMaterialTechniqueOverride( const string& technique_sid,
                                              const string& pass_sid )
{
    m_tech_override_ref = technique_sid;
    m_tech_override_pass = pass_sid;
}

void
Render::setLightNodeId( size_t light_index, const string& id )
{
    m_light_nodes[ light_index ] = id;
}

const string&
Render::lightNodeId( size_t light_index ) const
{
    return m_light_nodes[ light_index ];
}


} // of namespace Scene
