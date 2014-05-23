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

#include <string>
#include <stdexcept>
#include "scene/Parameter.hpp"
#include "scene/Utils.hpp"
#include "scene/Log.hpp"

namespace Scene {
    using std::string;
    using std::runtime_error;

Parameter::Parameter()
    : m_semantic( RUNTIME_SEMANTIC_N ),
      m_value( new Value() )
{}

Parameter::Parameter( const Parameter& p )
    : m_sid( p.sid() ),
      m_semantic( p.semantic() ),
      m_value( new Value( *p.value() ) )
{
}

void
Parameter::set( const std::string&      sid,
                const RuntimeSemantic&  semantic,
                const Value&            value )
{
    m_sid = sid;
    m_semantic = semantic;
    *m_value = value;
}



Parameter::~Parameter()
{
    delete m_value;
    DEADBEEF( m_value );
}

const Value*
Parameter::value() const
{
    return m_value;
}

void
Parameter::setValue( const Value& value )
{
    *m_value = value;
}



} // of namespace Scene
