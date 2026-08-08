#!/bin/bash
# F29 / INTENT §5.46 - one FRESH launch per binary (fresh-launch precondition).
# Preconditions asserted, never assumed: concurrent-instance probe (§0.5, the
# /proc/<pid>/comm form F26 replaced the pgrep pattern with), frozen
# config/ssystem md5 in == out, config.ini restored byte-identically.
#
#   ./f29_run.sh [outdir] [driver]
#   SC_BIN=/abs/pre-fix-binary ./f29_run.sh <outdir>
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:-$HERE/artifacts/f29}
DRIVER=${2:-f29_upchain.py}
mkdir -p "$OUT"
rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log

export XAUTHORITY="${XAUTHORITY_OVERRIDE:-$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.* 2>/dev/null | head -1)}"
export DISPLAY="${DISPLAY_OVERRIDE:-:2}"
xdpyinfo > /dev/null 2>&1 || { echo "FAIL no usable display ($DISPLAY / $XAUTHORITY)"; exit 3; }

echo "binary: $BIN"; ls -l --time-style=full-iso "$BIN"
echo "binary md5: $(md5sum "$BIN" | cut -d' ' -f1)"

# Concurrent-instance assert (§11.121(m), instrument per §11.134(b)): reads
# /proc/<pid>/comm (the executable's own name - world-readable, covers every
# account, carries no command-line text so it cannot self-match).
CONC=$(/usr/bin/grep -l -x 'spacecrafter' /proc/[0-9]*/comm 2>/dev/null | wc -l)
if [ "${CONC:-0}" != "0" ]; then
    for c in $(/usr/bin/grep -l -x 'spacecrafter' /proc/[0-9]*/comm 2>/dev/null); do
        echo "  running: $c cmdline=[$(tr '\0' ' ' < "$(dirname "$c")/cmdline")]"
    done
    echo "ABORT: concurrent spacecrafter instance"; exit 2
fi
echo "concurrent instances: 0"

md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | tee "$OUT/md5.in"
cp ~/.spacecrafter/config.ini "$OUT/config.ini.bak"

"$BIN" > "$OUT/app.log" 2>&1 &
APPPID=$!
echo "app pid=$APPPID"
for i in $(seq 1 40); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i}x2s"; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; tail -20 "$OUT/app.log"; exit 1; }
done
sleep 8   # let the initial async texture loads quiesce

python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
RC=$?
echo "driver exit=$RC"
sleep 2
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 5; kill -9 $APPPID 2>/dev/null; }

cp "$OUT/config.ini.bak" /tmp/f29_cfg_ref.ini
if ! cmp -s /tmp/f29_cfg_ref.ini ~/.spacecrafter/config.ini; then
    echo "FAIL config.ini not restored byte-identically"; RC=1
fi
md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | tee "$OUT/md5.out"
cmp -s "$OUT/md5.in" "$OUT/md5.out" || { echo "FAIL field-data md5 changed"; RC=1; }
echo "--- driver tail ---"
tail -50 "$OUT/drive.log"
exit $RC
