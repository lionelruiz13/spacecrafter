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

INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src)

set(SRCS2
	AbstractCamera.cpp
	FreeCamera.cpp
	main.cpp
	TargetCamera.cpp
	SDLFacade.cpp
	../../src/ojm.cpp
	../../src/shader.cpp
	../../src/log.cpp
	../../src/s_texture.cpp
	../../src/utility.cpp
	)

set(HEADERS2
	../../src/stb_image.h
	../../src/ojm.hpp
	../../src/shader.hpp
	../../src/log.hpp
	../../src/stateGL.hpp
	../../src/s_texture.hpp
	../../src/utility.hpp
	../../src/matrix4.hpp
	../../src/vecmath.hpp
	../../src/vector2.hpp
	../../src/vector3.hpp
	../../src/vector4.hpp
	../../src/fmath.hpp

	AbstractCamera.hpp
	FreeCamera.hpp
	TargetCamera.hpp
	SDLFacade.hpp
    )

set (CMAKE_CXX_STANDARD 14)

add_executable(ojm_viewer ${SRCS2} ${HEADERS2})
target_link_libraries(ojm_viewer ${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${GLEW_LIBRARY} )
INSTALL(TARGETS ojm_viewer DESTINATION bin)
INSTALL(FILES shaderOJM.frag shaderOJM.vert DESTINATION share/spacecrafter/shaders)
