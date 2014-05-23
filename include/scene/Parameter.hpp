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

#include <string>
#include "scene/Scene.hpp"
#include "scene/Value.hpp"

namespace Scene {

class Parameter
{
public:

    Parameter();

    Parameter( const Parameter& p );

    ~Parameter();

    void
    set( const std::string&      sid,
        const RuntimeSemantic&  semantic,
        const Value&            value );


    const std::string&
    sid() const { return m_sid; }

    const RuntimeSemantic
    semantic() const { return m_semantic; }


    const Value*
    value() const;

    void
    setValue( const Value& value );

protected:
    std::string      m_sid;
    RuntimeSemantic  m_semantic;

    // Value is new'ed to make sure that a pointer to it remains valid.
    Value*           m_value;


};

} // of namespace Scene
