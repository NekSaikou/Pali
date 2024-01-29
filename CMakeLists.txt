cmake_minimum_required(VERSION 3.25)

project(
  Fodder
  VERSION 0.1
  LANGUAGES CXX
  )

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SRC_FILES src/*.cpp src/Core/*.cpp)

add_compile_options(
  -O3
  -flto -funroll-loops -fno-exceptions
  -march=native
  -Wall -pedantic
  )

add_link_options(-flto)

add_executable(Fodder ${SRC_FILES})
