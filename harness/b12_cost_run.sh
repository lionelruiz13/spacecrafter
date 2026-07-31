#!/bin/bash
# B12 near-surface star family, slice 1 (INTENT §11.123). One FRESH launch per
# invocation; SC_BIN selects the binary so the same driver produces the pre/post
# pair the star-gated inertness check needs:
#
#   SC_BIN=<pre>  ./b12_run.sh artifacts/b12_pre
#   SC_BIN=<post> ./b12_run.sh artifacts/b12_post
#   ./b12_limb.py artifacts/b12_post artifacts/b12_pre
#
# Validation layer on (debug_layer in config); the app log is kept so the
# "no missing loader" and "zero VUID" claims have a source.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:-$HERE/artifacts/b12}
mkdir -p "$OUT"
rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log

echo "binary: $BIN"
ls -la "$BIN"
# VK_LOADER_DEBUG=layer makes the layer's presence POSITIVELY observable in the
# log (a silent no-op probe turns observation into fiction - §11.44's rule).
DISPLAY=${DISPLAY:-:2} VK_LOADER_DEBUG=layer "$BIN" > "$OUT/app.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID"

for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i} x2s"; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; tail -30 "$OUT/app.log"; exit 1; }
done
sleep 8

python3 "$HERE/b12_cost.py" "$OUT" > "$OUT/drive.log" 2>&1
echo "driver exit=$?"
sleep 3
kill -0 $APPPID 2>/dev/null && { echo "shutting app"; kill -INT $APPPID; sleep 6; kill -9 $APPPID 2>/dev/null; }

echo "--- validation layer present? ---"
grep -ac "Loading layer library libVkLayer_khronos_validation" "$OUT/app.log" | sed 's/^/khronos_validation load lines: /'
grep -a "Insert instance layer .*validation" "$OUT/app.log" | head -2
echo "--- VUID / errors ---"
grep -ac "VUID" "$OUT/app.log" | sed 's/^/VUID lines: /'
grep -a "VUID" "$OUT/app.log" | head -5
echo "--- missing loader warnings ---"
grep -ac "No loader available" "$OUT/app.log" | sed 's/^/missing-loader lines: /'
echo "--- driver tail ---"
tail -25 "$OUT/drive.log"
