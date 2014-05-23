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
#include <tinia/trell/IPCGLJobController.hpp>
#include "job/ViewerJob.hpp"


// Example args: --perf --renderlist /work/projects/scene/data/rubberducky.xml /work/projects/scene/data/rubberducky_ addon.xml
int
main( int argc, char** argv )
{
    Scene::initLogger( &argc, argv );

    std::list<std::string> files;
//    for(int i=1; i<argc; i++ ) {
//        files.push_back( argv[i] );
//    }
    std::cerr << "adding /tmp/bridgeTest.dae" << std::endl;
    files.push_back("/tmp/bridgeTest.dae");
    std::cerr << "added /tmp/bridgeTest.dae, will now load it" << std::endl;
    TiniaViewerJob job( files );
    std::cerr << "has added files to TiniaViewer job, something should happen"<<std::endl;
    tinia::trell::IPCGLJobController controller;
    controller.setJob( &job );
    std::cerr << "job added to controller, will now call run to enter mainloop"<<std::endl;
    //controller.addScript( resources::cameramanipulator );
    controller.run( argc, argv );
    return 0;
}
