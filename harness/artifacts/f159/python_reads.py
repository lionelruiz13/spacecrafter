#!/usr/bin/env python3
"""The acceptance reads of F159 (INTENT 11.332): both halves of `script` and `camera`
and the vocabulary, from the working tree. usage: python3 -I python_reads.py (any cwd)"""
import json
R = "/home/claude/spacecrafter/util/scedit/grammar/"
g = json.load(open(R + "sc-grammar.json"))
K = ["handler", "lines", "aliases", "registration", "doc", "key_grammar", "branches", "exclusive_groups",
     "notes", "flagged", "signature", "handler_body_lines", "wait_parameter"]
for u, c in ((2, "script"), (4, "camera")):
    f = json.load(open(R + "args/unit-%d.json" % u))
    m, x = g["families"]["commands"][c], f["commands"][c]
    diff = [k for k in K if k in x and m.get(k) != x[k]] + (["args"] if m["args"] != x["args"] else [])
    a = m["args"]["action"]
    print("%s: halves differ in %s | action values %d, completable == values %s, value_docs %d | args %s | branches %d %s"
          % (c, diff, len(a["values"]), sorted(a["completable"]) == sorted(a["values"]), len(a["value_docs"]),
             sorted(m["args"]), len(m["branches"]),
             [b["selector"] for b in m["branches"] if b["selector"].split()[-1] in ("bind", "move_lat", "move_lon", "raise_height", "lower_height")]))
    for k in (("key", "press", "release", "scope") if c == "script" else ("value",)):
        s = m["args"][k]
        print("   %s.%s: required %r, fields %s" % (c, k, s["required"], sorted(s)))
v = g["argument_token_vocabulary"]
t = v["tokens"]
print("vocabulary: tokens %d, _expected_count %d, both_roles %d, release %s, _source %r"
      % (len(t), v["_expected_count"], len(v["both_roles"]), {k: t["release"][k] for k in ("role", "key_units", "value_units")}, v["_source"]))
for w in ("move_lat", "move_lon", "raise_height", "lower_height", "bind", "key", "press", "scope", "once", "script"):
    print("   %-13s %s" % (w, t[w]))
print("expected_counts", g["_meta"]["expected_counts"], "| amended lines", len(g["_meta"]["amended"]), "| anchor_pin", g["_meta"]["anchor_pin"][:15])
