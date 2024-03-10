#
#	Spacecrafter project !
#


cmake_minimum_required(VERSION 3.12)

########### Project name ###########


message("---------------------------------------------")
message(" Project Spacecrafter")
message("---------------------------------------------")

project(spacecrafter)
SET(SPACECRAFTER_YEAR "24")
SET(SPACECRAFTER_MONTH "03")
SET(SPACECRAFTER_DAY "10")
SET(SPACECRAFTER_USER_NAME "LSS Open Project")
SET(SPACECRAFTER_USER_EDITION "Linux Edition Vulkan 2024")
SET(SPACECRAFTER_VERSION "${SPACECRAFTER_YEAR}.${SPACECRAFTER_MONTH}.${SPACECRAFTER_DAY}")
SET(PACKAGE spacecrafter)

CONFIGURE_FILE (
  "${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/config.h"
  )

set (CMAKE_CXX_STANDARD 20)

################# plateform configuration ############################

add_compile_definitions(LINUX=1)

################# Flags configuration ############################

message("---------------------------------------------")
message(" Spacecrafter CMAKE")
message("---------------------------------------------")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
  message("Build type empty, so it's fixed to debug mode. Use -DCMAKE_BUILD_TYPE=Release to activate release mode")
  #set(CMAKE_BUILD_TYPE "Debug")
endif(NOT CMAKE_BUILD_TYPE)

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
  SET(CMAKE_C_FLAGS "-ggdb3 -Wextra -Wall -Wfatal-errors -Wno-unused-parameter -Wunreachable-code")
  SET(CMAKE_CXX_FLAGS "-ggdb3 -Wextra -Wall -Wno-unused-parameter -Wmissing-declarations -Wredundant-decls -Wunreachable-code -fconcepts")
  message("Build type Debug. Use -DCMAKE_BUILD_TYPE=Release to activate Release mode")
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Release" )
if(UNIX)
SET(CMAKE_C_FLAGS "-ggdb3 -Ofast -Wall")
SET(CMAKE_CXX_FLAGS "-ggdb3 -Ofast -Wall -fconcepts")
endif()
message("Build type Release. Use -DCMAKE_BUILD_TYPE=Debug to activate Debug mode")
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Coverage" )
SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage -fconcepts")
message("Build type Coverage. Use -DCMAKE_BUILD_TYPE=Release to activate Release mode")
endif()

###########  additional compile flags ###########
if(UNIX)
# SET(CMAKE_C_FLAGS "-g -Wextra -Wall -Wno-ignored-qualifiers -Wno-unused-parameter")
# SET(CMAKE_CXX_FLAGS "-g -Wextra -Wall -Wno-ignored-qualifiers -Wno-unused-parameter")
endif()

########### Find packages ###########
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# search for pkg-config
include (FindPkgConfig)
if (NOT PKG_CONFIG_FOUND)
    message (FATAL_ERROR "pkg-config not found")
endif ()

# check for libpng
pkg_check_modules (LIBPNG libpng16 REQUIRED)
if (NOT LIBPNG_FOUND)
    message(FATAL_ERROR "You don't seem to have libpng16 development libraries installed")
else ()
    include_directories (${LIBPNG_INCLUDE_DIRS})
    link_directories (${LIBPNG_LIBRARY_DIRS})
    link_libraries (${LIBPNG_LIBRARIES})
endif ()

FIND_PACKAGE(Vulkan REQUIRED)

if (UNIX)
FIND_PACKAGE(SDL2 REQUIRED)
FIND_PACKAGE(SDL2_mixer REQUIRED)
FIND_PACKAGE(SDL2_ttf REQUIRED)
FIND_PACKAGE(SDL2_net REQUIRED)
FIND_PACKAGE(FFmpeg REQUIRED)
FIND_PACKAGE(PNG REQUIRED)

INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${FFmpeg_INCLUDE_DIR})
else()
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES($ENV{VULKAN_SDK}/Include)
link_directories($ENV{VULKAN_SDK}/Lib)
endif()

INCLUDE_DIRECTORIES(${Vulkan_INCLUDE_DIR})

# set include path
include_directories(${PROJECT_SOURCE_DIR}/src)
#include_directories(${PROJECT_SOURCE_DIR}/iniparser)
#include_directories($(GLEW_INCLUDE_DIRS)) # Can't find initparser/initparser.h without this statement
#INCLUDE_DIRECTORIES(${Vulkan_INCLUDE_DIRS})

########### Packaging info for CPack ###########
INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Astronomical Simulation and Visualization")
SET(CPACK_PACKAGE_VENDOR "LSS Open Project Team")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR ${SPACECRAFTER_YEAR})
SET(CPACK_PACKAGE_VERSION_MINOR ${SPACECRAFTER_MONTH})
SET(CPACK_PACKAGE_VERSION_PATCH ${SPACECRAFTER_DAY})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "spacecrafter")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "spacecrafter-${VERSION}")
SET(CPACK_SOURCE_GENERATOR "TGZ")
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_STRIP_FILES "bin/spacecrafter")
SET(CPACK_PACKAGE_EXECUTABLES "spacecrafter" "Spacecrafter")
set(CPACK_SOURCE_IGNORE_FILES "/.git/" "/build/" "/doc/" "www/" "utils/")
INCLUDE(CPack)

###########  subdirectory  ###########
add_subdirectory(shaders)
add_subdirectory(src)
add_subdirectory(data)
ADD_SUBDIRECTORY( planetsephems )
ADD_SUBDIRECTORY( iniparser )



########### Récapitulatif ###########

message("---------------------------------------------")
message(" Spacecrafter conclusion")
message("---------------------------------------------")
message("")
message("Build type : " ${CMAKE_BUILD_TYPE})
message("C compiler: ${CMAKE_C_COMPILER}")
message("C++ compiler: ${CMAKE_CXX_COMPILER}")
message("System: " ${CMAKE_SYSTEM_NAME} )
message("")
message("C++ version " ${CMAKE_CXX_STANDARD})
message("C flags: " ${CMAKE_C_FLAGS} )
message("C++ flags: " ${CMAKE_CXX_FLAGS} )
message("---------------------------------------------")
