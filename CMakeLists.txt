cmake_minimum_required(VERSION 3.10)

project(compiler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG")
set(CMAKE_BUILD_TYPE DEBUG)

file(GLOB_RECURSE SRC "src/*.cpp")

include_directories(src)
include_directories(3rd_party/antlr4-runtime)

add_subdirectory(3rd_party/antlr4-runtime)

add_executable(compiler ${SRC})
target_link_libraries(compiler antlr4_runtime)
