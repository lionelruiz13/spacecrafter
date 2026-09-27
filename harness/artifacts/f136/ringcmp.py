#!/usr/bin/env python3
# Compare the OLD engine's ring columns (projector fov, fovAfter, dFov, dt) of Z_ramp between two leg-Z records, bit for bit;
# the Camera's columns are printed as the max |difference| only.
import json, math, re, sys
def clean(ln):
    return re.sub(r'(?<![\w"])(-?)(nan|inf)(?![\w"])', lambda m: 'null' if m.group(2) == 'nan' else m.group(1) + '1e308', ln.strip())
def act(d):
    return [x for x in json.loads(clean(open(d + "/Z_ramp.json").readline()))["ramp"]["steps"] if x["active"] and x["dFov"] != 0]
a, b = act(sys.argv[1]), act(sys.argv[2])
same = len(a) == len(b) and all(x[k] == y[k] for x, y in zip(a, b) for k in ("fov", "fovAfter", "dFov", "dt"))
cam = max(abs(x["halfFovAfter"] - y["halfFovAfter"]) for x, y in zip(a, b))
print("%s vs %s: %d / %d steps; projector fov, fovAfter, dFov, dt bit-identical: %s; camera halfFovAfter max |diff| %.3e rad"
      % (sys.argv[1], sys.argv[2], len(a), len(b), same, cam))
