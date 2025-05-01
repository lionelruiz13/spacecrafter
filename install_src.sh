#!/bin/bash

# Install for ubuntu

echo  ---------------------------------
echo  ---                           ---
echo  ---      Spacecrafter         ---
echo  ---       Installing          ---
echo  ---                           ---
echo  ---------------------------------

if [ "$1" = "--update" ]
then
	JOBS=$2
else
	rm -fr build
	mkdir build
	JOBS=$1
fi

cd build
git submodule update --init || (cd ../src && git clone https://github.com/Calvin-Ruiz/EntityCore.git)
cmake .. -DCMAKE_BUILD_TYPE=Release
if [ "$JOBS" = "" ]
then
	NPROC=$(nproc)
	JOBS=$(free --giga | grep Mem | grep -E --only-matching '[0-9]+$')
	if [ $JOBS -lt $NPROC ]
	then
		echo "Using $JOBS logical cores to leave 1Go per job"
		JOBS="-j$JOBS"
	else
		echo "Using all $NPROC logical cores"
		JOBS="-j$NPROC"
	fi
fi
chrt --batch 0 cmake --build . $JOBS --config Release
sudo cmake --install . --config Release
cd ..

echo -e "\033[32mScript completed.\033[0m"
