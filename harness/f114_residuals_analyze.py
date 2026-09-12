#!/usr/bin/env python3
"""F114 -- scores the two residual legs (f114_residuals.py). Usage: <dir>"""
import json, math, os, sys, gzip

D = sys.argv[1]


def hdr(n):
    p = os.path.join(D, n + ".json")
    f = open(p) if os.path.exists(p) else gzip.open(p + ".gz", "rt")
    h = json.loads(f.readline()); f.close(); return h


def has(n):
    p = os.path.join(D, n + ".json")
    return os.path.exists(p) or os.path.exists(p + ".gz")


def norm(v):
    m = math.sqrt(sum(c * c for c in v)); return [c / m for c in v]


def of(h):
    m = h["helioToEye"]; return norm([-m[2], -m[6], -m[10]])


def nf(h):
    return norm(h["camera"]["absFwd"])


def ang(a, b):
    return math.degrees(math.acos(max(-1, min(1, sum(x * y for x, y in zip(a, b))))))


print("R1 -- is the drawn path's tracking body-only? (`select star` + `zoom auto in`)")
for n in ("star_selected", "star_zoomed", "planet_zoomed"):
    if not has(n):
        continue
    h = hdr(n)
    print("  %-15s oldFlagTraking=%s camera.tracked=%-8r camera.selected=%-8r "
          "halfFov=%.6g angle(old,new)=%.6g deg"
          % (n, h["oldView"]["nav"]["flagTraking"], h["camera"]["tracked"],
             h["camera"].get("selected"), h["camera"]["halfFov"], ang(of(h), nf(h))))
print("  NOTE: on THIS host the star leg is inconclusive -- nothing was selected")
print("  (camera.selected = '' and old flagTraking stayed 0, i.e. autoZoomIn took")
print("  its `if (!selected_object) return;`). name.fab carries Bayer designations")
print("  (first line `   677|alpha_And`), 0 occurrences of 'Sirius', and the app")
print("  reads its catalogues from /usr/local/share/spacecrafter/stars where the")
print("  config's v0.8 zone files are absent (the smoke suite's S1 row, S5.77).")
print()

print("R2 -- a commanded duration under the camera's 0.2 s view-plan floor")
if has("d01_00"):
    h0 = hdr("d01_00"); o0, n0 = of(h0), nf(h0); jd0 = h0["jd"]
    hA = hdr("d01_after")
    print("  asked: `zoom auto initial duration 0.1`")
    print("  %-8s %-10s %-10s %-10s %-9s %-10s"
          % ("t (s)", "old prog", "new prog", "oldAutoMove", "newViewT", "angle(o,n)"))
    for i in range(10):
        n = "d01_%02d" % i
        if not has(n):
            continue
        h = hdr(n); t = (h["jd"] - jd0) * 86400
        print("  %-8.3f %-10.4f %-10.4f %-10s %-9.4f %-10.4f"
              % (t, ang(o0, of(h)), ang(n0, nf(h)),
                 h["oldView"]["nav"]["plans"]["flagAutoMove"],
                 h["camera"]["plans"]["viewT"], ang(of(h), nf(h))))
    print("  after : angle(old,new)=%.6g deg  viewT=%s  autoMove=%s"
          % (ang(of(hA), nf(hA)), hA["camera"]["plans"]["viewT"],
             hA["oldView"]["nav"]["plans"]["flagAutoMove"]))
    print("  the plan's own duration is dumped as camera.plans.viewT = 0.2000 for a")
    print("  commanded 0.1 -- the floor, read off the app rather than off the source.")
