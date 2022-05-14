cmake_minimum_required(VERSION 3.14)

project(compiler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

file(GLOB_RECURSE SRC "src/*.cpp")
file(GLOB_RECURSE ANTLR4_SRC "3rd_party/antlr4-runtime/*.cpp")

include_directories(src)
include_directories(3rd_party/antlr4-runtime)

add_executable(compiler ${SRC} ${ANTLR4_SRC})
