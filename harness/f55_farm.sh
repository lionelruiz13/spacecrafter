#!/bin/bash
# F55 farm — b3_farm.sh plus ONE addition: a WRITABLE scripts/fscripts.
#
# WHY THIS EXISTS.  b3_farm.sh symlinks every ~/.spacecrafter entry it does not
# name, and `scripts` is one of them — so in a b3 farm, `scripts/fscripts/` IS
# the real directory.  F55's sampling channel is the app's own STARTUP SCRIPT
# (`ScriptMgr::playStartupScript` reads `<HOME>/.spacecrafter/scripts/fscripts/
# startup.sts` [observed: script_mgr.cpp:384-388, app.cpp:689]), which means the
# driver must WRITE a startup.sts — and writing it through a b3 farm would write
# it into the REAL home, overwriting the owner's own startup.sts.
#
# So this farm rebuilds `scripts/` as a real directory of symlinks to the real
# children, and `scripts/fscripts/` as a real directory of symlinks to the real
# fscripts entries — EXCEPT `startup.sts`, which the caller writes as a real
# file.  Everything else the app can reach through the script directory is the
# authored data, unchanged, read-only-by-construction (symlink targets are only
# read; nothing in this harness writes through them).
#
#   usage: f55_farm.sh <farmdir>
#
# The caller then writes <farmdir>/.spacecrafter/scripts/fscripts/startup.sts
# and <farmdir>/.spacecrafter/modularSystem/SolarSystem.ini.
set -eu
FARM="$1"
HERE=$(cd "$(dirname "$0")" && pwd)
SRC="$HOME/.spacecrafter"
DST="$FARM/.spacecrafter"

"$HERE/b3_farm.sh" "$FARM"

# scripts/ : real dir, children symlinked, fscripts/ real with children symlinked
rm -f "$DST/scripts"
mkdir -p "$DST/scripts/fscripts"
for e in "$SRC"/scripts/*; do
    n=$(basename "$e")
    [ "$n" = "fscripts" ] && continue
    ln -s "$e" "$DST/scripts/$n"
done
for e in "$SRC"/scripts/fscripts/*; do
    n=$(basename "$e")
    [ "$n" = "startup.sts" ] && continue      # the caller writes this one
    ln -s "$e" "$DST/scripts/fscripts/$n"
done
echo "f55 farm ready: $DST (scripts/fscripts writable)"
