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

#pragma once

#include <string>
#ifdef SCENE_LOG4CXX
#include <log4cxx/logger.h>
#else
#include <iostream>
#endif

namespace Scene {


/** Convenience function to initialize logger framework.
 *
 * If Scene is compiled against log4cxx, this framework is initialized,
 * otherwise this function is a no-op.
 */
void
initLogger( int* argc, char** argv );

#if SCENE_LOG4CXX
typedef log4cxx::LoggerPtr Logger;
static inline Logger getLogger( const std::string name ) { return log4cxx::Logger::getLogger( name ); }
#define SCENELOG_TRACE(a,b) LOG4CXX_TRACE(a,b)
#define SCENELOG_INFO(a,b) LOG4CXX_INFO(a,b)
#define SCENELOG_DEBUG(a,b) LOG4CXX_DEBUG(a,b)
#define SCENELOG_WARN(a,b) LOG4CXX_WARN(a,b)
#define SCENELOG_ERROR(a,b) LOG4CXX_ERROR(a,b)
#define SCENELOG_FATAL(a,b) LOG4CXX_FATAL(a,b)
#else
typedef std::string Logger;
inline Logger getLogger( const std::string& component ) { return component; }

#define SCENELOG_OUTPUT(c,a,b) \
    do { \
        std::cerr << c << a; \
        for(size_t __i=(a).length(); __i<45; __i++) { \
            std::cerr << ' '; \
        } \
        std::cerr << "    " << b << std::endl; \
} while(0)
#define SCENELOG_TRACE(a,b)
#ifdef DEBUG
#define SCENELOG_DEBUG(a,b) SCENELOG_OUTPUT("[D]",a,b)
#define SCENELOG_INFO(a,b) SCENELOG_OUTPUT("[I]",a,b)
#else
#define SCENELOG_DEBUG(a,b)
#define SCENELOG_INFO(a,b)
#endif

#define SCENELOG_WARN(a,b)  SCENELOG_OUTPUT("[W]",a,b)
#define SCENELOG_ERROR(a,b) SCENELOG_OUTPUT("[E]",a,b)
#define SCENELOG_FATAL(a,b) SCENELOG_OUTPUT("[F]",a,b)
#endif

#define SCENELOG_ASSERT(a,b) do { if(!(b)) { SCENELOG_FATAL(a, __FILE__ << '@' << __LINE__<< ": Assertion " << #b << " failed." ); abort(); } } while(0)

}
