project( scene )
cmake_minimum_required(VERSION 2.8)

OPTION(EXTEND_CMAKE_MODULE_PATH "Extend the CMAKE_MODULE_PATH variable with user directories?" ON)
OPTION(SCENE_DEBUG              "Enable debug symbols" ON )
OPTION(SCENE_OPTIMIZE           "Enable optimization"  ON )
OPTION(SCENE_THREADS            "Use thread pools"     OFF )
OPTION(SCENE_SSE4_2             "Enable use of SSE4.2 intrinsics" ON )
OPTION(SCENE_PROFILING          "Enable profiling" OFF)
OPTION(SCENE_CHECK_TYPES        "Enable run-time checks of types" OFF )
OPTION(SCENE_RL_CHUNKS          "Use new renderlist chunking" ON )
OPTION(SCENE_TINIA              "Build bridge against Tinia renderlists" OFF )
IF(NOT WIN32)
    OPTION(SCENE_UNITTEST "Build unit tests" ON)
    OPTION(SCENE_LOG4CXX  "Build against log4cxx logging framwork" ON)
ELSE()
    OPTION(SCENE_UNITTEST "Build unit tests" OFF)
    OPTION(SCENE_LOG4CXX  "Build against log4cxx logging framwork" OFF)
ENDIF()

IF( EXTEND_CMAKE_MODULE_PATH )
  SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "/work/projects/tinia/cmake/Modules"
    "${CMAKE_INSTALL_PREFIX}/share/cmake/Modules"
    "./cmake/Modules"
    "~/cmake/Modules"
    "$ENV{HOMEPATH}/cmake/Modules"
    "../siut/cmakemacros"
    "../siut"
    "."
    "C:/cmake/Modules"
    )
ENDIF( EXTEND_CMAKE_MODULE_PATH )

IF(WIN32)
  SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/scene_cxx_thirdparty/libxml2-2.7.7.win32/lib" )
  FIND_PACKAGE( PNGWin32 REQUIRED )
  FIND_PACKAGE( LibXml2Win32 REQUIRED )
  FIND_PACKAGE( ZLIBWin32 REQUIRED )
ELSE()
  FIND_PACKAGE( PNG REQUIRED )
  FIND_PACKAGE( LibXml2 REQUIRED )
ENDIF()

IF(SCENE_TINIA)
    # For renderlist and renderlistgl
    FIND_PACKAGE( Tinia REQUIRED COMPONENTS Server Desktop )
ELSE()
    SET( tinia_INCLUDE_DIR )
ENDIF()

IF(SCENE_LOG4CXX)
    FIND_PACKAGE( Log4Cxx REQUIRED)
ELSE()
    SET( LOG4CXX_INCLUDE_DIR )
    SET( LOG4CXX_LIBRARIES )
ENDIF()

FIND_PACKAGE( SIUT2 REQUIRED)
SET(Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0")
FIND_PACKAGE( Boost REQUIRED )
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( GLEW REQUIRED )
FIND_PACKAGE( GLM REQUIRED )

SET( PLATFORM_DEP_LIBS )
IF(WIN32)
  FIND_PACKAGE(ICONV)
  INCLUDE_DIRECTORIES(${ICONV_INCLUDE_DIR})
  SET(PLATFORM_DEP_LIBS ${PLATFORM_DEP_LIBS} ${ICONV_LIBRARY})

  FIND_PACKAGE( ZLIBWin32 REQUIRED )
  SET( PLATFORM_DEP_LIBS ${PLATFORM_DEP_LIBS} ${ZLIB_LIBRARY} )
ELSE(WIN32)
    ADD_DEFINITIONS( -DUSE_POSIX )
ENDIF(WIN32)

IF(APPLE)
  FIND_PACKAGE( GLUT )
  SET(Freeglut_LIBRARY ${GLUT_LIBRARY})
ELSE(APPLE)
  ADD_DEFINITIONS( -DHAVE_FREEGLUT )
  FIND_PACKAGE(Freeglut)
ENDIF(APPLE)

ADD_DEFINITIONS( -DUSE_GLEW )
ADD_DEFINITIONS( -DXYZZY )

#
# --- Set compiler flags
#
IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
    SET( CMAKE_CXX_FLAGS "-Wall -fPIC -std=gnu++0x" )

    IF(SCENE_LOG4CXX)
        SET(CMAKE_CXX_FLAGS "-DSCENE_LOG4CXX ${CMAKE_CXX_FLAGS}" )
    ENDIF()

    IF(SCENE_RL_CHUNKS)
        SET(CMAKE_CXX_FLAGS "-DSCENE_RL_CHUNKS ${CMAKE_CXX_FLAGS}" )
    ENDIF()

    IF(SCENE_TINIA)
        SET(CMAKE_CXX_FLAGS "-DSCENE_TINIA ${CMAKE_CXX_FLAGS}" )
    ENDIF()

    IF(SCENE_PROFILING)
        SET(CMAKE_CXX_FLAGS "-pg ${CMAKE_CXX_FLAGS}" )
    ENDIF()

    IF( SCENE_PROFILING OR SCENE_DEBUG )
        SET( CMAKE_CXX_FLAGS "-g3 ${CMAKE_CXX_FLAGS}" )
    ENDIF()

    IF( SCENE_CHECK_TYPES )
        SET( CMAKE_CXX_FLAGS "-DSCENE_CHECK_TYPES ${CMAKE_CXX_FLAGS}" )
    ENDIF()

    IF( SCENE_DEBUG )
        ADD_DEFINITIONS( -DDEBUG )
    ENDIF()

    IF( SCENE_SSE4_2 )
        SET( CMAKE_CXX_FLAGS "-msse4 ${CMAKE_CXX_FLAGS}" )
    ENDIF()

    IF( SCENE_OPTIMIZE)
        SET( CMAKE_CXX_FLAGS "-O3 ${CMAKE_CXX_FLAGS}" )
    ENDIF()

    IF( SCENE_THREADS )
        SET( CMAKE_CXX_FLAGS "-DSCENE_USE_THREADS ${CMAKE_CXX_FLAGS}" )
    ENDIF()
ENDIF()
IF(MSVC10)
        #Enable multiprocessor compilation for speed
        SET(CMAKE_CXX_FLAGS " /MP ${CMAKE_CXX_FLAGS}" CACHE STRING " " FORCE)
ENDIF()


INCLUDE_DIRECTORIES(
    ${GLM_INCLUDE_DIR}
    ${TINIA_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${Freeglut_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${LOG4CXX_INCLUDE_DIR}
    ${SIUT2_INCLUDE_DIR}
    "./include"
    "../siut/"
)

LINK_DIRECTORIES(
        ${Boost_LIBRARY_DIRS}
)

# --- libscene: Scene core library -------------------------------------------
FILE( GLOB LIBSCENE_DTA "data/*.xml" "data/*.png" )
SOURCE_GROUP( "Scene sample data" FILES ${LIBSCENE_DTA} )

FILE( GLOB LIBSCENE_HDR "include/scene/*.hpp"
                        "include/scene/runtime/*.hpp"
                        "include/scene/tools/*.hpp" )
SOURCE_GROUP( "Scene headers" FILES ${LIBSCENE_HDR} )

FILE( GLOB LIBSCENE_SRC "src/*.cpp"
                        "src/runtime/*.cpp"
                        "src/tools/*.cpp" )
SOURCE_GROUP( "Scene sources" FILES ${LIBSCENE_SRC} )

SET_SOURCE_FILES_PROPERTIES( ${LIBSCENE_DTA} ${LIBSCENE_HDR} PROPERTIES HEADER_FILE_ONLY TRUE )
ADD_LIBRARY( scene STATIC ${LIBSCENE_DTA} ${LIBSCENE_HDR} ${LIBSCENE_SRC} )
TARGET_LINK_LIBRARIES( scene ${LIBXML2_LIBRARIES} ${LOG4CXX_LIBRARIES} )
SET_TARGET_PROPERTIES(scene PROPERTIES DEBUG_POSTFIX "d")

# --- libscene_glsl: Scene GLSL runtime --------------------------------------
FILE( GLOB LIBSCENE_GLSL_HDR "include/scene/glsl/*.hpp" )
SOURCE_GROUP( "GLSL runtime headers" FILES ${LIBSCENE_GLSL_HDR} )

FILE( GLOB LIBSCENE_GLSL_SRC "src/glsl/*.cpp" )
SOURCE_GROUP( "GLSL runtime sources" FILES ${LIBSCENE_GLSL_SRC} )

SET_SOURCE_FILES_PROPERTIES( ${LIBSCENE_GLSL_HDR} PROPERTIES HEADER_FILE_ONLY TRUE )
ADD_LIBRARY( scene_glsl STATIC ${LIBSCENE_GLSL_HDR} ${LIBSCENE_GLSL_SRC} )
TARGET_LINK_LIBRARIES( scene_glsl scene ${LIBXML2_LIBRARIES}  ${LOG4CXX_LIBRARIES} )
SET_TARGET_PROPERTIES(scene_glsl PROPERTIES DEBUG_POSTFIX "d")

# --- libscene_collada: Scene COLLADA XML input and output -------------------
FILE( GLOB LIBSCENE_COLLADA_HDR "include/scene/collada/*.hpp" )
SOURCE_GROUP( "COLLADA headers" FILES ${LIBSCENE_COLLADA_HDR} )

FILE( GLOB LIBSCENE_COLLADA_SRC "src/collada/*.cpp" )
SOURCE_GROUP( "COLLADA sources" FILES ${LIBSCENE_COLLADA_SRC} )

SET_SOURCE_FILES_PROPERTIES( ${LIBSCENE_COLLADA_HDR} PROPERTIES HEADER_FILE_ONLY TRUE )
ADD_LIBRARY( scene_collada STATIC ${LIBSCENE_COLLADA_HDR} ${LIBSCENE_COLLADA_SRC} )
TARGET_LINK_LIBRARIES( scene_collada scene ${Boost_FILESYSTEM_LIBRARY} ${PNG_LIBRARY} ${ZLIB_LIBRARY} ${LIBXML2_LIBRARIES} ${LOG4CXX_LIBRARIES} )
SET_TARGET_PROPERTIES(scene_collada PROPERTIES DEBUG_POSTFIX "d")

# --- libscene_tinia: Scene Tinia bridge -------------------------------------
IF( SCENE_TINIA )
    FILE( GLOB LIBSCENE_TINIA_HDR "include/scene/tinia/*.hpp" )
    SOURCE_GROUP( "Tinia bridge headers" FILES ${LIBSCENE_TINIA_HDR} )

    FILE( GLOB LIBSCENE_TINIA_SRC "src/tinia/*.cpp" )
    SOURCE_GROUP( "Tinia bridge sources" FILES ${LIBSCENE_TINIA_SRC} )

    SET_SOURCE_FILES_PROPERTIES( ${LIBSCENE_TINIA_HDR} PROPERTIES HEADER_FILE_ONLY TRUE )
    ADD_LIBRARY( scene_tinia STATIC ${LIBSCENE_TINIA_HDR} ${LIBSCENE_TINIA_SRC} )
    TARGET_LINK_LIBRARIES( scene_tinia ${LOG4CXX_LIBRARIES} ${tinia_LibRenderList_LIBRARY} )
    SET_TARGET_PROPERTIES(scene_tinia PROPERTIES DEBUG_POSTFIX "d")
    SET( SCENE_TINIA_LIB scene_tinia )
ELSE()
    SET( SCENE_TINIA_LIB )
ENDIF()

# --- glut viewer app --------------------------------------------------------
FILE( GLOB APP_VIEWER_SRC "apps/viewer/*.cpp" )
SOURCE_GROUP( "app-viewer" FILES ${APP_VIEWER_SRC} )

ADD_EXECUTABLE( viewer ${APP_VIEWER_SRC} )
TARGET_LINK_LIBRARIES( viewer
                       scene
                       scene_glsl
                       scene_collada
                       ${SCENE_TINIA_LIB}
                       ${tinia_RenderList_LIBRARY}
                       ${PLATFORM_DEP_LIBS}
                       ${SIUT2_LIBRARY}
                       ${Freeglut_LIBRARY}
                       ${GLEW_LIBRARY}
                       ${OPENGL_LIBRARIES}
                       ${LIBXML2_LIBRARIES}
                       ${LOG4CXX_LIBRARIES}
                       ${Boost_SYSTEM_LIBRARY}
                       ${PNG_LIBRARIES}
)

# --- command line filter app ------------------------------------------------
FILE( GLOB APP_FILTER_SRC "apps/filter/*.cpp" )
SOURCE_GROUP( "app-filter" FILES ${APP_FILTER_SRC} )

ADD_EXECUTABLE( filter ${APP_FILTER_SRC} )
TARGET_LINK_LIBRARIES( filter
                       scene
                       scene_collada
                       ${SCENE_TINIA_LIB}
                       ${tinia_RenderList_LIBRARY}
                       ${PLATFORM_DEP_LIBS}
                       ${LIBXML2_LIBRARIES}
                       ${LOG4CXX_LIBRARIES}
                       ${Boost_SYSTEM_LIBRARY}
                       ${PNG_LIBRARIES}
)


if( ${SCENE_UNITTEST} )
        ENABLE_TESTING()
        IF(WIN32)
          SET(GTEST_ROOT "C:/scene_cxx_thirdparty/gtest-1.5.0")
        ENDIF()
    FIND_PACKAGE( GTest REQUIRED
    )
    INCLUDE_DIRECTORIES(
                ${GTEST_INCLUDE_DIR}
        )
    ADD_EXECUTABLE( scene_unit
                    "test/unittest/main.cpp"
                    "test/unittest/TimeStampTest.cpp"
                    "test/unittest/LibraryLightsTest.cpp"
                    "test/unittest/LibraryVisScenesTest.cpp"
                    "test/unittest/StringEnumMappings.cpp"
                    "test/unittest/BuilderImport.cpp"
                    "test/unittest/BuilderExport.cpp"
    )
    TARGET_LINK_LIBRARIES( scene_unit
                           scene
                           scene_glsl
                           scene_collada
                           ${PLATFORM_DEP_LIBS}
                           ${Freeglut_LIBRARY}
                           ${GLEW}
                           ${OPENGL_LIBRARIES}
                           ${LIBXML2_LIBRARIES}
                           ${LOG4CXX_LIBRARIES}
                           ${GTEST_LIBRARY}
                           ${GTEST_MAIN_LIBRARY}
                           ${Boost_SYSTEM_LIBRARY}
    )
    ADD_TEST( AllTestsInscene_unit scene_unit)
ENDIF( ${SCENE_UNITTEST} )


# 'install' target
IF(NOT WIN32)
  INSTALL(TARGETS scene ${SCENE_TINIA_LIB} scene_glsl scene_collada DESTINATION lib)
  INSTALL(DIRECTORY include/scene
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN ".svn" EXCLUDE)
ELSE()
  SET(CMAKE_INSTALL_PREFIX CACHE INTERNAL "")
  SET(scene_INSTALL_PREFIX "$ENV{PROGRAMFILES(x86)}/SINTEF/scene"
    CACHE PATH "Path to install scene")
  INSTALL(TARGETS scene ${SCENE_TINIA_LIB} scene_glsl scene_collada
    DESTINATION ${scene_INSTALL_PREFIX}/lib)
  INSTALL(DIRECTORY include/scene
    DESTINATION ${scene_INSTALL_PREFIX}/include
    FILES_MATCHING PATTERN "*.hpp"
    PATTERN ".svn" EXCLUDE)

ENDIF()
