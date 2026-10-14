import sys
sys.path.insert(0, '/home/claude/spacecrafter/util/scedit/tests')
import anchor_gate as ag
rev = sys.argv[1]
orig = ag.blob
def blob_at(r, path, repo=ag.REPO):
    if r == ag.WORKTREE:
        if repo != ag.REPO:
            return orig(r, path, repo=repo)
        return orig(rev, path, repo=repo)
    return orig(r, path, repo=repo)
ag.blob = blob_at
tally, hard = ag.run()
print(rev, dict(sorted(tally.items())), 'hard=%d' % len(hard))
