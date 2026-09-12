#!/bin/bash
# F116 -- the pre-launch asserts, in one place, so every leg of this task uses
# the same three and a reader can see which three they were.  Exit 0 = clear to
# launch; 2 = another engine is live; 3 = not enough GPU headroom.
#
#   ./f116_assert.sh [label]
#
# (i)   /proc/<pid>/comm == "spacecrafter" -- the standing probe, which is BLIND
#       to a renamed or copied binary (Sec.11.231(j2));
# (ii)  /proc/<pid>/exe resolving under a staging or build tree -- the blind spot
#       itself, which is where every binary this task builds lives;
# (iii) TCP 7805 held by anyone -- an engine that answers is an engine, whatever
#       its file is called.
# Plus the GPU headroom gate this host needed on 2026-09-12: a resident LLM in
# ollama left 1591 MiB and the app died at "Failed to allocate chunk of 256 MiB"
# (HOST-EVENTS 2026-09-12).  4000 MiB used is the dispatch's line; the holder is
# NAMED on a refusal and never unloaded here (Sec.11.174(h)).
set -u
LABEL=${1:-f116}
n=0
for p in /proc/[0-9]*; do
    [ "$(cat "$p/comm" 2>/dev/null)" = "spacecrafter" ] && { n=$((n+1)); echo "  comm hit : $p"; }
    e=$(readlink "$p/exe" 2>/dev/null || true)
    case "$e" in
        /home/claude/sc-*/*|*/build*/src/*) n=$((n+1)); echo "  exe hit  : $p -> $e";;
    esac
done
port=$(ss -ltn 2>/dev/null | grep -c ':7805 ')
echo "[$LABEL] instances (comm+exe) = $n ; port 7805 holders = $port"
[ "$n" -eq 0 ] && [ "$port" -eq 0 ] || { echo "[$LABEL] ABORT: another engine is live"; exit 2; }

vram=$(nvidia-smi --query-gpu=memory.used --format=csv,noheader,nounits)
echo "[$LABEL] gpu used = $vram MiB (limit 4000)"
[ "$vram" -le 4000 ] || {
    echo "[$LABEL] ABORT: VRAM used $vram MiB > 4000 -- the holder, not unloaded:"
    nvidia-smi --query-compute-apps=pid,process_name,used_memory --format=csv
    exit 3
}
echo "[$LABEL] clear to launch"
exit 0
