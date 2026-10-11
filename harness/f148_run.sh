#!/bin/bash
# F148 -- run the reference-decider instrument: one fresh launch per scene through f124_run.sh, judged by the driver itself.
#
#   bash f148_run.sh <label> ladder|control|switch|night before|after [on|off [--hold=switch|warp]]
#     ladder   the crossings of INTENT 11.313 (h) P11-P14, P23, P25, P26, P28, P29 (about 8 minutes)
#     control  the free-move walk's switchToBody at Earth's influence: position and view before / after
#     night    initial_night.sts lines 28-40 as commands with dumps between; who calls Camera::relocateToBody
#     switch   (a binary without the decider) the decider's two calls made through gdb in P11 / P13's scene;
#              a fifth argument --hold=switch|warp makes each crossing with that entry + placeAt(positionRelativeTo(dst), true)
#     before|after  the readings the run is judged against: no decider / F144's predictions
#     on|off   the pinned path (default on, the new path; the Camera runs on both)
#   SC_BIN another binary (read by f124_run.sh).
#
# Records: artifacts/f148/<label>/ (meta.txt, drive.log, the dumps, the launch's logs) and <label>.canary.log beside it. Asserts before the
# launch: the --no-scene environment canary, the instance probe; inside f124_run.sh: binary md5, the probe again, the GPU
# gate, the field pair in == out, teardown by pid. Exit: the driver's (0 all PASS, 1 a FAIL, 3 a scene not reached),
# 6 the canary refused, 2 an engine is live, else f124_run.sh's.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LABEL=${1:?label}; SCENE=${2:?ladder, control, switch or night}; EXPECT=${3:?before or after}; PIN=${4:-on}; HOLDARG=${5:-}
umask 0002
export SC_SEAM_RECORD=1 PYTHONDONTWRITEBYTECODE=1
export F124_OUT_ROOT="$HERE/artifacts/f148" F124_TIMEOUT=${F124_TIMEOUT:-1200}
mkdir -p "$F124_OUT_ROOT"
bash "$HERE/f56_canary.sh" --no-scene --out "$F124_OUT_ROOT/$LABEL.canary" > "$F124_OUT_ROOT/$LABEL.canary.log" 2>&1
CRC=$?
echo "canary --no-scene exit $CRC"
[ "$CRC" -eq 0 ] || { echo "STOP: the environment canary refused, nothing launched - $F124_OUT_ROOT/$LABEL.canary.log"; exit 6; }
bash "$HERE/sc_instances.sh" --assert "f148-$LABEL" || exit 2
bash "$HERE/f124_run.sh" "$LABEL" f148_cross.py --scene "$SCENE" --expect "$EXPECT" --pin "$PIN" $HOLDARG
