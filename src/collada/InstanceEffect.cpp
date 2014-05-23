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


#include "scene/Material.hpp"
#include "scene/DataBase.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;

bool
Importer::parseInstanceEffect( Material* material,
                               xmlNodePtr instance_effect_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseInstanceEffect" );
    if( !assertNode( instance_effect_node, "instance_effect" ) ) {
        return false;
    }

    string effect_id = attribute( instance_effect_node, "url" );
    if( effect_id.size() > 0 && effect_id[0] == '#' ) {
        effect_id = effect_id.substr( 1 );
    }

    if( effect_id.empty() ) {
        SCENELOG_ERROR( log, "<instance_effect> with empty 'url' attribute." );
        return false;
    }

    SCENELOG_DEBUG( log, "effect_id='" << effect_id << "'" );

    material->setEffectId( effect_id );

    xmlNodePtr n = instance_effect_node->children;

    // Nought to many <technique_hint>
    while( n!=NULL && xmlStrEqual( n->name, BAD_CAST "technique_hint" ) ) {
        const string platform_str = attribute( n, "platform" );
        const string ref_str = attribute( n, "ref" );
        const string profile_str = attribute( n, "profile" );
        ProfileType profile = PROFILE_COMMON;
        if( profile_str == "BRIDGE" ) {
            profile = PROFILE_BRIDGE;
        }
        else if( profile_str == "CG" ) {
            profile = PROFILE_CG;
        }
        else if( profile_str == "GLES" ) {
            profile = PROFILE_GLES;
        }
        else if( profile_str == "GLES2" ) {
            profile = PROFILE_GLES2;
        }
        else  if( profile_str == "GLSL" ) {
            profile = PROFILE_GLSL;
        }

        material->addTechniqueHint( profile, platform_str, ref_str );

        //ignore
        n=n->next;
    }

    // Nought to many <setparam>
    while( n!=NULL && xmlStrEqual( n->name, BAD_CAST "setparam" ) ) {
        string ref = attribute( n, "ref" );
        if( ref.empty() ) {
            SCENELOG_ERROR( log, "<setparam> with empty 'ref' attribute." );
            return false;
        }

        if( n->children != NULL ) {
            Value value;
            if( parseValue( &value, n->children, VALUE_CONTEXT_FX_SETPARAM ) ) {
                material->setParam( ref, value );
            }
            else {
                SCENELOG_ERROR( log, "Failed to parse value of <setparam>." );
                return false;
            }
        }
        else {
            SCENELOG_ERROR( log, "Missing required value child of <setparam>" );
            return false;
        }

        n=n->next;
    }

    // Nought to many <extra>
    while( n!=NULL && xmlStrEqual( n->name, BAD_CAST "extra" ) ) {
        // ignore
        n=n->next;
    }

    // Nag if there is something left
    while( n != NULL ) {
        SCENELOG_WARN( log, "Unexpected node '"<<
                       reinterpret_cast<const char*>( n->name) <<
                       "'." );
        n = n->next;
    }
    return true;
}

xmlNodePtr
Exporter::createInstanceEffect( Context&  context,
                                const Scene::Material*    material ) const
{
    Logger log = getLogger( "Scene.XML.Exporter.createInstanceEffect" );

    xmlNodePtr ie_node = newNode( NULL, "instance_effect" );
    addProperty( ie_node, "url", "#"+material->effectId() );

    // 0..n <technique_hint>
    for(size_t i=0; i<material->techniqueHints(); i++ ) {
        ProfileType ptype;
        std::string profile, platform, ref;
        material->techniqueHint( ptype, platform, ref, i );
        if( ref.empty() ) {
            continue; // ref is required
        }
        switch( ptype ) {
        case PROFILE_BRIDGE: profile = "BRIDGE"; break;
        case PROFILE_CG:     profile = "CG"; break;
        case PROFILE_GLES:   profile = "GLES"; break;
        case PROFILE_GLES2:  profile = "GLES2"; break;
        case PROFILE_GLSL:   profile = "GLSL"; break;
        default:
            break;
        }

        xmlNodePtr th_node = newChild( ie_node, NULL, "technique_hint" );
        if( !platform.empty() ) {
            addProperty( th_node, "platform", platform );
        }
        addProperty( th_node, "ref", "#" + ref );
        if( !profile.empty() ) {
            addProperty( th_node, "profile", profile );
        }
    }

    // 0..n <setparam>
    for(size_t i=0; i<material->setParams(); i++ ) {
        xmlNodePtr sp_node = newChild( ie_node, NULL, "setparam" );
        addProperty( sp_node, "ref", material->setParamReference(i) );
        xmlAddChild( sp_node, createValue( material->setParamValue(i),
                                           VALUE_CONTEXT_FX_SETPARAM ) );
    }

    return ie_node;
}


    } // of namespace XML
} // of namespace Scene
