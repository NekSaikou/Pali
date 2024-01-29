# create build directory if it doesn't exist already
if [ ! -d ./build/ ]; then
  mkdir build
fi

# move into build directory
cd build/

# generate CMakeFiles
cmake -D CMAKE_CXX_COMPILER=clang++ ..

cmake --build .

./Fodder
