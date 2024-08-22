cmake_minimum_required(VERSION 3.25)

project(
  Pali
  VERSION 0.1
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

file(GLOB_RECURSE SRC_FILES src/*)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_compile_definitions(
  RELEASE_BUILD
  VERSION_NUMBER=${CMAKE_PROJECT_VERSION}
)

add_compile_options(
  -O3
  -flto -funroll-loops -fno-exceptions
  -march=native
  -Wall -pedantic
  # -fsanitize=undefined -fsanitize=address
)

add_link_options(
  -flto
  # -fsanitize=undefined -fsanitize=address
)

add_executable(Pali ${SRC_FILES})

target_link_libraries(Pali)
