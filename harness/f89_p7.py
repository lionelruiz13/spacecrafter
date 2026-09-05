#!/usr/bin/env python3
"""F89 - what makes `b4_anchors`' P7 SCREEN-WITNESS control flip between two
binaries (INTENT §11.208; the red recorded at §11.205(g), the two failed
experiments at §11.205(h)).

This is an ANALYSIS instrument, not a launcher: it reads run directories that
`b4_anchors.py` already wrote (its `b4_*.json` dual dumps + `b4_orb_t{0,1}.png`
+ `b4_result.json`) and answers, per subcommand, the questions F89's mandate
asks.  It never launches anything, so it can be re-run on committed artifacts.

  margins  DIR...      the control's TWO compared scalars per shot and the
                       MARGIN each run passed/failed by (mandate (0)).  Reads
                       `b4_result.json` (or a .json.gz of one) only, so it works
                       on the five committed F86 records.
  scalars  DIR...      the four discriminating scalars per run, from the dumps:
                       the OLD path's distance to the Moon at the anchor legs,
                       the old-vs-new view-direction angle, `nbStarsToDraw`, and
                       the effective twinkle amount.
  leafdiff A B [tag]   every differing leaf of the two runs' dump HEADERS at the
                       precision the app printed (the old-view block is %.17g).
                       This is the H2 channel: `camera.mat` and the new-path
                       anchor body are NOT where a navModule defect shows.
  sets     A B         the P7 windows' lit-pixel SETS (px>8) diffed: within-run,
                       and between the two runs, per window and frame-wide.

The window geometry is `b4_anchors.py:444-453` reproduced exactly (radius from
the dumped R/d/halfFov, `win = max(8, int(2*rpx))`), not a tuned number: this
file must read the same box the gate reads or its numbers mean nothing.
"""
import json, math, os, sys, gzip

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from dumpread import sanitize_nonfinite, unquote_nonfinite   # noqa: E402

AU_KM = 149597870.7


def _open(path):
    return gzip.open(path, 'rt', errors='replace') if path.endswith('.gz') \
        else open(path, errors='replace')


def load_dump(path):
    """-> (header, {name: entry}) for one `body action dual_dump` file."""
    hdr, bodies = None, {}
    with _open(path) as f:
        for line in f:
            d = unquote_nonfinite(json.loads(sanitize_nonfinite(line)))
            if d.get("type") == "header":
                hdr = d
            elif d.get("type") == "body":
                bodies[d["name"]] = d
    return hdr, bodies


def result_path(d):
    for cand in (os.path.join(d, "b4_result.json"), d, d + ".json.gz"):
        if os.path.isfile(cand):
            return cand
    raise SystemExit("no b4_result.json for %s" % d)


def cmd_margins(dirs):
    """The control at `b4_anchors.py:476-480` compares MAX luminance in the
    Moon's own window against MAX luminance in the OTHER date's window, in each
    of the two shots.  Both numbers are in the check's own text - that is the
    only place they are recorded, so they are parsed from it."""
    import re
    print("%-22s | %-24s | %-24s | verdict" % ("run", "shot t0: own vs other", "shot t1: own vs other"))
    for d in dirs:
        with _open(result_path(d)) as f:
            rep = json.load(f)
        cells = None
        for c in rep["checks"]:
            if "SCREEN WITNESS control" in c["text"]:
                m = re.search(r"\((\d+) < (\d+) and (\d+) < (\d+)\)", c["text"])
                cells = (int(m.group(2)), int(m.group(1)), int(m.group(4)), int(m.group(3)))
                ok = c["ok"]
        own0, oth0, own1, oth1 = cells
        print("%-22s | own %3d  other %3d  m=%+4d | own %3d  other %3d  m=%+4d | %s (%d failure(s))"
              % (os.path.basename(d.rstrip('/')), own0, oth0, own0 - oth0, own1, oth1, own1 - oth1,
                 "PASS" if ok else "FAIL", rep["summary"]["failures"]))


def _angle(a, b):
    n = lambda v: math.sqrt(sum(x * x for x in v))
    d = sum(x * y for x, y in zip(a, b)) / (n(a) * n(b))
    return math.degrees(math.acos(max(-1.0, min(1.0, d))))


def cmd_scalars(dirs, tags=("base", "floor4", "orb_t0", "orb_t1", "fix_t0")):
    print("%-22s %-9s %13s %13s %9s %12s %8s" %
          ("run", "tag", "OLDdistMoon_km", "NEWdistMoon_km", "old^new_deg", "nbStarsToDraw", "twinkEff"))
    for d in dirs:
        for t in tags:
            p = os.path.join(d, "b4_%s.json" % t)
            if not os.path.isfile(p):
                continue
            h, b = load_dump(p)
            mo, mn = b["Moon"].get("old"), b["Moon"].get("new")
            s = h["oldView"]["stars"]
            print("%-22s %-9s %13.1f %13.1f %9.3f %12s %8.4f" % (
                os.path.basename(d.rstrip('/')), t,
                (mo["dist"] * AU_KM) if mo else float('nan'),
                (mn["dist"] * AU_KM) if mn else float('nan'),
                _angle(h["oldLocalVision"], h["camera"]["absFwd"]),
                s["nbStarsToDraw"], s["twinkleAmountEff"]))


def _flat(o, p=""):
    if isinstance(o, dict):
        for k, v in o.items():
            yield from _flat(v, p + "." + k)
    elif isinstance(o, list):
        for i, v in enumerate(o):
            yield from _flat(v, p + "[%d]" % i)
    else:
        yield p, o


def cmd_leafdiff(a, b, tag="orb_t0", show=200):
    A = dict(_flat(load_dump(os.path.join(a, "b4_%s.json" % tag))[0]))
    B = dict(_flat(load_dump(os.path.join(b, "b4_%s.json" % tag))[0]))
    keys = sorted(set(A) | set(B))
    diffs = [(k, A.get(k), B.get(k)) for k in keys if A.get(k) != B.get(k)]
    print("%s vs %s @ %s: %d of %d header leaves differ" %
          (os.path.basename(a.rstrip('/')), os.path.basename(b.rstrip('/')), tag, len(diffs), len(keys)))
    for k, x, y in diffs[:show]:
        print("   %-52s %r != %r" % (k, x, y))


def _window(dirpath):
    """The P7 window geometry, from the run's own dumps (b4_anchors.py:444-453)."""
    import numpy as np
    from PIL import Image
    h0, b0 = load_dump(os.path.join(dirpath, "b4_orb_t0.json"))
    h1, b1 = load_dump(os.path.join(dirpath, "b4_orb_t1.json"))
    im = np.asarray(Image.open(os.path.join(dirpath, "b4_orb_t0.png")).convert("RGB"))
    H = im.shape[0]
    hf = h0["camera"]["halfFov"]
    rpx = math.atan(b0["Moon"]["new"]["scaledDatumRadius"] / b0["Moon"]["new"]["dist"]) / hf * (H / 2)
    win = max(8, int(2 * rpx))
    P = {"t0": b0["Moon"]["new"]["screen"], "t1": b1["Moon"]["new"]["screen"]}
    return win, P, im.shape


def _mask(dirpath, shot, ndc, win, shape, thr=8):
    import numpy as np
    from PIL import Image
    a = np.asarray(Image.open(os.path.join(dirpath, "b4_%s.png" % shot)).convert("RGB")).max(axis=2)
    H, W = a.shape
    x = int((ndc[0] * 0.5 + 0.5) * W)
    y = int((1.0 - (ndc[1] * 0.5 + 0.5)) * H)
    sub = a[max(0, y - win):y + win, max(0, x - win):x + win]
    return (sub > thr), a


def cmd_sets(A, B):
    import numpy as np
    win, P, shape = _window(A)
    winB, PB, _ = _window(B)
    assert win == winB and P == PB, "the two runs' window geometry differs - not comparable"
    print("window +-%d px at NDC t0=%s t1=%s (frame %s)" % (win, P["t0"], P["t1"], shape))
    print("%-28s %-9s %-9s %8s %8s %8s %8s" %
          ("pair", "shot", "window", "|A|", "|B|", "A^B", "AandB"))
    for shot in ("orb_t0", "orb_t1"):
        for w in ("t0", "t1"):
            ma, fa = _mask(A, shot, P[w], win, shape)
            mb, fb = _mask(B, shot, P[w], win, shape)
            sym = int((ma ^ mb).sum())
            print("%-28s %-9s %-9s %8d %8d %8d %8d" % (
                os.path.basename(A.rstrip('/')) + " vs " + os.path.basename(B.rstrip('/')),
                shot, "@" + w, int(ma.sum()), int(mb.sum()), sym, int((ma & mb).sum())))
        fa8 = fa > 8
        fb8 = fb > 8
        print("%-28s %-9s %-9s %8d %8d %8d %8d   <- frame-wide" % (
            "", shot, "FRAME", int(fa8.sum()), int(fb8.sum()), int((fa8 ^ fb8).sum()), int((fa8 & fb8).sum())))


if __name__ == "__main__":
    cmd = sys.argv[1]
    if cmd == "margins":
        cmd_margins(sys.argv[2:])
    elif cmd == "scalars":
        cmd_scalars(sys.argv[2:])
    elif cmd == "leafdiff":
        cmd_leafdiff(sys.argv[2], sys.argv[3], *(sys.argv[4:5] or []))
    elif cmd == "sets":
        cmd_sets(sys.argv[2], sys.argv[3])
    else:
        raise SystemExit(__doc__)
