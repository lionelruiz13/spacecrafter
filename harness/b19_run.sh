#!/bin/bash
# B19 hidden-body position-freshness run - one FRESH launch (INTENT 11.54).
# Added 2026-07-30 (B39 / INTENT 11.117): the gate itself is unchanged, but it
# had no runner, and B39 retires the mechanism it locks (the translation tick)
# in favour of the D8 use-site barrier - so it has to be RE-RUN, not inherited
# (§11.113(b)(iii) says exactly that). The outdir is absolute: the app writes
# dumps from ITS OWN cwd.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
DRIVER=${1:-b19_hidden_tick.py}
OUT=$(readlink -m "${2:-$HERE/artifacts/b19}")
mkdir -p "$OUT"
rm -f "$OUT"/*.json "$OUT"/*.log

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
sleep 8

python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
RC=$?
echo "driver exit=$RC"
sleep 2
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 5; kill -9 $APPPID 2>/dev/null; }

if ! cmp -s "$OUT/config.ini.bak" ~/.spacecrafter/config.ini; then
    echo "FAIL config.ini not restored byte-identically"; RC=1
fi
md5sum ~/.spacecrafter/config.ini ~/.spacecrafter/ssystem.ini | tee "$OUT/md5.out"
cmp -s "$OUT/md5.in" "$OUT/md5.out" || { echo "FAIL field-data md5 changed"; RC=1; }
echo "--- driver tail ---"
tail -40 "$OUT/drive.log"
exit $RC
