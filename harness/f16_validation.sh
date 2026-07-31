#!/bin/bash
# ============================================================================
# F16 (INTENT §11.124) — validation-layer leg of the §5.51 check.
#
# Why this exists as its own instrument: making `virtual ~BodyModule()` real
# runs 14 subclass destructors for the first time ever, and what they release
# is Vulkan state (descriptor Sets back to registry pools, VertexBuffer/
# SharedBuffer ranges back to their BufferMgrs, s_texture references into the
# deferred texture-release ring). Resource-release ORDER is validation-visible,
# so the layer is the on-point detector — the ASan leg sees the C++ half, this
# leg sees the Vulkan half.
#
# Two teardown entries, one per invocation arm:
#   ARM=cmd  — `body action reload` (module destruction MID-SESSION, between
#              frames, with the device live and no waitIdle) then
#              `shutdown action now`.
#   ARM=sig  — same drive, SIGINT teardown (~App does waitIdle first).
# The reload arm is the one that matters most: at shutdown the device is idle
# by construction (App::~App calls VulkanMgr::waitIdle before core.reset()),
# but a reload frees module resources while frames may still be in flight.
#
# The drive turns ON the module families whose resources are created LAZILY at
# first draw (orbit/trail/axis/grid lines): a module whose buffer was never
# built has nothing to release, so a run that did not draw them would prove
# nothing about their release.
#
# POSITIVE confirmation that the layer is loaded (a silent no-op probe turns
# observation into fiction, §11.44's rule): VK_LOADER_DEBUG=layer must produce
# the loader's own "Insert instance layer VK_LAYER_KHRONOS_validation" line.
#
# WHERE THE LAYER'S MESSAGES GO (instrument chain, measured 2026-07-31): the
# VK_LOADER_DEBUG lines land on stderr (app.log), but the LAYER's own messages
# do not — VulkanMgrCreateInfo sets .redirectLog=cLog::writeECLog, so every
# debugCallback message is written to ~/.spacecrafter/log/vulkan.log. Counting
# VUIDs in app.log alone is a silent no-op probe: it would read 0 forever. Both
# files are counted here and vulkan.log is copied into the artifact dir.
# Also: the app's stdout is a FILE, hence block-buffered, so nothing may be
# grepped from app.log until the process has exited. Every count below runs
# post-exit for that reason.
#
# Usage: DISPLAY=:2 [SC_BIN=...] [ARM=cmd|sig] [GDB=1]
#        [STEPS=all|noreload|noflags|axis|noaxis] [AXIS=on|off] [POSCONTROL=1]
#        ./f16_validation.sh <outdir>
# AXIS defaults to OFF in the full drive on purpose: drawing the rotation axis
# even once aborts the process at exit, in a static-teardown defect that has
# nothing to do with the modules (INTENT §5.55, found here, recorded not
# fixed) — leaving it on would end every run at rc=134 for an unrelated
# reason. STEPS=axis / STEPS=noaxis are that defect's own minimal both-ways
# probe; AXIS=on turns it on inside the full drive.
# POSCONTROL=1 turns the layer's BestPractices checks on (VK_LAYER_ENABLES),
# which makes an otherwise-silent channel produce messages on demand: it is the
# proof that a 0-VUID reading is a measurement and not a dead wire.
# ============================================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
ARM="${ARM:-cmd}"
STEPS="${STEPS:-all}"
POSCONTROL="${POSCONTROL:-0}"
OUT=${1:-$HERE/artifacts/f16_validation_$ARM}
CFG=~/.spacecrafter/config.ini
mkdir -p "$OUT"
rm -f "$OUT"/*.png "$OUT"/*.log

MD5_REF=$(md5sum "$CFG" | cut -d' ' -f1)
SSMD5_REF=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
echo "binary: $BIN  ($(stat -c %y "$BIN"))"
echo "arm=$ARM steps=$STEPS poscontrol=$POSCONTROL  config md5=$MD5_REF  ssystem md5=$SSMD5_REF"
grep -a -i "debug_layer" "$CFG" | sed 's/^/config: /'
pgrep -x spacecrafter >/dev/null && { echo "FATAL: a spacecrafter instance is already running"; exit 2; }

tcp_send() {
  { exec 3<>/dev/tcp/127.0.0.1/7805 || return 1
    local c
    for c in "$@"; do printf '%s\n' "$c" >&3; sleep 0.25; done
    sleep 0.3
    exec 3>&-
  } 2>/dev/null
}

LAYER_ENABLES=""
[ "$POSCONTROL" = 1 ] && LAYER_ENABLES="VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT"
# GDB=1 launches the app UNDER gdb (ptrace_scope=1 blocks attach) with the B7
# hunt probe, so a teardown fire is captured WITH a backtrace. gdb perturbs
# teardown timing, so this is a context-capture mode, never a rate measurement.
if [ "${GDB:-0}" = 1 ]; then
  DISPLAY=${DISPLAY:-:2} VK_LOADER_DEBUG=layer VK_LAYER_ENABLES="$LAYER_ENABLES" \
    gdb -q -batch -x "$HERE/b7_hunt_probe.gdb" --args "$BIN" > "$OUT/app.log" 2>&1 &
else
  DISPLAY=${DISPLAY:-:2} VK_LOADER_DEBUG=layer VK_LAYER_ENABLES="$LAYER_ENABLES" "$BIN" > "$OUT/app.log" 2>&1 &
fi
APPPID=$!
echo "app pid=$APPPID"
for i in $(seq 1 60); do
    sleep 1
    ss -ltn 2>/dev/null | grep -q ':7805 ' && { echo "tcp up after ${i}s"; break; }
    kill -0 $APPPID 2>/dev/null || { echo "app died before tcp"; tail -30 "$OUT/app.log"; exit 1; }
done
sleep 5

# Draw the lazily-built module families so that their resources EXIST and are
# therefore released at teardown (see header).
if [ "$STEPS" = axis ] || [ "$STEPS" = noaxis ]; then
  # Minimal both-ways probe for the AxisFamilyData static-teardown fire
  # (INTENT §5.55): the ONLY difference between the two arms is whether the
  # rotation-axis line is ever drawn, i.e. whether the file-static family data
  # in AxisModule.cpp is ever constructed.
  AXFLAG=off; [ "$STEPS" = axis ] && AXFLAG=on
  tcp_send "flag experimental_path on" "timerate rate 0" "meteors zhr 0" \
           "date jday 2461234.0" "select planet Earth" "flag track_object on" \
           "flag planets_axis $AXFLAG"
  sleep 4
  tcp_send "body action screenshot filename $OUT/shot_axis_$AXFLAG.png"
  sleep 2
elif [ "$STEPS" != noflags ]; then
  tcp_send "flag experimental_path on" "timerate rate 0" "meteors zhr 0" \
           "date jday 2461234.0" "flag atmosphere on" \
           "flag planet_orbits on" "flag satellite_orbits on" "flag object_trails on" \
           "flag planets_axis ${AXIS:-off}" "flag planet_grid on" "flag star_lines on"
  sleep 2
  tcp_send "select planet Saturn" "flag track_object on" "zoom fov 10 duration 0"
  sleep 3
  tcp_send "body action screenshot filename $OUT/shot_saturn.png"
  sleep 2
  tcp_send "select planet Earth" "moveto altitude 200000 duration 0"
  sleep 3
  tcp_send "body action screenshot filename $OUT/shot_earth.png"
  sleep 2
else
  tcp_send "flag experimental_path on" "timerate rate 0" "meteors zhr 0" \
           "date jday 2461234.0"
  sleep 3
fi

# THE new path: destroy every body (and every module) mid-session, rebuild.
if [ "$STEPS" != noreload ] && [ "$STEPS" != axis ] && [ "$STEPS" != noaxis ]; then
  echo "--- reload (mid-session module destruction) ---"
  tcp_send "body action reload"
  sleep 6
  tcp_send "body action screenshot filename $OUT/shot_after_reload.png"
  sleep 2
fi

# ---- teardown ----
if [ "$ARM" = sig ]; then
  kill -INT "$(pgrep -x spacecrafter | head -1)"
else
  tcp_send "shutdown action now"
fi
EX=0
for i in $(seq 1 45); do kill -0 $APPPID 2>/dev/null || { EX=1; break; }; sleep 1; done
if [ "$EX" != 1 ]; then echo "!! HUNG (no exit in 45s)"; kill -9 $APPPID 2>/dev/null; sleep 1; RC=hung
else wait $APPPID 2>/dev/null; RC=$?; fi
echo "exit rc=$RC"
# The layer's messages live HERE, not in app.log (see header).
cp ~/.spacecrafter/log/vulkan.log "$OUT/vulkan.log" 2>/dev/null
echo "reload confirmations in log: $(grep -ac 'reloaded (observer state kept)' "$OUT/app.log")"
echo "abort/terminate lines: $(grep -ac 'terminate called' "$OUT/app.log")"
grep -a "terminate called" "$OUT/app.log" | head -2

echo "--- validation layer present? (positive confirmation, step 1: loaded) ---"
echo "loader 'Loading layer library ...validation' lines: $(grep -ac 'Loading layer library libVkLayer_khronos_validation' "$OUT/app.log")"
grep -a "Insert instance layer .*validation" "$OUT/app.log" | head -2
echo "--- layer message channel (step 2: messages reach vulkan.log) ---"
echo "vulkan.log lines: $(wc -l < "$OUT/vulkan.log" 2>/dev/null || echo MISSING)"
echo "'messageIDName' lines (any layer message at all): $(grep -ac 'messageIDName' "$OUT/vulkan.log")"
grep -a "messageIDName" "$OUT/vulkan.log" | head -5
echo "--- VUID (both channels) ---"
for f in "$OUT/app.log" "$OUT/vulkan.log"; do
  echo "  $(basename "$f"): VUID=$(grep -ac 'VUID' "$f") 'Validation'=$(grep -ac 'Validation' "$f") 'eError'=$(grep -ac 'eError' "$f") 'eWarning'=$(grep -ac 'eWarning' "$f")"
done
grep -a "VUID" "$OUT/vulkan.log" | head -10
echo "--- missing loader warnings ---"
echo "'No loader available' lines: $(grep -ac 'No loader available' "$OUT/app.log")"
echo "--- frozen pair in==out ---"
M=$(md5sum "$CFG" | cut -d' ' -f1); S=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)
echo "config $M $([ "$M" = "$MD5_REF" ] && echo MATCH || echo DRIFT)"
echo "ssystem $S $([ "$S" = "$SSMD5_REF" ] && echo MATCH || echo DRIFT)"
