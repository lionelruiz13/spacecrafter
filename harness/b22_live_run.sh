#!/bin/bash
# B22 live cross-fade fresh-launch runner (INTENT 11.64 / 11.80 / 13.B B22).
# Drives the universe-executor collapse band on the composed screen.
#
# Hard-won launch discipline (all three cost real debugging time, recorded so the
# next galactic-mode harness need not re-learn them):
#  1. STALE INSTANCES: the app persists NOTHING to disk, but a not-fully-killed
#     prior instance keeps owning port 7805, and a "fresh" driver then connects to
#     the OLD session's galactic state (moveto lat/lon lands at the 3.2e9 datum,
#     select fails). The app FORKS, so killing the captured pid is insufficient -
#     kill every instance + fuser the port, and assert the port is free.
#     [CORRECTED 2026-09-12, F109: this line used to say "kill by command-line
#     match", and that is exactly what broke - see kill_instances() below. The
#     match is on /proc/<pid>/exe now, which no shell can ever satisfy.]
#  2. INTERMITTENT STARTUP CRASH (§11.15d class): the async texture loader crashes
#     during nebulae/dso streaming under host contention (a concurrent Minecraft
#     JVM was hammering this box). Retry the launch until one survives to TCP.
#  3. CONFIG DRIFT: the app rewrites config.ini on clean shutdown (b13 note); a bak
#     taken after a prior run left init_fov=340 perpetuates it. Restore from a
#     KNOWN-pristine bak and assert md5, or the fov silently changes px calibration.
# Fov is ALSO forced by command in the drivers (zoom fov 340) - do not rely on the
# config value surviving.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
DRIVER=${1:?driver.py}
OUT=${2:-$HERE/artifacts/b22live}
STATS=${STATS:-false}          # STATS=true -> query_statistics (cost measurement)
CFG=~/.spacecrafter/config.ini
mkdir -p "$OUT" "$OUT/log"
rm -f "$OUT"/*.png "$OUT"/*.json "$OUT"/statistics.dat "$OUT"/app.log

# Kill any running spacecrafter instance by /proc IDENTITY, never by
# command-line text. MEASURED 2026-09-12 (F109, INTENT 11.231): the previous
# form, `pkill -9 -f "/sc_"`, matches EVERY process whose command line contains
# that string - including the shell that invoked this runner as
# `SC_BIN=/home/claude/sc-f109/sc_f109_iso ./b22_live_run.sh ...`, which is the
# documented way to drive a staging binary (they are named sc_*). It killed the
# caller at attempt 1 of an F109 arm; the sweep survived only because this
# script was already orphaned by then, and a kill one loop-turn later would
# have skipped the config restore below and left the field's init_fov at 340.
# /proc/<pid>/exe is a real file, so a shell can never match it: the F26
# /proc-identity probe (INTENT 11.134(b), 11.121(m)) in kill form. NB the same
# blind spot in the OTHER direction lives in f26_epoch.sh:46, f27_reply.py:107
# and f56_canary.sh:470, which assert `comm == "spacecrafter"` exactly and so
# do NOT see a running staging binary at all (its comm is its own basename).
kill_instances() {
    for p in /proc/[0-9]*; do
        exe=$(readlink "$p/exe" 2>/dev/null) || continue
        case "${exe##*/}" in
            spacecrafter|sc_*) kill -9 "${p#/proc/}" 2>/dev/null ;;
        esac
    done
}

MD5_CFG_IN=$(md5sum "$CFG" | cut -d' ' -f1)
MD5_SS_IN=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
cp "$CFG" "$OUT/config.ini.bak"
# The field's config.ini comes back on EVERY exit path, not just the two the
# happy path and the no-tcp path cover: an interrupted or killed run used to
# leave init_fov=340 in the field, and every later task asserts md5 03fbee59
# (F109; the explicit restores below stay, the trap is idempotent).
trap 'cp "$OUT/config.ini.bak" "$CFG" 2>/dev/null' EXIT INT TERM
sed -i 's/^init_fov *=.*/init_fov                        = 340/' "$CFG"
sed -i "s/^query_statistics *=.*/query_statistics               = $STATS/" "$CFG"

echo "binary: $BIN"; ls -l --time-style=full-iso "$BIN"; echo "STATS=$STATS"
UP=0
for attempt in $(seq 1 8); do
    kill_instances; fuser -k 7805/tcp 2>/dev/null; sleep 2
    ss -ltn 2>/dev/null | grep -q ':7805 ' && { echo "port still held, waiting"; sleep 3; }
    ( cd "$OUT" && DISPLAY=${DISPLAY:-:2} "$BIN" > "$OUT/app.log" 2>&1 & echo $! > "$OUT/app.pid" )
    APPPID=$(cat "$OUT/app.pid"); echo "launch attempt $attempt pid=$APPPID"
    for i in $(seq 1 60); do
        sleep 2
        ss -ltn 2>/dev/null | grep -q ':7805 ' && { UP=1; echo "tcp up ${i}x2s"; break; }
        kill -0 $APPPID 2>/dev/null || { echo "app died before tcp (attempt $attempt)"; break; }
    done
    [ "$UP" = 1 ] && break
done
[ "$UP" != 1 ] && { echo "app never reached tcp"; cp "$OUT/config.ini.bak" "$CFG"; exit 1; }
sleep 8

python3 "$HERE/$DRIVER" "$OUT" > "$OUT/drive.log" 2>&1
DRC=$?; echo "driver exit=$DRC"
sleep 2
kill -INT $APPPID 2>/dev/null; sleep 4
kill_instances; sleep 2
# query_statistics writes ~/.spacecrafter/log/statistics.dat (fixed path, not cwd)
[ -f ~/.spacecrafter/log/statistics.dat ] && cp ~/.spacecrafter/log/statistics.dat "$OUT/statistics.dat"

cp "$OUT/config.ini.bak" "$CFG"
MD5_CFG_OUT=$(md5sum "$CFG" | cut -d' ' -f1)
MD5_SS_OUT=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
echo "config md5 in=$MD5_CFG_IN out=$MD5_CFG_OUT $([ "$MD5_CFG_IN" = "$MD5_CFG_OUT" ] && echo OK || echo MISMATCH)"
echo "ssystem md5 in=$MD5_SS_IN out=$MD5_SS_OUT $([ "$MD5_SS_IN" = "$MD5_SS_OUT" ] && echo OK || echo MISMATCH)"
echo "--- driver log ---"; cat "$OUT/drive.log"
exit $DRC
