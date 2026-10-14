#!/usr/bin/env python3
"""doc_dump.py <outdir> -- `scedit --doc` over every command, every key of each,
and every member of the four name families, one file per query (F158 BEFORE/AFTER).

The command list and the key lists are read from the grammar the binary reads,
so BEFORE and AFTER query the same pages (the sweep changes no name). stdout,
stderr and the exit code of each query are kept; nothing is filtered.
"""
import json, os, subprocess, sys

BIN = "/home/claude/spacecrafter/util/scedit/build-f158/scedit"
GRAMMAR = "/home/claude/spacecrafter/util/scedit/grammar/sc-grammar.json"
out = sys.argv[1]
os.makedirs(out, exist_ok=True)
g = json.load(open(GRAMMAR, encoding="utf-8"))
fam = g["families"]
queries = [[]]
for name, e in fam["commands"].items():
    if not isinstance(e, dict):
        continue
    queries.append([name])
    for k in sorted((e.get("args") or {}).keys()):
        queries.append([name, k])
for cmd, family in (("flag", "flags"), ("set", "set_names"), ("color", "color_names"), ("font", "font_targets")):
    for n in fam[family]["names"]:
        nm = n if isinstance(n, str) else n.get("name")
        if nm:
            queries.append([cmd, nm])
seen = set()
for q in queries:
    key = "__".join(q) if q else "_catalogue"
    if key in seen:
        continue
    seen.add(key)
    r = subprocess.run([BIN, "--grammar", GRAMMAR, "--doc"] + q, capture_output=True)
    with open(os.path.join(out, key.replace("/", "_") + ".txt"), "wb") as f:
        f.write(b"exit %d\n" % r.returncode)
        f.write(r.stdout)
        if r.stderr:
            f.write(b"--- stderr ---\n" + r.stderr)
print("%d queries -> %s" % (len(seen), out))
