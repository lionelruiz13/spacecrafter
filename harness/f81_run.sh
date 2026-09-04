#!/bin/bash
# F81 runner - the view offset's two couplings.
#
#   f81_run.sh prep    <outdir>
#   f81_run.sh config  <outdir> <jd> <x,y,z> [offset]
#   f81_run.sh command <outdir> <jd> <x,y,z> [offset]
#
# Every launch runs on the temp-HOME farm (b3_farm.sh); the REAL ~/.spacecrafter
# config.ini + ssystem.ini md5 are asserted in == out around the whole stage, in
# ADDITION to the assert f81_offset.py makes around the launch itself.
# SC_BIN overridable; display per HOST-EVENTS (DISPLAY forced, never defaulted
# from the inherited value - see harness/README.md).
set -eu
HERE="$(cd "$(dirname "$0")" && pwd)"
STAGE="$1"; OUT="$2"; shift 2
mkdir -p "$OUT"

export DISPLAY="${SC_DISPLAY:-:2}"
export XAUTHORITY="${XAUTHORITY_OVERRIDE:-$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.* | head -1)}"
export SC_BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
export F81_FARM="${F81_FARM:-/tmp/f81_farm}"

before=$(md5sum "$HOME/.spacecrafter/config.ini" "$HOME/.spacecrafter/ssystem.ini")
echo "$before" > "$OUT/real_home_md5_before.txt"

case "$STAGE" in
  prep)
    python3 "$HERE/f81_offset.py" prep "$OUT" --bin "$SC_BIN"
    rc=$?
    ;;
  config|command)
    JD="$1"; IV="$2"; OFF="${3:-0.3}"
    python3 "$HERE/f81_offset.py" run "$OUT" --channel "$STAGE" \
        --jd "$JD" --initview "$IV" --offset "$OFF" --bin "$SC_BIN"
    rc=$?
    ;;
  *) echo "unknown stage $STAGE" >&2; exit 4 ;;
esac

after=$(md5sum "$HOME/.spacecrafter/config.ini" "$HOME/.spacecrafter/ssystem.ini")
echo "$after" > "$OUT/real_home_md5_after.txt"
if [ "$before" != "$after" ]; then
    echo "FAIL: real ~/.spacecrafter md5 moved across stage $STAGE" >&2
    exit 5
fi
echo "md5 in == out on the real ~/.spacecrafter"
exit $rc
