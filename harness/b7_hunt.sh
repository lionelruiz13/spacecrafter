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
# Env:  B7_MODE=gdb|plain  B7_OUT=<subdir>  B7_APPEND=0|1  B7_CYC0=<int offset>
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
CFG=~/.spacecrafter/config.ini
BETA=~/.spacecrafter/beta_features.ini
mkdir -p "$OUT"
CSV="$OUT/campaign.csv"
SUMMARY="$OUT/summary.log"

[ -x "$BIN" ] || { echo "FATAL: binary missing $BIN"; exit 2; }
[ "$MODE" = plain ] || [ -f "$PROBE" ] || { echo "FATAL: probe missing $PROBE"; exit 2; }

MD5_REF=$(md5sum "$CFG" | cut -d' ' -f1)
cp "$CFG" "$OUT/config.ini.ref"
if [ "$APPEND" != 1 ]; then
  echo "cycle,variant,path,entry,teardown,tcp_secs,outcome,detail" > "$CSV"
  : > "$SUMMARY"
fi

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
    echo "$cyc,$variant,$path,$entry,$teardown,$tcp_secs,STARTUP-DIED,$sd" >> "$CSV"
    cp "$OUT/config.ini.ref" "$CFG"; sleep 1
    return
  fi

  sleep 4   # async texture quiesce
  local APPPID
  if [ "$MODE" = plain ]; then APPPID="$PROC"
  else APPPID=$(pgrep -x spacecrafter | head -1); fi

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
  echo "$cyc,$variant,$path,$entry,$teardown,$tcp_secs,$outcome,\"$detail\"" >> "$CSV"

  # restore config + assert; never leave a beta_features file
  cp "$OUT/config.ini.ref" "$CFG"
  local m; m=$(md5sum "$CFG" | cut -d' ' -f1)
  [ "$m" = "$MD5_REF" ] || log "cyc $cyc: !! config md5 drift $m != $MD5_REF"
  [ -f "$BETA" ] && { log "cyc $cyc: !! beta_features.ini appeared — removing"; rm -f "$BETA"; }
  sleep 1
}

# ---- main loop --------------------------------------------------------------
MIXSRC="${1:-}"
if [ -n "$MIXSRC" ] && [ "$MIXSRC" != "-" ] && [ -f "$MIXSRC" ]; then
  mapfile -t VARIANTS < "$MIXSRC"
else
  log "FATAL: no mix file"; exit 2
fi

log "batch teardowns: ${#VARIANTS[@]}  (cycle ids $((CYC0+1))..$((CYC0+${#VARIANTS[@]})))"
n=0
for v in "${VARIANTS[@]}"; do
  [ -z "$v" ] && continue
  n=$((n+1))
  run_cycle "$((CYC0+n))" "$v"
done

log "==== B7 HUNT [$MODE] batch done $(date -Iseconds) ===="
FINAL_MD5=$(md5sum "$CFG" | cut -d' ' -f1)
log "final config md5=$FINAL_MD5  ref=$MD5_REF  $([ "$FINAL_MD5" = "$MD5_REF" ] && echo MATCH || echo DRIFT!!)"
[ -f "$BETA" ] && log "WARNING beta_features present at end"
log "--- outcome tally (this out dir, cumulative) ---"
tail -n +2 "$CSV" | cut -d, -f7 | sort | uniq -c | tee -a "$SUMMARY"
grep -aq ',FIRE,' "$CSV" && { log "*** FIRE ROWS ***"; grep -a ',FIRE,' "$CSV" | tee -a "$SUMMARY"; } || true
