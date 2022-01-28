cmake_minimum_required(VERSION 3.16)
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/../../src/")

project(CaptureAnalysis)

message("${PROJECT_SOURCE_DIR}/../../src")

file(GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/../../src/EntityCore/Tools/CaptureMetrics.cpp"
    "${PROJECT_SOURCE_DIR}/*.cpp"
    )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

SET(CMAKE_CXX_FLAGS "-g3 -Wextra -Wall -Wno-sign-compare")
set(CPACK_SOURCE_IGNORE_FILES "/.git/" "/build/")

add_executable(CaptureAnalysis ${all_SRCS})

