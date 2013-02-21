#include <list>
#include <string>
#include <scene/Log.hpp>

#if SCENE_LOG4CXX
#include <log4cxx/logger.h>
#include <log4cxx/helpers/properties.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
#endif

namespace Scene {


void
initLogger( int* argc, char** argv )
{
#if SCENE_LOG4CXX
    enum DebugLevel {
        LEVEL_FATAL,
        LEVEL_ERROR,
        LEVEL_WARN,
        LEVEL_INFO,
        LEVEL_DEBUG,
        LEVEL_TRACE
    };
    DebugLevel level = LEVEL_WARN;

    std::list<std::string> confs;

    for( int i=1; i<*argc;  ) {
        int consume = 0;
        std::string param( argv[i]);
        if( param == "--loglevel" ) {
            if( i+1 < *argc ) {
                std::string value( argv[i+1] );
                if( value == "fatal" ) {
                    level = LEVEL_FATAL;
                }
                else if ( value == "error" ) {
                    level = LEVEL_ERROR;
                }
                else if( value == "warn" ) {
                    level = LEVEL_WARN;
                }
                else if( value == "info" ) {
                    level = LEVEL_INFO;
                }
                else if( value == "debug" ) {
                    level = LEVEL_DEBUG;
                }
                else if( value == "trace" ) {
                    level = LEVEL_TRACE;
                }
                consume = 2;
            }
            else {
                consume = 1;
            }
        }
        else if ( param.length() > 7 && (param.substr( param.length()-7 ) == ".config" ) ) {
            confs.push_back( param );
            consume = 1;
        }
        if( consume > 0 ) {
            *argc = *argc - consume;
            for(int k=i; k < *argc; k++ ) {
                argv[k] = argv[k+consume];
            }
        }
        else {
            i++;
        }
    }

    log4cxx::helpers::Properties props;
    switch( level ) {
    case LEVEL_FATAL:
        props.setProperty( LOG4CXX_STR( "log4j.rootLogger" ), LOG4CXX_STR( "FATAL, A1" ) );
        break;
    case LEVEL_ERROR:
        props.setProperty( LOG4CXX_STR( "log4j.rootLogger" ), LOG4CXX_STR( "ERROR, A1" ) );
        break;
    case LEVEL_WARN:
        props.setProperty( LOG4CXX_STR( "log4j.rootLogger" ), LOG4CXX_STR( "WARN, A1" ) );
        break;
    case LEVEL_INFO:
        props.setProperty( LOG4CXX_STR( "log4j.rootLogger" ), LOG4CXX_STR( "INFO, A1" ) );
        break;
    case LEVEL_DEBUG:
        props.setProperty( LOG4CXX_STR( "log4j.rootLogger" ), LOG4CXX_STR( "DEBUG, A1" ) );
        break;
    case LEVEL_TRACE:
        props.setProperty( LOG4CXX_STR( "log4j.rootLogger" ), LOG4CXX_STR( "TRACE, A1" ) );
        break;
    }
    props.setProperty( LOG4CXX_STR( "log4j.appender.A1" ), LOG4CXX_STR( "org.apache.log4j.ConsoleAppender" ) );
    props.setProperty( LOG4CXX_STR( "log4j.appender.A1.layout" ), LOG4CXX_STR( "org.apache.log4j.PatternLayout" ) );
    props.setProperty( LOG4CXX_STR( "log4j.appender.A1.layout.ConversionPattern" ), LOG4CXX_STR( "%-4r %-5p %40c - %m%n" ) );
    log4cxx::PropertyConfigurator::configure( props );

    for(auto it=confs.begin(); it!=confs.end(); ++it ) {
        log4cxx::PropertyConfigurator::configure( it->c_str() );
    }
#endif
}


} // of namespace Scene

