cmake_minimum_required(VERSION 3.3)

########### Project name ###########


message("---------------------------------------------")
message(" Project ojm_viewer")
message("---------------------------------------------")

project(ojm_viewer)



########### Find packages ###########
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

FIND_PACKAGE(SDL2 REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)

INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${GL_INCLUDE_DIR})

INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR})


file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/*.h"
        "${PROJECT_SOURCE_DIR}/.hpp"
        "${PROJECT_SOURCE_DIR}/*.cpp"

        "${PROJECT_SOURCE_DIR}/ojmModule/*.hpp"
        "${PROJECT_SOURCE_DIR}/ojmModule/*.cpp"   

        "${PROJECT_SOURCE_DIR}/tools/*.hpp"
        "${PROJECT_SOURCE_DIR}/tools/*.cpp"
    )

set (CMAKE_CXX_STANDARD 14)

add_executable(ojm_viewer ${all_SRCS})
target_link_libraries(ojm_viewer ${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${GLEW_LIBRARY} )
INSTALL(TARGETS ojm_viewer DESTINATION bin)
INSTALL(FILES shaderOJM.frag shaderOJM.vert DESTINATION share/spacecrafter/shaders)
