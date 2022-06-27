#!/bin/sh

cd src &&
  java -jar ../3rd_party/antlr-4.8-complete.jar \
    -Dlanguage=Cpp -visitor -no-listener \
    frontend/SysY.g4 -o . -package frontend
