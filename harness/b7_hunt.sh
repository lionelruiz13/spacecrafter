#!/bin/bash
# ============================================================================
# B7 §11.15d shutdown-segfault ACTIVE HUNT (dispatch row B7, scoped §11.74(f)).
# 2026-07-24, Claude Opus 4.8. INVESTIGATION — no product code changed.
#
# The one true §11.15d fire (B23, §11.74(c)) was a teardown RACE after the
# driver exited 0, on a SIGINT teardown, on the task that ran the MOST
# launch/teardown cycles — observed at NATIVE timing, never under gdb. The row's
# own hypothesis (§11.74(d)) is sample-size: more teardowns = more chances for a
# low-probability race. This harness runs MANY instrumented teardown cycles,
# varying what the recorded evidence says matters:
#   - SIGNAL entry (SIGINT/SIGTERM/SIGQUIT) — the fire's own entry path,
#     vs the command entry (`shutdown action now`) which §11.74 already covered.
#   - OLD render path toggled (`flag experimental_path off`) — the named
#     uncovered candidate (§11.74(d)): old-path teardown structures the four
#     structural fixes (4130edb3/092008ad/9368828d/e3d0f4e1) did not cover.
#   - ACTIVITY before teardown, and QUIT-DURING-ACTIVITY (reload + immediate
#     teardown) — B23's high-activity correlate + the race window.
#   - GALACTIC escalation before teardown — extra loaded content (Tully) to free.
#
# TWO instrument channels (B7_MODE):
#   gdb   — b7_hunt_probe.gdb ARMED: catches the fire WITH a backtrace + thread
#           states (the WHERE the exit-code channel lacks; §11.74(e)). But gdb
#           PERTURBS teardown timing (frame-stall watchdog trips) — not native.
#   plain — no gdb: the app's own exit code is the positive instrument
#           (139=SIGSEGV, 134=SIGABRT; core-file channel DEAD per §11.74(e)).
#           NATIVE timing — the regime the one true fire was seen in.
# ASan/TSan weighed + DECLINED: multi-hour rebuild whose timing perturbation
# could SUPPRESS the very race, driver-thread noise uninstrumented; recorded as
# the follow-up if a caught fire's object is ambiguous.
#
# Fresh launch per cycle (protocol). Config restored byte-identical + md5
# asserted every cycle. No stale instance between cycles. No beta_features.ini
# written (old path via runtime command, not the file).
#
# ---- 2026-07-31 EXTENSION (F8 / hunt-3, INTENT §11.122) ---------------------
# B7_ROVERS=K adds the COMPOSED-BODY-COUNT axis (§11.97(f): an ABORT-path exit,
# `terminate called without an active exception`, followed an 8-composed-rover
# run while the 2-rover run exited 0). K>0 composes K grounded OJM rovers into
# ~/.spacecrafter/modularSystem/SolarSystem.ini (twin + appended `type=`
# sections — the §11.97 composed-file route; the push-channel route
# `body action load ... coord_func surface_point` KILLS the app, §5.50) and adds
# a preamble that navigates to them and takes one frame, so the modules are
# LOADED and DRAWN before teardown. K=0 (default) reproduces §11.95 exactly.
# Instrument chain for that axis (a scene that silently failed to load would
# make the axis fiction): every cycle counts the app's own
# `Loading body Rover<i>` lines and records rovload; rovload != K => INVALID,
# never a silent CLEAN.
# Third detector added: `terminate called` in the log => FIRE (the §11.97(f)
# abort signature by name, independent of the exit code). Strictly ADDITIVE —
# it can only make the instrument more sensitive than §11.95's.
# CSV gains three trailing columns: rovers,rovload,load1 (appended at the end,
# so column-indexed readers of the §11.95 CSVs are unaffected). load1 = the
# 1-minute load average AT THE CYCLE, i.e. contention is recorded per teardown,
# not asserted once per batch.
#
# Env:  B7_MODE=gdb|plain  B7_OUT=<subdir>  B7_APPEND=0|1  B7_CYC0=<int offset>
#       B7_ROVERS=<K>  (0 = §11.95 behaviour)
# Usage: DISPLAY=:2 [env...] ./b7_hunt.sh [mixfile]
# ============================================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
# Hardcode the binary (do NOT honour SC_BIN — the §11.81(d)/pkill-SC_BIN trap).
BIN="$HERE/../../build-claude/src/spacecrafter"
PROBE="$HERE/b7_hunt_probe.gdb"
MODE="${B7_MODE:-gdb}"
OUT="$HERE/artifacts/${B7_OUT:-b7_hunt}"
APPEND="${B7_APPEND:-0}"
CYC0="${B7_CYC0:-0}"
ROVERS="${B7_ROVERS:-0}"
CFG=~/.spacecrafter/config.ini
BETA=~/.spacecrafter/beta_features.ini
MODDIR=~/.spacecrafter/modularSystem
TWIN="$MODDIR/SolarSystem.ini.disabled"
SCENE="$MODDIR/SolarSystem.ini"
mkdir -p "$OUT"
CSV="$OUT/campaign.csv"
SUMMARY="$OUT/summary.log"

[ -x "$BIN" ] || { echo "FATAL: binary missing $BIN"; exit 2; }
[ "$MODE" = plain ] || [ -f "$PROBE" ] || { echo "FATAL: probe missing $PROBE"; exit 2; }

MD5_REF=$(md5sum "$CFG" | cut -d' ' -f1)
cp "$CFG" "$OUT/config.ini.ref"
if [ "$APPEND" != 1 ]; then
  echo "cycle,variant,path,entry,teardown,tcp_secs,outcome,detail,rovers,rovload,load1" > "$CSV"
  : > "$SUMMARY"
fi

# ---- composed-body-count axis (§11.97(f)) -----------------------------------
# ONE layout function so that K=2 and K=8 differ in COUNT ALONE (the axis under
# test) — same per-rover geometry, same OJM model, same radius; only how many.
# Grounded OJM rovers spread along the sub-observer meridian band so all K are
# in the preamble's field of view and therefore actually DRAWN.
build_scene() {
  local k="$1" i lon
  [ -f "$TWIN" ] || { log "FATAL: composed twin missing $TWIN"; exit 2; }
  if [ "$k" -le 0 ]; then rm -f "$SCENE"; return; fi
  cp "$TWIN" "$SCENE"
  for i in $(seq 0 $((k-1))); do
    lon=$(( 60 + (i * 7) - ((k-1)*7/2) ))
    cat >> "$SCENE" <<EOF

[Rover$i]
name = Rover$i
parent = Moon
relation = grounded
compose = explicit
type = Artificial
coord_func = surface_point
orbit_lon = $lon
orbit_lat = 0
orbit_alt = 0
radius = 400
model_name = Curiosity
halo = false
[Rover$i:OJM]
body = Rover$i
type = OJM
EOF
  done
}

log() { echo "$*" | tee -a "$SUMMARY"; }
log "==== B7 HUNT [$MODE] batch start $(date -Iseconds)  out=$OUT append=$APPEND cyc0=$CYC0 ===="
log "BIN mtime=$(stat -c %y "$BIN")  code HEAD=$(git -C "$HERE/../.." rev-parse --short HEAD 2>/dev/null)  harness HEAD=$(git -C "$HERE" rev-parse --short HEAD 2>/dev/null)"
log "config md5 ref=$MD5_REF   ssystem md5=$(md5sum ~/.spacecrafter/ssystem.ini 2>/dev/null | cut -d' ' -f1)"
[ -f "$BETA" ] && log "WARNING beta_features.ini present at start (unexpected)"

# ---- TCP command sender (bash /dev/tcp; one connection, all lines) ----------
tcp_send() {
  { exec 3<>/dev/tcp/127.0.0.1/7805 || return 1
    local c
    for c in "$@"; do printf '%s\n' "$c" >&3; sleep 0.25; done
    sleep 0.3
    exec 3>&-
  } 2>/dev/null
}

# ---- one cycle --------------------------------------------------------------
run_cycle() {
  local cyc="$1" variant="$2"
  local path="new" teardown="cmd" sig="INT"
  case "$variant" in
    frsig)   path=new; teardown=sig; sig=INT ;;
    frcmd)   path=new; teardown=cmd ;;
    oldsig)  path=old; teardown=sig; sig=INT ;;
    oldcmd)  path=old; teardown=cmd ;;
    actsig)  path=new; teardown=sig; sig=INT ;;
    actquit) path=new; teardown=sig; sig=QUIT ;;
    qda)     path=new; teardown=sig; sig=INT ;;
    qdaterm) path=new; teardown=sig; sig=TERM ;;
    galsig)  path=new; teardown=sig; sig=INT ;;
    # POSITIVE CONTROLS (2026-07-31, F8): an injected fatal signal at the same
    # point in the cycle a real fire would occur. They run through the SAME
    # classifier as every hunted cycle -- a copy of the classifier would prove
    # nothing about the one actually deciding CLEAN (I2). pcsegv = the §11.15d
    # SIGSEGV class; pcabrt = the §11.97(f) ABORT class (same signal std::
    # terminate raises). Both MUST come out FIRE, or no CLEAN in the batch means
    # anything.
    pcsegv)  path=new; teardown=sig; sig=SEGV ;;
    pcabrt)  path=new; teardown=sig; sig=ABRT ;;
    *)       path=new; teardown=cmd ;;
  esac
  local entry; entry=$([ "$teardown" = sig ] && echo "SIG$sig" || echo "shutdown-cmd")

  local LOG="$OUT/${MODE}_$(printf '%03d' "$cyc")_${variant}.log"
  local PROC   # the PID we wait on (gdb in gdb-mode, the app in plain-mode)
  if [ "$MODE" = gdb ]; then
    DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$PROBE" --args "$BIN" > "$LOG" 2>&1 &
  else
    DISPLAY=${DISPLAY:-:2} "$BIN" > "$LOG" 2>&1 &
  fi
  PROC=$!

  # wait for TCP up OR startup death
  local UP=0 t0 tcp_secs i
  t0=$(date +%s)
  for i in $(seq 1 60); do
    sleep 1
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then UP=1; break; fi
    kill -0 "$PROC" 2>/dev/null || break
  done
  tcp_secs=$(( $(date +%s) - t0 ))

  if [ "$UP" != 1 ]; then
    # startup death or no-tcp: the quiet-host startup datum (§11.82 class)
    local sd="startup-notcp" rc=""
    if ! kill -0 "$PROC" 2>/dev/null; then
      if [ "$MODE" = plain ]; then
        wait "$PROC" 2>/dev/null; rc=$?
        case "$rc" in 139) sd="startup-SEGV";; 134) sd="startup-ABRT";; 0) sd="startup-exited0";; *) sd="startup-rc$rc";; esac
      else
        grep -aq "received signal SIGSEGV" "$LOG" && sd="startup-SEGV"
        grep -aq "received signal SIGABRT" "$LOG" && sd="startup-ABRT"
        grep -aq "exited normally" "$LOG" && sd="startup-exited0"
      fi
    fi
    kill -9 "$PROC" 2>/dev/null
    log "cyc $cyc $variant: *** NO TCP ($sd) tcp_secs=$tcp_secs ***"
    echo "$cyc,$variant,$path,$entry,$teardown,$tcp_secs,STARTUP-DIED,$sd,$ROVERS,0,$(cut -d' ' -f1 /proc/loadavg)" >> "$CSV"
    cp "$OUT/config.ini.ref" "$CFG"; sleep 1
    return
  fi

  sleep 4   # async texture quiesce
  local APPPID
  if [ "$MODE" = plain ]; then APPPID="$PROC"
  else APPPID=$(pgrep -x spacecrafter | head -1); fi

  # ---- composed-rover preamble (only when the axis is on) ----
  # Navigate to the grounded rovers and render one frame, so the K OJM modules
  # are loaded AND drawn before teardown — §11.97(f)'s abort followed "valid
  # shots". moon_scaled off is the §5.27/§11.100 instrument precondition
  # (shipped moon_scale=5 swallows grounded children, D21 pending).
  if [ "$ROVERS" -gt 0 ]; then
    tcp_send "flag experimental_path on" "timerate rate 0" "meteors zhr 0" \
             "date jday 2461234.0" "set home_planet Moon" \
             "camera action free_mode state on" "flag atmosphere off" \
             "flag landscape off" "flag moon_scaled off" "select planet Moon" \
             "moveto lat 0 lon 60 alt 4000000 duration 0"
    sleep 3
    # track_object is what AIMS the camera (b24_screen's order). Without it the
    # frame is empty sky and the rovers are loaded-but-never-drawn — measured
    # 2026-07-31 on the recon cycle, which is why this is not optional.
    # Turned off again after the shot for B30 determinism, as b24_screen does.
    tcp_send "flag track_object on"
    sleep 2
    tcp_send "zoom fov 20 duration 0"
    sleep 1
    tcp_send "body action screenshot filename $OUT/rovershot.png"
    sleep 2
    tcp_send "flag track_object off"
    sleep 1
  fi

  # ---- variant activity ----
  case "$variant" in
    old*)  tcp_send "flag experimental_path off"; sleep 2 ;;
    act*)  tcp_send "select planet Mars" "flag track_object on" "moveto altitude 500 duration 0" \
                    "flag planet_orbits on" "flag object_trails on" "flag atmosphere on" \
                    "body name Mars color r 1 g 0 b 0" ; sleep 3 ;;
    gal*)  tcp_send "moveto altitude 1e12 duration 0" ; sleep 2 ;
           tcp_send "moveto altitude 1e15 duration 0" ; sleep 3 ;;
    *)     sleep 1 ;;
  esac

  # ---- teardown ----
  if [ "$variant" = qda ] || [ "$variant" = qdaterm ]; then
    tcp_send "body action reload" &
    sleep 0.05
    kill -"$sig" "$APPPID" 2>/dev/null
  elif [ "$teardown" = sig ]; then
    kill -"$sig" "$APPPID" 2>/dev/null
  else
    tcp_send "shutdown action now"
  fi

  # ---- wait for exit ----
  local ex=0
  for i in $(seq 1 45); do kill -0 "$PROC" 2>/dev/null || { ex=1; break; }; sleep 1; done
  local hung=0 rc=""
  if [ "$ex" != 1 ]; then
    hung=1; kill -9 "$PROC" 2>/dev/null; sleep 1
  elif [ "$MODE" = plain ]; then
    wait "$PROC" 2>/dev/null; rc=$?
  fi
  pkill -9 -x spacecrafter 2>/dev/null   # stale-instance discipline

  # ---- classify ----
  local outcome detail
  # Composed-body-count axis instrument (see header): the app's own
  # "Loading body Rover<i>" lines are the positive evidence that the K modules
  # were really instantiated this cycle. Counted BEFORE outcome so a scene that
  # failed to load can never be banked as a CLEAN teardown of K rovers.
  local rovload=0 termd=0
  [ "$ROVERS" -gt 0 ] && rovload=$(grep -ac "Loading body Rover" "$LOG")
  termd=$(grep -ac "terminate called" "$LOG")
  if [ "$MODE" = plain ]; then
    if [ "$hung" = 1 ]; then
      outcome=HUNG; detail="no-exit-in-45s"
      log "cyc $cyc $variant: HUNG (no exit in 45s)"
    else
      case "$rc" in
        139) outcome=FIRE; detail="rc=139 SIGSEGV"; log "cyc $cyc $variant: *** *** FIRE *** *** rc=139 SIGSEGV  see $LOG" ;;
        134) outcome=FIRE; detail="rc=134 SIGABRT"; log "cyc $cyc $variant: *** *** FIRE *** *** rc=134 SIGABRT  see $LOG" ;;
        0)   outcome=CLEAN; detail="rc=0"; log "cyc $cyc $variant [$path/$entry] tcp=${tcp_secs}s: CLEAN (rc=0)" ;;
        *)   outcome=OTHER; detail="rc=$rc"; log "cyc $cyc $variant: OTHER rc=$rc (see $LOG)" ;;
      esac
    fi
  else
    local segv abrt cleanx stall stackframe
    segv=$(grep -ac "received signal SIGSEGV" "$LOG")
    abrt=$(grep -ac "received signal SIGABRT" "$LOG")
    cleanx=$(grep -ac "exited normally" "$LOG")
    stall=$(grep -ac "Frame stall" "$LOG")
    stackframe=$(awk '/POST-RUN STACK/{f=1} f&&/^#0 /{print; exit}' "$LOG")
    if [ "$segv" -gt 0 ] || [ "$abrt" -gt 0 ]; then
      outcome=FIRE; detail="segv=$segv abrt=$abrt frame0=[${stackframe:0:90}]"
      log "cyc $cyc $variant: *** *** FIRE *** *** $detail"; log "     see $LOG"
    elif [ "$hung" = 1 ]; then
      outcome=HUNG; detail="no-exit-in-45s stall=$stall"
      log "cyc $cyc $variant: HUNG (no exit in 45s) stall=$stall"
    elif [ "$cleanx" -gt 0 ]; then
      outcome=CLEAN; detail="stall_during_run=$stall"
      [ "$stall" -gt 0 ] && detail="clean-exit-but-had-frame-stall=$stall"
      log "cyc $cyc $variant [$path/$entry] tcp=${tcp_secs}s: CLEAN$([ "$stall" -gt 0 ] && echo ' (frame-stall during run)')"
    else
      outcome=OTHER; detail="segv=$segv abrt=$abrt clean=$cleanx stall=$stall"
      log "cyc $cyc $variant: OTHER $detail (see $LOG)"
    fi
  fi
  # Third detector, ADDITIVE to §11.95's two: the §11.97(f) abort signature by
  # name, independent of the exit code (a `terminate called` line is a fire even
  # if the wait status were somehow lost).
  if [ "$termd" -gt 0 ]; then
    detail="$detail terminate-called=$termd"
    if [ "$outcome" != FIRE ]; then
      outcome=FIRE
      log "cyc $cyc $variant: *** *** FIRE *** *** 'terminate called' abort path (§11.97(f)) see $LOG"
    fi
  fi
  # Axis instrument: a cycle whose composed scene did not load is not a valid
  # denominator for "K rovers torn down" — it is INVALID, never a silent CLEAN.
  # The admissible band is per-variant, not a constant: qda/qdaterm send
  # `body action reload`, which loads the composed system a SECOND time, so
  # those cycles legitimately log up to 2K "Loading body Rover" lines — and
  # because the reload RACES the teardown signal (0.05 s later, by design), the
  # count there is legitimately either K or 2K. Measured 2026-07-31: an
  # expectation of exactly K flagged 3/14 qda cycles as INVALID when the scene
  # had in fact loaded correctly twice. The rule states the requirement — the
  # scene was instantiated at least once and no more often than the variant
  # can explain — instead of hard-coding one variant's count.
  if [ "$ROVERS" -gt 0 ] && [ "$outcome" != FIRE ]; then
    local rmax="$ROVERS"
    case "$variant" in qda|qdaterm) rmax=$((ROVERS*2)) ;; esac
    if [ "$rovload" -lt "$ROVERS" ] || [ "$rovload" -gt "$rmax" ]; then
      outcome=INVALID; detail="$detail rovload=$rovload outside [$ROVERS,$rmax]"
      log "cyc $cyc $variant: !! INVALID — composed scene loaded $rovload rovers, admissible [$ROVERS,$rmax]"
    fi
  fi
  echo "$cyc,$variant,$path,$entry,$teardown,$tcp_secs,$outcome,\"$detail\",$ROVERS,$rovload,$(cut -d' ' -f1 /proc/loadavg)" >> "$CSV"

  # restore config + assert; never leave a beta_features file
  cp "$OUT/config.ini.ref" "$CFG"
  local m; m=$(md5sum "$CFG" | cut -d' ' -f1)
  [ "$m" = "$MD5_REF" ] || log "cyc $cyc: !! config md5 drift $m != $MD5_REF"
  [ -f "$BETA" ] && { log "cyc $cyc: !! beta_features.ini appeared — removing"; rm -f "$BETA"; }
  # the composed scene is an instrument, not shipped state: assert it survived
  if [ "$ROVERS" -gt 0 ]; then
    local sm; sm=$(md5sum "$SCENE" 2>/dev/null | cut -d' ' -f1)
    [ "$sm" = "$SCENE_MD5" ] || log "cyc $cyc: !! composed-scene md5 drift $sm != $SCENE_MD5"
  fi
  sleep 1
}

# ---- main loop --------------------------------------------------------------
MIXSRC="${1:-}"
if [ -n "$MIXSRC" ] && [ "$MIXSRC" != "-" ] && [ -f "$MIXSRC" ]; then
  mapfile -t VARIANTS < "$MIXSRC"
else
  log "FATAL: no mix file"; exit 2
fi

SCENE_PRE_EXISTS=0
[ -f "$SCENE" ] && SCENE_PRE_EXISTS=1
build_scene "$ROVERS"
SCENE_MD5=$(md5sum "$SCENE" 2>/dev/null | cut -d' ' -f1)
log "composed-body axis: B7_ROVERS=$ROVERS  scene=$([ "$ROVERS" -gt 0 ] && echo "$SCENE md5=$SCENE_MD5" || echo 'none (§11.95 reproduction)')  scene_pre_existed=$SCENE_PRE_EXISTS"
log "host at batch start: loadavg=$(cut -d' ' -f1-3 /proc/loadavg)  memavail=$(awk '/MemAvailable/{printf "%.1fGiB", $2/1048576}' /proc/meminfo)"

log "batch teardowns: ${#VARIANTS[@]}  (cycle ids $((CYC0+1))..$((CYC0+${#VARIANTS[@]})))"
n=0
for v in "${VARIANTS[@]}"; do
  [ -z "$v" ] && continue
  n=$((n+1))
  run_cycle "$((CYC0+n))" "$v"
done

log "==== B7 HUNT [$MODE] batch done $(date -Iseconds) ===="
log "host at batch end: loadavg=$(cut -d' ' -f1-3 /proc/loadavg)  memavail=$(awk '/MemAvailable/{printf "%.1fGiB", $2/1048576}' /proc/meminfo)"
# The composed scene is an instrument: leave the user dir as found (the shipped
# default is NO enabled SolarSystem.ini — the legacy ssystem.ini then wins).
if [ "$ROVERS" -gt 0 ] && [ "$SCENE_PRE_EXISTS" = 0 ]; then
  rm -f "$SCENE"; log "composed scene removed (user dir restored to shipped default)"
fi
log "ssystem md5 at end=$(md5sum ~/.spacecrafter/ssystem.ini | cut -d' ' -f1)"
FINAL_MD5=$(md5sum "$CFG" | cut -d' ' -f1)
log "final config md5=$FINAL_MD5  ref=$MD5_REF  $([ "$FINAL_MD5" = "$MD5_REF" ] && echo MATCH || echo DRIFT!!)"
[ -f "$BETA" ] && log "WARNING beta_features present at end"
log "--- outcome tally (this out dir, cumulative) ---"
tail -n +2 "$CSV" | cut -d, -f7 | sort | uniq -c | tee -a "$SUMMARY"
grep -aq ',FIRE,' "$CSV" && { log "*** FIRE ROWS ***"; grep -a ',FIRE,' "$CSV" | tee -a "$SUMMARY"; } || true
