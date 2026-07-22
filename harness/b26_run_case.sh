#!/bin/bash
# B26 dual-path default-flip observable check - one case per invocation.
# INTENT.md 11.53 (measurements) / 11.50(c) (the change being verified).
#
#   ./b26_run_case.sh <tag> [outdir]
#
# The CALLER places or removes ~/.spacecrafter/beta_features.ini before the
# run - the case IS that file's state, so the runner never writes it (a
# runner that authored the input would be testing itself).
#
# The app is launched UNDER gdb (ptrace_scope=1 blocks attach) with
# b26_probe.gdb, which prints drawModularSystem/pathPinned read from process
# memory at every screenshot: the path-identity evidence that does NOT come
# from the pixels.  Requires a display (DISPLAY=:2 here) and enable_tcp.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
BIN="${SC_BIN:-$HERE/../../build-claude/src/spacecrafter}"
TAG=$1
OUT=${2:-$HERE/artifacts/b26}
mkdir -p "$OUT"
rm -f "$OUT/${TAG}_"*.png "$OUT/${TAG}_"*.json

echo "--- beta_features.ini state ---"
ls -la ~/.spacecrafter/beta_features.ini 2>&1 || true
[ -f ~/.spacecrafter/beta_features.ini ] && { echo "--- content ---"; cat ~/.spacecrafter/beta_features.ini; }

DISPLAY=${DISPLAY:-:2} gdb -q -batch -x "$HERE/b26_probe.gdb" --args "$BIN" > "$OUT/gdb_${TAG}.log" 2>&1 &
GDBPID=$!
echo "gdb pid=$GDBPID"

for i in $(seq 1 60); do
    sleep 2
    if ss -ltn 2>/dev/null | grep -q ':7805 '; then echo "tcp up after ${i} x2s"; break; fi
    kill -0 $GDBPID 2>/dev/null || { echo "gdb died before tcp"; exit 1; }
done
sleep 8   # let the initial async texture loads quiesce

python3 "$HERE/b26_default_flip.py" "$TAG" "$OUT" > "$OUT/drive_${TAG}.log" 2>&1
echo "driver exit=$?"

for i in $(seq 1 30); do kill -0 $GDBPID 2>/dev/null || break; sleep 1; done
kill -0 $GDBPID 2>/dev/null && { echo "gdb still alive, killing"; kill -9 $GDBPID; }
echo "--- probe lines (path state read from memory at each capture) ---"
grep -a "PROBE" "$OUT/gdb_${TAG}.log" | sed 's#.*/##'
echo "--- app exit ---"
grep -a "Inferior 1" "$OUT/gdb_${TAG}.log" || echo "(no inferior-exit line)"
