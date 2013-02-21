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
