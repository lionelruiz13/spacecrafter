#!/bin/sh
echo -en "\ec"
cd build
cmake ..
make -j8 && cd .. && build/mini_projet.out
