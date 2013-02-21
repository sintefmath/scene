#pragma once

#include <vector>
#include <unordered_map>
#include <list>
#include "scene/Bind.hpp"
#include "scene/Scene.hpp"

namespace Scene {


/** In a Node, specify geometry to instantiate. */
class InstanceGeometry
{
public:

    InstanceGeometry( const std::string geometry_url );

    const std::string&
    geometryId() const;

    void
    addMaterialBinding( const std::string& symbol, const std::string& target );

    const std::string&
    materialBindingTargetId( const std::string& symbol ) const;

    void
    addMaterialBindingBind( const std::string& symbol, const Bind& bind );

    const std::vector<Bind>&
    materialBindingBind( const std::string& symbol ) const;

    std::list<std::string>
    materialBindingSymbols() const;

protected:
    std::string                                      m_geometry_url;
    struct MaterialBinding
    {
        std::string m_symbol;
        std::string m_target_id;
        std::vector<Bind>  m_bind;
    };
    std::unordered_map<std::string,MaterialBinding>  m_material_bindings;

};





};
