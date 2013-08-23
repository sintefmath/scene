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
