#!/bin/bash
# F18 §5.54 second-width gate runner. EDITS config.ini render_size, so it
# restores it and ASSERTS the restore (the §11.101(g) class: a failed restore is
# silent otherwise). One fresh launch per width.
#
#   ./f18_gate_run.sh <outdir> <width> [body]
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT=${1:?outdir}; case "$OUT" in /*) ;; *) OUT="$PWD/$OUT";; esac
W=${2:?width}
BODY=${3:-Mars}
CFG=~/.spacecrafter/config.ini
mkdir -p "$OUT"
rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/*.log

CONC=$(pgrep -c -f '[s]pacecrafter/build.*/src/spacecrafter' || true)
[ "${CONC:-0}" = "0" ] || { echo "ABORT: concurrent instance ($CONC)"; exit 2; }

cp "$CFG" "$OUT/config.ini.bak"
IN_MD5=$(md5sum "$CFG" | cut -d' ' -f1)
sed -i "s/^render_size .*/render_size                    = $W/" "$CFG"
echo "render_size now: $(/usr/bin/grep -a render_size "$CFG")"

DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.log" 2>&1 &
APPPID=$!
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i} x2s"; break; fi
    kill -0 $APPPID 2>/dev/null || { echo "app died"; tail -20 "$OUT/app.log"; cp "$OUT/config.ini.bak" "$CFG"; exit 1; }
done
sleep 8
python3 "$HERE/f18_gate.py" "$OUT" "$W" "$BODY" > "$OUT/drive.log" 2>&1
echo "driver exit=$?"
sleep 2
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 6; kill -9 $APPPID 2>/dev/null; }
sleep 2   # the app rewrites config.ini on shutdown - restore AFTER it is dead

cp "$OUT/config.ini.bak" "$CFG"
OUT_MD5=$(md5sum "$CFG" | cut -d' ' -f1)
[ "$IN_MD5" = "$OUT_MD5" ] || { echo "FATAL config.ini not restored ($IN_MD5 != $OUT_MD5)"; exit 3; }
echo "config.ini restored md5 $OUT_MD5 OK"
tail -40 "$OUT/drive.log"
