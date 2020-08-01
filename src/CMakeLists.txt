ADD_SUBDIRECTORY( planetsephems ) 
ADD_SUBDIRECTORY( iniparser ) 


INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/planetsephems ${CMAKE_SOURCE_DIR}/src/iniparser )

file(GLOB_RECURSE all_SRCS ${PROJECT_SOURCE_DIR} *.h *.hpp *.cpp )

# création de spacecrafter
add_executable(spacecrafter ${all_SRCS})
target_link_libraries(spacecrafter iniparser planetsephems ${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_TTF_LIBRARY} ${SDL2_NET_LIBRARY} ${GLEW_LIBRARY} ${GLEW_LIBRARIES} ${FFMPEG_LIBRARIES} )
INSTALL(TARGETS spacecrafter DESTINATION bin)
