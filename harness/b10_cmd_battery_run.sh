#!/bin/bash
# B10-cmd full regression battery (product code changed => full battery).
# Fresh launch 1: drive_scenes.py (scenes A-D) -> predict.py P1-P5 per scene +
# orientation_check.py (orientation spectrum + P-d). Fresh launch 2:
# scene_e_spine.py. init_fov=340, FISHEYE (already the config default).
# Config restored byte-identical (md5 asserted). Stale-instance guard.
# EXIT CODES: 0 green - 1 a launch never came up - 7 scene E failed - 4 the
# frozen ssystem corpus is not the pristine one - 3 config.ini was not restored
# byte-identically.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:-$HERE/artifacts/b10cmd_battery}
CFG=~/.spacecrafter/config.ini
mkdir -p "$OUT"

launch() {  # $1 = attempt label; sets APPPID, UP
    pkill -f "build-claude/src/spacecrafter" 2>/dev/null && sleep 3
    fuser -k 7805/tcp 2>/dev/null && sleep 2
    for attempt in 1 2 3; do
        DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.$1.$attempt.log" 2>&1 &
        APPPID=$!
        UP=0
        for i in $(seq 1 45); do
            sleep 2
            if ss -ltn 2>/dev/null | grep -q ':7805 '; then UP=1; break; fi
            kill -0 $APPPID 2>/dev/null || break
        done
        [ "$UP" = 1 ] && { echo "[$1] tcp up (attempt $attempt)"; return 0; }
        kill -9 $APPPID 2>/dev/null; sleep 3
    done
    return 1
}

MD5_IN=$(md5sum "$CFG" | cut -d' ' -f1)
SSYS_IN=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
# The frozen shipped corpus this battery's numbers belong to. Overridable for a
# different delivery; measured 2026-07-30 on the installed ssystem.ini, which is
# the same value §11.103/§11.106/§11.107/§11.115 asserted.
SSYS_PRISTINE="${SSYS_PRISTINE:-545a51ef76294891579a1fc2fe13792b}"
cp "$CFG" "$OUT/config.ini.bak"
sed -i 's/^init_fov *=.*/init_fov                        = 340/' "$CFG"

# ============ LAUNCH 1: scenes A-D + orientation ============
rm -f /tmp/gen_a.json /tmp/gen_b.json /tmp/gen_moon.json /tmp/gen_mars.json /tmp/gen_mars_2.json
launch scenesAD || { echo "FATAL launch1"; cp "$OUT/config.ini.bak" "$CFG"; exit 1; }
sleep 8
python3 "$HERE/drive_scenes.py" > "$OUT/drive_scenes.log" 2>&1
echo "drive_scenes exit=$?"
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 4; kill -9 $APPPID 2>/dev/null; }

echo "=== P1-P5 per scene ==="
for sc in a:SceneA_earth100 b:SceneB_earth50k moon:SceneC_onMoon mars:SceneD_onMars_d1 mars_2:SceneD_onMars_d2; do
    f=${sc%%:*}; label=${sc##*:}
    dump=/tmp/gen_${f}.json
    [ -f "$dump" ] || { echo "$label MISSING dump"; continue; }
    python3 "$HERE/predict.py" "$dump" > "$OUT/predict_${f}.log" 2>&1
    echo "--- $label ($dump) ---"
    grep -a "== P4\|== P3\|== P5\|== P1\|== P2" "$OUT/predict_${f}.log" | head -6
done
echo "=== orientation + P-d (scene A) ==="
python3 "$HERE/orientation_check.py" /tmp/gen_a.json > "$OUT/orientation.log" 2>&1
grep -a "parity-RESTORED\|P-d reference" "$OUT/orientation.log"

# ============ LAUNCH 2: scene E ============
launch sceneE || { echo "FATAL launch2"; cp "$OUT/config.ini.bak" "$CFG"; exit 1; }
sleep 8
python3 "$HERE/scene_e_spine.py" "$OUT" > "$OUT/scene_e.log" 2>&1
SE=$?
echo "scene_e_spine exit=$SE"
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 4; kill -9 $APPPID 2>/dev/null; }
echo "=== scene E result ==="
grep -a "^OK\|^FAIL" "$OUT/scene_e.log" | tail -30
FAILN=$(grep -ac "^FAIL" "$OUT/scene_e.log")
echo "scene E FAIL lines = $FAILN"

# ============ restore ============
cp "$OUT/config.ini.bak" "$CFG"
MD5_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
SSYS_OUT=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
# ASSERT, not echo (§11.101(g) class, F0 precedent in b5_oort_run.sh /
# b5_ladder_run.sh): this runner EDITS config.ini, so a failed restore silently
# re-specifies every later run (the B26 leftover-file class); and it reads the
# frozen ssystem corpus, so a corpus that is not the pristine one makes every
# number it prints belong to a different product (§2.0 D9).
# The printed ssystem expectation was 62239656ee1fb3835e58acc1fb34f5ba - STALE
# by many waves and never compared to anything, which is exactly what an echo
# buys. It is now the measured pristine value and it is asserted.
if [ "$MD5_IN" = "$MD5_OUT" ]; then MD5RC=0
else MD5RC=3; fi
echo "config md5 in=$MD5_IN out=$MD5_OUT $([ $MD5RC -eq 0 ] && echo OK || echo 'MISMATCH - config.ini NOT restored byte-identically')"
if [ "$SSYS_IN" = "$SSYS_OUT" ] && [ "$SSYS_OUT" = "$SSYS_PRISTINE" ]; then SSYSRC=0
else SSYSRC=4; fi
echo "ssystem md5 in=$SSYS_IN out=$SSYS_OUT pristine=$SSYS_PRISTINE $([ $SSYSRC -eq 0 ] && echo OK || echo 'MISMATCH - the frozen corpus is not the pristine one')"
[ "$SE" = 0 ] && [ "$FAILN" = 0 ] && echo "BATTERY: scene E GREEN" || echo "BATTERY: scene E has failures"
# Precedence: a driver failure dominates (it is the reason the run exists), then
# the corpus identity, then the config restore.
[ "$SE" = 0 ] && [ "$FAILN" = 0 ] || exit 7
[ $SSYSRC -ne 0 ] && exit $SSYSRC
exit $MD5RC
