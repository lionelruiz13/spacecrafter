cmake_minimum_required(VERSION 3.10)
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src )

project(mini_projet.out)

file(GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/*.cpp"
    )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(mini_projet.out ${all_SRCS})
target_link_libraries(mini_projet.out ${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_TTF_LIBRARY} ${GLEW_LIBRARY} ${GLEW_LIBRARIES})
