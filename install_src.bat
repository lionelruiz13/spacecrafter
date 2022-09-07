mkdir build
cd build
git submodule update --init
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --parallel 4
cmake --install .