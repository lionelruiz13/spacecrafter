INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src )


file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        )

set (CMAKE_CXX_STANDARD 17)


# création de spacecrafter
add_executable(earth ${all_SRCS})
target_link_libraries(earth ${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_TTF_LIBRARY} ${SDL2_NET_LIBRARY} ${GLEW_LIBRARY} )

