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
    if(argc > 1){
    for(int i=1; i<argc; i++ ) {
      files.push_back( argv[i] );
    }
    }else{
      std::cerr << "adding /usr/var/trell/apps/bridgeData/chemco.dae" << std::endl;
      files.push_back("/usr/var/trell/apps/bridgeData/chemco.dae");
      std::cerr << "added /usr/var/trell/apps/bridgeData/chemco.dae, will now load it" << std::endl;
    }
    TiniaViewerJob job( files );
    tinia::trell::IPCGLJobController controller;
    controller.setJob( &job );
    controller.run( argc, argv );
    return 0;
}
