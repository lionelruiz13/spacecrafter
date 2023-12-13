INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/planetsephems ${CMAKE_SOURCE_DIR}/iniparser )

file(GLOB_RECURSE all_SRCS ${PROJECT_SOURCE_DIR} *.h *.hpp *.cpp )

# creation of spacecrafter
add_executable(spacecrafter ${all_SRCS})
target_link_libraries(spacecrafter iniparser planetsephems ${Vulkan_LIBRARIES})

if (UNIX)
target_link_libraries(spacecrafter ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_ttf_LIBRARY} ${SDL2_NET_LIBRARY} ${FFmpeg_LIBRARIES})
else()
target_link_libraries(spacecrafter SDL2 SDL2_mixer SDL2_ttf SDL2_net avdevice avfilter avformat avcodec avutil swscale)
file(GLOB CARRIED_DLL ${PROJECT_SOURCE_DIR}/lib/*.dll)
INSTALL(FILES ${CARRIED_DLL} DESTINATION bin)
endif()
INSTALL(TARGETS spacecrafter DESTINATION bin)
