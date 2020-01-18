#
#	Spacecrafter project !
#


cmake_minimum_required(VERSION 3.3)

########### Project name ###########


message("---------------------------------------------")
message(" Project Earth by Lartillot Jérôme")
message("---------------------------------------------")

project(earth)

SET(CMAKE_C_FLAGS "-g -Wextra -Wall -Wno-unused-parameter -Wunreachable-code")
SET(CMAKE_CXX_FLAGS "-g -Wextra -Wall -Wno-unused-parameter -Wmissing-declarations -Wredundant-decls -Wunreachable-code")


########### Find packages ###########
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

FIND_PACKAGE(SDL2 REQUIRED)
FIND_PACKAGE(SDL2_mixer REQUIRED)
FIND_PACKAGE(SDL2_ttf REQUIRED)
FIND_PACKAGE(SDL2_net REQUIRED)
SET(OpenGL_GL_PREFERENCE "GLVND")
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)


INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${GL_INCLUDE_DIR})

# set include path
include_directories(${PROJECT_SOURCE_DIR}/src)

###########  subdirectory  ########### 
#add_subdirectory(shaders)
add_subdirectory(src)
#add_subdirectory(data)
