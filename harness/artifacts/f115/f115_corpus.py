#!/usr/bin/env python3
"""F115 - what the tester's own corpus asks of the 1 MiB uniform pool, per option.

Reads the SHOWS (field data, cited fetch - never recall) and prices every
authored body against the carved sizes f115_sizes measured, then states, per
option, the body count 1 MiB holds and where the wall lands on 06.sts + 14.sts.

The classification of a body into its module set is NOT guessed: it is the
engine's own deduction, read at
  ModularBody::deduceBodyModuleList  src/experimentalModule/ModularBody.cpp:876-940
  BasicMeshLoader::isLikely          .../moduleLoader/BasicMeshLoader.cpp:22-27  (bid 16)
  LayeredMeshLoader::isLikely        .../moduleLoader/LayeredMeshLoader.cpp:10-17 (bid 24)
  PhotosphereLoader::isLikely        .../moduleLoader/PhotosphereLoader.cpp:6-13  (bid 200)
  Moon::selectShader                 src/bodyModule/body_moon.cpp:248-288
  AppCommandInterface::commandBody   src/interfaceModule/app_command_interface.cpp:4119-4126
and reimplemented here with each gate's file:line beside it.

Usage:  python3 f115_corpus.py [--json <out>] [--shows <dir>]
Default shows dir = ~/.spacecrafter/scripts/fscripts (the installed field set).
"""
import argparse
import collections
import json
import os
import sys

# ---- the carved sizes, from f115_sizes at alignment 64 (measured, not typed) --
# key -> carved bytes. Kept in ONE place; f115_sizes.tsv is the authority and
# --tsv-check re-reads it.
CARVED = {
    "OLD_moon_plain": 320,       # globalVertProj 192 + globalFrag 128
    "NEW_basicmesh": 1024,       # globalVertProj 192 + meshFrag 832
    "NEW_basicmesh_cap1": 320,   # globalVertProj 192 + meshFrag_cap1 128
    "NEW_basicmesh_cap0": 256,   # globalVertProj 192 + meshFrag_cap0 64
    "INGALAXY": 128,             # OjmContainer::uniformData
    "NEW_ojm": 1216,
    "OLD_artificial": 256,
}

def body_lines(path):
    """Uncommented body-authoring lines, BOTH word orders (11.218(i): a
    `grep -c 'body action load'` census misses 06.sts entirely)."""
    with open(path, "rb") as f:
        txt = f.read().decode("latin-1")
    out = []
    for l in txt.splitlines():
        s = l.strip()
        if not s or s.startswith("#"):
            continue
        if not s.startswith("body"):
            continue
        if "action load" not in s:
            continue
        out.append(s)
    return out


def params(line):
    """key -> value, EXACTLY as the engine pairs them.

    AppCommandInterface::parseCommand (src/interfaceModule/app_command_interface.cpp:125)
    reads `while (commandstr >> key >> value)` - STRICT index pairing from the
    first token after the command word. There is no key whitelist and no
    resynchronisation: one malformed token shifts every pair after it, and the
    engine then sees keys that were values. A census that skips unknown tokens
    "helpfully" reports a corpus the engine does not read - this parser is the
    engine's, deliberately, including that failure mode.

    Measured instance in the field data: 06old.sts line 286 ("TDRS 3") writes
    `color0.5,0.5,0.5` with the space missing, so from that token on the engine
    pairs `color0.5,0.5,0.5 -> tex_map`, `earth_sats/sattext001.png -> halo`,
    ... and the body is loaded with NO tex_map. F102's f102_corpus.py counted it
    as `no_texmap` for exactly this reason and was RIGHT; an earlier F115 parser
    that skipped unrecognised tokens counted 170 textured bodies in that file
    and was wrong. The typo is field data (D9, frozen) - recorded, not fixed.
    """
    s = line
    # '#' outside a quoted run starts a comment (parseCommand :131-142)
    inq = False
    for i, ch in enumerate(s):
        if ch == '"':
            inq = not inq
        elif ch == '#' and not inq:
            s = s[:i]
            break
    s = s.lstrip(" \t")
    while ' " ' in s:                       # parseCommand :151-155
        j = s.find(' " ')
        s = s[:j + 2] + s[j + 3:]
    toks = s.split()
    if not toks:
        return {}
    p = {}
    i = 1                                   # commandstr >> command  (the word "body")
    while i + 1 < len(toks):
        key, value = toks[i], toks[i + 1]
        nxt = i + 2                         # where the stream stands after this pair
        if value.startswith('"'):           # pull in all text inside quotes
            if len(value) > 1 and value.endswith('"'):
                value = value[1:-1]
            else:
                parts = [value[1:]]
                k = i + 2
                while k < len(toks):
                    if toks[k].endswith('"'):
                        parts.append(toks[k][:-1])
                        k += 1
                        break
                    parts.append(toks[k])
                    k += 1
                value = " ".join(parts)
                nxt = k
        p.setdefault(key, value)
        i = nxt
    return p


def classify(p):
    """Return (class, old_bytes, new_bytes) for ONE authored body, at HEAD."""
    # app_command_interface.cpp:4119 - the OJM short-circuit takes THREE modes,
    # not one: (argMode=="in_universe" || argMode=="in_galaxy" ||
    # argMode=="in_sandbox") -> CoreLink::BodyOJMLoad -> OjmMgr::load
    # (ojm_mgr.cpp:77), ONE 128 B uniform, NEITHER body path touched. The
    # corpus uses in_universe too (S10.sts lines 42-43), so a census keyed on
    # "in_galaxy" alone misroutes those bodies.
    if p.get("mode") in ("in_galaxy", "in_universe", "in_sandbox"):
        return ("in_galaxy", 0, 0)
    if "tex_map" not in p:
        # no MESH on the new path (deduceBodyModuleList :885) and the old path's
        # classes still build, but the uniform carve is shader-selected off
        # tex_* - F102 measured these at zero.
        return ("no_tex_map", 0, 0)
    typ = p.get("type", "")
    layered = any(k in p for k in ("tex_night", "tex_norm", "tex_normal", "tex_heightmap"))
    if typ[:4] == "Arti":
        return ("artificial", CARVED["OLD_artificial"], CARVED["NEW_ojm"])
    if layered:
        return ("layered", -1, -1)   # priced per-variant, not in this corpus
    # plain tex_map body. OLD: Moon -> body_moon.cpp:284-285 eager in the ctor;
    # BigBody/SmallBody -> the same two blocks but LAZY at first draw
    # (body_smallbody.cpp:176-177 via drawBody's !initialized, body_bigbody.cpp:260-263).
    eager_old = (typ == "Moon")
    return ("plain_moon" if eager_old else "plain_lazy",
            CARVED["OLD_moon_plain"], CARVED["NEW_basicmesh"])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--shows", default=os.path.expanduser("~/.spacecrafter/scripts/fscripts"))
    ap.add_argument("--json", default=None)
    a = ap.parse_args()

    per_show = {}
    classes = collections.Counter()
    for fn in sorted(os.listdir(a.shows)):
        if not fn.endswith(".sts"):
            continue
        lines = body_lines(os.path.join(a.shows, fn))
        if not lines:
            continue
        c = collections.Counter()
        for l in lines:
            k, _o, _n = classify(params(l))
            c[k] += 1
            classes[k] += 1
        per_show[fn] = dict(c)
    total = sum(classes.values())

    # ---- the arithmetic ----------------------------------------------------
    POOL = 1048576
    BASELINE = 142529           # 11.222(e): the shipped launch scene's own draw,
                                # measured by inversion at 142 529-142 592 B
    solar = classes["plain_moon"] + classes["plain_lazy"]
    ingal = classes["in_galaxy"]
    arti = classes["artificial"]

    def corpus_bytes(per_solar, per_ingal=128, per_arti=CARVED["OLD_artificial"] + CARVED["NEW_ojm"]):
        return solar * per_solar + ingal * per_ingal + arti * per_arti

    options = [
        ("today (both paths, 06.sts body = 1344 B)", 1344),
        ("O4  B8 retires the old path (new only)", CARVED["NEW_basicmesh"]),
        ("O2  cap the receive array at 1, both paths", 320 + CARVED["NEW_basicmesh_cap1"]),
        ("O2  cap at 1 AND O4 (new only)", CARVED["NEW_basicmesh_cap1"]),
        ("O2' array out of the block, both paths", 320 + CARVED["NEW_basicmesh_cap0"]),
        ("O2'+O4  array out, new only", CARVED["NEW_basicmesh_cap0"]),
        ("O3  + all-bodies-identical fields shared (see the entry)", 128),
        ("O3' the per-body minimum (model matrix only)", 64),
    ]
    report = {"shows": per_show, "classes": dict(classes), "total_authored": total,
              "pool": POOL, "launch_baseline": BASELINE, "options": []}
    for label, per in options:
        need = corpus_bytes(per)
        free = POOL - BASELINE
        holds = free // per
        # the wall on 06.sts specifically: bodies of 06.sts are all plain_moon
        n06 = per_show.get("06.sts", {}).get("plain_moon", 0)
        wall = holds  # 06.sts is the first show played in directory order
        report["options"].append({
            "label": label, "bytes_per_solar_body": per,
            "corpus_bytes": need, "corpus_MiB": round(need / 1048576.0, 3),
            "bodies_per_free_MiB": holds,
            "06sts_authored": n06,
            "06sts_fits": bool(n06 <= wall),
            "06sts_wall_at_body": (None if n06 <= wall else wall + 1),
            "corpus_fits_1MiB": bool(need + BASELINE <= POOL),
        })

    print("== F115 corpus census (%s) ==" % a.shows)
    print("shows with authored bodies : %d" % len(per_show))
    for fn in sorted(per_show, key=lambda f: -sum(per_show[f].values())):
        print("   %-14s %4d  %s" % (fn, sum(per_show[fn].values()), per_show[fn]))
    print("TOTAL authored bodies      : %d   %s" % (total, dict(classes)))
    print()
    print("pool %d B, launch-scene baseline %d B (11.222(e)), free %d B"
          % (POOL, BASELINE, POOL - BASELINE))
    print("%-52s %7s %10s %8s %9s %s" %
          ("option", "B/body", "corpus B", "MiB", "holds", "06.sts wall"))
    for o in report["options"]:
        print("%-52s %7d %10d %8.3f %9d %s" %
              (o["label"], o["bytes_per_solar_body"], o["corpus_bytes"],
               o["corpus_MiB"], o["bodies_per_free_MiB"],
               "none" if o["06sts_fits"] else ("body %d" % o["06sts_wall_at_body"])))
    if a.json:
        with open(a.json, "w") as f:
            json.dump(report, f, indent=1, sort_keys=True)
        print("\nwrote %s" % a.json)
    return 0


if __name__ == "__main__":
    sys.exit(main())
