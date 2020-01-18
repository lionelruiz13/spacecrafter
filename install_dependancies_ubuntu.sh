#!/bin/bash

# Install for kubuntu

echo  ---------------------------------
echo  ---                           ---
echo  ---      Spacecrafter         ---
echo  ---  Installing dependancies  ---
echo  ---                           ---
echo  ---------------------------------

# sudo apt-get install -y libegl1-mesa-dev
sudo apt-get install -y libglew-dev

sudo apt-get install -y libsdl2-dev
#sudo apt-get install -y libsdl2-gfx-dev
sudo apt-get install -y libsdl2-mixer-dev
sudo apt-get install -y libsdl2-ttf-dev	 	
sudo apt-get install -y libsdl2-image-dev
sudo apt-get install -y libsdl2-net-dev
#sudo apt-get install -y libfreetype6-dev

sudo apt-get install -y gettext
sudo apt-get install -y make
sudo apt-get install -y g++
sudo apt-get install -y cmake
#player vidéo
sudo apt-get install -y libavcodec-dev
sudo apt-get install -y libavformat-dev
sudo apt-get install -y libavutil-dev
sudo apt-get install -y libswscale-dev
sudo apt-get install -y libavfilter-dev
sudo apt-get install -y libswresample-dev 

sudo apt-get install -y mplayer

echo Install completed.
