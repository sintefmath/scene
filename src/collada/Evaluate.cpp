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

#include <boost/lexical_cast.hpp>
#include "scene/Log.hpp"
#include "scene/Pass.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;

static
size_t
parseCubeMapFace( const std::string& value )
{
    if( value == "POSITIVE_X" ) {
        return 0;
    }
    else if( value == "NEGATIVE_X" ) {
        return 1;
    }
    else if( value == "POSITIVE_Y" ) {
        return 2;
    }
    else if( value == "NEGATIVE_Y" ) {
        return 3;
    }
    else if( value == "POSITIVE_Z" ) {
        return 4;
    }
    else if( value == "NEGATIVE_Z" ) {
        return 5;
    }
    else {
        Logger log = getLogger( "Scene.XML.Import.parseCubeMapFace" );
        SCENELOG_ERROR( log, "Unknown face '" << value << "'.");
        return 0;
    }
}


bool
Importer::parseEvaluate( Pass*       pass,
                         xmlNodePtr  evaluate_node )
{
    Logger log = getLogger( "Scene.XML.Importer.parseEvaluate" );

    if(!assertNode( evaluate_node, "evaluate" ) ) {
        return false;
    }

    xmlNodePtr n = evaluate_node->children;
    for( ; n!=NULL; n=n->next) {
        RenderTarget target;
        if( xmlStrEqual( n->name, BAD_CAST "color_target" ) ) {
            target = RENDER_TARGET_COLOR;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "depth_target" ) ) {
            target = RENDER_TARGET_DEPTH;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "stencil_target" ) ) {
            target = RENDER_TARGET_STENCIL;
        }
        else {
            break;
        }

        // attribs
        size_t index = 0;
        const string index_str = attribute( n, "index" );
        if( !index_str.empty() ) {
            index = boost::lexical_cast<size_t>( index_str );
        }
        size_t slice = 0;
        const string slice_str = attribute( n, "slice" );
        const string face_str = attribute( n, "face" );
        if( !slice_str.empty() ) {
            slice = boost::lexical_cast<size_t>( slice_str );
        }
        else if( !face_str.empty() ) {
            slice = parseCubeMapFace( face_str );
        }
        size_t mip = 0;
        const string mip_str = attribute( n, "mip" );
        if(!mip_str.empty() ) {
            mip = boost::lexical_cast<size_t>( mip_str );
        }

        if( n->children == NULL ) {
            SCENELOG_ERROR( log, "render target requires eiter <param> or <instance_image> child." );
            return false;
        }
        else if( xmlStrEqual( n->children->name, BAD_CAST "param" ) ) {
            const string ref = attribute( n->children, "ref" );
            if( ref.empty() ) {
                SCENELOG_ERROR( log, "Required <param> attribute 'ref' empty." );
                return false;
            }
            pass->addRenderTarget( target, index, "", ref, slice, mip );
        }
        else if( xmlStrEqual( n->children->name, BAD_CAST "instance_image" ) ) {
            string url = attribute( n->children, "url" );
            if( url.size() > 0 && url[0] == '#' ) {
                url = url.substr(1);
            }
            if( url.empty() ) {
                SCENELOG_ERROR( log, "Required <instance_image> attribute 'url' empty." );
                return false;
            }
            pass->addRenderTarget( target, index, url, "", slice, mip );
        }
    }

    for( ; n!=NULL; n=n->next) {
        RenderTarget target;
        if( xmlStrEqual( n->name, BAD_CAST "color_clear" ) ) {
            target = RENDER_TARGET_COLOR;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "depth_clear" ) ) {
            target = RENDER_TARGET_DEPTH;
        }
        else if( xmlStrEqual( n->name, BAD_CAST "stencil_clear" ) ) {
            target = RENDER_TARGET_STENCIL;
        }
        else {
            break;
        }
        size_t index = 0;
        const string index_str = attribute( n, "index" );
        if( !index_str.empty() ) {
            index = boost::lexical_cast<size_t>( index_str );
        }
        pass->setRenderTargetClear( target, index, true );
    }

    if( n!=NULL && xmlStrEqual( n->name, BAD_CAST "draw" ) ) {
        const string body = getBody( n );
        if( body == "GEOMETRY" ) {
            pass->setDraw( DRAW_GEOMETRY );
        }
        else if( body == "SCENE_GEOMETRY" ) {
            pass->setDraw( DRAW_SCENE_GEOMETRY );
        }
        else if( body == "SCENE_IMAGE" ) {
            pass->setDraw( DRAW_SCENE_IMAGE );
        }
        else if( body == "FULL_SCREEN_QUAD" ) {
            pass->setDraw( DRAW_FULL_SCREEN_QUAD );
        }
        else {
            SCENELOG_ERROR( log, "Unrecognized draw '" << body << "'");
            return false;
        }
        n=n->next;
    }

    nagAboutRemainingNodes( log, n );

    return true;
}

struct Target {
    RenderTarget    m_target;
    std::string     m_name;
};

Target
target_order[3] =
{
    { RENDER_TARGET_COLOR,   "color_target" },
    { RENDER_TARGET_DEPTH,   "depth_target" },
    { RENDER_TARGET_STENCIL, "stencil_target" }
};

xmlNodePtr
Exporter::createEvaluate( Context& context, const Pass* pass ) const
{
    if( pass == NULL ) {
        return NULL;
    }
    if( (pass->renderTargets() == 0) && (pass->draw() == DRAW_GEOMETRY) ) {
        return NULL;
    }

    xmlNodePtr eval_node = newNode( NULL, "evaluate" );
    for( unsigned int j=0; j<3; j++ ) {
        for(size_t i=0; i<pass->renderTargets(); i++ ) {
            if( pass->renderTargetTarget(i) == target_order[j].m_target ) {

                std::stringstream tmp;
                xmlNodePtr trg_node = newChild( eval_node, NULL, target_order[j].m_name );

                tmp.str( "" );
                tmp << pass->renderTargetIndex(i);
                addProperty( trg_node, "index", tmp.str() );

                tmp.str( "" );
                tmp << pass->renderTargetSlice(i);
                addProperty( trg_node, "slice", tmp.str() );

                tmp.str( "" );
                tmp << pass->renderTargetMipLevel(i);
                addProperty( trg_node, "mip", tmp.str() );

                switch( pass->renderTargetFace(i) ) {
                case 0:
                    addProperty( trg_node, "face", "POSITIVE_X" );
                    break;
                case 1:
                    addProperty( trg_node, "face", "NEGATIVE_X" );
                    break;
                case 2:
                    addProperty( trg_node, "face", "POSITIVE_Y" );
                    break;
                case 3:
                    addProperty( trg_node, "face", "NEGATIVE_Y" );
                    break;
                case 4:
                    addProperty( trg_node, "face", "POSITIVE_Z" );
                    break;
                case 5:
                    addProperty( trg_node, "face", "NEGATIVE_Z" );
                    break;
                default:
                    break;
                }
                if( !pass->renderTargetParameterReference(i).empty() ) {
                    xmlNodePtr ref_node = newChild( trg_node, NULL, "param" );
                    addProperty( ref_node, "ref", pass->renderTargetParameterReference(i));
                }
                else {
                    xmlNodePtr img_node = newChild( trg_node, NULL, "instance_image" );
                    addProperty( img_node, "url", "#" + pass->renderTargetImageReference(i) );
                }
            }
        }
    }
    switch( pass->draw() ) {
    case DRAW_GEOMETRY:
        break;
    case DRAW_SCENE_GEOMETRY:
        newChild( eval_node, NULL, "draw", "SCENE_GEOMETRY" );
        break;
    case DRAW_SCENE_IMAGE:
        newChild( eval_node, NULL, "draw", "SCENE_IMAGE" );
        break;
    case DRAW_FULL_SCREEN_QUAD:
        newChild( eval_node, NULL, "draw", "FULL_SCREEN_QUAD" );
        break;
    }


    return eval_node;
}


    } // of namespace XML
} // of namespace Scene
