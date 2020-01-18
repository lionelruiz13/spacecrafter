#!/bin/bash

# Init
FILE="/tmp/out.$$"
GREP="/bin/grep"
#....
# Make sure only root can run our script
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi


# Install for mageia4

echo  ---------------------------------
echo  ---                           ---
echo  ---      Stellarium 360       ---
echo  ---  Installing dependancies  ---
echo  ---                           ---
echo  ---------------------------------

urpmi --auto lib64mesaegl1-devel
urpmi --auto lib64sdl2.0-devel
urpmi --auto lib64SDL_gfx-devel

urpmi --auto lib64sdl2_mixer-devel
urpmi --auto lib64sdl2_ttf-devel
urpmi --auto lib64sdl2_image-devel

urpmi --auto libfreetype6-devel
urpmi --auto automake
urpmi --auto gettext
urpmi --auto make
urpmi --auto gcc-c++
urpmi --auto libtool
urpmi --auto graphicsmagick

urpmi --auto graphicsmagick-devel

urpmi --auto mplayer
urpmi --auto libglew-devel

echo Install completed.
