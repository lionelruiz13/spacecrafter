#!/bin/bash
# F82 runner - the portrait leg (window taller than wide) and its square control.
#
#   f82_run.sh portrait <outdir>
#   f82_run.sh square   <outdir>
#   f82_run.sh table    <artifacts/f82 dir>
#
# Every launch runs on the temp-HOME farm (b3_farm.sh); ALL config edits happen on
# that farm copy.  The REAL ~/.spacecrafter config.ini + ssystem.ini md5 are asserted
# in == out around the whole stage, in ADDITION to the assert f82_portrait.py makes
# around the launch itself.  SC_BIN overridable; display per HOST-EVENTS (DISPLAY
# forced, never defaulted from the inherited value - see harness/README.md).
set -eu
HERE="$(cd "$(dirname "$0")" && pwd)"
STAGE="$1"; OUT="$2"; shift 2
mkdir -p "$OUT"

export DISPLAY="${SC_DISPLAY:-:2}"
export XAUTHORITY="${XAUTHORITY_OVERRIDE:-$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.* | head -1)}"
export SC_BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
export F82_FARM="${F82_FARM:-/tmp/f82_farm}"

if [ "$STAGE" = "table" ]; then
    python3 "$HERE/f82_portrait.py" table "$OUT"
    exit $?
fi

before=$(md5sum "$HOME/.spacecrafter/config.ini" "$HOME/.spacecrafter/ssystem.ini")
echo "$before" > "$OUT/real_home_md5_before.txt"

case "$STAGE" in
  portrait|square)
    python3 "$HERE/f82_portrait.py" run "$OUT" --aspect "$STAGE" --bin "$SC_BIN"
    rc=$?
    ;;
  defaultrender)
    # SUPPLEMENTARY leg: portrait at the DEFAULT render_size (0), the branch a
    # config-less install takes (checkConfig.cpp:106). See
    # f82_predictions_supplement.json, committed before that launch.
    python3 "$HERE/f82_portrait.py" defaultrender "$OUT" --bin "$SC_BIN"
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
