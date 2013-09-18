#include <sstream>
#include "scene/Log.hpp"
#include <boost/algorithm/string.hpp>
#include "scene/Asset.hpp"
#include "scene/Node.hpp"
#include "scene/DataBase.hpp"
#include "scene/VisualScene.hpp"
#include "scene/collada/Importer.hpp"
#include "scene/collada/Exporter.hpp"

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif



namespace Scene {
    namespace Collada {
        using std::string;
        using std::vector;

bool
Importer::parseNode( Node*           parent_node,
                     const Asset&    parent_asset,
                     xmlNodePtr      node_node )
{
    Logger log = getLogger( "Scene.XML.parseNode" );
    if( !assertNode( node_node, "node" ) ) {
        return false;
    }

    const string id = attribute( node_node, "id" );
    const string sid = attribute( node_node, "sid" );

    string context = "In node ";
    if( !id.empty() ) {
        context += "id='" + id + "', ";
    }
    else if( !sid.empty() ) {
        context += "sid='" + sid + "', ";
    }
    else {
        context += "<anonymous>, ";
    }

    Node* node = m_database.library<Node>().add( id );
    if( node == NULL ) {
        SCENELOG_ERROR( log, "Failed to create node '" << sid << "'." );
        return false;
    }
    if( !node->setParent( parent_node ) ) {
        SCENELOG_WARN( log, "Failed to set parent of node '" << sid << "'." );
        return false;
    }
    if( !sid.empty() ) {
        node->setSid( sid );
    }

    // A node might specify whihc layers in which it should be included.
    string layer = attribute( node_node, "layer" );
    if(!layer.empty() ) {
        vector<string> layers;
        boost::split( layers, layer, boost::is_any_of( "\t " ) );
        for( auto it=layers.begin(); it!=layers.end(); ++it ) {
            node->addToLayer( *it );
        }
    }

    Asset node_asset = parent_asset;

    
    // Parse child nodes. We are quite forgiving wrt the order of xml nodes, as
    // some exporters create illegal COLLADA.
    
    
    bool success = true;
    bool first_profile_include = true;

    
    for( xmlNodePtr n = node_node->children; n != NULL; n = n->next ) {
        if( checkNode( n, "asset" ) ) {
            Asset asset;
            if( parseAsset( asset, n ) ) {
                node_asset = asset;
            }
            else {
                SCENELOG_WARN( log, context << "failed to parse <asset>" );
            }
        }
        // --- <lookat> --------------------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "lookat" ) ) {
            SCENELOG_WARN( log, context << "<lookat> not implemented.");
        }
        // --- <matrix> --------------------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "matrix" ) ) {
            string sid = attribute( n, "sid" );
            std::vector<float> values(16);
            if( !parseBodyAsFloats( values, n, 16 ) ) {
                SCENELOG_ERROR( log, context << "malformed <matrix>, ignoring." );
                success = false;
            }
            else {
                if( !sid.empty() ) {
                    size_t ix = node->transformIndexBySid( sid );
                    if( ix != ~0u ) {
                        SCENELOG_WARN( log, context << "sid '" << sid << "' already defined." );
                        sid = "";
                    }
                }
                Value m = Value::createFloat4x4( values[0], values[1], values[2], values[3],
                                                 values[4], values[5], values[6], values[7],
                                                 values[8], values[9], values[10], values[11],
                                                 values[12], values[13], values[14], values[15] );
                node->transformSetMatrix( node->transformAdd(sid), m );
            }
        }
        // --- <rotate> --------------------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "rotate" ) ) {
            string sid = attribute( n, "sid" );
            std::vector<float> values(4);
            if( !parseBodyAsFloats( values, n, 4 ) ) {
                SCENELOG_ERROR( log, context << "malformed <rotate>, ignoring." );
                success = false;
            }
            else {
                if( !sid.empty() ) {
                    size_t ix = node->transformIndexBySid( sid );
                    if( ix != ~0u ) {
                        SCENELOG_WARN( log, context << "sid '" << sid << "' already defined." );
                        sid = "";
                    }
                }
                if( values[0]*values[0] + values[1]*values[1] + values[2]*values[2] < std::numeric_limits<float>::epsilon() ) {
                    SCENELOG_WARN( log, "Degenerate rotation axis, using x-axis." );
                    values[0] = 1.f;
                    values[1] = 0.f;
                    values[2] = 0.f;
                } 
                
                
                node->transformSetRotate( node->transformAdd( sid ),
                                          values[0],
                                          values[1],
                                          values[2],
                                          (M_PI/180.f)*values[3] );
            }        
        }
        // --- <scale> ---------------------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "scale" ) ) {
            string sid = attribute( n, "sid" );
            std::vector<float> values(4);
            if( !parseBodyAsFloats( values, n, 3 ) ) {
                SCENELOG_ERROR( log, context << "malformed <scale>, ignoring." );
                success = false;
            }
            else {
                if( !sid.empty() ) {
                    size_t ix = node->transformIndexBySid( sid );
                    if( ix != ~0u ) {
                        SCENELOG_WARN( log, context << "sid '" << sid << "' already defined." );
                        sid = "";
                    }
                }
                node->transformSetScale( node->transformAdd( sid ),
                                         values[0],
                                         values[1],
                                         values[2] );
            }
        }
        // --- <skew> ----------------------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "skew" ) ) {
            SCENELOG_WARN( log, context << "<skew> not implemented.");
        }
        // --- <translate> -----------------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "translate" ) ) {
            string sid = attribute( n, "sid" );
            std::vector<float> values(3);
            if( !parseBodyAsFloats(values, n, 3) ) {
                SCENELOG_ERROR( log, context << "malformed <translate>, ignoring." );
                success = false;
            }
            else {
                if( !sid.empty() ) {
                    size_t ix = node->transformIndexBySid( sid );
                    if( ix != ~0u ) {
                        SCENELOG_WARN( log, context << "sid '" << sid << "' already defined." );
                        sid = "";
                    }
                }
                node->transformSetTranslate( node->transformAdd( sid ),
                                             values[0],
                                             values[1],
                                             values[2] );
            }        }
        // --- <instance_camera> -----------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "instance_camera" ) ) {
            string sid = attribute( n, "sid" );
            string ref = cleanRef( attribute( n, "url" ) );
            if( !ref.empty() ) {
                node->addInstanceCamera( sid, ref );
            }        }
        // --- <instance_controller> -------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "instance_controller" ) ) {
            SCENELOG_WARN( log, context << "<instance_controller> not implemented" );
        }
        // --- <instance_geometry> ---------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "instance_geometry" ) ) {
            if( !parseInstanceGeometry( node, n ) ) {
                SCENELOG_ERROR( log, "Failed to parse <instance_geometry>" );
                success = false;
            }
        }
        // --- <instance_light> ------------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "instance_light" ) ) {
            string sid = attribute( n, "sid" );
            string ref = cleanRef( attribute( n, "url" ) );
            if( !ref.empty() ) {
                node->addInstanceLight( ref, sid );
            }
        }
        // --- <instance_node> -------------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "instance_node" ) ) {
            string sid = attribute( n, "sid" );
            string name = attribute( n, "name" );
            string url = attribute( n, "url" );
            if( url.size() > 0 && url[0] == '#' ) {
                url = url.substr(1);
            }
            string proxy = attribute( n, "proxy" );
            if( proxy.size() > 0 && proxy[0] == '#' ) {
                proxy = proxy.substr(1);
            }
    
            if( url.empty() ) {
                SCENELOG_ERROR( log, "Required <instance_node> attribute 'url' empty." );
                success = false;
            }
            else {
                node->addInstanceNode( sid, name, url, proxy );
            }
        }
        // --- <node> ----------------------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "node" ) ) {
            if( !parseNode( node, node_asset, n ) ) {
                SCENELOG_ERROR( log, "Failed to parse <node>" );
                success = false;
            }
        }
        // --- <extra> ---------------------------------------------------------
        else if( xmlStrEqual( n->name, BAD_CAST "extra" ) ) {
            xmlNodePtr m = n->children;
            if( checkNode( m, "asset" ) ) {
                // ignore
                m = m->next;
            }
            for( ; checkNode( m, "technique" ); m = m->next ) {
                const string profile = attribute( m, "profile" );
                if( profile == "Scene" || profile == "scene" ) {
                    xmlNodePtr o = m->children;
                    for( ; o != NULL; o = o->next ) {
                        if( checkNode( o, "profile" ) ) {
                            string profile = getBody(o);
                            if( first_profile_include ) {
                                node->includeInNoProfiles();
                                first_profile_include = false;
                            }
                            if( profile == "BRIDGE" ) {
                                node->includeInProfile( PROFILE_BRIDGE );
                            }
                            else if( profile == "CG" ) {
                                node->includeInProfile( PROFILE_CG );
                            }
                            else if( profile == "COMMON" ) {
                                node->includeInProfile( PROFILE_COMMON );
                            }
                            else if( profile == "GLES" ) {
                                node->includeInProfile( PROFILE_GLES );
                            }
                            else if( profile == "GLES2" ) {
                                node->includeInProfile( PROFILE_GLES2 );
                            }
                            else if( profile == "GLSL" ) {
                                node->includeInProfile( PROFILE_GLSL );
                            }
                            else {
                                SCENELOG_ERROR( log, "Profile specified '"
                                                << profile
                                                << "' not recognized." );
                            }
                        }
                    }
                }
            }
        }
        else {
            SCENELOG_WARN( log, context << "unexpected node <" << reinterpret_cast<const char*>( n->name ) << ">." );
        }
    }

    node->setAsset( node_asset );
    return success;
}

xmlNodePtr
Exporter::createNode( Context& context, const Node* node ) const
{

    if( node == NULL ) {
        return NULL;
    }
    Logger log = getLogger( "Scene.XML.Exporter.createNode["+node->debugString()+"]" );

    xmlNodePtr node_node = newNode( NULL, "node" );
    if( !node->id().empty() ) {
        addProperty( node_node, "id", node->id() );
    }
    if( !node->sid().empty() ) {
        addProperty( node_node, "sid", node->sid() );
    }
    // attribute 'layer', space-separated list of layers node belongs to
    std::string layers;
    for( size_t i=0; i<node->layers(); i++ ) {
        layers += (layers.empty()?"":" ") + node->layer(i);
    }
    if( !layers.empty() ) {
        addProperty( node_node, "layer", layers );
    }

    // list of transforms
    for( size_t i=0; i<node->transforms(); i++ ) {
        xmlNodePtr transform_node = NULL;
        switch( node->transformType(i) ) {
        case TRANSFORM_LOOKAT:
            if( node->transformValue(i).type() == VALUE_TYPE_FLOAT3X3 ) {
                const float* m = node->transformValue(i).floatData();
                std::stringstream o;
                o << m[0] << " " << m[3] << " " << m[6] << std::endl; // eye position
                o << m[1] << " " << m[4] << " " << m[7] << std::endl; // interest position
                o << m[2] << " " << m[5] << " " << m[8]; // up-vector
                transform_node = newChild( node_node, NULL, "lookat", o.str() );
            }
            else {
                SCENELOG_WARN( log, "lookat transform has value of type "
                               << ValueType(node->transformValue(i).type() ) );
            }
            break;
        case TRANSFORM_MATRIX:
            if( node->transformValue(i).type() == VALUE_TYPE_FLOAT4X4 ) {
                const float* m = node->transformValue(i).floatData();
                std::stringstream o;
                o << m[0] << " " << m[4] << " " << m[ 8] << " " << m[12] << std::endl;
                o << m[1] << " " << m[5] << " " << m[ 9] << " " << m[13] << std::endl;
                o << m[2] << " " << m[6] << " " << m[10] << " " << m[14] << std::endl;
                o << m[3] << " " << m[7] << " " << m[11] << " " << m[15];
                transform_node = newChild( node_node, NULL, "matrix", o.str() );
            }
            else {
                SCENELOG_WARN( log, "matrix transform has value of type "
                               << ValueType(node->transformValue(i).type() ) );
            }
            break;
        case TRANSFORM_ROTATE:
            if( node->transformValue(i).type() == VALUE_TYPE_FLOAT4 ) {
                std::stringstream o;
                const float* v = node->transformValue(i).floatData();
                o << v[0] << " " << v[1] << " " << v[2] << " " << ((180.f/M_PI)*v[3]);
                transform_node = newChild( node_node, NULL, "rotate", o.str() );
            }
            else {
                SCENELOG_WARN( log, "rotate transform has value of type "
                               << ValueType(node->transformValue(i).type() ) );
            }
            break;
        case TRANSFORM_SCALE:
            if( node->transformValue(i).type() == VALUE_TYPE_FLOAT3 ) {
                std::stringstream o;
                const float* v = node->transformValue(i).floatData();
                o << v[0] << " " << v[1] << " " << v[2];
                transform_node = newChild( node_node, NULL, "scale", o.str() );
            }
            else {
                SCENELOG_WARN( log, "scale transform has value of type "
                               << ValueType(node->transformValue(i).type() ) );
            }
            break;
        case TRANSFORM_SKEW:
            SCENELOG_ERROR( log, "skew transform not yet supported." );
            break;
        case TRANSFORM_TRANSLATE:
            if( node->transformValue(i).type() == VALUE_TYPE_FLOAT3 ) {
                std::stringstream o;
                const float* v = node->transformValue(i).floatData();
                o << v[0] << " " << v[1] << " " << v[2];
                transform_node = newChild( node_node, NULL, "translate", o.str() );
            }
            else {
                SCENELOG_WARN( log, "translate transform has value of type "
                               << ValueType(node->transformValue(i).type() ) );
            }
            break;
        case TRANSFORM_N:
            break;
        }
        if( (transform_node != NULL) && (!node->transformSid(i).empty()) ) {
            addProperty( transform_node, "sid", node->transformSid(i) );
        }
    }

    // instance camera
    for( size_t i=0; i<node->instanceCameras(); i++ ) {
        xmlNodePtr instance_node = newChild( node_node, NULL, "instance_camera" );
        addProperty( instance_node, "url", "#"+node->instanceCameraURL(i) );
    }

    // instance controller (not yet supported)

    // instance geometry
    for( size_t i=0; i<node->geometryInstances(); i++ ) {
        xmlAddChild( node_node, createInstanceGeometry( context, node->geometryInstance(i) ) );
    }

    // instance light
    for( size_t i=0; i<node->instanceLights(); i++ ) {
        xmlNodePtr instance_node = newChild( node_node, NULL, "instance_light" );
        if( !node->instanceLightSid( i ).empty() ) {
            addProperty( instance_node, "sid", node->instanceLightSid( i ) );
        }
        addProperty( instance_node, "url", "#" + node->instanceLightRef( i ) );
    }

    // instance node
    for( size_t i=0; i<node->instanceNodes(); i++ ) {
        xmlNodePtr instance_node = newChild( node_node, NULL, "instance_node" );
        addProperty( instance_node, "url", "#" + node->instanceNode(i) );
    }

    // add child nodes
    for( size_t i=0; i<node->children(); i++ ) {
        xmlAddChild( node_node, createNode( context, node->child(i) ) );
    }

    return node_node;
}


    } // of namespace XML
} // of namespace Scene
