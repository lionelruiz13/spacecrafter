#!/bin/bash
# F128 -- ONE command per leg of the identity instrument of row B47 (INTENT 11.267).
#
#   bash f128_leg.sh SOL|GAL|UNI [--layer milky_way] [--phase run|floor|able] [--rebank]
#   bash f128_leg.sh TILT [--expect system|owner] [--phase run|able]
#     SC_BIN  another binary (a rebuilt one after a code change); DISPLAY defaults to :2
#
# It launches NOTHING itself: every capture is one fresh launch through f124_run.sh (binary md5,
# instance probe, GPU gate, field pair in == out, teardown by pid) with the driver f128_legs.py;
# the pixels are judged by f128_compare.py. Records: artifacts/f128/<LEG>_<layer>/<phase>_<clock>/.
#   floor  the same pin twice, both pins (4 launches): the measured floor -> floor.json, and the
#          first capture of each pin becomes the STORED REFERENCE of that pin. Refuses to replace
#          an existing floor.json without --rebank.
#   able   one-pin perturbations of the new pin against the stored old-pin reference: each MUST
#          come out red (exit 0 = red as it must be, 1 = the leg is blind). Run before trusting.
#   run    (default) a fresh capture per pin: old against new, and each against the stored
#          reference of the SAME pin (before / after a code change). Exit 0 = every check PASS.
# TILT: run = the new pin judged under --expect, the old pin as control (2 launches);
#       able = the poke withheld (must FAIL). `--expect owner` is red at 47b348bf by prediction.
set -u
umask 0002
HERE="$(cd "$(dirname "$0")" && pwd)"
LEG=${1:?leg: SOL GAL UNI TILT}; shift
LAYER=milky_way; PHASE=run; EXPECT=system; REBANK=0
while [ $# -gt 0 ]; do
    case "$1" in
        --layer) LAYER=$2; shift 2;;
        --phase) PHASE=$2; shift 2;;
        --expect) EXPECT=$2; shift 2;;
        --rebank) REBANK=1; shift;;
        *) echo "unknown argument $1"; exit 2;;
    esac
done
export DISPLAY=${DISPLAY:-:2}
export F124_OUT_ROOT="$HERE/artifacts/f128"
export F124_TIMEOUT=${F124_TIMEOUT:-420}
SCENE="${LEG}_${LAYER}"
ROOT="$F124_OUT_ROOT/$SCENE"
STAMP=$(date '+%Y%m%d-%H%M%S')
RUN="$ROOT/${PHASE}_$STAMP"
mkdir -p "$RUN"
FLOOR="$ROOT/floor.json"
RED=0

launch() {  # launch <name> <driver args...> : one fresh launch, the runner's exit in LRC
    local name=$1; shift
    bash "$HERE/f124_run.sh" "$SCENE/${PHASE}_$STAMP/$name" f128_legs.py --leg "$LEG" --layer "$LAYER" "$@" > "$RUN/$name.runner.txt" 2>&1
    LRC=$?
    echo "launch $name ($*): runner exit $LRC  [$(LC_ALL=C /usr/bin/grep -c '^PASS' "$RUN/$name.runner.txt") PASS, $(LC_ALL=C /usr/bin/grep -c '^FAIL' "$RUN/$name.runner.txt") FAIL]"
    LC_ALL=C /usr/bin/grep -aE '^(FAIL|NOTE|--|===)' "$RUN/$name.runner.txt"
    case $LRC in 2|3|4|5) echo "STOP: the runner refused or the scene was not reached (exit $LRC)"; exit $LRC;; esac
}

judge() {   # judge <name> <want same|differ> <dirA> <dirB> : RED counts a check that did not come out as wanted
    python3 "$HERE/f128_compare.py" pair "$RUN/$1.json" "$FLOOR" "$3" "$4" --want "$2" --name "$1" || RED=$((RED + 1))
}

ref_of() {  # the stored capture of pin $1 (old|new), from floor.json
    python3 - "$FLOOR" "$1" <<'EOF'
import json, sys
for p in json.load(open(sys.argv[1]))["pairs"]:
    if p["a"]["state"].get("drawnPath") == sys.argv[2]:
        print(p["a"]["dir"]); break
EOF
}

{
echo "=== F128 leg $LEG layer $LAYER phase $PHASE expect $EXPECT  $(date '+%F %T %Z')  binary md5 $(md5sum "${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}" | cut -c1-8) ==="
if [ "$LEG" = TILT ]; then
    if [ "$PHASE" = able ]; then
        launch new_withheld --pin on --expect "$EXPECT" --perturb withhold-poke
        if [ "$LRC" -eq 1 ]; then echo "=== ABLE-TO-FAIL TILT: RED as it must be with the poke withheld ==="; VERDICT=0
        else echo "=== ABLE-TO-FAIL TILT: STAYED GREEN (exit $LRC) - THE LEG IS BLIND ==="; VERDICT=1; fi
    else
        launch new --pin on --expect "$EXPECT"; NRC=$LRC
        launch old_control --pin off --expect "$EXPECT"; ORC=$LRC
        VERDICT=$(( NRC != 0 || ORC != 0 ))
        echo "=== TILT --expect $EXPECT: $([ $VERDICT -eq 0 ] && echo PASS || echo FAIL) (new pin exit $NRC, old-pin control exit $ORC) ==="
    fi
elif [ "$PHASE" = floor ]; then
    if [ -e "$FLOOR" ] && [ "$REBANK" -eq 0 ]; then echo "floor.json exists: --rebank to replace the stored reference"; exit 2; fi
    launch oldA --pin off; launch oldB --pin off; launch newA --pin on; launch newB --pin on
    python3 "$HERE/f128_compare.py" floor "$FLOOR" "$SCENE" "$RUN/oldA" "$RUN/oldB" "$RUN/newA" "$RUN/newB"; VERDICT=$?
    cp "$FLOOR" "$RUN/floor.json"
else
    [ -e "$FLOOR" ] || { echo "no floor.json for $SCENE: run --phase floor first"; exit 2; }
    REF_OLD=$(ref_of old); REF_NEW=$(ref_of new)
    echo "stored reference: old = $REF_OLD ; new = $REF_NEW"
    if [ "$PHASE" = able ]; then
        launch new_intensity --pin on --perturb intensity
        judge able_intensity_vs_old_ref differ "$REF_OLD" "$RUN/new_intensity"
        judge able_intensity_vs_new_ref differ "$REF_NEW" "$RUN/new_intensity"
        if [ "$LEG" = SOL ]; then
            launch new_tilt --pin on --perturb tilt-system
            judge able_tilt_vs_old_ref differ "$REF_OLD" "$RUN/new_tilt"
            judge able_tilt_vs_new_ref differ "$REF_NEW" "$RUN/new_tilt"
        fi
        VERDICT=$(( RED != 0 ))
        echo "=== ABLE-TO-FAIL $SCENE: $([ $RED -eq 0 ] && echo 'every perturbed comparison came out RED as it must' || echo "$RED comparison(s) STAYED GREEN - THE LEG IS BLIND") ==="
    else
        launch old --pin off; launch new --pin on
        judge cross_pin_old_vs_new same "$RUN/old" "$RUN/new"
        judge old_vs_stored_old same "$REF_OLD" "$RUN/old"
        judge new_vs_stored_new same "$REF_NEW" "$RUN/new"
        VERDICT=$(( RED != 0 ))
        echo "=== $SCENE: $([ $RED -eq 0 ] && echo 'ALL PASS' || echo "$RED comparison(s) FAILED") ==="
    fi
fi
echo "records: $RUN   end $(date '+%F %T %Z')"
bash "$HERE/sc_instances.sh" --assert "f128-$LEG-close"
exit $VERDICT
} 2>&1 | tee "$RUN/summary.txt"
exit ${PIPESTATUS[0]}
