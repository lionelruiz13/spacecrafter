#!/bin/bash

# Install for Ubuntu

echo  ---------------------------------
echo  ---                           ---
echo  ---      Spacecrafter         ---
echo  ---  Installing dependancies  ---
echo  ---                           ---
echo  ---------------------------------

# sudo apt-get install -y libegl1-mesa-dev
sudo apt-get install -y libglew-dev

sudo apt install -y git
sudo apt install -y libsdl2-dev
#sudo apt-get install -y libsdl2-gfx-dev
sudo apt install -y libsdl2-mixer-dev
sudo apt install -y libsdl2-ttf-dev	 	
sudo apt install -y libsdl2-image-dev
sudo apt install -y libsdl2-net-dev
#sudo apt-get install -y libfreetype6-dev
sudo apt install -y libpng-dev

sudo apt install -y gettext
sudo apt install -y make
sudo apt install -y g++
sudo apt install -y cmake
#player vidéo
sudo apt install -y libavcodec-dev
sudo apt install -y libavformat-dev
sudo apt install -y libavutil-dev
sudo apt install -y libswscale-dev
sudo apt install -y libavfilter-dev
sudo apt install -y libswresample-dev 
#In case you don't have a NVIDIA graphic card (insane)
#sudo apt install -y libvdpau-va-gl1

sudo apt install -y mplayer

sudo apt install -y vulkan-tools
sudo apt install -y libvulkan-dev
sudo apt install -y vulkan-headers
sudo apt install -y vulkan-validationlayers-dev

#wget -qO - http://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
#sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-focal.list http://packages.lunarg.com/vulkan/lunarg-vulkan-focal.list
#sudo apt update
#sudo apt install -y vulkan-sdk

echo Install completed.


