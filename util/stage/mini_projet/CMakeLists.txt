cmake_minimum_required(VERSION 3.10)
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src )

project(mini_projet.out)

file(GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/*.cpp"
    "${PROJECT_SOURCE_DIR}/*.c"
    )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../../cmake)

FIND_PACKAGE(SDL2 REQUIRED)
FIND_PACKAGE(SDL2_mixer REQUIRED)
FIND_PACKAGE(SDL2_ttf REQUIRED)
FIND_PACKAGE(SDL2_net REQUIRED)
SET(OpenGL_GL_PREFERENCE "GLVND")
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)

SET(CMAKE_C_FLAGS "-g3 -Wextra -Wall -Wno-unused-parameter -Wunreachable-code")
SET(CMAKE_CXX_FLAGS "-g3 -Wextra -Wall -Wno-unused-parameter -Wmissing-declarations -Wredundant-decls -Wunreachable-code")

add_executable(mini_projet.out ${all_SRCS})
target_link_libraries(mini_projet.out ${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_TTF_LIBRARY} ${GLEW_LIBRARY} ${GLEW_LIBRARIES} vulkan glfw3 dl X11 pthread)
