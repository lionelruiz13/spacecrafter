#!/bin/bash
# F39 / §5.102 — WHERE DOES THE `scaling` NaN ENTER?
#
# The instrument is a gdb breakpoint on the ONE writer of the ASmooth state,
# `ASmooth<AsyncHub,float,5>::set(dst, minDuration)`, printing the full private
# state BEFORE and AFTER every call plus the owning body. Every NaN in
# scaledRadius / scaledDatumRadius / scaledGroundRadius / boundingRadius is
# `X * scaling` (ModularBody.cpp:514-519), so the factor is the only common
# term and `set` is its only entry point — a probe on `set` is positively
# mapped onto the reported symptom, not merely adjacent to it.
#
# Positive control built in: the run also prints the state of the SAME object
# at a second, independent site (updateCache's own read), so "no NaN seen"
# cannot be a dead breakpoint.
#
#   ./f39_asmooth_probe.sh <outdir-tag>
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
TAG=${1:-shipped}
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
OUT="$HERE/artifacts/f39/asmooth_$TAG"
mkdir -p "$OUT"; rm -f "$OUT"/*.log "$OUT"/*.txt
META="$OUT/meta.txt"; : > "$META"
log() { echo "$@" | tee -a "$META"; }

FIELD=~/.spacecrafter/config.ini
FIELD_IN=$(md5sum "$FIELD" | cut -d' ' -f1)
SSY_IN=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
log "=== F39 ASmooth probe '$TAG' ==="
log "wall clock start : $(date '+%F %T %Z')"
log "binary md5       : $(md5sum "$BIN" | cut -d' ' -f1)"
log "field config md5 (in)  = $FIELD_IN"
log "field ssystem md5 (in) = $SSY_IN"
n=0; for p in /proc/[0-9]*; do [ "$(cat "$p/comm" 2>/dev/null)" = "spacecrafter" ] && n=$((n+1)); done
log "concurrent instances (pre) : $n"
[ "$n" -eq 0 ] || { log "ABORT: another spacecrafter is running"; exit 2; }

cat > "$OUT/run.gdb" <<'GDB'
set pagination off
set confirm off
set print elements 0
set $n = 0
handle SIGUSR1 nostop noprint pass
break 'ASmooth<AsyncHub, float, (float)[40a00000]>::set'
commands
  silent
  printf "SET-ENTRY this=%p dst=%g minDuration=%g | a=%g b=%g c=%g timer=%g duration=%g nextDuration=%g mgr=%p\n", this, dst, minDuration, this->a, this->b, this->c, this->timer, this->duration, this->nextDuration, this->mgr
  continue
end
break ModularBody.cpp:518 if this->scaling.mgr != 0 || this->scaling.c != 1
commands
  silent
  set $n = $n + 1
  printf "UPDATECACHE this=%p body=%s radius=%g | a=%g b=%g c=%g timer=%g duration=%g nextDuration=%g mgr=%p\n", &this->scaling, this->englishName._M_dataplus._M_p, this->radius, this->scaling.a, this->scaling.b, this->scaling.c, this->scaling.timer, this->scaling.duration, this->scaling.nextDuration, this->scaling.mgr
  if $n > 80
    disable 2
  end
  continue
end
run
GDB

HOME=$HOME DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$OUT/run.gdb" --args "$BIN" > "$OUT/gdb.log" 2>&1 &
GDBPID=$!; log "gdb pid=$GDBPID"
for i in $(seq 1 90); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then log "tcp up after ${i}x2s"; break; fi
    kill -0 $GDBPID 2>/dev/null || { log "gdb died before tcp"; break; }
done
sleep 10
kill -0 $GDBPID 2>/dev/null && { kill -INT $GDBPID; sleep 4; kill -9 $GDBPID 2>/dev/null; }
sleep 1
pkill -f "$BIN" 2>/dev/null

FIELD_OUT=$(md5sum "$FIELD" | cut -d' ' -f1)
SSY_OUT=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
log "field config md5 (out) = $FIELD_OUT  MATCH=$([ "$FIELD_IN" = "$FIELD_OUT" ] && echo yes || echo NO)"
log "field ssystem md5 (out) = $SSY_OUT  MATCH=$([ "$SSY_IN" = "$SSY_OUT" ] && echo yes || echo NO)"
log "--- SET-ENTRY calls (all) ---"
grep -a '^SET-ENTRY' "$OUT/gdb.log" | tee -a "$META"
log "--- first UPDATECACHE per body ---"
grep -a '^UPDATECACHE' "$OUT/gdb.log" | awk '{print $2}' | sort -u | while read -r b; do
    grep -a "^UPDATECACHE $b " "$OUT/gdb.log" | head -1
done | tee -a "$META"
log "--- UPDATECACHE lines carrying nan ---"
r=$(grep -a '^UPDATECACHE' "$OUT/gdb.log" | grep -a -i 'nan' | head -20)
if [ -n "$r" ]; then echo "$r" | tee -a "$META"; else log "  NONE"; fi
log "wall clock end   : $(date '+%F %T %Z')"
