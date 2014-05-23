
#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include <iostream>
#include "Viewer.hpp"
#include "Utils.hpp"

namespace {

ViewerApp* app_instance = NULL;


void
reshape( int w, int h )
{
    app_instance->viewer().setWindowSize( w, h );
    app_instance->reshape( w, h );
}

void
idle()
{
   glutPostRedisplay(); // trigger continuous rendering...
}

void
wheel( int w, int d, int x, int y )
{
    app_instance->viewer().startMotion( ViewManipulator::DOLLY, 0, 0 );
    app_instance->viewer().motion(d * static_cast<int>( app_instance->viewer().getWindowSize()[0]) / 8, 0 );
    app_instance->viewer().startMotion( ViewManipulator::NONE, d, 0 );
    glutPostRedisplay();    
}

void
mouse( int b, int s, int x, int y )
{
    if( s == GLUT_DOWN ) {
        int modifier = glutGetModifiers();
        if( b == GLUT_LEFT_BUTTON ) {
            if( modifier & GLUT_ACTIVE_SHIFT ) {
                app_instance->viewer().startMotion( ViewManipulator::ORIENT, x, y );
            }
            else {
                app_instance->viewer().startMotion( ViewManipulator::ROTATE, x, y);
            }
        }
        else if( b == GLUT_MIDDLE_BUTTON ) {
            app_instance->viewer().startMotion( ViewManipulator::PAN, x, y);
        }
        else if(b == GLUT_RIGHT_BUTTON) {
            if( modifier & GLUT_ACTIVE_SHIFT ) {
                app_instance->viewer().startMotion( ViewManipulator::FOLLOW, x, y);
            }
            else {
                app_instance->viewer().startMotion( ViewManipulator::ZOOM, x, y);
            }
        }   
    }
    else {
        app_instance->viewer().endMotion(x, y);
    }
    glutPostRedisplay();  
}

void
motion( int x, int y )
{
    app_instance->viewer().motion( x, y );
    glutPostRedisplay();
}

void
display()
{
    glm::vec2 winsize = app_instance->viewer().getWindowSize();
    if( (winsize[0] < 1) || (winsize[1] < 1) ) {
        return;
    }
    static double last_time = getCurrentTime();
    static int    frames    = 0;
    double time = getCurrentTime();
    
    frames++;
    if( (time - last_time) > 1.f ) {
        std::cerr << (frames/(time - last_time)) << " fps." << std::endl;
        frames = 0;
        last_time = time;
    }
  
    glViewport( 0, 0, (GLsizei)winsize[0], (GLsizei)winsize[1] );
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    app_instance->viewer().update();
    app_instance->render();

    glutSwapBuffers();
}

void
keyboard( unsigned char key, int x, int y )
{
    if( key == 'r' ) {
        app_instance->reload();
    }
    else if( ( '0' <= key) && ( key <= '9' ) ) {
        app_instance->moveToCameraInstance( key - '0' );
    }
    else if ( key == ' ' ) {
        app_instance->nextVisualScene();
    }
#ifdef SCENE_TINIA
    else if( key == 'e' ) {
        app_instance->dumpCurrentRenderlistXML();
    }
#endif
    else if(key == 'q' || key == 27) {
        glutLeaveMainLoop();
    }
    glutPostRedisplay();
}


} // of anonymous namespace

int
main( int argc, char** argv )
{
    Scene::initLogger( &argc, argv );

    // --- parse command line arguments ----------------------------------------
    bool stereo       = false;
    bool torture      = false;
    bool auto_shader  = true;
    bool auto_flatten = true;
    float scale       = 1.f;
    std::vector<std::string> source_files;
    
    for(int i=1; i<argc; i++) {
        std::string param( argv[i] );
        if( param == "--stereo" ) {
            stereo = true;
        }
        else if( param == "--torture" ) {
            torture = true;
        }
        else if( param == "--auto-shader" ) {
            auto_shader = true;
        }
        else if( param == "--no-auto-shader" ) {
            auto_shader = false;
        }
        else if( param == "--auto-flatten" ) {
            auto_flatten = true;
        }
        else if( param == "--no-auto-flatten" ) {
            auto_flatten = false;
        }
        else if( param.length() > 7 && (param.substr( param.length()-7 ) == ".config" ) ) {
            // skip
        }
        else if( param.length() > 4 && (param.substr( param.length()-4 ) == ".xml" ) ) {
            source_files.push_back( argv[i] );
        }
        else if(param.length() > 4 && (param.substr( param.length()-4 ) == ".dae"  ) ) {
            source_files.push_back( argv[i] );
        }
        else if(param.length() > 4 && (param.substr( param.length()-4 ) == ".DAE"  ) ) {
            source_files.push_back( argv[i] );
        }
        else {
            scale = static_cast<float>( atof( param.c_str() ) );
            if( scale == 0.0f ) {
                scale = 1.0f;
            }
        }
    }    
    
    
    glutInit( &argc, argv );
    if( stereo ) {
        glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB | GLUT_STEREO );
    }
    else {
        glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    }
    glutInitWindowSize( 1280, 768 );
    glutCreateWindow( argv[0] );
    glewInit();
    
    glutReshapeFunc( reshape );
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutIdleFunc( idle );
    glutMouseFunc( mouse );
    glutMotionFunc( motion );
    //glutSpecialFunc( special );
    glutMouseWheelFunc( wheel );
    
    app_instance = new ViewerApp( source_files, stereo, torture, auto_shader, auto_flatten, scale );
    glutMainLoop();
    delete app_instance;
    
    return EXIT_SUCCESS;
}
