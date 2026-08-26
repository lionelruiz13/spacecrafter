#!/bin/bash
# F39 — temp-HOME farm builder + short-run driver, the f38_config.sh pattern.
#
#   f39_farm.sh <outdir> [--fov N] [--sed 's/.../.../'] ...
# Builds /tmp/f39home as a symlink mirror of ~/.spacecrafter with config.ini and
# log/ as REAL files, launches $SC_BIN on it, waits for the TCP port, optionally
# runs a python driver, then kills it and reports the log.
#
# The FIELD config/ssystem md5s are asserted in and out; the farm's config is
# copied out before and after so any rewrite is visible.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
OUT=${1:?outdir}
shift
FARM=/tmp/f39home
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
DRIVER=""
DWELL=25
SEDS=()
ENVS=()
while [ $# -gt 0 ]; do
    case "$1" in
        --sed) SEDS+=("$2"); shift 2 ;;
        --driver) DRIVER="$2"; shift 2 ;;
        --dwell) DWELL="$2"; shift 2 ;;
        --env) ENVS+=("$2"); shift 2 ;;
        *) echo "unknown arg $1"; exit 2 ;;
    esac
done
mkdir -p "$OUT"
META="$OUT/meta.txt"; : > "$META"
log() { echo "$@" | tee -a "$META"; }

FIELD=~/.spacecrafter/config.ini
FIELD_IN=$(md5sum "$FIELD" | cut -d' ' -f1)
SSY_IN=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
log "=== F39 farm run -> $OUT ==="
log "wall clock start : $(date '+%F %T %Z')"
log "binary           : $BIN"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "field config md5 (in)  = $FIELD_IN"
log "field ssystem md5 (in) = $SSY_IN"
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
CFG="$FARM/.spacecrafter/config.ini"
for e in ${SEDS+"${SEDS[@]}"}; do sed -i "$e" "$CFG"; done
cp "$CFG" "$OUT/config_before.ini"
CFG_IN=$(md5sum "$CFG" | cut -d' ' -f1); log "farm config md5 (in) = $CFG_IN"
log "farm scaling keys: $(/usr/bin/grep -a '^flag_moon_scaled\|^moon_scale\|^flag_sun_scaled\|^sun_scale\|^init_fov\|^projection ' "$CFG" | tr '\n' ' ')"

log "extra env        : ${ENVS[*]-(none)}"
env HOME=$FARM DISPLAY=${DISPLAY:-:2} ${ENVS+"${ENVS[@]}"} "$BIN" > "$OUT/stdout.log" 2>&1 &
APPPID=$!; log "app pid=$APPPID"
UP=no
for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then log "tcp up after ${i}x2s"; UP=yes; break; fi
    kill -0 $APPPID 2>/dev/null || { log "app died before tcp"; break; }
done
if [ "$UP" = yes ]; then
    if [ -n "$DRIVER" ]; then
        sleep 8
        python3 "$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
        log "driver exit=$?"
        sleep 2
    else
        sleep "$DWELL"
    fi
fi
kill -0 $APPPID 2>/dev/null && { kill -INT $APPPID; sleep 5; kill -9 $APPPID 2>/dev/null; }
sleep 1

CFG_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
log "farm config md5 (out) = $CFG_OUT  REWRITTEN=$([ "$CFG_IN" = "$CFG_OUT" ] && echo no || echo YES)"
FIELD_OUT=$(md5sum "$FIELD" | cut -d' ' -f1)
SSY_OUT=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
log "field config md5 (out) = $FIELD_OUT  MATCH=$([ "$FIELD_IN" = "$FIELD_OUT" ] && echo yes || echo NO)"
log "field ssystem md5 (out) = $SSY_OUT  MATCH=$([ "$SSY_IN" = "$SSY_OUT" ] && echo yes || echo NO)"
LOGF=$(ls -t "$FARM"/.spacecrafter/log/spacecrafter*.log 2>/dev/null | head -1)
if [ -n "$LOGF" ]; then cp "$LOGF" "$OUT/app.log"; log "app log copied   : $LOGF"; else log "app log          : NONE"; fi
log "wall clock end   : $(date '+%F %T %Z')"
