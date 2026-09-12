#!/usr/bin/env python3
"""F114 analyzer -- scores an `f38_gaps.py` run (pre or post) against S5.100 and
S5.101, and diffs two runs.

THE PARITY OBSERVABLE, and why it is this one: the dual dump's header carries
BOTH paths' look direction in ONE frame -- old's `helioToEye` (helio -> eye, so
the eye forward in helio coords is -(m[2], m[6], m[10])) and the camera's
`absFwd`, which Camera::update writes as the eye forward expressed in ROOT
coords (Camera.cpp:730-732, INTENT S11.61/B13). They are the SAME frame:
measured on the landed F38 record, in the one state where both paths hold the
same body (g2_before, `flag track_object on` -- the dual setter), the angle
between them is 1.707547e-06 deg. That number is the FLOOR of this observable
and every parity claim here is stated against it.

  G1 (S5.100) `zoom auto in`      : camera.tracked, old/new Mars screen
                                    position, the angle, across a 0.05 d advance
  G2 (S5.101) `zoom auto initial` : the angle across the command

Usage: f114_gaps.py <dir> [<dir2>]     (two dirs = pre/post table)
"""
import sys, os, json, math, gzip


def opendump(d, name):
    """Landed dumps are gzipped in the repository; a fresh run's are not."""
    p = os.path.join(d, name + ".json")
    if os.path.exists(p):
        return open(p)
    return gzip.open(p + ".gz", "rt")


def exists(d, name):
    p = os.path.join(d, name + ".json")
    return os.path.exists(p) or os.path.exists(p + ".gz")


def header(d, name):
    with opendump(d, name) as f:
        return json.loads(f.readline())


def body(d, name, who):
    with opendump(d, name) as f:
        f.readline()
        for line in f:
            r = json.loads(line)
            if r.get("name") == who:
                return r
    return None


def norm(v):
    n = math.sqrt(sum(c * c for c in v))
    return [c / n for c in v] if n else v


def old_fwd(h):
    m = h["helioToEye"]
    return norm([-m[2], -m[6], -m[10]])


def new_fwd(h):
    return norm(h["camera"]["absFwd"])


def angle_deg(a, b):
    c = max(-1.0, min(1.0, sum(x * y for x, y in zip(a, b))))
    return math.degrees(math.acos(c))


SCENES = ["g1_selected", "g1_after_zoom_in", "g1_t0", "g1_t1",
          "g2_before", "g2_after"]


def read(d):
    rows = []
    for s in SCENES:
        if not exists(d, s):
            continue
        h = header(d, s)
        mars = body(d, s, "Mars")
        o = mars.get("old", {}) if mars else {}
        n = mars.get("new", {}) if mars else {}
        rows.append(dict(
            scene=s,
            jd=h["jd"],
            tracked=h["camera"]["tracked"],
            flagTraking=h["oldView"]["nav"]["flagTraking"],
            skyLocked=h["camera"]["skyLocked"],
            flagLockEquPos=h["oldView"]["nav"]["flagLockEquPos"],
            halfFov=h["camera"]["halfFov"],
            viewT=h["camera"]["plans"]["viewT"],
            autoMove=h["oldView"]["nav"]["plans"]["flagAutoMove"],
            angle=angle_deg(old_fwd(h), new_fwd(h)),
            oldScreen=o.get("screen"),
            newScreen=n.get("screen"),
            newLastJD=n.get("lastJD"),
            oldFwd=old_fwd(h),
            newFwd=new_fwd(h),
        ))
    return rows


def show(d):
    print("== %s" % d)
    print("%-18s %-8s %-6s %-5s %-11s %-12s %-24s %-22s" %
          ("scene", "tracked", "oldTrk", "skyL", "angle(deg)", "halfFov",
           "old Mars screen px", "new Mars screen"))
    for r in read(d):
        print("%-18s %-8s %-6s %-5s %-11.6g %-12.6g %-24s %-22s" % (
            r["scene"], repr(r["tracked"]), r["flagTraking"], r["skyLocked"],
            r["angle"], r["halfFov"],
            "[%.4f, %.4f]" % tuple(r["oldScreen"]) if r["oldScreen"] else "-",
            "[%.6g, %.6g]" % tuple(r["newScreen"]) if r["newScreen"] else "-"))
    print()


def diff(d1, d2):
    a = {r["scene"]: r for r in read(d1)}
    b = {r["scene"]: r for r in read(d2)}
    print("== PRE %s  ->  POST %s" % (d1, d2))
    print("%-18s %-22s %-22s %-24s" %
          ("scene", "tracked pre->post", "angle pre->post (deg)",
           "new Mars screen pre->post"))
    for s in SCENES:
        if s not in a or s not in b:
            continue
        ns1 = "[%.5g,%.5g]" % tuple(a[s]["newScreen"]) if a[s]["newScreen"] else "-"
        ns2 = "[%.5g,%.5g]" % tuple(b[s]["newScreen"]) if b[s]["newScreen"] else "-"
        print("%-18s %-22s %-22s %-24s" % (
            s,
            "%r -> %r" % (a[s]["tracked"], b[s]["tracked"]),
            "%.6g -> %.6g" % (a[s]["angle"], b[s]["angle"]),
            "%s -> %s" % (ns1, ns2)))
    print()
    print("old-side identity (must not move): old Mars screen, old flagTraking")
    for s in SCENES:
        if s not in a or s not in b:
            continue
        o1, o2 = a[s]["oldScreen"], b[s]["oldScreen"]
        d = (max(abs(o1[0] - o2[0]), abs(o1[1] - o2[1]))
             if o1 and o2 else float("nan"))
        print("  %-18s old screen %-26s -> %-26s  max|d| = %.6g px ; flagTraking %s -> %s"
              % (s, o1, o2, d, a[s]["flagTraking"], b[s]["flagTraking"]))


if __name__ == "__main__":
    if len(sys.argv) == 2:
        show(sys.argv[1])
    else:
        show(sys.argv[1]); show(sys.argv[2]); diff(sys.argv[1], sys.argv[2])
