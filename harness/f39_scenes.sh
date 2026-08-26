#!/bin/bash
# F39 (adapted from f38_scenes.sh) — the A–D scene battery (drive_scenes.py, §11.16/§11.17) run PRE and POST
# on the same config, because the battery passes THROUGH one of the four sites
# this task changed: it does `select planet Moon` + `flag track_object on`, then
# later `select planet Earth` WHILE TRACKING — which is exactly the core.cpp:2313
# ENABLE. A battery that merely "still passes" would not say whether the change
# moved anything; two legs on one config do.
#
#   ./f38_scenes.sh <leg> <binary>
#
# `drive_scenes.py` requires `init_fov = 340` (README), so it runs on the
# temp-HOME farm — the FIELD config stays pristine (03fbee59 asserted in/out).
# The driver writes to fixed /tmp/gen_*.json paths, so each leg copies them out
# before the next one runs.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LEG=${1:?leg}
BIN=${2:?binary}
FARM=/tmp/f39home_scenes
OUT="$HERE/artifacts/f39/scenes_$LEG"
mkdir -p "$OUT"; rm -f "$OUT"/*.json "$OUT"/*.txt "$OUT"/*.log
META="$OUT/meta.txt"; : > "$META"
log() { echo "$@" | tee -a "$META"; }

FIELD=~/.spacecrafter/config.ini
FIELD_IN=$(md5sum "$FIELD" | cut -d' ' -f1)
log "=== F39 scenes leg '$LEG' ==="
log "wall clock start : $(date '+%F %T %Z')"
log "binary           : $BIN"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "field config md5 (in) = $FIELD_IN"
n=0; for p in /proc/[0-9]*; do [ "$(cat "$p/comm" 2>/dev/null)" = "spacecrafter" ] && n=$((n+1)); done
log "concurrent instances (pre) : $n"
[ "$n" -eq 0 ] || { log "ABORT: another spacecrafter is running"; exit 2; }

rm -rf "$FARM"; mkdir -p "$FARM/.spacecrafter"
for f in ~/.spacecrafter/*; do
    b=$(basename "$f")
    [ "$b" = "config.ini" ] && continue
    [ "$b" = "log" ] && continue
    ln -s "$f" "$FARM/.spacecrafter/$b"
done
mkdir -p "$FARM/.spacecrafter/log"
cp "$FIELD" "$FARM/.spacecrafter/config.ini"
sed -i 's/^init_fov .*= .*/init_fov                       = 340/' "$FARM/.spacecrafter/config.ini"
log "farm init_fov    : $(grep -a '^init_fov' "$FARM/.spacecrafter/config.ini")"
rm -f /tmp/gen_*.json

cat > "$OUT/run.gdb" <<'GDB'
set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
run
GDB
HOME=$FARM DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$OUT/run.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!; log "gdb pid=$GDBPID"
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then log "tcp up after ${i}x2s"; break; fi
    kill -0 $GDBPID 2>/dev/null || { log "gdb died before tcp"; exit 1; }
done
sleep 8
python3 "$HERE/drive_scenes.py" > "$OUT/drive.log" 2>&1
log "driver exit=$?"
sleep 2
kill -0 $GDBPID 2>/dev/null && { kill -INT $GDBPID; sleep 4; kill -9 $GDBPID 2>/dev/null; }

for f in /tmp/gen_*.json; do [ -e "$f" ] && cp "$f" "$OUT/"; done
log "dumps captured   : $(ls "$OUT"/gen_*.json 2>/dev/null | wc -l)"
for f in "$OUT"/gen_*.json; do
    [ -e "$f" ] || continue
    b=$(basename "$f" .json)
    python3 "$HERE/analyze.py" "$f" > "$OUT/$b.analysis.txt" 2>&1
    log "--- $b ---"
    tail -6 "$OUT/$b.analysis.txt" | tee -a "$META"
done
FIELD_OUT=$(md5sum "$FIELD" | cut -d' ' -f1)
log "field config md5 (out) = $FIELD_OUT  MATCH=$([ "$FIELD_IN" = "$FIELD_OUT" ] && echo yes || echo NO)"
log "wall clock end   : $(date '+%F %T %Z')"
