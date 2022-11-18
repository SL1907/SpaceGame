cmake_minimum_required(VERSION 3.23)
project(SteveClub)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(SteveClub
        game.cpp
        olcPixelGameEngine.h)
