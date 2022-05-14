project(compiler CXX)
cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE SRC "frontend/*.cpp" "main.cpp")
file(GLOB_RECURSE LIB_SRC "3rd_party/*/*.cpp" "3rd_party/*.cpp")

add_compile_options(-g -std=c++17 -O2)
include_directories(frontend)
include_directories(3rd_party/antlr4-runtime)


add_executable(compiler ${SRC} ${IB_SRC})
