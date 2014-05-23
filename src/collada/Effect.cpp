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

bool
Importer::parseEffect( const Asset&  asset_parent,
                       xmlNodePtr    effect_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseEffect" );
    if(!assertNode( effect_node, "effect" ) ) {
        return false;
    }

    const string id = attribute( effect_node, "id" );
    if( id.empty() ) {
        SCENELOG_ERROR( log, "Required attribute 'id' empty" );
        return false;
    }

    Effect* effect = m_database.library<Effect>().add( id );
    if( effect == NULL ) {
        SCENELOG_ERROR( log, "Failed to create effect '" << id << '\'' );
        return false;
    }

    xmlNodePtr n = effect_node->children;
    if( n != NULL && xmlStrEqual( n->name, BAD_CAST "asset" ) ) {
        Asset asset;
        if(parseAsset( asset, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <asset>" );
            return false;
        }
        effect->setAsset( asset );
    }
    else {
        effect->setAsset( asset_parent );
    }

    while( n!=NULL && xmlStrEqual( n->name, BAD_CAST "annotate" ) ) {
        SCENELOG_DEBUG( log, "<annotate> ignored." );
        n = n->next;
    }
    while( n!=NULL && xmlStrEqual( n->name, BAD_CAST "newparam" ) ) {




        Parameter p;
        if(!parseNewParam( p, n, VALUE_CONTEXT_FX_NEWPARAM ) ) {
            SCENELOG_ERROR( log, "Failed to parse parameter" );
            return false;
        }
        effect->addParameter( p );

        n = n->next;
    }

    while( n!=NULL && (xmlStrEqual( n->name, BAD_CAST "profile_BRIDGE" ) ||
                       xmlStrEqual( n->name, BAD_CAST "profile_CG" ) ||
                       xmlStrEqual( n->name, BAD_CAST "profile_GLES" ) ||
                       xmlStrEqual( n->name, BAD_CAST "profile_GLES2" ) ||
                       xmlStrEqual( n->name, BAD_CAST "profile_GLSL" ) ||
                       xmlStrEqual( n->name, BAD_CAST "profile_COMMON" ) ) )
    {
        if(!parseProfile( effect, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse profile." );
            return false;
        }
        n = n->next;
    }

    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );

    return true;
}

xmlNodePtr
Exporter::createEffect( Context& context, const Effect* effect ) const
{
    if( effect == NULL ) {
        return NULL;
    }

    xmlNodePtr e_node = newNode( NULL, "effect" );
    addProperty( e_node, "id", effect->id() );

    for(size_t i=0; i<effect->parameters(); i++ ) {
        const Parameter* p = effect->parameter(i);
        xmlAddChild( e_node, createNewParam( context,
                                             VALUE_CONTEXT_FX_NEWPARAM,
                                             p ) );
    }
    if( (context.m_profile_mask & PROFILE_COMMON) != 0u ) {
        xmlAddChild( e_node, createProfile( context, effect->profile( PROFILE_COMMON ) ) );
    }
    if( (context.m_profile_mask & PROFILE_GLSL) != 0u ) {
        xmlAddChild( e_node, createProfile( context, effect->profile( PROFILE_GLSL ) ) );
    }
    if( (context.m_profile_mask & PROFILE_GLES) != 0u ) {
        xmlAddChild( e_node, createProfile( context, effect->profile( PROFILE_GLES ) ) );
    }
    if( (context.m_profile_mask & PROFILE_GLES2) != 0u ) {
        xmlAddChild( e_node, createProfile( context, effect->profile( PROFILE_GLES2 ) ) );
    }
    return e_node;
}



    } // of namespace XML
} // of namespace Scene
