/* Copyright STIFTELSEN SINTEF 2014
 * 
 * This file is part of Scene.
 * 
 * Scene is free software: you can redistribute it and/or modifyit under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * Scene is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more
 * details.
 *  
 * You should have received a copy of the GNU Affero General Public License
 * along with the Scene.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "scene/Value.hpp"
#include "scene/Scene.hpp"
#include <scene/SeqPos.hpp>

namespace Scene {


/** Class encapsulating non-shader shading model.
  *
  * In a common technique, a generic shading model may be specified. The shading
  * model is composed on a type and a set of components.
  *
  * The type is either
  * - blinn (emission, ambient, diffuse, specular, shininess, reflective,
  *   reflectivity, transparent, transparency, index of refraction).
  * - constant (emission, reflective, reflectivity, transparent, transparency,
  *   index of refraction).
  * - lambert (emission, ambient, diffuse, reflective, reflectivity,
  *   transparent, transparency, index_of_refraction).
  * - phong (emission, ambient, diffuse, specular, shininess, reflective,
  *   reflectivity, transparent, transparency, index of refraction).
  *
  * The components can be either a specific value, a value taken from the effect
  * or profile parameters, or from a texture.
  *
  * The components can be:
  * - emission (color) Amount and color of light emitted from surface.
  * - ambient (color) Amount and color of ambient light re-emitted from surface.
  * - diffuse (color) Amount and color of diffuse light re-emitted from surface.
  * - specular (color) Color of light specularly reflected from surface.
  * - shininess (scalar) The roughness of the specular reflection lobe.
  * - reflective (color) Color of perfect mirror reflection.
  * - reflectivity (scalar) Amount of perfectly reflected light.
  * - transparent (color) Color of perfectly refracted light.
  * - transparency (scalar) Amount of perfectly refracted light.
  * - refindex (scalar) Refraction index for perfectly refracted light.
  *
  */
class CommonShadingModel : public StructureValueSequences
{
    friend class Technique;
public:

    const Technique*
    technique() const;

    ~CommonShadingModel();

    const ShadingModelType
    shadingModel() const;

    void
    setShadingModel( const ShadingModelType model );


    void
    clearComponentValue( const ShadingModelComponentType comp );

    void
    setComponentValue( const ShadingModelComponentType comp, const Value& value );

    void
    setComponentParameterReference( const ShadingModelComponentType comp, const std::string& reference );

    void
    setComponentImageReference( const ShadingModelComponentType comp, const std::string& reference );


    bool
    isComponentSet( const ShadingModelComponentType comp ) const;

    bool
    isComponentValue( const ShadingModelComponentType comp ) const;

    const Value*
    componentValue( const ShadingModelComponentType comp ) const;

    bool
    isComponentParameterReference( const ShadingModelComponentType comp ) const;

    const std::string&
    componentParameterReference( const ShadingModelComponentType comp ) const;

    bool
    isComponentImageReference( const ShadingModelComponentType comp ) const;

    const std::string&
    componentImageReference( const ShadingModelComponentType comp ) const;


protected:
    struct ShadingModelComponent {
        enum {
            NOT_SET,
            VALUE,
            PARAM_REF,
            IMAGE_REF
        }               m_state;
        Value*          m_value;
        std::string     m_reference;
    };

    DataBase&                      m_db;
    Effect*                        m_effect;
    Profile*                       m_profile;
    Technique*                     m_technique;
    ShadingModelType        m_model;
    ShadingModelComponent   m_components[ SHADING_COMP_COMP_N ];

    CommonShadingModel( DataBase& db,
                        Effect*    effect,
                        Profile*   profile,
                        Technique* technique );

};




}
