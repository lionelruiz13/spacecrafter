mkdir build
cd build
git submodule update --init
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --parallel 8
cmake --install . --config Release
pause
