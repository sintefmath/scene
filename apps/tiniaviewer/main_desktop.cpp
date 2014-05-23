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
#include <scene/Log.hpp>
#include <tinia/qtcontroller/QTController.hpp>
#include "job/ViewerJob.hpp"

// Example args: --perf --renderlist /work/projects/scene/data/rubberducky.xml /work/projects/scene/data/rubberducky_addon.xml
int
main( int argc, char** argv )
{
    Scene::initLogger( &argc, argv );

    std::list<std::string> files;
    for(int i=1; i<argc; i++ ) {
        files.push_back( argv[i] );
    }

    TiniaViewerJob job( files );
    tinia::qtcontroller::QTController controller;
    controller.setJob( &job );
    //controller.addScript( resources::cameramanipulator );
    controller.run( argc, argv );
    return 0;
}
