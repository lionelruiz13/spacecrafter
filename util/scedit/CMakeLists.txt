cmake_minimum_required(VERSION 3.16)
project(scedit LANGUAGES CXX)

# Standalone tool — deliberately NOT wired into the spacecrafter build
# (util/ siblings pattern). Build:
#   cmake -B build && cmake --build build
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(scedit src/main.cpp)
target_include_directories(scedit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
