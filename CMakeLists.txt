#
#	Spacecrafter project !
#


cmake_minimum_required(VERSION 3.12)

########### Project name ###########


message("---------------------------------------------")
message(" Project Spacecrafter")
message("---------------------------------------------")

project(spacecrafter)
SET(SPACECRAFTER_YEAR "2026")
SET(SPACECRAFTER_MONTH "07")
SET(SPACECRAFTER_DAY "11")
SET(SPACECRAFTER_USER_NAME "LSS Open Project")
SET(SPACECRAFTER_USER_EDITION "Vulkan Edition 2026")
SET(SPACECRAFTER_VERSION "${SPACECRAFTER_YEAR}.${SPACECRAFTER_MONTH}.${SPACECRAFTER_DAY}")
SET(PACKAGE spacecrafter)

CONFIGURE_FILE (
  "${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/config.h"
  )

set (CMAKE_CXX_STANDARD 23)

################# plateform configuration ############################

if(WIN32)
  set(PLATFORM "windows")
  add_compile_definitions(WIN32=1)
elseif(UNIX)
  set(PLATFORM "linux")
  add_compile_definitions(LINUX=1)
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

################# Flags configuration ############################

message("---------------------------------------------")
message(" Spacecrafter CMAKE")
message("---------------------------------------------")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
  message("Build type empty, so it's fixed to debug mode. Use -DCMAKE_BUILD_TYPE=Release to activate release mode or -DCMAKE_BUILD_TYPE=LocalRelease to take advantage of the currently installed CPU's instructions")
  #set(CMAKE_BUILD_TYPE "Debug")
endif(NOT CMAKE_BUILD_TYPE)

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
if (UNIX)
  SET(CMAKE_C_FLAGS "-ggdb3 -Og -Wextra -Wall -Wfatal-errors -Wno-unused-parameter -Wunreachable-code")
  SET(CMAKE_CXX_FLAGS "-ggdb3 -Og -Wextra -Wall -Wno-unused-parameter -Wmissing-declarations -Wredundant-decls -Wunreachable-code -fconcepts")
endif()
if (MSVC)
    add_compile_options(/fsanitize=address /Zi)
    # C4244: conversion from 'type1' to 'type2', possible loss of data
    # C4267: conversion from 'size_t' to 'type', possible loss of data
    # C4305: truncation from 'type1' to 'type2'
    # C4200: nonstandard extension used : zero-sized array in struct/union
    # C4005: 'macro' : macro redefinition (M_PI_2)
    add_compile_options(/wd4244 /wd4267 /wd4305 /wd4200 /wd4005) # disable warning C4244 and C4267 and C4305 and C4200 and C4005
endif()
  message("Build type Debug. Use -DCMAKE_BUILD_TYPE=Release to activate Release mode")
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Release" )
if(UNIX)
SET(CMAKE_C_FLAGS "-ggdb3 -Ofast -Wall")
SET(CMAKE_CXX_FLAGS "-ggdb3 -Ofast -Wall -fconcepts")
endif()
if (MSVC)
    # C4244: conversion from 'type1' to 'type2', possible loss of data
    # C4267: conversion from 'size_t' to 'type', possible loss of data
    # C4305: truncation from 'type1' to 'type2'
    # C4200: nonstandard extension used : zero-sized array in struct/union
    # C4005: 'macro' : macro redefinition (M_PI_2)
    add_compile_options(/wd4244 /wd4267 /wd4305 /wd4200 /wd4005) # disable warning C4244 and C4267 and C4305 and C4200 and C4005
endif()
message("Build type Release. Use -DCMAKE_BUILD_TYPE=Debug to activate Debug mode")
endif()

if( CMAKE_BUILD_TYPE STREQUAL "LocalRelease" )
if(UNIX)
SET(CMAKE_C_FLAGS "-ggdb3 -Ofast -Wall -march=native")
SET(CMAKE_CXX_FLAGS "-ggdb3 -Ofast -Wall -fconcepts -march=native")
endif()
message("Build type Release. Use -DCMAKE_BUILD_TYPE=Debug to activate Debug mode")
endif()

if( CMAKE_BUILD_TYPE STREQUAL "LocalRelease" )
if(UNIX)
SET(CMAKE_C_FLAGS "-ggdb3 -Ofast -Wall -march=native")
SET(CMAKE_CXX_FLAGS "-ggdb3 -Ofast -Wall -fconcepts -march=native")
endif()
message("Build type LocalRelease (optimized for currently installed CPU). Use -DCMAKE_BUILD_TYPE=Debug to activate Debug mode, or -DCMAKE_BUILD_TYPE=Release to disable CPU-specific optimisation and instruction.")
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Coverage" )
SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage -fconcepts")
message("Build type Coverage. Use -DCMAKE_BUILD_TYPE=Release to activate Release mode")
endif()


########### Find packages ###########
if (UNIX)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
endif()

if(UNIX)
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

endif()

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
link_libraries(stdc++exp)
else()
FIND_PACKAGE(SDL2 CONFIG REQUIRED)
FIND_PACKAGE(SDL2_mixer CONFIG REQUIRED)
FIND_PACKAGE(SDL2_ttf CONFIG REQUIRED)
FIND_PACKAGE(SDL2_net CONFIG REQUIRED)
FIND_PACKAGE(FFmpeg REQUIRED)
# INCLUDE_DIRECTORIES("${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include")
# link_directories("${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")


# INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
# link_directories(${PROJECT_SOURCE_DIR}/lib)
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
