INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/planetsephems ${CMAKE_SOURCE_DIR}/iniparser )

file(GLOB_RECURSE all_SRCS ${PROJECT_SOURCE_DIR} *.h *.hpp *.cpp )

# creation of spacecrafter
add_executable(spacecrafter ${all_SRCS})
target_link_libraries(spacecrafter iniparser planetsephems ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_TTF_LIBRARY} ${SDL2_NET_LIBRARY} ${FFMPEG_LIBRARIES} ${Vulkan_LIBRARIES})
INSTALL(TARGETS spacecrafter DESTINATION bin)
