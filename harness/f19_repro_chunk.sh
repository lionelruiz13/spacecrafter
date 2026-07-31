#!/bin/bash
# ============================================================================
# F19 REPRODUCER ARMS (INTENT §11.126) — the §11.125(g) regime, run as an
# interleaved A/B so a drift in host conditions cannot masquerade as an arm
# difference. One CHUNK per invocation (foreground-sized, §0.5).
#
#   ARM P = the F19 binary (the fix under test)
#   ARM R = the pre-fix reference, code `550b3f9f`/`ebb41ab2` source, which is
#           the binary §11.125 measured 25 FIRE / 26 on. Its identity is
#           asserted by md5 here, never inherited from a note.
#
# Regime, verbatim from §11.125(f)/(g): B7_ROVERS=8 composed grounded OJM
# rovers, the `qda`/`qdaterm` mix (`body action reload` with the teardown
# signal 0.05 s behind it), stress load PRE-WARMED to steady state before the
# first cycle. Everything else is b7_hunt.sh unchanged.
#
# Usage: DISPLAY=:2 ./f19_repro_chunk.sh <P|R> <cyc0> [mixfile]
# ============================================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ARM="${1:?arm P or R}"
CYC0="${2:?cycle offset}"
MIX="${3:-$HERE/f17_qda_mix.txt}"

POST="$HERE/../../build-claude/src/spacecrafter"
PRE="/tmp/claude-1003/-home-claude-spacecrafter/4938e05f-ed9f-4612-af84-2c1b3d213193/scratchpad/sc_pre/spacecrafter"
# Identity of the pre-fix arm, asserted (§11.125(m) states this md5 for the
# binary the 25/26 was measured on; our rebuild of that source reproduced it
# bit-identically, which is also the check that the checkout was complete).
PRE_MD5=1155a0b51e9c8069db30b2498ba943a8

case "$ARM" in
  # P = the first F19 commit (83324455), the arm whose ONE fire in 30 was the
  # shutdown-site race the second commit closed. Q = the delivered binary
  # (071b817a). Kept as separate arms on purpose: they are different binaries
  # and merging them would hide which one was measured.
  P) BIN="$POST"; OUT=f19_repro_P ;;
  Q) BIN="$POST"; OUT=f19_repro_Q ;;
  # S = the delivered binary after the waitAllFrames correction (a11fbb65).
  S) BIN="$POST"; OUT=f19_repro_S ;;
  # T = the DELIVERED binary (96a94a46): the class fix, with 5.59's
  # cancellation withdrawn after it was measured to convert the deep-stall
  # teardown from a hang into a crash.
  T) BIN="$POST"; OUT=f19_repro_T ;;
  R) BIN="$PRE";  OUT=f19_repro_R
     got=$(md5sum "$BIN" | cut -d' ' -f1)
     [ "$got" = "$PRE_MD5" ] || { echo "ARM R md5 MISMATCH: $got != $PRE_MD5"; exit 3; }
     ;;
  *) echo "arm must be P or R"; exit 2 ;;
esac

LOGDIR="$HERE/artifacts/f19_repro"
mkdir -p "$LOGDIR"
"$HERE/b7_stress.sh" report "chunk ARM=$ARM cyc0=$CYC0 bin=$(md5sum "$BIN" | cut -c1-8)" "$LOGDIR/host.log"

DISPLAY=${DISPLAY:-:2} B7_MODE=plain B7_OUT="$OUT" B7_APPEND=1 B7_CYC0="$CYC0" \
  B7_ROVERS=8 B7_BIN="$BIN" "$HERE/b7_hunt.sh" "$MIX"
