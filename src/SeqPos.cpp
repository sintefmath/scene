#include <sstream>
#include <scene/SeqPos.hpp>

namespace Scene {

size_t SeqPos::m_global_next_pos = 0u;

const std::string
SeqPos::string() const
{
    std::stringstream o;
    o << m_pos;
    return o.str();
}

const std::string
SeqPos::debugString() const
{
    std::stringstream o;
    o << "SeqPos[" << m_pos << ']';
    return o.str();
}


Identifiable::Id Identifiable::m_identity_pool = 1u;

const std::string
Identifiable::idString() const
{
    std::stringstream o;
    o << "Id[" << m_identity << ']';
    return o.str();
}

} // of namespace Scene
