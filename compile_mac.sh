#!/bin/bash
export CPATH=/opt/homebrew/include
export LIBRARY_PATH=/opt/homebrew/lib
clang++ -arch arm64 -std=c++17 -lpng -mmacosx-version-min=10.15 -Wall -framework OpenGL -framework GLUT -framework Carbon game.cpp -o game
