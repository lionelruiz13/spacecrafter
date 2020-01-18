cmake_minimum_required(VERSION 3.10)

project(ojm_conv)

SET(CMAKE_CXX_FLAGS "-g -Wextra -Wall -Wno-unused-parameter -Wmissing-declarations -Wredundant-decls")

INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src)

set(SRCS1
	converter.cpp
	obj3D.cpp
	obj_to_ojm.cpp
	)
    
set(HEADERS1
	obj3D.hpp
	obj_common.hpp
	obj_to_ojm.hpp
    )

set (CMAKE_CXX_STANDARD 14)

add_executable(ojm_conv ${SRCS1} ${HEADERS1})
target_link_libraries(ojm_conv)
INSTALL(TARGETS ojm_conv DESTINATION bin)

