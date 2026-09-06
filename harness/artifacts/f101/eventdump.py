"""Prints the LINES the scan counts as raw events, by exec'ing the scan's own
source with ONE textual substitution (`events += 1` -> also record the line), so
the grammar is the instrument's and not a copy of it."""
import sys, io, contextlib
src = open(sys.argv[1]).read()
assert src.count("            events += 1\n") == 1
src = src.replace("            events += 1\n",
                  "            events += 1; _HITS.append((src, line.strip()[:110]))\n")
g = {"__name__": "__main__", "_HITS": [], "sys": sys}
sys.argv = [sys.argv[1], sys.argv[2]]
with contextlib.redirect_stdout(io.StringIO()):
    exec(compile(src, sys.argv[0], "exec"), g)
for s, l in g["_HITS"]:
    print("%s\t%s" % (s, l))
