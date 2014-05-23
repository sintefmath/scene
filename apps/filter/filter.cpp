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

#include <list>
#include <iostream>

#include <libxml/tree.h>

//#include <log4cxx/logger.h>
//#include <log4cxx/helpers/properties.h>
//#include <log4cxx/basicconfigurator.h>
//#include <log4cxx/propertyconfigurator.h>
//#include <log4cxx/helpers/exception.h>

#include <scene/Log.hpp>
#include <scene/DataBase.hpp>
#include <scene/Geometry.hpp>
#include <scene/Node.hpp>
#include <scene/VisualScene.hpp>
#include <scene/collada/Importer.hpp>
#include <scene/collada/Exporter.hpp>
#ifdef SCENE_TINIA
#include <fstream>
#include <scene/tinia/Bridge.hpp>
#include <tinia/renderlist/XMLWriter.hpp>
#endif

// Do we need this? @jbt
#ifndef _MSC_VER
using std::tolower;
#endif

int
main( int argc, char** argv )
{
    Scene::initLogger( &argc, argv );

    std::string output_file;
    std::string output_renderlist;
    bool single_index = false;
    bool stats = false;

    bool lib_geometry     = true;
    bool lib_image        = true;
    bool lib_camera       = true;
    bool lib_light        = true;
    bool lib_effect       = true;
    bool lib_material     = true;
    bool lib_node         = true;
    bool lib_visual_scene = true;

    std::list<std::string> input;

    for( int i=1; i<argc; i++ ) {
        std::string param( argv[i] );
        if( (param == "-o") || (param == "--output" ) ) {
            if( (i+1) < argc ) {
                output_file = argv[i+1];
                i++;
                continue;
            }
        }
        else if( param == "--export-renderlist" ) {
            if( (i+1) < argc ) {
                output_renderlist = argv[i+1];
                i++;
                continue;
            }
        }
        else if( param == "--single-index" ) {
            single_index = true;
        }
        else if( param == "--stats" ) {
            stats = true;
        }
        else if( param == "--libs" ) {
            lib_geometry     = true;
            lib_image        = true;
            lib_camera       = true;
            lib_light        = true;
            lib_effect       = true;
            lib_material     = true;
            lib_node         = true;
            lib_visual_scene = true;
        }
        else if( param == "--no-libs" ) {
            lib_geometry     = false;
            lib_image        = false;
            lib_camera       = false;
            lib_light        = false;
            lib_effect       = false;
            lib_material     = false;
            lib_node         = false;
            lib_visual_scene = false;
        }
        else if( param == "--lib-geometry" ) {
            lib_geometry = true;
        }
        else if( param == "--no-lib-geometry" ) {
            lib_geometry = false;
        }
        else if( param == "--lib-image" ) {
            lib_image = true;
        }
        else if( param == "--no-lib-image" ) {
            lib_image = false;
        }
        else if( param == "--lib-camera" ) {
            lib_camera = true;
        }
        else if( param == "--no-lib-camera" ) {
            lib_camera = false;
        }
        else if( param == "--lib-light" ) {
            lib_light = true;
        }
        else if( param == "--no-lib-light" ) {
            lib_light = false;
        }
        else if( param == "--lib-effect" ) {
            lib_effect = true;
        }
        else if( param == "--no-lib-effect" ) {
            lib_effect = false;
        }
        else if( param == "--lib-material" ) {
            lib_material = true;
        }
        else if( param == "--no-lib-material" ) {
            lib_material = false;
        }
        else if( param == "--lib-node" ) {
            lib_node = true;
        }
        else if( param == "--no-lib-node" ) {
            lib_node = false;
        }
        else if( param == "--lib-visual-scene" ) {
            lib_visual_scene = true;
        }
        else if( param == "--no-lib-visual-scene" ) {
            lib_visual_scene = false;
        }
        else if ( (param == "-h") || (param == "--help") ) {
            std::cerr << argv[0] << " [options] [configfile.config] [-o output.dae] input.dae" << std::endl;
            std::cerr << "Options:" << std::endl;
            std::cerr << "  --loglevel level          Specify loglevel (trace, debug, info, warn, error, fatal)" << std::endl;
            std::cerr << "  --single-index            Convert multi-index geometry to single index." << std::endl;
            std::cerr << "  --export-renderlist file  Output renderlist" << std::endl;
            std::cerr << "  --stats                   Display statistics of imported data." << std::endl;
            std::cerr << "  --[no-]-libs              Enable/disable export of all libraries." << std::endl;
            std::cerr << "  --[no-]-lib-geometry      Enable/disable export of geometries." << std::endl;
            std::cerr << "  --[no-]-lib-image         Enable/disable export of images." << std::endl;
            std::cerr << "  --[no-]-lib-camera        Enable/disable export of cameras." << std::endl;
            std::cerr << "  --[no-]-lib-light         Enable/disable export of lights." << std::endl;
            std::cerr << "  --[no-]-lib-effect        Enable/disable export of effects." << std::endl;
            std::cerr << "  --[no-]-lib-material      Enable/disable export of materials." << std::endl;
            std::cerr << "  --[no-]-lib-visual-scene  Enable/disable export of visual scenes." << std::endl;
            exit( EXIT_SUCCESS );
        }
        else {
            std::string param( argv[i] );
            size_t dp = param.find_last_of( '.' );
            if( dp == std::string::npos ) {
                continue;
            }
            std::string suffix = param.substr( dp );
            for( size_t k=0; k<suffix.size(); k++ ) {
                suffix[k] = tolower( suffix[k] );
            }
            if( suffix == ".xml" ) {
                input.push_back( param );
            }
            else if( suffix == ".dae" ) {
                input.push_back( param );
            }
            else {
                std::cerr << "Unrecognized suffix '" << suffix << "'" << std::endl;
            }
        }
    }


    Scene::DataBase db;
    Scene::Collada::Importer importer( db );
    for( auto it=input.begin(); it!=input.end(); ++it ) {
        importer.parse( *it );
    }

    if( single_index ) {
        for( size_t i=0; i<db.library<Scene::Geometry>().size(); i++ ) {
            Scene::Geometry* g = db.library<Scene::Geometry>().get( i );
            if( g->hasSharedInputs() ) {
                std::cerr << "Converting geometry '" << g->id() << "\' from multi-index to single-index."  << std::endl;
                g->flatten();
            }
        }
    }

    if( stats ) {

        size_t primitive_count = 0;
        size_t primitive_sets = 0;
        for( size_t i=0; i<db.library<Scene::Geometry>().size(); i++ ) {
            const Scene::Geometry* geometry = db.library<Scene::Geometry>().get( i );
            for( size_t j=0; j<geometry->primitiveSets(); j++ ) {
                const Scene::Primitives* primitives = geometry->primitives(j);
                primitive_count += (primitives->vertexCount()/primitives->verticesPerPrimitive());
            }
            primitive_sets += geometry->primitiveSets();
        }

        size_t geometry_instances = 0;
        size_t node_instances = 0;
        for( size_t i=0; i<db.library<Scene::Node>().size(); i++ ) {
            const Scene::Node* node = db.library<Scene::Node>().get( i );
            geometry_instances += node->geometryInstances();
            node_instances += node->instanceNodes();
        }


        std::cout << "stats" << std::endl;
        std::cout << "+- geometries:            " << db.library<Scene::Geometry>().size() << std::endl;
        std::cout << "|  +- primitive sets:     " << primitive_sets << std::endl;
        std::cout << "|     +- primitive count: " << primitive_count << std::endl;
        std::cout << "+- nodes:                 " << db.library<Scene::Node>().size() << std::endl;
        std::cout << "|  +- geometry instances: " << geometry_instances << std::endl;
        std::cout << "|  +- node instances:     " << node_instances << std::endl;
    }

    if( !output_renderlist.empty() ) {
#ifdef SCENE_TINIA

        if( db.library<Scene::VisualScene>().size() == 0 ) {
                std::cerr << "No visual scenes." << std::endl;
         }
        else {
            std::ofstream out( output_renderlist );
            if( !out.is_open() ) {
                std::cerr << "Failed to open '" << output_renderlist << "' for writing." << std::endl;
            }
            else {
                Scene::Tinia::Bridge rt( db, Scene::PROFILE_GLES2, "WebGL" );
                rt.build( db.library<Scene::VisualScene>().get(0)->id() );
                out << tinia::renderlist::getUpdateXML( &rt.renderListDataBase(),
                                                        tinia::renderlist::ENCODING_PLAIN,
                                                        0 );
            }
        }


#else
        std::cerr << "Not built against Tinia, required for renderlist export." << std::endl;
#endif
    }


    if( !output_file.empty() ) {
        Scene::Collada::Exporter exporter( db );

        xmlDocPtr doc = xmlNewDoc( reinterpret_cast<const xmlChar*>( "1.0" ) );
        xmlDocSetRootElement( doc, exporter.create( lib_geometry,
                                                    lib_image,
                                                    lib_camera,
                                                    lib_light,
                                                    lib_effect,
                                                    lib_material,
                                                    lib_node,
                                                    lib_visual_scene ) );
        xmlSaveFormatFile( output_file.c_str(), doc, 1 );
    }

}
