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
