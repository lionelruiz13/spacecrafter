#!/bin/bash
# Temp-HOME symlink farm (INTENT 11.103(a), F0 recipe - generalized here into a
# reusable script). main.cpp:181-194 reads $HOME and current_path()s to
# $HOME/.spacecrafter/, so a farm of symlinks with a few REAL writable entries
# lets a run load AUTHORED data while the real ~/.spacecrafter is never written.
#
# usage: b3_farm.sh <farmdir>
#   creates <farmdir>/.spacecrafter with every ~/.spacecrafter entry symlinked,
#   except config.ini + ssystem.ini (real copies) and log/ screenshot/
#   modularSystem/ (real, empty, writable dirs).
# The caller authors <farmdir>/.spacecrafter/modularSystem/SolarSystem.ini.
set -eu
FARM="$1"
SRC="$HOME/.spacecrafter"
DST="$FARM/.spacecrafter"

rm -rf "$FARM"
mkdir -p "$DST"
for e in "$SRC"/*; do
    n=$(basename "$e")
    case "$n" in
        config.ini|ssystem.ini) cp "$e" "$DST/$n" ;;
        log|screenshot|modularSystem) ;;   # real dirs, created below
        *) ln -s "$e" "$DST/$n" ;;
    esac
done
mkdir -p "$DST/log" "$DST/screenshot" "$DST/modularSystem"
chmod u+w "$DST/config.ini" "$DST/ssystem.ini"
echo "farm ready: $DST"
