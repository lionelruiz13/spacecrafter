#!/bin/bash
# F18 mid-band driver (INTENT §5.52). One FRESH launch per invocation; SC_BIN
# selects the binary so the same driver produces the pre/post pair.
#
#   SC_BIN=<pre>  ./f18_run.sh artifacts/f18_pre
#   SC_BIN=<post> ./f18_run.sh artifacts/f18_post
#   ./f18_disc.py artifacts/f18_post
#
# Extra args after the outdir are the target body list (default Sun Mars Jupiter).
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:-$HERE/artifacts/f18}
case "$OUT" in /*) ;; *) OUT="$PWD/$OUT";; esac
shift || true
mkdir -p "$OUT"
rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log

# Concurrent-instance assert (§0.5 / §11.121(m)): ANY account's spacecrafter
# shares ~/.spacecrafter, so a concurrent launcher confounds the measurement.
CONC=$(pgrep -c -f '[s]pacecrafter/build.*/src/spacecrafter' || true)
echo "concurrent spacecrafter processes before launch: ${CONC:-0}"
if [ "${CONC:-0}" != "0" ]; then echo "ABORT: concurrent instance"; exit 2; fi

echo "binary: $BIN"
ls -la "$BIN"
DISPLAY=${DISPLAY:-:2} VK_LOADER_DEBUG=layer "$BIN" > "$OUT/app.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID"

for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i} x2s"; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; tail -30 "$OUT/app.log"; exit 1; }
done
sleep 8

python3 "$HERE/f18_midband.py" "$OUT" "$@" > "$OUT/drive.log" 2>&1
echo "driver exit=$?"
sleep 3
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 6; kill -9 $APPPID 2>/dev/null; }

echo "--- validation layer present? ---"
grep -ac "Loading layer library libVkLayer_khronos_validation" "$OUT/app.log" | sed 's/^/khronos_validation load lines: /'
echo "--- VUID ---"
grep -ac "VUID" "$OUT/app.log" | sed 's/^/VUID lines: /'
echo "--- probe lines (if the binary carries the F18 probe) ---"
grep -a "F18PROBE" "$OUT/app.log" | head -40
echo "--- driver tail ---"
tail -20 "$OUT/drive.log"
