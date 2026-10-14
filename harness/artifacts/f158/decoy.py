#!/usr/bin/env python3
"""decoy.py -- the gate shown able to fail on each new shape, in memory only.

Each leg runs tests/anchor_gate.py's own run() with ONE decoy applied to the
WORKING-TREE side of one engine file (an inserted line, in memory: no file of
the tree is written), and prints the tally next to the recorded one. A shape
that cannot discriminate shows no delta. Run with python3 -B.
"""
import sys
sys.path.insert(0, '/home/claude/spacecrafter/util/scedit/tests')
import anchor_gate as ag

orig = ag.blob

def leg(name, path, at_line, insert="// decoy\n"):
    def blob(r, p, repo=ag.REPO):
        b = orig(r, p, repo=repo)
        if r == ag.WORKTREE and p == path and b is not None:
            ls = b.split("\n")
            return "\n".join(ls[:at_line - 1] + [insert.rstrip("\n")] + ls[at_line - 1:])
        return b
    ag.blob = blob
    ag._blob_cache.clear(); ag._map_cache.clear()
    tally, hard = ag.run()
    ag.blob = orig
    ag._blob_cache.clear(); ag._map_cache.clear()
    print("%-34s %s hard=%d" % (name, dict(sorted(tally.items())), len(hard)))
    return tally

base = leg("no decoy", "", 0)
leg("1 line inserted at app_command_interface.cpp:100", "src/interfaceModule/app_command_interface.cpp", 100)
leg("1 line inserted at base_command_interface.hpp:77", "src/interfaceModule/base_command_interface.hpp", 77)
leg("1 line inserted at app_command_interface.cpp:4700", "src/interfaceModule/app_command_interface.cpp", 4700)
