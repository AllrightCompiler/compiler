file(GLOB_RECURSE ANTLR4_SRC "*.cpp")

add_library(antlr4_runtime STATIC ${ANTLR4_SRC})
