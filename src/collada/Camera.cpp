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

#include <sstream>
#include "scene/Log.hpp"
#include "scene/DataBase.hpp"
#include "scene/Library.hpp"
#include "scene/Camera.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

namespace Scene {
    namespace Collada {

bool
Importer::parseCamera( const Asset&  asset_parent,
                       xmlNodePtr    camera_node )
{
    Logger log = getLogger( "Scene.XML.parseCamera" );
    if( !assertNode( camera_node, "camera" ) ) {
        return false;
    }

    std::string id = attribute( camera_node, "id" );
    std::string name = attribute( camera_node, "name" );

    xmlNodePtr n = camera_node->children;

    // parse <asset> (optional)
    Asset asset;
    if( n != NULL && xmlStrEqual( n->name, BAD_CAST "asset" ) ) {
        if( !parseAsset( asset, n ) ) {
            SCENELOG_ERROR( log, "Failed to parse <asset>" );
            return false;
        }
        n = n->next;
    }
    else {
        asset = asset_parent;
    }


    // parse <optics> (required)
    if( n == NULL ) {
        SCENELOG_ERROR( log, "Expected <optics>, got NULL" );
        return false;
    }
    else if( !xmlStrEqual( n->name, BAD_CAST "optics" ) ) {
        SCENELOG_ERROR( log, "Expected <optics>, got " << n-> name );
        return false;
    }
    else {  // parse <optics>
        xmlNodePtr opt_n = n->children;
        if( opt_n == NULL ) {
            SCENELOG_ERROR( log, "Expected <optics>/<tecnique_common>, got NULL" );
            return false;
        }
        else if( !xmlStrEqual( opt_n->name, BAD_CAST "technique_common" ) ) {
            SCENELOG_ERROR( log, "Expected <optics>/<tecnique_common>, got " << opt_n->name );
            return false;
        }
        else {  // parse <optics>/<technique_comon>
            xmlNodePtr tc_n = opt_n->children;
            if( tc_n == NULL ) {
                SCENELOG_ERROR( log, "Expected <optics>/<technique_common>/<orthograpic|perspective>, got NULL" );
                return false;
            }
            else if( xmlStrEqual( tc_n->name, BAD_CAST "orthographic" ) ) {
                SCENELOG_DEBUG( log, "<camera>.... not yet fully implemented" << tc_n->name );
                tc_n = tc_n->next;
            }
            else if( xmlStrEqual( tc_n->name, BAD_CAST "perspective" ) ) {
                float xfov = 0.f;
                float yfov = 0.f;
                float aspect = 0.f;
                float znear = 0.f;
                float zfar = 0.f;
                bool has_xfov = false;
                bool has_yfov = false;
                bool has_aspect = false;
                bool has_znear = false;
                bool has_zfar = false;

                for( xmlNodePtr prsp_cn = tc_n->children; prsp_cn != NULL; prsp_cn = prsp_cn->next ) {
                    if( xmlStrEqual( prsp_cn->name, BAD_CAST "xfov" ) ) {
                        std::vector<float> v;
                        parseBodyAsFloats( v, prsp_cn, 1 );
                        if( v.size() >  0 ) {
                            xfov = v[0];
                            has_xfov = true;
                        }
                    }
                    else if( xmlStrEqual( prsp_cn->name, BAD_CAST "yfov" ) ) {
                        std::vector<float> v;
                        parseBodyAsFloats( v, prsp_cn, 1 );
                        if( v.size() >  0 ) {
                            yfov = v[0];
                            has_yfov = true;
                        }
                    }
                    else if( xmlStrEqual( prsp_cn->name, BAD_CAST "aspect_ratio" ) ) {
                        std::vector<float> v;
                        parseBodyAsFloats( v, prsp_cn, 1 );
                        if( v.size() >  0 ) {
                            aspect = v[0];
                            has_aspect = true;
                        }
                    }
                    else if( xmlStrEqual( prsp_cn->name, BAD_CAST "znear" ) ) {
                        std::vector<float> v;
                        parseBodyAsFloats( v, prsp_cn, 1 );
                        if( v.size() >  0 ) {
                            znear = v[0];
                            has_znear = true;
                        }
                    }
                    else if( xmlStrEqual( prsp_cn->name, BAD_CAST "zfar" ) ) {
                        std::vector<float> v;
                        parseBodyAsFloats( v, prsp_cn, 1 );
                        if( v.size() >  0 ) {
                            zfar = v[0];
                            has_zfar = true;
                        }
                    }
                    else {
                        SCENELOG_ERROR( log, "in <optics>/<technique_common>/<perspective>, unexpected node " << prsp_cn->name );
                    }
                }

                if( !has_znear ) {
                    SCENELOG_ERROR( log,  "in <optics>/<technique_common>/<perspective>, missing znear" );
                    return false;
                }
                if( !has_zfar ) {
                    SCENELOG_ERROR( log,  "in <optics>/<technique_common>/<perspective>, missing zfar" );
                    return false;
                }
                if( !has_xfov && !has_yfov ) {
                    SCENELOG_ERROR( log,  "in <optics>/<technique_common>/<perspective>, missing either xfov or yfov" );
                    return false;
                }
                if( (!has_xfov || !has_yfov) && !has_aspect ) {
                    if( !has_xfov ) {
                        xfov = yfov;
                    }
                    if( !has_yfov ) {
                        yfov = xfov;
                    }
                }
                else {
                    if( !has_xfov ) {
                        xfov = aspect * yfov;
                    }
                    if( !has_yfov ) {
                        yfov = xfov / aspect;
                    }
                }

                Camera* cam = m_database.library<Camera>().add( id );
                // TODO: add setasset, set name
                cam->setPerspective( xfov, yfov, znear, zfar );
                SCENELOG_TRACE( log, "Added perspective camera, id=" << id <<
                                ", xfov=" << xfov <<
                                ", yfov=" << yfov <<
                                ", near=" << znear <<
                                ", far=" << zfar );
                tc_n = tc_n->next;
            }
            else {
                SCENELOG_ERROR( log, "Expected <optics>/<technique_common>/<orthograpic|perspective>, got " << tc_n->name );
                return false;
            }
            nagAboutRemainingNodes( log, tc_n );

            opt_n = opt_n->next;
        }
        while( opt_n != NULL && xmlStrEqual( opt_n->name, BAD_CAST "technique" ) ) {
            SCENELOG_TRACE( log, "Ignoring technique subtree" );
            opt_n = opt_n->next;
        }
        ignoreExtraNodes( log, opt_n );
        nagAboutRemainingNodes( log, opt_n );
        n = n->next;
    }



    // parse <imager> (optional)
    if( n!= NULL && xmlStrEqual( n->name, BAD_CAST "imager" ) ) {

        n = n->next;
    }


    ignoreExtraNodes( log, n );
    nagAboutRemainingNodes( log, n );
    return true;
}

xmlNodePtr
Exporter::createCamera( Context& context, const Camera* camera ) const
{
    if( camera == NULL ) {
        return NULL;
    }
    xmlNodePtr cam_node = newNode( NULL, "camera" );
    xmlAddChild( cam_node, createAsset( context,
                                        m_database.library<Camera>().asset(),
                                        camera->asset() ) );
    if( !camera->id().empty() ) {
        addProperty( cam_node, "id", camera->id() );
    }
    xmlNodePtr opt_node = newChild( cam_node, NULL, "optics" );
    xmlNodePtr opt_tc_node = newChild( opt_node, NULL, "technique_common" );
    if( camera->cameraType() == CAMERA_PERSPECTIVE ) {
        std::stringstream tmp;
        xmlNodePtr persp_node = newChild( opt_tc_node, NULL, "perspective" );

        tmp.str("");
        tmp << camera->fovX();
        newChild( persp_node, NULL, "xfov", tmp.str() );

        tmp.str("");
        tmp << camera->fovY();
        newChild( persp_node, NULL, "yfov", tmp.str() );

        tmp.str("");
        tmp << camera->near();
        newChild( persp_node, NULL, "znear", tmp.str() );

        tmp.str("");
        tmp << camera->far();
        newChild( persp_node, NULL, "zfar", tmp.str() );
    }
    else if( camera->cameraType() == CAMERA_ORTHOGONAL ) {
        std::stringstream tmp;
        xmlNodePtr orto_node = newChild( opt_tc_node, NULL, "perspective" );

        tmp.str("");
        tmp << camera->magX();
        newChild( orto_node, NULL, "xmag", tmp.str() );

        tmp.str("");
        tmp << camera->magY();
        newChild( orto_node, NULL, "ymag", tmp.str() );

        tmp.str("");
        tmp << camera->near();
        newChild( orto_node, NULL, "znear", tmp.str() );

        tmp.str("");
        tmp << camera->far();
        newChild( orto_node, NULL, "zfar", tmp.str() );

    }
    else if( camera->cameraType() == CAMERA_CUSTOM_MATRIX ) {
        Logger log = getLogger( "Scene.XML.Exporter.createCamera" );
        SCENELOG_ERROR( log, "COLLADA doesn't support cameras with custom projection matrices" );
    }
    return cam_node;
}


    } // of namespace XML
} // of namespace Scene
