INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src )


file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        )

# création de spacecrafter
add_executable(proto ${all_SRCS})
# target_link_libraries(proto  ${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_TTF_LIBRARY} ${SDL2_NET_LIBRARY} ${GLEW_LIBRARY} ${GLEW_LIBRARIES} ${FFMPEG_LIBRARIES} )
target_link_libraries(proto  ${SDL2_LIBRARY} )

# INSTALL(TARGETS spacecrafter DESTINATION bin)
