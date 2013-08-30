/************************************************************************
 *
 *  File: GLDebugMessages.hpp
 *
 *  Created: 5. November 2012
 *
 *  Authors: Erik W. Bjønnes <erik.bjonnes@sintef.no>
 *
 *  Copyright (C) 2012 by SINTEF.  All rights reserved.
 *
 *  SINTEF, Pb 124 Blindern, N-0314 Oslo, Norway
 *  http://www.sintef.no
 *********************************************************************/
#pragma once

#include <GL/glew.h>
#include <iostream>
#include <ostream>
#include <string>

#ifndef APIENTRY
#define APIENTRY
#define SIUT3_APIENTRY
#endif


/** Helper header for setting up OpenGL debug messages.
 *  
 * If GL version does not support KHR_DEBUG will try with older ARB extension.
 * If neither is present, will give a lot of compiler errors!
 */
namespace siut3 { namespace gl_tools { namespace GLDebugMessages{

namespace impl {

std::string getTypeStringARB( GLenum type )
{
  switch(type) {
  case GL_DEBUG_TYPE_ERROR_ARB: 
    return("Error");
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
    return("Deprecated Behaviour");
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
    return("Undefined Behaviour");
  case GL_DEBUG_TYPE_PORTABILITY_ARB:
    return("Portability Issue");
  case GL_DEBUG_TYPE_PERFORMANCE_ARB:
    return("Performance Issue");
  case GL_DEBUG_TYPE_OTHER_ARB:
    return("Other");
  default:
    return("");
  }
}

std::string getSeverityStringARB( GLenum severity )
{
  switch(severity) {
  case GL_DEBUG_SEVERITY_HIGH_ARB: 
    return("High severity");
  case GL_DEBUG_SEVERITY_MEDIUM_ARB:
    return("Medium severity");
  case GL_DEBUG_SEVERITY_LOW_ARB:
    return("Low severity");
  default:
    return("Unknown severity");
  }
}


std::string getSourceStringARB( GLenum source )
{
  switch( source ) {
  case GL_DEBUG_SOURCE_API_ARB: 
    return("API");
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
    return("Window System");
  case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
    return("Shader Compiler");
  case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
    return("Third Party");
  case GL_DEBUG_SOURCE_APPLICATION_ARB:
    return("Application");
  case GL_DEBUG_SOURCE_OTHER_ARB:
    return("Other");
  default:
    return("");
  }
}

static void APIENTRY debugLoggerARB( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, void* outputStream )
{
  // std::static_cast<std::ostream&>( *outputStream );
 *(reinterpret_cast<std::ostream*>( outputStream )) << "\nGLDebugMessage:\n" << getSeverityStringARB(severity).c_str() << 
               " " << getTypeStringARB(type).c_str() << 
               " caused by " << getSourceStringARB(source).c_str() << 
               "; ID: " << id << "\n"  << message << 
               "\nEnd GLDebugMessage.\n" << std::endl;
}

#ifdef GL_DEBUG_TYPE_ERROR
std::string getTypeString( GLenum type )
{
  switch(type) {
  case GL_DEBUG_TYPE_ERROR: 
    return("Error");
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    return("Deprecated Behaviour");
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    return("Undefined Behaviour");
  case GL_DEBUG_TYPE_PORTABILITY:
    return("Portability Issue");
  case GL_DEBUG_TYPE_PERFORMANCE:
    return("Performance Issue");
  case GL_DEBUG_TYPE_OTHER:
    return("Other");
  default:
    return("");
  }
}

std::string getSeverityString( GLenum severity )
{
  switch(severity) {
  case GL_DEBUG_SEVERITY_HIGH: 
    return("High severity");
  case GL_DEBUG_SEVERITY_MEDIUM:
    return("Medium severity");
  case GL_DEBUG_SEVERITY_LOW:
    return("Low severity");
  case GL_DEBUG_SEVERITY_NOTIFICATION:
	  return("Notification");
  default:
    return("Unknown severity");
  }
}


std::string getSourceString( GLenum source )
{
  switch( source ) {
  case GL_DEBUG_SOURCE_API: 
    return("API");
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    return("Window System");
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    return("Shader Compiler");
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    return("Third Party");
  case GL_DEBUG_SOURCE_APPLICATION:
    return("Application");
  case GL_DEBUG_SOURCE_OTHER:
    return("Other");
  default:
    return("");
  }
}

static void APIENTRY debugLogger( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, void* outputStream )
{
  *(reinterpret_cast<std::ostream*>( outputStream )) << "\nGLDebugMessage:\n" << getSeverityString(severity).c_str() << 
               " " << getTypeString(type).c_str() << 
               " caused by " << getSourceString(source).c_str() << 
               "; ID: " << id << "\n" << message << 
               "\nEnd GLDebugMessage.\n" <<std::endl;
}
#endif
}// end namespace impl


/** setupGLDebugMessages is a helper function for enabling OpenGL debug messages in your application.
 *  It runs glewInit with glewExperimental = GL_TRUE to make sure GLEW is loaded.
 *  Then it checks if KHR_debug is available, meaning the debug messages are CORE functionality and works outside a debug context.
 *  If it is not available, it will check if ARB_debug_output is availabe, and use that extension. This might not work well outside a debug context, depending on drivers.
 *  
 *  By default it enables all debug messages, but this can be controlled manually by calling controlGLDebugMessages or glDebugMessageControl* directly.
 *  NOTE: OpenGL debug messages does not give any warnings or errors for shader compilations, you need to check that manually.
 *
 * @param outputStream is the ostream where the debug messages will be printed, default std::cerr
 * @returns status of whether debug messages where enabled or not. Error messages are printed to outputStream.
*/
bool setupGLDebugMessages( void* outputStream = &std::cerr )
{
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();

  std::cerr << "setting up GL debug messages" << std::endl;
  if( err != GLEW_OK )
    {
      *(reinterpret_cast<std::ostream*>( outputStream ))<< "Unable to create glew context, aborting" << std::endl;
      return false;
    }

  *(reinterpret_cast<std::ostream*>( outputStream )) << "setting up GL debug messages, called glewInit and it was ok" << std::endl;

#ifdef GL_DEBUG_TYPE_ERROR
  if( glewIsSupported( "GL_KHR_debug" ) )
    {
      glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
      glDebugMessageCallback( &impl::debugLogger, outputStream );
      glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE );
    }
  else 
#endif	  
	  if( glewIsSupported( "GL_ARB_debug_output" ) )
    {
      *(reinterpret_cast<std::ostream*>( outputStream )) << "KHR_DEBUG not supported, trying ARB extension GL_ARB_DEBUG_OUTPUT instead... " << std::endl;
      glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB );
      glDebugMessageCallbackARB( &impl::debugLoggerARB, outputStream );
      glDebugMessageControlARB( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE );
    }
  else
    {
      *(reinterpret_cast<std::ostream*>( outputStream )) << "OpenGL debug messages not supported by your current driver, please update! " << std::endl;
      return false;
    }
	  *(reinterpret_cast<std::ostream*>( outputStream )) << "finished setting up callback, only enable debug output left " <<std::endl;

  glEnable( GL_DEBUG_OUTPUT );
  std::cerr << "GL_DEBUG_OUTPUT enabled, returning true"<<std::endl;

  return true;
}

/** controlGLDebugMessages is a helper function to select what error messages should be logged or not.
 *  It takes the same parameters are glDebugMessageContrl, and calls glDebugMessageControl or glDebugMessageControlARB.
 *  If called before setupGLDebugMessages or glewInit, behaviour is undefined.
 *  To be sure the enums works, it is adviceable to use the _ARB versions, as they have the same value as the non_ARB versions.
 * @param source specify what sources message should be switched on or off.
 * @param type   specify what types of messages should be switched on or off.
 * @param severity specify what level of severity messages should be switched on or off.
 * @param count If sending a list of message IDs to be enabled/disabled, specify the count here.
 * @param IDs A const GLuint* list with the message IDs to be enabled/disabled, use NULL if empty.
 * @param enabled Enables specified messages with GL_TRUE, disables them with GL_FALSE.
*/
void controlGLDebugMessages( GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* IDs,  GLboolean enabled )
{
#ifdef GL_DEBUG_TYPE_ERROR
  if( glewIsSupported( "GL_KHR_debug" ) )
    {
      glDebugMessageControl( source, type, severity, count, IDs, enabled );
    }
  else 
#endif
    if( glewIsSupported( "GL_ARB_debug_output" ) )
    {
      glDebugMessageControlARB( source, type, severity, count, IDs, enabled );
    }
}

}}} //siut3::gl_tools

#ifdef SIUT3_APIENTRY
#undef APIENTRY
#endif
