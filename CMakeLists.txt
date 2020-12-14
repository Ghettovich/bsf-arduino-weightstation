cmake_minimum_required(VERSION 3.17)
project(bsf_arduino_weightstation)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(src/Recipe)

add_executable(bsf_arduino_weightstation
        src/Recipe/Recipe.cpp
        src/Recipe/Recipe.h
        bsf-arduino-weightstation.ino
        hx711.ino
        mqtt.ino)
