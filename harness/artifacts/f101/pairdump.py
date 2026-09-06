"""Prints the scan's own `pairs` set, by exec'ing its source unchanged."""
import sys, io, contextlib
src = open(sys.argv[1]).read()
g = {"__name__": "__main__"}
sys.argv = [sys.argv[1], sys.argv[2]]
with contextlib.redirect_stdout(io.StringIO()):
    exec(compile(src, sys.argv[0], "exec"), g)
for s, t in sorted(g["pairs"]):
    print("%s -> %s" % (s, t))
