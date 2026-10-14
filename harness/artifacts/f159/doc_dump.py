#!/usr/bin/env python3
"""Dump `scedit --doc <cmd>` and `--doc <cmd> <key>` for every command and key of
the grammar into one directory, one file per query (stdout + exit code), so a
BEFORE and an AFTER can be diffed query by query.
usage: doc_dump.py <scedit> <grammar> <outdir>"""
import json, os, subprocess, sys

scedit, grammar, out = sys.argv[1:4]
os.makedirs(out, exist_ok=True)
g = json.load(open(grammar, encoding="utf-8"))
queries = []
for name, e in g["families"]["commands"].items():
    if name.startswith("_") or not isinstance(e, dict):
        continue
    queries.append([name])
    for k, s in e.get("args", {}).items():
        if not k.startswith("_") and isinstance(s, dict):
            queries.append([name, k])
for q in queries:
    r = subprocess.run([scedit, "--grammar", grammar, "--doc"] + q, capture_output=True, text=True)
    with open(os.path.join(out, "_".join(q) + ".txt"), "w") as f:
        f.write(r.stdout + "exit %d\nstderr %d bytes\n" % (r.returncode, len(r.stderr)))
print("%d queries -> %s" % (len(queries), out))
