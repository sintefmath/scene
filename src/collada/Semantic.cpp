#include <string>
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;


const string Importer::m_vertex_semantics[ VERTEX_SEMANTIC_N ] =
{
    "POSITION",
    "COLOR",
    "NORMAL",
    "TEXCOORD",
    "TANGENT",
    "BINORMAL",
    "UV"
};

const string Exporter::m_vertex_semantics[ VERTEX_SEMANTIC_N ] =
{
    "POSITION",
    "COLOR",
    "NORMAL",
    "TEXCOORD",
    "TANGENT",
    "BINORMAL",
    "UV"
};



    }
}
