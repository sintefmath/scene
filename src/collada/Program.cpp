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

#include "scene/Profile.hpp"
#include "scene/Technique.hpp"
#include "scene/Pass.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;
        using std::unordered_map;

bool
Importer::parseProgram( Pass*                                pass,
                        const unordered_map<string,string>&  code_blocks,
                        xmlNodePtr                           program_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseProgram" );
    if(!assertNode( program_node, "program" ) ) {
        return false;
    }


    xmlNodePtr n = program_node->children;
    for( ; n!=NULL && xmlStrEqual( n->name, BAD_CAST "shader" ); n=n->next ) {
        ShaderStage stage = STAGE_N;
        const string stage_str = attribute( n, "stage" );
        if( stage_str == "VERTEX" ) {
            stage = STAGE_VERTEX;
        }
        else if( stage_str == "GEOMETRY" ) {
            stage = STAGE_GEOMETRY;
        }
        else if( stage_str == "TESSELLATION_CONTROL" ) {
            stage = STAGE_TESSELLATION_CONTROL;
        }
        else if( stage_str == "TESSELLATION_EVALUATION" ) {
            stage = STAGE_TESSELLATION_EVALUATION;
        }
        else if( stage_str == "FRAGMENT" ) {
            stage = STAGE_FRAGMENT;
        }
        else {
            SCENELOG_ERROR( log, "Unknown shader stage '" << stage << "'");
            return false;
        }

        string source;

        xmlNodePtr m = n->children;
        if( m != NULL && xmlStrEqual( m->name, BAD_CAST "sources" ) ) {
            xmlNodePtr o = m->children;
            for(; o!=NULL; o=o->next) {
                if( xmlStrEqual( o->name, BAD_CAST "inline" ) ) {
                    source += getBody( o );
                }
                else if( xmlStrEqual( o->name, BAD_CAST "import" ) ) {
                    const string ref = attribute( o, "ref" );
                    auto it = code_blocks.find( ref );
                    if( it == code_blocks.end() ) {
                        SCENELOG_ERROR( log, "Unable to resolve import ref '" << ref << "'." );
                        return false;
                    }
                    else {
                        source += it->second;
                    }
                }
                else {
                    break;
                }
            }
            nagAboutRemainingNodes( log, o );
            m = m->next;
        }
        else {
            SCENELOG_ERROR( log, "Required <shader> child <sources> missing." );
            return false;
        }
        pass->setShaderSource( stage, source );
    }

    for( ; n!=NULL && xmlStrEqual( n->name, BAD_CAST "bind_attribute" ); n=n->next ) {
        const string symbol = attribute( n, "symbol" );
        if( symbol.empty() ) {
            SCENELOG_ERROR( log, "Required <bind_attribute> attriubte 'symbol' empty" );
            return false;
        }
        if( n->children != NULL && xmlStrEqual( n->children->name, BAD_CAST "semantic" ) ) {
            const string semantic_str = getBody( n->children );
            VertexSemantic semantic = VERTEX_SEMANTIC_N;
            for(size_t i=0; i<VERTEX_SEMANTIC_N; i++) {
                if( m_vertex_semantics[i] == semantic_str ) {
                    semantic = (VertexSemantic)i;
                    break;
                }
            }
            if( semantic == VERTEX_SEMANTIC_N ) {
                SCENELOG_ERROR( log, "Unknown vertex semantic '" << semantic_str << "'");
                return false;
            }
            pass->setAttribute( semantic, symbol );
        }
        else {
            SCENELOG_ERROR( log, "Required <bind_attribute> child <semantic> missing." );
        }

    }

    for( ; n!=NULL && xmlStrEqual( n->name, BAD_CAST "bind_uniform" ); n=n->next ) {
        const string symbol = attribute( n, "symbol" );
        if( symbol.empty() ) {
            SCENELOG_ERROR( log, "Required <bind_uniform> attriubte 'symbol' empty" );
            return false;
        }

        xmlNodePtr m = n->children;
        if( m==NULL ) {
            SCENELOG_ERROR( log, "<bind_uniform> without children." );
            return false;
        }
        else if( xmlStrEqual( m->name, BAD_CAST "param" ) ) {
            const string ref = attribute( m, "ref" );
            if( ref.empty() ) {
                SCENELOG_ERROR( log, "Required <param> attribute 'ref' empty." );
                return false;
            }
            pass->setUniform( symbol, ref );
        }
        else {
            Value value;
            if( parseValue( &value, m, VALUE_CONTEXT_GLSL_GROUP ) ) {
                pass->setUniform( symbol, value );
            }
            else {
                SCENELOG_ERROR( log, "Failed to parse <param>" );
                return false;
            }
        }
    }

    nagAboutRemainingNodes( log, n );

    return true;
}

struct Stage {
    ShaderStage m_stage;
    std::string m_name;
};

static Stage stages[5] = {
    { STAGE_VERTEX,                  "VERTEX" },
    { STAGE_TESSELLATION_CONTROL,    "TESSELLATION_CONTROL" },
    { STAGE_TESSELLATION_EVALUATION, "TESSELLATION_EVALUATION" },
    { STAGE_GEOMETRY,                "GEOMETRY" },
    { STAGE_FRAGMENT,                "FRAGMENT" }
};

xmlNodePtr
Exporter::createProgram( Context& context, const Pass* pass ) const
{
    if( pass == NULL ) {
        return NULL;
    }
    ValueContext vc;
    switch( pass->technique()->profile()->type() ) {
    case PROFILE_GLSL:
        vc = VALUE_CONTEXT_GLSL_GROUP;
        break;
    case PROFILE_GLES2:
        vc = VALUE_CONTEXT_GLES2_GROUP;
        break;
    default:
        return NULL;
    }


    xmlNodePtr prg_node = newNode( NULL, "program" );
    for( unsigned int i=0; i<5; i++ ) {
        if( !pass->shaderSource( stages[i].m_stage ).empty() ) {
            xmlNodePtr stg_node = newChild( prg_node, NULL, "shader" );
            addProperty( stg_node, "stage", stages[i].m_name );
            xmlNodePtr src_node = newChild( stg_node, NULL, "sources" );
            newChild( src_node, NULL, "inline", pass->shaderSource( stages[i].m_stage ) );
        }
    }

    for( size_t i=0; i<pass->attributes(); i++ ) {
        xmlNodePtr ba_node = newChild( prg_node, NULL, "bind_attribute" );
        addProperty( ba_node, "symbol", pass->attributeSymbol(i) );
        newChild( ba_node, NULL, "semantic", m_vertex_semantics[ pass->attributeSemantic(i) ] );
    }


    for( size_t i=0; i<pass->uniforms(); i++ ) {
        xmlNodePtr bu_node = newChild( prg_node, NULL, "bind_uniform" );
        addProperty( bu_node, "symbol", pass->uniformSymbol(i) );
        if( pass->uniformIsParameterReference(i) ) {
            xmlNodePtr pr_node = newChild( bu_node, NULL, "param" );
            addProperty( pr_node, "ref", pass->uniformParameterReference(i));
        }
        else {
            xmlAddChild( bu_node, createValue( pass->uniformValue(i), vc ) );
        }
    }

    return prg_node;
}



    } // of namespace XML
} // of namespace Scene
