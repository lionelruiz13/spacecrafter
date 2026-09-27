# Run harness/shape/writers.py unchanged; rev WORKTREE reads the working-tree file instead of `git show`.
import subprocess, sys, types
_run = subprocess.run
def _fake(args, *a, **kw):
    if list(args[:2]) == ['git', 'show'] and str(args[2]).startswith('WORKTREE:'):
        return types.SimpleNamespace(stdout=open(str(args[2]).split(':', 1)[1], 'rb').read(), stderr=b'', returncode=0)
    return _run(args, *a, **kw)
subprocess.run = _fake
sys.argv = ['writers.py'] + sys.argv[1:]
exec(compile(open('claude/harness/shape/writers.py').read(), 'writers.py', 'exec'))
