#include <boost/lexical_cast.hpp>
#include "scene/Effect.hpp"
#include "scene/Profile.hpp"
#include "scene/Technique.hpp"
#include "scene/Pass.hpp"
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;
        using std::unordered_map;


bool
Importer::parsePass( Technique* technique,
                     const std::unordered_map<std::string,std::string>& code_blocks,
                     xmlNodePtr pass_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parsePass" );


    const string sid = attribute( pass_node, "sid" );
    Pass* pass = technique->createPass( sid );
    if( pass == NULL ) {
        SCENELOG_ERROR( log, "Failed to create pass." );
        return false;
    }

    xmlNodePtr n = pass_node->children;
    if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "annotate" ) ) {
        SCENELOG_WARN( log, "Ignoring <annotate>" );
        n=n->next;
    }

    if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "states" ) ) {
        if( !parseStates( pass, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <states>" );
            return false;
        }
        n=n->next;
    }

    if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "program" ) ) {
        if(!parseProgram( pass, code_blocks, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <program>" );
            return false;
        }

        n=n->next;
    }

    if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "evaluate" ) ) {
        if( !parseEvaluate( pass, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <evaluate>" );
            return false;
        }
        n=n->next;
    }
    for( ; checkNode( n, "extra" ); n = n->next ) {
        xmlNodePtr m = n->children;
        if( checkNode( m, "asset" ) ) {
            // ignore
            m = m->next;
        }
        for( ; checkNode( m, "technique"); m= m->next) {
            const string profile = attribute( m, "profile" );
            if( profile == "Scene" || profile == "scene" ) {
                xmlNodePtr o = m->children;
                for( ; o!=NULL; o=o->next ) {
                    if( checkNode( o, "primitive_override" ) ) {
                        PrimitiveType src;
                        const string source = attribute( o, "source" );
                        if( source == "POINTS") {
                            src = PRIMITIVE_POINTS;
                        }
                        else if( source == "LINES" ) {
                            src = PRIMITIVE_LINES;
                        }
                        else if( source == "TRIANGLES" ) {
                            src = PRIMITIVE_TRIANGLES;
                        }
                        else if( source == "QUADS" ) {
                            src = PRIMITIVE_QUADS;
                        }
                        else if( source == "PATCHES" ) {
                            src = PRIMITIVE_PATCHES;
                        }
                        else {
                            SCENELOG_ERROR( log, "Illegal source type " << source );
                            continue;
                        }
                        unsigned int vertices;
                        PrimitiveType trg;
                        const string target = attribute( o, "target" );
                        if( target == "POINTS") {
                            trg = PRIMITIVE_POINTS;
                            vertices = 1;
                        }
                        else if( target == "LINES" ) {
                            trg = PRIMITIVE_LINES;
                            vertices = 2;
                        }
                        else if( target == "TRIANGLES" ) {
                            trg = PRIMITIVE_TRIANGLES;
                            vertices = 3;
                        }
                        else if( target == "QUADS" ) {
                            trg = PRIMITIVE_QUADS;
                            vertices = 4;
                        }
                        else if( target == "PATCHES" ) {
                            trg = PRIMITIVE_PATCHES;
                            string v = attribute( o, "vertices" );
                            if( v.empty() ) {
                                SCENELOG_ERROR( log, "vertices attribure required for PATCHES" );
                                continue;
                            }
                            vertices = boost::lexical_cast<unsigned int>( v );
                        }
                        else {
                            SCENELOG_ERROR( log, "Illegal target type " << target );
                            continue;
                        }

                        unsigned int num = 1;
                        string numerator = attribute( o, "numerator" );
                        if( !numerator.empty() ) {
                            num = boost::lexical_cast<unsigned int>( numerator );
                        }

                        unsigned int den = 1;
                        string denominator = attribute( o, "denominator" );
                        if( !denominator.empty() ) {
                            den = boost::lexical_cast<unsigned int>( denominator );
                        }
                        pass->setPrimitiveOverride( src, trg, vertices, num, den );
                    }
                }
            }
        }
    }
    nagAboutRemainingNodes( log, n );
    return true;
}

xmlNodePtr
Exporter::createPass( Context& context, const Pass* pass ) const
{
    if( pass == NULL ) {
        return NULL;
    }
    xmlNodePtr pass_node = newNode( NULL, "pass" );
    if( !pass->sid().empty() ) {
        addProperty( pass_node, "sid", pass->sid() );
    }

    xmlAddChild( pass_node, createStates( context, pass ) );
    if( (pass->technique()->profile()->type() == PROFILE_CG ) ||
            (pass->technique()->profile()->type() == PROFILE_GLES2 ) ||
            (pass->technique()->profile()->type() == PROFILE_GLSL ) )
    {
        xmlAddChild( pass_node, createProgram( context, pass ) );
    }
    xmlAddChild( pass_node, createEvaluate( context, pass ) );


    xmlNodePtr extra_tech_node = NULL;
    for( int i=0; i<PRIMITIVE_N; i++ ) {
        PrimitiveType sp = static_cast<PrimitiveType>( i );

        if( pass->primitiveOverride( sp ) ) {
            if( extra_tech_node == NULL ) {
                xmlNodePtr extra_node = NULL;
                extra_node = newChild( pass_node, NULL, "extra" );
                extra_tech_node = newChild( extra_node, NULL, "technique" );
                addProperty( extra_tech_node, "profile", "Scene" );
            }
            xmlNodePtr po_node = newChild( extra_tech_node, NULL, "primitive_override" );
            switch( sp ) {
            case PRIMITIVE_POINTS:
                addProperty( po_node, "source", "POINTS" );
                break;
            case PRIMITIVE_LINES:
                addProperty( po_node, "source", "LINES" );
                break;
            case PRIMITIVE_TRIANGLES:
                addProperty( po_node, "source", "TRIANGLES" );
                break;
            case PRIMITIVE_QUADS:
                addProperty( po_node, "source", "QUADS" );
                break;
            case PRIMITIVE_PATCHES:
                addProperty( po_node, "source", "PATCHES" );
                break;
            case PRIMITIVE_N:
                break;
            }
            switch( pass->primitiveOverrideType( sp ) ) {
            case PRIMITIVE_POINTS:
                addProperty( po_node, "target", "POINTS" );
                break;
            case PRIMITIVE_LINES:
                addProperty( po_node, "target", "LINES" );
                break;
            case PRIMITIVE_TRIANGLES:
                addProperty( po_node, "target", "TRIANGLES" );
                break;
            case PRIMITIVE_QUADS:
                addProperty( po_node, "target", "QUADS" );
                break;
            case PRIMITIVE_PATCHES:
                addProperty( po_node, "target", "PATCHES" );
                break;
            case PRIMITIVE_N:
                break;
            }
            if( pass->primitiveOverrideType( sp ) == PRIMITIVE_PATCHES ) {
                std::stringstream o;
                o << pass->primitiveOverrideVertices( sp );
                addProperty( po_node, "vertices", o.str() );
            }
        }
    }

    return pass_node;
}




    } // of namespace XML
} // of namespace Scene
