#!/bin/bash

clang++ -std=c++17 -O2 -lm -lantlr4-runtime \
  -Isrc \
  -I3rd_party/antlr4-runtime \
  $(find src | grep .cpp) \
  -o compiler
