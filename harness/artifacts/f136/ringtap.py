#!/usr/bin/env python3
# Print the ramp ring's key-step rows of a leg-Z record: projector and Camera before -> after, and the per-call
# factor's error against the law with its cause (the drawn fov feeding the projector's step).
import json, math, re, sys
def clean(ln):
    return re.sub(r'(?<![\w"])(-?)(nan|inf)(?![\w"])', lambda m: 'null' if m.group(2) == 'nan' else m.group(1) + '1e308', ln.strip())
def rows(path):
    return json.loads(clean(open(path).readline()))["ramp"]["steps"]
d = sys.argv[1]
for tag in ("Z2_same_tap", "Z2_opposite_tap"):
    r = [x for x in rows("%s/%s.json" % (d, tag)) if x["active"] and x["dFov"] != 0][-1]
    print("%-16s projector %.4f -> %.4f | camera %.4f -> %.4f deg | camera jump %.4f deg"
          % (tag, r["fov"], r["fovAfter"], 2 * math.degrees(r["halfFov"]), 2 * math.degrees(r["halfFovAfter"]),
             abs(2 * math.degrees(r["halfFovAfter"]) - 2 * math.degrees(r["halfFov"]))))
K = 5 * 0.0001
for x in [x for x in rows("%s/Z_ramp.json" % d) if x["active"] and x["dFov"] != 0]:
    k = K * x["dt"]
    law = 1 - k if x["dFov"] < 0 else 1 + k
    err = x["fovAfter"] / x["fov"] - law
    drawn = 2 * math.degrees(x["halfFov"])
    rel = drawn / x["fov"] - 1
    print("Z_ramp dt=%d dFov=%+.9f factor err %+.3e | drawn/proj-1 %+.3e (%.2f float ulps of 2^-23) | k*rel %+.3e | err-sign*k*rel %+.1e | camera-proj after rel %+.2e"
          % (x["dt"], x["dFov"], err, rel, abs(rel) / 2**-23, k * rel, err - (1 if x["dFov"] > 0 else -1) * k * rel,
             (2 * math.degrees(x["halfFovAfter"]) - x["fovAfter"]) / x["fovAfter"]))
