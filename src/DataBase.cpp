#include <stdexcept>
#include <sstream>
#include <unordered_map>
#include "scene/DataBase.hpp"
#include "scene/Utils.hpp"
#include "scene/Geometry.hpp"
#include "scene/Primitives.hpp"
#include "scene/Material.hpp"
#include "scene/SourceBuffer.hpp"
#include "scene/Utils.hpp"
#include "scene/Log.hpp"

namespace Scene {
    using std::string;
    using std::vector;
    using std::stringstream;
    using std::runtime_error;
    using std::unordered_map;

static const string package = "Scene.DataBase";


DataBase::DataBase( DataBase* fallback )
: m_fallback( fallback )
{
    m_library_geometries.setDatabase( this );
    m_library_images.setDatabase( this );
    m_library_cameras.setDatabase( this );
    m_library_lights.setDatabase( this );
    m_library_effects.setDatabase( this );
    m_library_materials.setDatabase( this );
    m_library_nodes.setDatabase( this );
    m_library_source_buffers.setDatabase( this );
    m_library_visual_scenes.setDatabase( this );
}

template<> Library<Geometry>& DataBase::library() { return m_library_geometries; }
template<> Library<Image>& DataBase::library() { return m_library_images; }
template<> Library<Camera>& DataBase::library() { return m_library_cameras; }
template<> Library<Effect>& DataBase::library() { return m_library_effects; }
template<> Library<Light>& DataBase::library() { return m_library_lights; }
template<> Library<Material>& DataBase::library() { return m_library_materials; }
template<> Library<Node>& DataBase::library() { return m_library_nodes; }
template<> Library<SourceBuffer>& DataBase::library() { return m_library_source_buffers; }
template<> Library<VisualScene>& DataBase::library() { return m_library_visual_scenes; }

template<> const Library<Geometry>& DataBase::library() const { return m_library_geometries; }
template<> const Library<Image>& DataBase::library() const { return m_library_images; }
template<> const Library<Camera>& DataBase::library() const { return m_library_cameras; }
template<> const Library<Effect>& DataBase::library() const { return m_library_effects; }
template<> const Library<Light>& DataBase::library() const { return m_library_lights; }
template<> const Library<Material>& DataBase::library() const { return m_library_materials; }
template<> const Library<Node>& DataBase::library() const { return m_library_nodes; }
template<> const Library<SourceBuffer>& DataBase::library() const { return m_library_source_buffers; }
template<> const Library<VisualScene>& DataBase::library() const { return m_library_visual_scenes; }


DataBase::~DataBase()
{
}

void
DataBase::clear()
{
    m_library_geometries.clear();
    m_library_images.clear();
    m_library_cameras.clear();
    m_library_lights.clear();
    m_library_effects.clear();
    m_library_materials.clear();
    m_library_nodes.clear();
    m_library_source_buffers.clear();
    m_library_visual_scenes.clear();
    touchStructureChanged();
}


} // of namespace Scene


