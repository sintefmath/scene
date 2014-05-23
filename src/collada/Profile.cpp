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


#include "scene/Effect.hpp"
#include "scene/Profile.hpp"
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
Importer::parseProfile( Effect*     effect,
                        xmlNodePtr  profile_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseProfile" );

    Profile* profile = NULL;
    if( xmlStrEqual( profile_node->name, BAD_CAST "profile_BRIDGE" ) ) {
        SCENELOG_WARN( log, "Ignoring <profile_BRIDGE>" );
        return true;
    }
    else if( xmlStrEqual( profile_node->name, BAD_CAST "profile_CG" ) ) {
        SCENELOG_WARN( log, "Ignoring <profile_CG>" );
        return true;
    }
    else if( xmlStrEqual( profile_node->name, BAD_CAST "profile_GLES" ) ) {
        profile = effect->createProfile( PROFILE_GLES );
        if( profile == NULL ) {
            SCENELOG_ERROR( log, "Failed to create profile_GLES" );
            return false;
        }
    }
    else if( xmlStrEqual( profile_node->name, BAD_CAST "profile_GLES2" ) ) {
        profile = effect->createProfile( PROFILE_GLES2 );
        if( profile == NULL ) {
            SCENELOG_ERROR( log, "Failed to create profile_GLES2" );
            return false;
        }
    }
    else if( xmlStrEqual( profile_node->name, BAD_CAST "profile_GLSL" ) ) {
        profile = effect->createProfile( PROFILE_GLSL );
        if( profile == NULL ) {
            SCENELOG_ERROR( log, "Failed to create profile_GLSL" );
            return false;
        }
    }
    else if( xmlStrEqual( profile_node->name, BAD_CAST "profile_COMMON" ) ) {
        profile = effect->createProfile( PROFILE_COMMON );
        if( profile == NULL ) {
            SCENELOG_ERROR( log, "Failed to create profile_COMMON" );
            return false;
        }
    }
    else {
        SCENELOG_ERROR( log, "Unexpected node <" <<
                        reinterpret_cast<const char*>( profile_node->name ) << ">" );
        return false;
    }

    xmlNodePtr n = profile_node->children;
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "asset" ) ) {
        Asset asset;
        if(!parseAsset( asset, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <asset>");
            return false;
        }
        profile->setAsset( asset );
        n = n->next;
    }
    else {
        profile->setAsset( effect->asset() );
    }


    unordered_map<string,string> code_blocks;
    if( (profile->type() == PROFILE_CG) ||
        (profile->type() == PROFILE_GLSL ) ||
        (profile->type() == PROFILE_GLES2 ) )
    {
        // Nought to n <code> nodes
        for( ; n!= NULL && xmlStrEqual( n->name, BAD_CAST "code" ); n=n->next ) {
            const string sid = attribute( n, "sid" );
            if( sid.empty() ) {
                SCENELOG_ERROR( log, "Required <code> attribute 'sid'  empty." );
                return false;
            }
            if( code_blocks.find( sid ) != code_blocks.end() ) {
                SCENELOG_ERROR( log, "sid='" << sid << "' already defined" );
                return false;
            }
            code_blocks[ sid ] = getBody( n );
        }
        // Nough to n <include> nodes
        for( ; n!= NULL && xmlStrEqual( n->name, BAD_CAST "include" ); n=n->next ) {
            const string sid = attribute( n, "sid" );
            if( sid.empty() ) {
                SCENELOG_ERROR( log, "Required <include> attribute 'sid'  empty." );
                return false;
            }
            if( code_blocks.find( sid ) != code_blocks.end() ) {
                SCENELOG_ERROR( log, "sid='" << sid << "' already defined" );
                return false;
            }
            const string url = attribute( n, "url" );
            if( url.empty() ) {
                SCENELOG_ERROR( log, "Required <include> attribute 'sid' empty." );
                return false;
            }

            string body;
            if( !retrieveTextFile( body, url ) ) {
                SCENELOG_ERROR( log, "Failed to retrieve " << url );
                return false;
            }
            code_blocks[ sid ] = body;
        }
    }

    // Nought to n <newparam> nodes
    for( ; n!=NULL && strEq( n->name, "newparam"); n=n->next) {

        if( checkNode( n->children, "surface" ) ) {
            SCENELOG_WARN( log, "COLLADA 1.4-ism: newparam/surface construction deprecated in 1.5, handle in sampler2D/source" );
            continue;
        }

        Parameter p;
        if( parseNewParam( p, n, VALUE_CONTEXT_FX_NEWPARAM ) ) {
            profile->addParameter( p );
        }
        else {
            nagAboutParseError( log, n );
        }
    }

    // One to n <technique> nodes
    for( ; n!=NULL && xmlStrEqual( n->name, BAD_CAST "technique"); n=n->next) {
        if(!parseTechnique( profile, code_blocks, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <technique>" );
            return false;
        }
    }

    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );
    return true;
}


xmlNodePtr
Exporter::createProfile(Context &context, const Profile *profile) const
{
    if( profile == NULL ) {
        return NULL;
    }

    xmlNodePtr p_node = NULL;
    switch( profile->type() ) {
    case PROFILE_COMMON:
        p_node = newNode( NULL, "profile_COMMON" );
        break;
    case PROFILE_GLSL:
        p_node = newNode( NULL, "profile_GLSL" );
        break;
    case PROFILE_GLES:
        p_node = newNode( NULL, "profile_GLES" );
        break;
    case PROFILE_GLES2:
        p_node = newNode( NULL, "profile_GLES2" );
        break;
    default:
        return NULL;
    }

    xmlAddChild( p_node, createAsset( context, profile->effect()->asset(), profile->asset() ) );
    for( size_t i=0; i<profile->parameters(); i++ ) {
        const Parameter* p = profile->parameter(i);
        xmlAddChild( p_node, createNewParam( context,
                                             VALUE_CONTEXT_FX_NEWPARAM,
                                             p ) );
    }
    for( size_t i=0; i<profile->techniques(); i++ ) {
        xmlAddChild( p_node, createTechnique( context, profile->technique(i) ) );
    }



    return p_node;

//    return newNode( NULL, profile->type() )

}


/*

void
Profile::parseCodeNode( Program::CodeBlocks&  code_blocks,
                        xmlNodePtr            code_node )
{
    const string prefix = "Scene::Profile::parseCodeNode: ";

    string sid = XML::attribute( code_node, "sid" );
    if( sid.empty() ) {
        throw runtime_error( prefix + "<code> with empty sid attribute." );
    }
    if( code_blocks.find( sid ) != code_blocks.end() ) {
        throw runtime_error( prefix + "<code sid='"+sid+"'> already defined." );
    }
    code_blocks[sid] = XML::text( code_node );
}
void
Profile::parseProfileNode( xmlNodePtr profile_node )
{
    const string prefix = "Scene::Profile::parse: ";




    Program::CodeBlocks code_blocks;
    for( xmlNodePtr n=profile_node->children; n!=NULL; n=n->next) {

        if( xmlStrEqual( n->name, BAD_CAST "code" ) ) {
            if( m_profile_type & (PROFILE_GLSL|PROFILE_GLSL) ) {
                parseCodeNode( code_blocks, n );
            }
            else {
                throw runtime_error( prefix + "<code> not valid in this profile." );
            }
        }
        else if( xmlStrEqual( n->name, BAD_CAST "newparam" ) ) {
            m_parameters.push_back( new Parameter( ) );
            m_parameters.back()->parseNewparamNode( n );
        }
        else if( xmlStrEqual( n->name, BAD_CAST "technique" ) ) {
            Technique* technique = new Technique( m_db,
                                                  m_effect,
                                                  this );
            technique->parseTechniqueNode( code_blocks, n );
            m_techniques.push_back( technique );

        }


    }


}

void
Profile::addProfileNode( xmlNodePtr effect_node )
{
    Logger log = getLogger( "Scene.Profile.addProfileNode" );


    string profile_tag;

    switch( m_profile_type ) {
    case PROFILE_COMMON:  profile_tag = "profile_COMMON"; break;
    case PROFILE_GLES:  profile_tag = "profile_GLES"; break;
    case PROFILE_GLES2:  profile_tag = "profile_GLES2"; break;
    case PROFILE_GLSL:  profile_tag = "profile_GLSL"; break;
    default:
        throw runtime_error( "Illegal profile" );
    }


    xmlNodePtr profile_node = xmlNewChild( effect_node,
                                           NULL,
                                           BAD_CAST profile_tag.c_str(),
                                           NULL );
    for(auto p=m_parameters.begin(); p!=m_parameters.end(); ++p) {
        if( *p == NULL ) {
            SCENELOG_ERROR( log, "Intercepted null pointer" );
        }
        else {
            (*p)->addNewparamNode( profile_node );
        }
    }

    for(auto t=m_techniques.begin(); t!=m_techniques.end(); ++t ) {
        (*t)->addTechniqueNode( profile_node );
    }

}
*/



    } // of namespace XML
} // of namespace Scene
