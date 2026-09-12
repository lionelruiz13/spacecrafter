#!/usr/bin/env python3
"""F115 - the three redundancy axes, in bytes, from the measured tables.

Inputs (both landed beside this script):
  f115_sizes.tsv   - sizeof + carved per block, printed by f115_sizes (compiled
                     against the engine's own headers)
  f115_fields.tsv  - every field of every per-body block, its writing site
                     file:line and its variability class

Step 0 is a CONSISTENCY GATE that can fail: the field table's byte sums must
equal the compiled sizeof of each block. A field mis-transcribed, a field
forgotten, a padding slot invented - any of those breaks the sum and this
script stops. Without it the axes below would be arithmetic over a table
nobody checked.

Usage: python3 f115_redundancy.py [--json out.json]
"""
import argparse
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


def load_sizes():
    s = {}
    for line in open(os.path.join(HERE, "f115_sizes.tsv")):
        line = line.rstrip("\n")
        if not line:
            continue
        k, sz, carved = line.split("\t")
        s[k] = (int(sz), int(carved))
    return s


def load_fields():
    rows = []
    for line in open(os.path.join(HERE, "f115_fields.tsv")):
        if line.startswith("#") or not line.strip():
            continue
        p = line.rstrip("\n").split("\t")
        if p[0] == "block":
            continue
        while len(p) < 6:
            p.append("")
        rows.append(dict(block=p[0], field=p[1], bytes=int(p[2]),
                         site=p[3], cls=p[4], note=p[5]))
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--json")
    a = ap.parse_args()
    S = load_sizes()
    F = load_fields()
    out = {}

    # ---- step 0: the gate -------------------------------------------------
    sums = {}
    for r in F:
        sums[r["block"]] = sums.get(r["block"], 0) + r["bytes"]
    bad = []
    print("== step 0: the field table against the compiled sizeof ==")
    for blk in sorted(sums):
        want = S.get(blk, (None, None))[0]
        ok = (want == sums[blk])
        if not ok:
            bad.append((blk, sums[blk], want))
        print("  %-26s fields %5d   sizeof %5s   %s"
              % (blk, sums[blk], want, "OK" if ok else "MISMATCH"))
    if bad:
        print("\nGATE FAILED - the field table does not describe these structs:")
        for b in bad:
            print("   %s: fields sum to %d, sizeof is %s" % b)
        return 1
    print("  GATE PASSED: %d blocks, every field sum equals its sizeof\n" % len(sums))
    out["blocks_checked"] = len(sums)

    def blk(name, cls=None):
        return sum(r["bytes"] for r in F
                   if r["block"] == name and (cls is None or r["cls"] == cls))

    carved = lambda k: S[k][1]

    # ---- axis 1: CROSS-PATH ----------------------------------------------
    print("== AXIS 1  CROSS-PATH (one authored body, both paths live) ==")
    a1 = []
    a1.append(("globalVertProj carved TWICE for one body - same struct, same "
               "seven fields, same values",
               "body_moon.cpp:284 (fill :334-340)", "BasicMesh.cpp:20 (fill :79-85)",
               carved("globalVertProj")))
    a1.append(("the shadow-receive feed, two generations of the same job",
               "globalFrag body_moon.cpp:285 (fill :329-331)",
               "meshFrag BasicMesh.cpp:20 (fill :86)",
               carved("globalFrag")))   # what B8 retires of the pair
    a1.append(("the atmosphere shell block - FIELD-IDENTICAL declarations "
               "(f115_mirrors.py proves it)",
               "atm_ext.hpp:20-30, acquired atm_ext.cpp:78",
               "AtmExtModule.hpp:76-86, acquired AtmExtModule.cpp:73",
               carved("atmExtUBO")))
    a1.append(("the ring", "ring.cpp:146 RingUniform",
               "RingModule.cpp:74-75 bodyRingVert+bodyRingFrag",
               carved("RingUniform")))
    a1.append(("the artificial body",
               "body_artificial.cpp:84 artGeom+LightInfo+artVert",
               "OjmModule.cpp:97-98 ojmVert+ojmGeom+ojmLight+ojmShadowBlock",
               carved("artGeom") + carved("LightInfo") + carved("artVert")))
    for what, oldhome, newhome, b in a1:
        print("  %5d B  %s" % (b, what))
        print("           OLD %s" % oldhome)
        print("           NEW %s" % newhome)
    old_eager_06 = carved("globalVertProj") + carved("globalFrag")
    print("  -> a 06.sts body: the OLD path's whole eager carve is %d B, and B8 "
          "retires all of it" % old_eager_06)
    print("     (of which %d B is the LITERALLY identical struct, %d B the "
          "superseded receive feed)\n"
          % (carved("globalVertProj"), carved("globalFrag")))
    out["cross_path_06sts_bytes"] = old_eager_06
    out["cross_path_same_struct_bytes"] = carved("globalVertProj")

    # ---- axis 2: CROSS-MODULE --------------------------------------------
    print("== AXIS 2  CROSS-MODULE (two blocks of ONE body) ==")
    derived = [r for r in F if r["cls"] == "DERIVED"]
    d_total = sum(r["bytes"] for r in derived)
    print("  DERIVED fields - a pure function of another field already in the "
          "SAME block, %d B over %d fields:" % (d_total, len(derived)))
    for r in derived:
        print("     %5d B  %-26s %-26s %s" % (r["bytes"], r["block"], r["field"], r["site"]))
    # the receive array, per body, counted once per block that embeds it
    arr = [r for r in F if r["field"].startswith("shadowingBodies")]
    print("\n  the SAME receive array embedded in %d different blocks "
          "(bodyShaderInterface.hpp:71,:125,:149,:199), %d B each:"
          % (len(arr), arr[0]["bytes"]))
    for r in arr:
        print("     %-16s %s" % (r["block"], r["site"]))
    combos = [
        ("a plain body (06.sts): meshFrag only", ["meshFrag"]),
        ("a ray-capable layered body: meshFrag + rayMarchFrag",
         ["meshFrag", "rayMarchFrag"]),
        ("+ rings: also bodyRingFrag",
         ["meshFrag", "rayMarchFrag", "bodyRingFrag"]),
    ]
    print("\n  array bytes carried by ONE body, filled from ONE source "
          "(body->getReceivedShadows(), meshShadowFill.hpp:47 and :84):")
    for label, blocks in combos:
        n = sum(768 for b in blocks)
        print("     %5d B  %s" % (n, label))
    # the placement matrix across a body's blocks
    mv = [r for r in F if r["field"] == "ModelViewMatrix"]
    print("\n  the placement matrix, %d B, appears in %d distinct per-body blocks:"
          % (64, len(mv)))
    for r in mv:
        print("     %-26s %s" % (r["block"], r["site"]))
    # the clipping/fov triple
    cf = [r for r in F if "clipping_fov" in r["field"] or r["field"] == "zNear,zRange,fov"]
    cf_bytes = sum(r["bytes"] for r in cf)
    print("\n  the clipping/fov triple: %d B over %d slots in %d blocks"
          % (cf_bytes, len(cf), len(set(r["block"] for r in cf))))
    out["derived_bytes"] = d_total
    out["array_blocks"] = len(arr)

    # ---- axis 3: CROSS-BODY ----------------------------------------------
    print("\n== AXIS 3  CROSS-BODY (identical content, carved once per body) ==")
    # for the 06.sts body, block by block
    new_blocks = ["globalVertProj", "meshFrag"]
    old_blocks = ["globalVertProj", "globalFrag"]
    def by_class(blocks, cls):
        return sum(r["bytes"] for r in F if r["block"] in blocks and r["cls"] == cls)
    rows = [
        ("meshFrag.shadowingBodies[8] - unwritten for a body that receives "
         "nothing, and NEVER written for a body that is never drawn", 768,
         "bodyShaderInterface.hpp:71", "meshShadowFill.hpp:62-71"),
        ("meshFrag._pad[3] - no writer in the tree, no shader reader", 12,
         "bodyShaderInterface.hpp:55", "(no site)"),
        ("globalVertProj.LightPosition - a STATIC, byte-identical in every "
         "body's block", 12,
         "ModularBody.hpp:1827-1829", "BasicMesh.cpp:83"),
        ("globalVertProj.clipping_fov.v[2] = ModularBody::halfFov", 4,
         "Renderer.cpp:107,:118", "BasicMesh.cpp:81"),
        ("globalVertProj planetRadius+planetScaledRadius+oneMinusOblateness - "
         "per-body CONST in general, but IDENTICAL for all 1013 bodies of "
         "06.sts (radius 0.1, oblateness 0.0, measured)", 12,
         "BasicMesh.cpp:82,:84,:85", "fscripts/06.sts (field data)"),
    ]
    tot3_new = 0
    for what, b, h1, h2 in rows:
        tot3_new += b
        print("  %5d B  %s" % (b, what))
        print("           %s  |  %s" % (h1, h2))
    new_sizeof = sum(S[b][0] for b in new_blocks)
    print("  -> NEW path, one 06.sts body: %d of %d sizeof bytes (%.0f %%) hold "
          "content no other body's block would differ in"
          % (tot3_new, new_sizeof, 100.0 * tot3_new / new_sizeof))
    old3 = 48 + 12 + 4 + 12   # moon2-4 dead + light + fov + the three radii
    old_sizeof = sum(S[b][0] for b in old_blocks)
    print("     OLD path, same body: %d of %d sizeof bytes (%.0f %%) - "
          "globalFrag.MoonPosition2..4/MoonRadius2..4 are 48 B no site writes "
          "for a Moon (bodyShaderInterface.hpp:33-38 | body_moon.cpp:329-331)"
          % (old3, old_sizeof, 100.0 * old3 / old_sizeof))
    print("     BOTH: %d of %d sizeof bytes" % (tot3_new + old3, new_sizeof + old_sizeof))
    out["cross_body_new_bytes"] = tot3_new
    out["cross_body_both_bytes"] = tot3_new + old3

    # ---- the requirement --------------------------------------------------
    print("\n== THE REQUIREMENT: what a body must own per frame ==")
    req = [
        ("ModelViewMatrix", 64, "the body's placement - nothing else can carry it"),
        ("clipping_fov.v[0..1]", 8, "the body's own depth bracket (Renderer.hpp:150)"),
        ("nbShadowingBodies", 4, "how many entries this body receives"),
    ]
    const = [
        ("planetRadius", 4, ""),
        ("planetScaledRadius", 4, ""),
        ("planetOneMinusOblateness", 4, ""),
    ]
    rq = sum(b for _n, b, _c in req)
    cq = sum(b for _n, b, _c in const)
    for n, b, c in req:
        print("  %3d B  %-26s %s" % (b, n, c))
    print("  ---- per body per FRAME: %d B" % rq)
    for n, b, c in const:
        print("  %3d B  %-26s per body, constant after load" % (b, n))
    print("  ---- plus per-body CONSTANT: %d B   TOTAL PAYLOAD %d B" % (cq, rq + cq))
    A = S["ALIGNMENT"][0]
    minimum = ((rq + cq - 1) // A + 1) * A
    print("  carved at alignment %d -> %d B per body" % (A, minimum))
    print("  (+64 B, i.e. %d carved, if NormalMatrix stays on the CPU side "
          "instead of being derived in the shader)"
          % (((rq + cq + 64 - 1) // A + 1) * A))
    POOL = S["POOL"][0]
    BASE = 142529
    print("  bodies per MiB at that minimum: %d  (of the free %d B after the "
          "launch scene's own %d B: %d bodies)"
          % (POOL // minimum, POOL - BASE, BASE, (POOL - BASE) // minimum))
    out["per_body_minimum_payload"] = rq + cq
    out["per_body_minimum_carved"] = minimum
    out["bodies_per_MiB_at_minimum"] = POOL // minimum

    if a.json:
        json.dump(out, open(a.json, "w"), indent=1, sort_keys=True)
        print("\nwrote %s" % a.json)
    return 0


if __name__ == "__main__":
    sys.exit(main())
