#!/usr/bin/env python3
"""F121 -- read the per-frame seam trace out of a dual dump and score it.

The trace is `Core::recordSeamStep`'s ring (core.cpp, INTENT S11.245), written
into the dual-path dump's header as the "seam" member. One record per frame,
taken at the end of Executor::update -- the one point in the frame where BOTH
engines have advanced for the SAME frame.

"Identical" is delta zero ON EVERY FRAME of a transition, not only at its end,
so every statistic here is a MAX over a frame window and never a mean.

Usage:
  f121_seam.py <dump.json> [--marks <marks.txt>] [--csv <out.csv>]
                           [--window a:b] [--quiet]
  f121_seam.py --diff <before.json> <after.json>

A marks file is one `<frame> <label>` per line (a driver writes it from the
`frames` counter the dump reports at each command), which cuts the trace into
per-command windows so a row of the table is a TRANSITION and not a run.
"""
import sys, os, json, math, gzip

CHANNELS = [
    # key             label                       unit
    ("dFovDeg",      "fov (old full - new full)", "deg"),
    ("viewAngleAbs", "view angle, root frame",    "deg"),
    ("viewAngleLocal", "view angle, local frame", "deg"),
    ("posDelta",     "observer position",         "AU"),
    ("dHeadingDeg",  "heading (old - new)",       "deg"),
    ("dLonDeg",      "place longitude",           "deg"),
    ("dLatDeg",      "place latitude",            "deg"),
    ("dAltMetres",   "place altitude",            "m"),
]

FLAGBITS = [(1, "oldAutoMove"), (2, "oldHdgRamp"), (4, "oldTracking"),
            (8, "newTracked"), (16, "trackNameEq"), (32, "refNameEq"),
            (64, "newFreeMode")]


def openmaybegz(p):
    if os.path.exists(p):
        return open(p)
    return gzip.open(p + ".gz", "rt")


def load(path):
    """The dump is JSON-lines: header first, then one line per body."""
    with openmaybegz(path) as f:
        head = json.loads(f.readline())
    seam = head.get("seam")
    if seam is None:
        raise SystemExit("%s: no \"seam\" member -- the binary predates the "
                         "recorder, or SC_SEAM_RECORD was unset" % path)
    return head, seam


def derive(step):
    """The two derived channels the recorder deliberately does NOT store:
    both are differences of quantities kept in their OWN units, so that the
    conversion is written once, here, and the record stays raw."""
    d = dict(step)
    d["dFovDeg"] = step["fovOld"] - step["halfFovNew"] * 360.0 / math.pi
    # WRAPPED into (-180, 180]. Old's getHeading() already wraps to that
    # branch (navigator.hpp) and the camera's `heading` is a raw radian
    # parameter that is never wrapped, so the RAW difference reads 360 deg for
    # two headings that are the same angle -- measured on the first control
    # leg. The record keeps both raw, which is right; the wrap is a reader's
    # job because it is a statement about what the two numbers MEAN.
    h = step["headingOldDeg"] - step["headingNewRad"] * 180.0 / math.pi
    h -= math.floor((h + 180.0) / 360.0) * 360.0
    d["dHeadingDeg"] = h
    return d


def read_marks(path):
    marks = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            n, _, label = line.partition(" ")
            marks.append((int(n), label.strip()))
    return marks


def windows(steps, marks):
    """[(label, first_index, last_index)] over the step list."""
    if not marks:
        return [("whole trace", 0, len(steps) - 1)]
    byframe = {s["frame"]: i for i, s in enumerate(steps)}
    out = []
    for k, (n, label) in enumerate(marks):
        start = None
        for f in range(n, n + 4000):
            if f in byframe:
                start = byframe[f]
                break
        if start is None:
            continue
        if k + 1 < len(marks):
            end = None
            for f in range(marks[k + 1][0], marks[k + 1][0] + 4000):
                if f in byframe:
                    end = byframe[f] - 1
                    break
            if end is None or end < start:
                end = len(steps) - 1
        else:
            end = len(steps) - 1
        out.append((label, start, end))
    return out


def maxabs(steps, key):
    best, at = 0.0, None
    for s in steps:
        v = s.get(key)
        if v is None:
            continue
        if isinstance(v, str):      # "inf"/"nan" guard
            continue
        if v < 0 and key in ("viewAngleAbs", "viewAngleLocal"):
            continue                # -1 == degenerate, reported separately
        if abs(v) > best:
            best, at = abs(v), s["frame"]
    return best, at


def flagsummary(steps):
    on = {}
    for bit, name in FLAGBITS:
        n = sum(1 for s in steps if s["flags"] & bit)
        if n:
            on[name] = n
    return on


def report(path, marks_path=None, csv_path=None, quiet=False):
    head, seam = load(path)
    steps = [derive(s) for s in seam["steps"]]
    print("== %s ==" % path)
    print("armed=%s  total=%d  dropped=%d  frames=%d  kept=%d"
          % (seam["armed"], seam["total"], seam["dropped"], seam["frames"], len(steps)))
    if not steps:
        print("NO RECORDS -- the recorder was not armed for this run.")
        return head, seam, steps
    marks = read_marks(marks_path) if marks_path else []
    print()
    hdr = "%-34s %7s %6s" % ("window", "frames", "")
    for _, label, unit in CHANNELS:
        hdr += " %18s" % (label.split(",")[0][:16] + "/" + unit)
    print(hdr)
    for label, a, b in windows(steps, marks):
        seg = steps[a:b + 1]
        row = "%-34s %7d %6s" % (label[:34], len(seg), "")
        for key, _, _ in CHANNELS:
            m, at = maxabs(seg, key)
            row += " %18.6g" % m
        print(row)
        if not quiet:
            fl = flagsummary(seg)
            if fl:
                print("      flags: " + ", ".join("%s=%d" % kv for kv in sorted(fl.items())))
    if csv_path:
        with open(csv_path, "w") as f:
            keys = ["frame", "dt", "jd", "fovOld", "aimFovOld", "halfFovNew",
                    "zoomSrcNew", "zoomDstNew", "dFovDeg", "viewAngleAbs",
                    "viewAngleLocal", "posDelta", "headingOldDeg",
                    "headingNewRad", "dHeadingDeg", "dLonDeg", "dLatDeg",
                    "dAltMetres", "moveCoefOld", "viewTNew", "hdgTNew",
                    "zoomTNew", "moveTNew", "flags"]
            f.write(",".join(keys) + "\n")
            for s in steps:
                f.write(",".join(repr(s.get(k, "")) for k in keys) + "\n")
        print("\ncsv -> %s" % csv_path)
    return head, seam, steps


def diff(p1, p2):
    _, s1 = load(p1)
    _, s2 = load(p2)
    a = [derive(s) for s in s1["steps"]]
    b = [derive(s) for s in s2["steps"]]
    print("%-28s %16s %16s   verdict" % ("channel (max |delta|)", "BEFORE", "AFTER"))
    for key, label, unit in CHANNELS:
        m1, _ = maxabs(a, key)
        m2, _ = maxabs(b, key)
        v = "ZERO" if m2 == 0.0 else ("improved x%.3g" % (m1 / m2) if m2 and m1 > m2
                                      else ("unchanged" if m1 == m2 else "WORSE"))
        print("%-28s %16.6g %16.6g   %s" % (label + " /" + unit, m1, m2, v))


if __name__ == "__main__":
    args = sys.argv[1:]
    if not args:
        raise SystemExit(__doc__)
    if args[0] == "--diff":
        diff(args[1], args[2])
    else:
        path = args[0]
        marks = csv = None
        quiet = False
        i = 1
        while i < len(args):
            if args[i] == "--marks":
                marks = args[i + 1]; i += 2
            elif args[i] == "--csv":
                csv = args[i + 1]; i += 2
            elif args[i] == "--quiet":
                quiet = True; i += 1
            else:
                i += 1
        report(path, marks, csv, quiet)
