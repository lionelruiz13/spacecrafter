#!/bin/bash
# B39 hidden = as-if-nonexistent run - one FRESH launch (INTENT §11.117).
# Instruments: the process DUMP (ModularBody::dumpTrace - relation, trail
# accumulateCount, evalCount) and the composed SCREEN (self-referential px
# diffs against a measured noise floor).  No gdb probe.
#
# The known intermittent shutdown segfault (INTENT §11.15d) fires AFTER the
# driver exits and does not affect the artifacts.
#
#   ./b39_run.sh [driver] [outdir]
#   SC_BIN=<other binary> ./b39_run.sh    # counterfactual / pre-fix binary
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
DRIVER=${1:-b39_hidden.py}
OUT=${2:-$HERE/artifacts/b39}
ARG=${3:-}
mkdir -p "$OUT"
rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log

echo "binary: $BIN"; ls -l --time-style=full-iso "$BIN"
md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | tee "$OUT/md5.in"
cp ~/.spacecrafter/config.ini "$OUT/config.ini.bak"

DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID"
for i in $(seq 1 40); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i}x2s"; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; tail -20 "$OUT/app.log"; exit 1; }
done
sleep 8   # let the initial async texture loads quiesce

python3 "$HERE/$DRIVER" "$OUT" $ARG > "$OUT/drive.log" 2>&1
RC=$?
echo "driver exit=$RC"
sleep 2
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 5; kill -9 $APPPID 2>/dev/null; }

# The app rewrites config.ini at shutdown - assert it came back byte-identical
# (F0 §11.101(g): assert, never echo).
cp "$OUT/config.ini.bak" /tmp/b39_cfg_ref.ini
if ! cmp -s /tmp/b39_cfg_ref.ini ~/.spacecrafter/config.ini; then
    echo "FAIL config.ini not restored byte-identically"; RC=1
fi
md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | tee "$OUT/md5.out"
cmp -s "$OUT/md5.in" "$OUT/md5.out" || { echo "FAIL field-data md5 changed"; RC=1; }
echo "--- driver tail ---"
tail -60 "$OUT/drive.log"
exit $RC
