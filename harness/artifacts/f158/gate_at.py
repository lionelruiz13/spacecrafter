#!/usr/bin/env python3
"""gate_at.py <rev> [--full] -- tests/anchor_gate.py's own run(), its WORKING-TREE side read from <rev>.

F155's probe (artifacts/f155/gate_at.py), kept for F158 because a parallel task
(F154) edits src/uiModule/ in the working tree while this one runs: the grammar
files are read from the working tree as the real gate reads them, every engine
file from <rev>. --full prints what the real gate prints (FAIL lines + tally);
default prints one line. Run with python3 -B (no bytecode in the code tree).
"""
import sys
sys.path.insert(0, '/home/claude/spacecrafter/util/scedit/tests')
import anchor_gate as ag

rev = sys.argv[1]
orig = ag.blob
GRAMMAR = tuple(ag.TARGETS)

def blob_at(r, path, repo=ag.REPO):
    if r == ag.WORKTREE and repo == ag.REPO and path not in GRAMMAR:
        return orig(rev, path, repo=repo)
    return orig(r, path, repo=repo)

ag.blob = blob_at
tally, hard = ag.run()
if '--full' in sys.argv:
    for h in hard:
        print('FAIL  ' + h)
    for k in sorted(tally):
        print('%-24s %d' % (k, tally[k]))
else:
    print(rev, dict(sorted(tally.items())), 'hard=%d' % len(hard))
