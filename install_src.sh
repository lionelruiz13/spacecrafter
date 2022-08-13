#!/bin/bash

# Install for ubuntu

echo  ---------------------------------
echo  ---                           ---
echo  ---      Spacecrafter         ---
echo  ---       Installing          ---
echo  ---                           ---
echo  ---------------------------------

if [ ! -d build ]
then
mkdir build
cd build
else
cd build
rm * *.* -rf
fi

git submodule update --init || (cd ../src && git clone https://github.com/Calvin-Ruiz/EntityCore.git)
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . $1 --config Release
sudo cmake --install . --config Release
cd ..

echo -e "\033[32mScript completed.\033[0m"
