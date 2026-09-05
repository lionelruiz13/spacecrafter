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

[ "$BUILD_MODE" = "" ] &&BUILD_MODE=Release

cd build
git submodule update --init || (cd ../src && git clone https://github.com/Calvin-Ruiz/EntityCore.git)
[ -z "$BUILD" ] &&BUILD=Release # Use "BUILD=LocalRelease ./install_src.sh" for optimisation specifics to currently installed CPU (in which case it should be recompiled again when replacing the CPU)
cmake .. -DCMAKE_BUILD_TYPE=$BUILD || exit $?
if [ "$JOBS" = "" ]
then
	NPROC=$(nproc)
	JOBS=$(free --giga | grep Mem | grep -E --only-matching '[0-9]+$')
	JOBS=$((JOBS*2/3))
	if [ $JOBS -lt $NPROC ]
	then
		echo "Using $JOBS logical cores to leave 1.5Go per job"
		JOBS="-j$JOBS"
	else
		echo "Using all $NPROC logical cores"
		JOBS="-j$NPROC"
	fi
fi
chrt --batch 0 cmake --build . $JOBS --config $BUILD_MODE || exit $?
sudo cmake --install . --config $BUILD_MODE || exit $?
cd ..

echo -e "\033[32mScript completed.\033[0m"
