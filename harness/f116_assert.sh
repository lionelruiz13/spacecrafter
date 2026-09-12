#!/bin/bash
# F116 -- the pre-launch asserts, in one place, so every leg of that task used
# the same three and a reader can see which three they were.  Exit 0 = clear to
# launch; 2 = another engine is live; 3 = not enough GPU headroom.
#
#   ./f116_assert.sh [label]
#
# [ROUTED 2026-09-12, F112 / Sec.11.238.  This file WAS the seed: it was the first
# place in the corpus where the three identity channels stood together, written
# inline here and copy-pasted into f114_run.sh.  Both halves now have a home and
# this file is one of their callers, which is the point -- two copies of a
# criterion is the desync I2 forbids, and the 44 copy-pasted `comm` tests are what
# that costs.  What moved, and what it cost to leave it here:
#   (i)   the comm/exe/port probe  -> sc_instances.sh.  The inline version's exe
#         arm matched `*/build*/src/*`, which is the DELIVERED binary's own path,
#         so a delivered-binary launch of the app under test would have been
#         reported as a concurrent instance by its own precondition; the home
#         distinguishes them (and covers cross-account comm, which the inline
#         readlink cannot: /proc/<pid>/exe is EACCES across uids, measured).
#   (ii)  the GPU gate -> sc_gpu.py against BANK_GPU_NEED_MIB in f56_canary.sh's
#         VALUES block.  The inline line was `used <= 4000`, which says nothing
#         about how much room is LEFT on a 32 GiB card: measured 2026-09-12 15:48
#         it refused a host with 27.6 GiB free because the owner's java held 2.9.
#         The gate now reads memory.FREE against a need derived from the app's own
#         init log (1717 MiB floor, 5323 MiB measured peak, 444 MiB reservation).
#   (iii) the holder naming -> `nvidia-smi -q -d PIDS`.  The inline call,
#         --query-compute-apps, is MEASURED blind to graphics processes: at 15:35
#         it named 240 MiB of the 3293 in use and missed java at 1385.
# Nothing about what F116 measured changes; this file's behaviour on a clear host
# is the same 0 it always returned.]
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
LABEL=${1:-f116}

bash "$HERE/sc_instances.sh" --assert "$LABEL"
rc=$?
[ "$rc" -eq 0 ] || exit 2

python3 "$HERE/sc_gpu.py" --need bank --label "$LABEL"
rc=$?
[ "$rc" -eq 0 ] || exit 3

echo "[$LABEL] clear to launch"
exit 0
