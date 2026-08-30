#!/bin/bash
# =====================================================================================
# F56 — THE ENVIRONMENT CANARY.  One preflight instrument that answers, LOUDLY, the
# question no instrument in this corpus could answer on 2026-08-29: "is this the
# rendering stack every committed baseline was measured on?"
#
# WHY IT EXISTS (§11.174(e), and the fault it is built from).  Two dispatch sessions
# (14 and 15, both 2026-08-29, tasks F42-F52) ran without a Wayland display; the
# rendered scene came out ~2.7x dark; nothing in the harness noticed, so a whole
# session's photometric evidence went into the ledger looking green.  §11.157(f)'s
# rebuilt-stack verification reached mode, geometry and GPU-real -- but NOT PHOTOMETRY,
# and that is exactly the member that failed.  This script is that verification
# surface's missing member plus the fingerprint members the post-hoc chase had to
# reconstruct by hand (§11.174(f)(g)(j)): compositor identity, logind session table,
# runtime dir, socket map, VRAM, and the RDP connection state.
#
# WHAT IT IS NOT.  It is not a fix for the 2026-08-29 fault -- that fault's mechanism
# is UNKNOWN and the live chase is closed at six exonerations (§11.174(j): driver,
# VRAM residency, dispatch env, server identity, windowing path, RDP connection state
# all measured NOT to sustain it).  The surviving model is a latched state.  A canary
# does not explain a latch; it catches the next one at the door, with the numbers to
# name it.
#
# THE BANKING DECISION, MADE HERE AND FLAGGED FOR THE OWNER.  The band below is banked
# on DISPLAY :2 -- F43's self-owned substitute stack, which is the harness default and
# the display every healthy value in this corpus was measured on (§11.174(f)(3): the
# harness has NEVER rendered on the owner's own session, in either era).  Which display
# is the CANONICAL render host -- :2 (survives the owner's disconnects) or :4 (the real
# logind session, lifetime-coupled to his) -- is the OWNER'S FORK, open at §11.174(f).
# Re-banking on :4 costs exactly one edit of the VALUES block below plus one re-measured
# reference run; nothing else in this script knows a display number.
#
# usage:
#   f56_canary.sh [--out DIR]            full: fingerprint + cache manifest + scene
#   f56_canary.sh --no-scene [--out DIR] fingerprint + cache manifest only (seconds)
#   f56_canary.sh --check-json FILE      photometric band applied to an existing
#                                        f51_dwell.json (no launch)
#   options: --display X | --expect-display X | --expect-dims WxH | --keep-frames
#
# exit: 0 green | 1 photometric out of band | 2 fingerprint mismatch
#     | 3 environment MISSING (no auth file / display unreachable) | 4 harness error
# A non-zero exit is a STOP, not a hint: see the PREVENTION line each failure prints.
# =====================================================================================
set -u

# ============================ THE VALUES BLOCK ========================================
# The only banked state in this instrument.  Every number here is [measured] and dated;
# re-banking is an explicit act with an argument, never a tolerance widened to fit.
#
# DISPLAY STACK -- measured 2026-08-30 (and identical to §11.174(f)'s probe of the same
# processes): F43's substitute, born 2026-08-29 14:51:01, absent from loginctl by
# construction (a hand-built process tree, not a session).
BANK_DISPLAY=":2"
BANK_DIMS="2448x1332"
BANK_XDG_RUNTIME_DIR="/tmp/rt-claude"
BANK_XAUTH_GLOB="/tmp/rt-claude/.mutter-Xwaylandauth.*"
BANK_COMPOSITOR_CMD="gnome-shell --headless --virtual-monitor 2448x1332"
BANK_COMPOSITOR_START=1788007861          # epoch of /proc/<pid>, = 2026-08-29 14:51:01
BANK_XSERVER_MATCH="Xwayland :2"
BANK_XSERVER_START=1788007886             # = 2026-08-29 14:51:26
BANK_HOST_BOOT="2026-08-27 07:32:05"
#
# PHOTOMETRIC BAND -- the reference scene is `f51_run.sh`'s (§11.174(e) names it): the
# Moon `base` scene at fov 10, the app's own 2048^2 readback, metric = §11.164(c)'s
# disc_mean / hf_mean as implemented in `f51_disc.py`.
#   CENTRE  [measured 2026-08-30]: new path 165.258 / 6.644, old path 160.142 / 6.603.
#   SPREAD  [measured]: 0.000.  NINE launches on 2026-08-30 return those numbers to the
#           last printed digit -- six in the §11.174(j) bracket, plus `envB_scrubbed`,
#           plus `toggle_indeterminate` (artifacts/s16_dim_bracket/), plus F55's re-run
#           of F51's own driver (artifacts/f55/f51_today/).  The dwell frames are
#           BYTE-IDENTICAL across all eight bracket-family runs (one md5, banked below).
#           Cross-epoch: the committed JULY frame reads 165.258 / 6.644 on the same
#           metric, five weeks and a different session earlier (§11.172(e)).
#   WIDTH   [derived]: the widest disagreement between two HEALTHY readings of this disc
#           anywhere in the corpus is 0.066 (0.04 %), and that is across two different
#           readback paths (2048^2 app readback vs 1024^2 window grab, §11.172(c)) --
#           the canary always uses the first, so 0.066 is an upper bound it never even
#           incurs.  The band is 15x that, +-1.0 on disc_mean (0.6 %) and +-0.15 on
#           hf_mean (2.3 %).  It therefore tolerates every benign variation this corpus
#           has ever measured, while the fault class it exists to catch -- 61.431 /
#           2.464 new, 42.476 / 1.744 old (§11.164(c), §11.172(f)) -- sits 104 band
#           widths (disc) and 28 band widths (hf) outside it.  A band that cannot be
#           passed by chance and cannot be failed by noise.
#   INNER   [derived]: any non-zero delta is NOTED (not failed) -- with a measured
#           spread of exactly 0.000 over nine launches, drift is news before it is a
#           fault.
BANK_NEW_DISC=165.258
BANK_NEW_HF=6.644
BANK_OLD_DISC=160.142
BANK_OLD_HF=6.603
BANK_TOL_DISC=1.0
BANK_TOL_HF=0.15
BANK_EXACT_SPREAD=0.001
BANK_FRAME_MD5="5215565b11d24328a9bb01198c875b25"
#
# VRAM -- NOT a gate.  H-VRAM was posed and REFUTED (§11.174(h): the owner verifies VRAM
# before dispatch, and the model was not resident during the dim era), so gating on it
# would encode a refuted hypothesis.  It is recorded, and a large resident consumer is
# NOTED because the ledger's chase would have been shorter with the number in hand.
BANK_VRAM_NOTE_MIB=8192
# =====================================================================================

HERE="$(cd "$(dirname "$0")" && pwd)"
STAMP=$(date '+%Y%m%d-%H%M%S')
OUT="$HERE/artifacts/f56/canary/$STAMP"
DPY="$BANK_DISPLAY"
EXPECT_DPY=""
EXPECT_DIMS=""
DO_SCENE=1
CHECK_JSON=""
KEEP_FRAMES=0
XAUTH_ARG=""
while [ $# -gt 0 ]; do
    case "$1" in
        --out) OUT="$2"; shift 2 ;;
        --no-scene) DO_SCENE=0; shift ;;
        --check-json) CHECK_JSON="$2"; DO_SCENE=0; shift 2 ;;
        --display) DPY="$2"; shift 2 ;;
        --xauth) XAUTH_ARG="$2"; shift 2 ;;
        --expect-display) EXPECT_DPY="$2"; shift 2 ;;
        --expect-dims) EXPECT_DIMS="$2"; shift 2 ;;
        --keep-frames) KEEP_FRAMES=1; shift ;;
        -h|--help) sed -n '1,45p' "$0"; exit 0 ;;
        *) echo "unknown option: $1"; exit 4 ;;
    esac
done
[ -n "$EXPECT_DPY" ] || EXPECT_DPY="$BANK_DISPLAY"
[ -n "$EXPECT_DIMS" ] || EXPECT_DIMS="$BANK_DIMS"
mkdir -p "$OUT" || exit 4

FP="$OUT/fingerprint.txt"                 # key<TAB>value, the fingerprint's one source
LOG="$OUT/canary.log"
: > "$FP"
: > "$LOG"
RC=0
NFAIL=0
NNOTE=0

say()  { echo "$*" | tee -a "$LOG"; }
kv()   { printf '%s\t%s\n' "$1" "$2" >> "$FP"; say "  $1 = $2"; }
worse(){ [ "$1" -gt "$RC" ] && RC=$1; return 0; }
# §11.169 error schema: WHAT / CONSEQUENCES / PREVENTION, self-contained action.
fail() {  # fail <code> <member> <what> <consequences> <prevention>
    NFAIL=$((NFAIL+1)); worse "$1"
    say ""
    say "CANARY FAIL [$2]"
    say "  WHAT        : $3"
    say "  CONSEQUENCES: $4"
    say "  PREVENTION  : $5"
}
note() { NNOTE=$((NNOTE+1)); say ""; say "CANARY NOTE [$1]"; say "  $2"; }
# §11.169 acting-default schema: CAUSE / CONTENT / OVERRIDE.
acted(){ say ""; say "CANARY DEFAULT [$1]"; say "  CAUSE   : $2"; say "  CONTENT : $3"; say "  OVERRIDE: $4"; }

say "=== F56 ENVIRONMENT CANARY  $(date '+%F %T %Z')  out=$OUT"
say "    banked stack: DISPLAY $EXPECT_DPY $EXPECT_DIMS | band: new $BANK_NEW_DISC/$BANK_NEW_HF old $BANK_OLD_DISC/$BANK_OLD_HF (+-$BANK_TOL_DISC/+-$BANK_TOL_HF)"

# ---------------------------------------------------------------- check-json mode
if [ -n "$CHECK_JSON" ]; then
    say "--- photometric arm only, on $CHECK_JSON (no launch) ---"
    python3 "$HERE/f56_band.py" "$CHECK_JSON" \
        --new-disc "$BANK_NEW_DISC" --new-hf "$BANK_NEW_HF" \
        --old-disc "$BANK_OLD_DISC" --old-hf "$BANK_OLD_HF" \
        --tol-disc "$BANK_TOL_DISC" --tol-hf "$BANK_TOL_HF" \
        --exact-spread "$BANK_EXACT_SPREAD" --frame-md5 "$BANK_FRAME_MD5" \
        --label "$CHECK_JSON" --out "$OUT/band.json" 2>&1 | tee -a "$LOG"
    BRC=${PIPESTATUS[0]}
    [ "$BRC" -ne 0 ] && worse 1
    say ""
    say "=== CANARY VERDICT: exit $RC (photometric arm only)"
    exit $RC
fi

# ---------------------------------------------------------------- (a) DISPLAY STACK
say ""
say "--- (a) display stack ---"
kv host.boot "$(uptime -s)"
kv host.now_epoch "$(date +%s)"
kv display.target "$DPY"
kv display.expected "$EXPECT_DPY"
if [ "$DPY" != "$EXPECT_DPY" ]; then
    fail 2 "display.target" \
      "rendering would go to DISPLAY $DPY but the band is banked on $EXPECT_DPY." \
      "the banked photometric numbers were measured on $EXPECT_DPY; a number taken on a different display is not comparable with the ledger's, and §11.174(f)(3) measured that the two displays on this host are DIFFERENT stacks (a hand-built substitute vs a logind session), not two names for one." \
      "run without --display, or re-bank deliberately: measure f51_run.sh on the new display and edit the VALUES block at the top of $0 (BANK_DISPLAY + the four BANK_*_DISC/HF keys). The choice of canonical display is the OWNER'S open fork (§11.174(f)) -- report, do not decide."
fi
# The auth cookie is a property of the BANKED STACK, not of the environment: this
# session inherits an XAUTHORITY, and on 2026-08-30 the inherited one was
# /run/user/1003/.mutter-Xwaylandauth.* -- the OWNER'S :4 session cookie, which :2
# refuses with `Invalid MIT-MAGIC-COOKIE-1 key`.  That is §0.5(a)'s hazard in its
# post-remmina form (same uid now, different stack), and it is why the resolution is
# banked rather than inherited.  --xauth overrides deliberately.
kv xauthority.inherited "${XAUTHORITY:-<unset>}"
if [ -n "$XAUTH_ARG" ]; then
    export XAUTHORITY="$XAUTH_ARG"
    acted "xauthority" "--xauth given on the command line" \
      "using $XAUTH_ARG" "omit --xauth to resolve from the banked glob $BANK_XAUTH_GLOB"
else
    XA=$(ls $BANK_XAUTH_GLOB 2>/dev/null | head -1)
    if [ -n "$XA" ]; then
        if [ "${XAUTHORITY:-}" != "$XA" ]; then
            acted "xauthority" \
              "the inherited XAUTHORITY (${XAUTHORITY:-<unset>}) is not the banked stack's cookie; a cookie from another stack is refused by $BANK_DISPLAY (§0.5(a))" \
              "overriding with $XA, the first match of $BANK_XAUTH_GLOB (banked with the stack, recorded above as xauthority.inherited)" \
              "pass --xauth <file> to choose another, or re-bank BANK_XAUTH_GLOB if the stack moved"
        fi
        export XAUTHORITY="$XA"
    fi
fi
kv xauthority "${XAUTHORITY:-<NONE>}"
if [ -z "${XAUTHORITY:-}" ] || [ ! -r "${XAUTHORITY:-/nonexistent}" ]; then
    fail 3 "display.xauth" \
      "no readable Xwayland auth file (looked for $BANK_XAUTH_GLOB; XAUTHORITY=${XAUTHORITY:-<unset>})." \
      "every X connection will be refused, so nothing renders and no measurement is possible. This is the §11.138/§11.157(f) failure mode, and improvising a replacement stack is what hid the 2026-08-29 fault for a full day." \
      "STOP and report to the dispatcher/owner: the host lost its display provisioning (a reboot is the known cause). Environment-fault mitigations are OWNER VETO ITEMS (§0.5, §11.174(h)) -- 'your correction may differ from my mitigation, say the word'. Do not build a substitute stack silently."
fi
export DISPLAY="$DPY"
XDPY=$(xdpyinfo 2>&1)
DIMS=$(printf '%s\n' "$XDPY" | awk '/dimensions:/ {print $2; exit}')
NSCREENS=$(printf '%s\n' "$XDPY" | awk '/number of screens:/ {print $4; exit}')
kv display.dimensions "${DIMS:-<none>}"
kv display.screens "${NSCREENS:-<none>}"
if [ -z "$DIMS" ]; then
    fail 3 "display.reachable" \
      "xdpyinfo cannot talk to $DPY: $(printf '%s' "$XDPY" | head -1)" \
      "the display is dead or not ours; a launch would either fail or render into nothing, and the app's own comment (app_command_interface.cpp:4035-4039) says external grabs see black on a headless X, so the failure can look like a black scene rather than a crash." \
      "STOP and report the host state (this is an OWNER VETO ITEM, §11.174(h)). Check: is the compositor alive ('$BANK_COMPOSITOR_CMD'), is /tmp/.X11-unix/X${DPY#:} present, is XAUTHORITY readable?"
elif [ "$DIMS" != "$EXPECT_DIMS" ]; then
    fail 2 "display.geometry" \
      "$DPY reports $DIMS, banked $EXPECT_DIMS." \
      "the geometry is part of the instrument: the banked photometric numbers come from a 2448x1332 virtual monitor with a 1024x1024 swapchain, and a different mode changes both the app's window and what a grab reads (§11.106, §11.157(f))." \
      "if the stack was rebuilt deliberately, report it and re-bank (re-measure f51_run.sh, edit BANK_DIMS + the band in $0); if not, report the change -- an unexplained mode change is exactly the class this canary exists to catch."
fi
kv xdg_runtime_dir "${XDG_RUNTIME_DIR:-<unset>}"
ls -la /tmp/.X11-unix/ > "$OUT/x11_sockets.txt" 2>&1
kv x11.sockets "$(ls /tmp/.X11-unix/ 2>/dev/null | tr '\n' ' ')"

# ---------------------------------------------------------------- (a2) COMPOSITOR
say ""
say "--- (a2) compositor identity ---"
COMP_PID=""
for p in /proc/[0-9]*; do
    c=$(tr '\0' ' ' < "$p/cmdline" 2>/dev/null)
    case "$c" in "$BANK_COMPOSITOR_CMD "*|"$BANK_COMPOSITOR_CMD") COMP_PID=$(basename "$p"); break ;; esac
done
kv compositor.cmd_banked "$BANK_COMPOSITOR_CMD"
kv compositor.pid "${COMP_PID:-<absent>}"
if [ -z "$COMP_PID" ]; then
    fail 2 "compositor.absent" \
      "no process matching the banked compositor command line is alive." \
      "the display stack every banked number was measured on no longer exists; whatever answers $DPY now is a different stack, and §11.174 is the record of what an unnoticed stack change costs (a full session of dark, green-looking evidence)." \
      "STOP and report: the host's display provisioning changed (reboot, logout, or a manual rebuild). This is an OWNER VETO ITEM (§0.5, §11.174(h)) -- report the failure rather than rebuilding a substitute; if a rebuild IS ratified, re-bank BANK_COMPOSITOR_* and re-measure the band."
else
    CSTART=$(stat -c %Y "/proc/$COMP_PID" 2>/dev/null)
    kv compositor.start_epoch "${CSTART:-<unknown>}"
    kv compositor.start_iso "$(date -d @${CSTART:-0} '+%F %T' 2>/dev/null)"
    kv compositor.start_banked "$BANK_COMPOSITOR_START"
    if [ "${CSTART:-0}" != "$BANK_COMPOSITOR_START" ]; then
        fail 2 "compositor.restarted" \
          "the compositor is alive but was started at ${CSTART:-?} ($(date -d @${CSTART:-0} '+%F %T' 2>/dev/null)), banked $BANK_COMPOSITOR_START ($(date -d @$BANK_COMPOSITOR_START '+%F %T' 2>/dev/null))." \
          "a restarted compositor is a NEW stack: §0.5 standing rule is that a stack change (compositor, streamer, headless X, screen power state) means report + re-baseline, and §11.174(j)'s surviving model for the dim era is a LATCHED rendering state whose trigger candidates are exactly events like this one." \
          "report the restart with its timestamp, then re-bank: re-measure f51_run.sh (six runs is what the ledger's own spread argument used) and edit BANK_COMPOSITOR_START plus, if the numbers moved, the band."
    fi
fi
XS_PID=""; XS_START=""
for p in /proc/[0-9]*; do
    c=$(tr '\0' ' ' < "$p/cmdline" 2>/dev/null)
    case "$c" in *"$BANK_XSERVER_MATCH"*) XS_PID=$(basename "$p"); XS_START=$(stat -c %Y "$p"); break ;; esac
done
kv xserver.pid "${XS_PID:-<absent>}"
kv xserver.start_epoch "${XS_START:-<unknown>}"
kv xserver.start_banked "$BANK_XSERVER_START"
if [ -n "$XS_START" ] && [ "$XS_START" != "$BANK_XSERVER_START" ]; then
    note "xserver.restarted" "the X server serving $DPY was started at $XS_START ($(date -d @$XS_START '+%F %T')), banked $BANK_XSERVER_START -- a restarted server under an unchanged compositor. Recorded, not gated (the compositor member above is the gating one); if this appears without a compositor change, it is a new fact and belongs in the delivery."
fi

# ---------------------------------------------------------------- (b) GPU
say ""
say "--- (b) GPU ---"
if command -v nvidia-smi > /dev/null 2>&1; then
    nvidia-smi > "$OUT/nvidia_smi.txt" 2>&1
    nvidia-smi --query-compute-apps=pid,process_name,used_memory --format=csv \
        > "$OUT/nvidia_procs.csv" 2>&1
    GQ=$(nvidia-smi --query-gpu=name,driver_version,memory.used,memory.total,utilization.gpu \
         --format=csv,noheader 2>/dev/null | head -1)
    kv gpu.query "${GQ:-<none>}"
    USED=$(printf '%s' "$GQ" | awk -F', ' '{print $3}' | awk '{print $1}')
    kv gpu.used_mib "${USED:-<unknown>}"
    case "${USED:-}" in
        ''|*[!0-9]*) : ;;
        *) if [ "$USED" -gt "$BANK_VRAM_NOTE_MIB" ]; then
               note "gpu.vram_pressure" "$USED MiB of VRAM in use (> $BANK_VRAM_NOTE_MIB MiB note threshold). NOT a gate: H-VRAM was posed and refuted (§11.174(h)). Recorded because the dim-era chase spent a round-trip reconstructing this number after the fact; the process list is in nvidia_procs.csv."
           fi ;;
    esac
else
    note "gpu.absent" "nvidia-smi not on PATH; the GPU member of the fingerprint is EMPTY for this run (recorded as absent rather than silently skipped)."
fi

# ---------------------------------------------------------------- (c) RDP + (d) dispatch
say ""
say "--- (c) RDP connection state / (d) dispatch method ---"
ss -tn state established 2>/dev/null > "$OUT/ss_estab.txt"
RDP=$(awk '/:3389([^0-9]|$)/ && !/Address/ {n++} END {print n+0}' "$OUT/ss_estab.txt" 2>/dev/null)
kv rdp.estab_3389 "${RDP:-0}"
kv rdp.note "count of ESTAB rows mentioning :3389 -- one loopback connection appears TWICE (both endpoints). Recorded, never gated: RDP connection state was measured NOT to sustain the dim state (§11.174(j), 896 s disconnected bracket, five runs at the banked value)"
loginctl list-sessions > "$OUT/loginctl.txt" 2>&1
CS=$(awk '$3 == "claude" {print $1}' "$OUT/loginctl.txt" 2>/dev/null | tr '\n' ' ')
kv logind.claude_sessions "${CS:-<none>}"
kv dispatch.has_logind_session "$([ -n "${CS:-}" ] && echo yes || echo no)"
kv dispatch.wayland_display "${WAYLAND_DISPLAY:-<unset>}"
kv dispatch.dbus "${DBUS_SESSION_BUS_ADDRESS:-<unset>}"
kv dispatch.inherited_display "${SC_CANARY_INHERITED_DISPLAY:-${DISPLAY:-<unset>}}"
if [ -z "${CS:-}" ]; then
    note "dispatch.no_session" "claude has NO logind session right now. That is the sessions-14/15 dispatch shape (§11.174(g): sudo login shell from the owner's console, no runtime dir) -- and it is ALSO the normal state whenever the owner's remmina session is closed, which §11.174(j) measured as photometrically harmless. Recorded as a fingerprint member, not a verdict."
fi

# ---------------------------------------------------------------- fingerprint.json
python3 - "$FP" "$OUT/fingerprint.json" <<'PY'
import json, sys
src, dst = sys.argv[1], sys.argv[2]
d = {}
for line in open(src, encoding="utf-8", errors="replace"):
    if "\t" in line:
        k, v = line.rstrip("\n").split("\t", 1)
        d[k] = v
json.dump(d, open(dst, "w"), indent=1, sort_keys=True)
print(f"fingerprint: {len(d)} members -> {dst}")
PY

if [ "$DO_SCENE" -eq 0 ]; then
    say ""
    say "--- cache manifest (standalone) ---"
    python3 "$HERE/f56_manifest.py" snapshot "$HOME/.spacecrafter/cache" \
        "$OUT/cache_manifest.json" 2>&1 | tee -a "$LOG"
    say ""
    say "=== CANARY VERDICT: exit $RC  ($NFAIL fail, $NNOTE note; scene arm NOT run)"
    [ "$RC" -eq 0 ] && say "    fingerprint green -- but the photometric member is the one 2026-08-29 needed: run without --no-scene before trusting a measurement."
    exit $RC
fi

# ---------------------------------------------------------------- (e) THE SCENE
say ""
say "--- (e) reference scene (f51_run.sh, the §11.174(e) scene) ---"
n=0; for p in /proc/[0-9]*; do [ "$(cat "$p/comm" 2>/dev/null)" = "spacecrafter" ] && n=$((n+1)); done
kv concurrency.spacecrafter_pre "$n"
if [ "$n" -ne 0 ]; then
    fail 4 "concurrency" \
      "$n spacecrafter process(es) already running (/proc/<pid>/comm probe, §11.134(b))." \
      "concurrent instances share the real ~/.spacecrafter (cache, screenshots, logs), which is the confound §11.121(m) recorded; a reference measurement taken now is not a reference." \
      "wait for the other instance to exit, then re-run. If it is not yours, report it -- another account's launcher is a host-state fact the dispatcher needs."
    say ""
    say "=== CANARY VERDICT: exit $RC (scene arm skipped)"
    exit $RC
fi

python3 "$HERE/f56_manifest.py" snapshot "$HOME/.spacecrafter/cache" \
    "$OUT/cache_pre.json" 2>&1 | tee -a "$LOG"

SCENE="$OUT/scene"
say "    launching: f51_run.sh $SCENE --samples 2   (timeout 900 s, plain timeout per §0.5(b))"
timeout 900 "$HERE/f51_run.sh" "$SCENE" --samples 2 > "$OUT/scene_run.log" 2>&1
SRC_RC=$?
kv scene.f51_run_rc "$SRC_RC"
tail -5 "$OUT/scene_run.log" | tee -a "$LOG" > /dev/null
MD5LINE=$(grep -a 'md5 in=out OK' "$OUT/scene_run.log" 2>/dev/null | head -1)
kv scene.md5_in_eq_out "$([ -n "$MD5LINE" ] && echo yes || echo NO)"
if [ -z "$MD5LINE" ]; then
    fail 4 "scene.md5_assert" \
      "f51_run.sh did not report 'config/ssystem md5 in=out OK' (exit $SRC_RC); see $OUT/scene_run.log." \
      "the run either failed or leaked into the real ~/.spacecrafter; either way its numbers are not a reference measurement, and a leak is a data mutation this repo forbids (D9, the frozen field)." \
      "read $OUT/scene_run.log; if the md5s differ, restore them from git/backup and report the leak before any further run."
fi

python3 "$HERE/f56_manifest.py" snapshot "$HOME/.spacecrafter/cache" \
    "$OUT/cache_post.json" 2>&1 | tee -a "$LOG"
python3 "$HERE/f56_manifest.py" diff "$OUT/cache_pre.json" "$OUT/cache_post.json" \
    --out "$OUT/cache_diff.json" 2>&1 | tee -a "$LOG"
NMUT=$(python3 -c "import json,sys; print(json.load(open(sys.argv[1]))['n_mutations'])" \
       "$OUT/cache_diff.json" 2>/dev/null)
kv cache.mutations "${NMUT:-<unknown>}"
if [ "${NMUT:-0}" != "0" ]; then
    note "cache.mutated" "the shared texture cache ~/.spacecrafter/cache changed across this run ($NMUT file mutation(s), per-file deltas in cache_diff.json). EXPECTED and NOT prevented: the temp-HOME farm symlinks cache/ (§11.172(i)), so every farm run in this corpus reads and writes it. Isolating it would make every run a cold-cache run -- a different measurement condition, not a better instrument -- so the manifest REPORTS and the ledger decides."
fi

if [ -f "$SCENE/f51_dwell.json" ]; then
    python3 "$HERE/f56_band.py" "$SCENE/f51_dwell.json" \
        --new-disc "$BANK_NEW_DISC" --new-hf "$BANK_NEW_HF" \
        --old-disc "$BANK_OLD_DISC" --old-hf "$BANK_OLD_HF" \
        --tol-disc "$BANK_TOL_DISC" --tol-hf "$BANK_TOL_HF" \
        --exact-spread "$BANK_EXACT_SPREAD" --frame-md5 "$BANK_FRAME_MD5" \
        --label "canary $STAMP" --out "$OUT/band.json" 2>&1 | tee -a "$LOG"
    BRC=${PIPESTATUS[0]}
    [ "$BRC" -ne 0 ] && worse 1
else
    fail 4 "scene.no_output" \
      "f51_run.sh produced no f51_dwell.json at $SCENE (exit $SRC_RC)." \
      "the photometric member -- the one member the 2026-08-29 verification surface was missing -- did not run, so this canary has verified nothing about rendering." \
      "read $OUT/scene_run.log and $SCENE/dwell.applog; a launch that dies before the TCP port opens leaves the applog's last lines as the evidence."
fi

if [ "$KEEP_FRAMES" -eq 0 ] && [ -d "$SCENE/frames" ]; then
    KEEP=$(ls "$SCENE/frames" 2>/dev/null | head -1)
    [ -n "$KEEP" ] && find "$SCENE/frames" -type f ! -name "$KEEP" -delete
    acted "frames.pruned" \
      "the reference scene writes 2048x2048 PNGs (~4 MB each) and this is a preflight, not an experiment" \
      "kept the first frame ($KEEP) as the visual record, deleted the rest of $SCENE/frames" \
      "pass --keep-frames to keep every frame"
fi

say ""
say "=== CANARY VERDICT: exit $RC  ($NFAIL fail, $NNOTE note)  artifacts: $OUT"
exit $RC
