mkdir build
cd build
:redo
clear
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Debug --parallel 8 && exit
pause
goto redo
