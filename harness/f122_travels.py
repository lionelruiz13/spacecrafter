#!/usr/bin/env python3
"""F122 -- read `seam.travels` out of a dual dump and DIFFERENCE the two
registries' travel inputs term for term (INTENT S11.246).

The per-frame ring (f121_seam.py) says WHERE the two engines are. This says
WHAT EACH TRAVEL WAS HANDED. Both registries keep the same five values in the
same units -- root/heliocentric AU and JD -- so the comparison needs no frame
conversion and no convention undoing:

    start      the travel's origin        AU, root frame
    dir        unit direction             root frame
    distance   length travelled along dir AU
    startTime  the law's start date       JD
    endTime    the law's arrival date     JD

An install pairs old (engine 0) with new (engine 1) when the two records are
within `--pair-window` recorder frames of each other; an unpaired record is
printed alone and SAID to be unpaired rather than silently matched to the
nearest thing (old has no install counter, so a zero-duration old install is
invisible by construction -- see the SeamTravel contract in core.hpp).

Usage:
  f122_travels.py <dump.json|.gz> [<dump2> ...] [--pair-window N]
"""
import sys, os, json, math, gzip


def openmaybegz(p):
    if p.endswith(".gz"):
        return gzip.open(p, "rt")
    if os.path.exists(p):
        return open(p)
    return gzip.open(p + ".gz", "rt")


def load(path):
    with openmaybegz(path) as f:
        head = json.loads(f.readline())
    seam = head.get("seam")
    if seam is None:
        raise SystemExit("%s: no \"seam\" member" % path)
    return head, seam


def vsub(a, b):
    return [a[i] - b[i] for i in range(3)]


def vlen(a):
    return math.sqrt(sum(x * x for x in a))


def angle_deg(a, b):
    la, lb = vlen(a), vlen(b)
    if not (la > 0 and lb > 0):
        return -1.0
    c = sum(a[i] * b[i] for i in range(3)) / (la * lb)
    c = max(-1.0, min(1.0, c))
    return math.degrees(math.acos(c))


def endpoint(t):
    return [t["start"][i] + t["dir"][i] * t["distance"] for i in range(3)]


AU_KM = 149597870.691   # sc_const.hpp's AU, in km -- printed so a reader can
                        # check the conversion rather than trust it


def report(path, pair_window=8):
    head, seam = load(path)
    travels = seam.get("travels")
    if travels is None:
        raise SystemExit("%s: no \"seam.travels\" -- the binary predates "
                         "S11.246's recorder extension" % path)
    print("== %s ==" % path)
    print("travelsSeen=%d  kept=%d  frames=%d"
          % (seam.get("travelsSeen", -1), len(travels), seam["frames"]))
    if not travels:
        print("NO TRAVEL INSTALLS in this run.")
        return
    print()
    for t in travels:
        who = "old AnchorManager" if t["engine"] == 0 else "new CameraAnchors"
        e = endpoint(t)
        print("frame %6d  %-18s  jd=%.9f" % (t["frame"], who, t["jd"]))
        print("    start    = [% .12e, % .12e, % .12e]  |start| = %.6e AU"
              % (t["start"][0], t["start"][1], t["start"][2], vlen(t["start"])))
        print("    dir      = [% .9f, % .9f, % .9f]"
              % (t["dir"][0], t["dir"][1], t["dir"][2]))
        print("    distance = %.12e AU    startTime = %.9f  endTime = %.9f"
              % (t["distance"], t["startTime"], t["endTime"]))
        print("    endpoint = [% .12e, % .12e, % .12e]" % (e[0], e[1], e[2]))
    # --- pairing -------------------------------------------------------
    olds = [t for t in travels if t["engine"] == 0]
    news = [t for t in travels if t["engine"] == 1]
    used = set()
    print("\n-- PAIRED INSTALLS (|frame difference| <= %d) --" % pair_window)
    npaired = 0
    for o in olds:
        best, bd = None, None
        for i, n in enumerate(news):
            if i in used:
                continue
            d = abs(n["frame"] - o["frame"])
            if d <= pair_window and (bd is None or d < bd):
                best, bd = i, d
        if best is None:
            print("old install at frame %d: UNPAIRED" % o["frame"])
            continue
        used.add(best)
        n = news[best]
        npaired += 1
        ds = vsub(o["start"], n["start"])
        de = vsub(endpoint(o), endpoint(n))
        print("\n[%d] old frame %d  vs  new frame %d" % (npaired, o["frame"], n["frame"]))
        print("    |start_old - start_new|    = %.6e AU  = %.3f km"
              % (vlen(ds), vlen(ds) * AU_KM))
        print("    angle(dir_old, dir_new)    = %.6f deg" % angle_deg(o["dir"], n["dir"]))
        print("    distance_old - distance_new= %.6e AU  = %.3f km"
              % (o["distance"] - n["distance"], (o["distance"] - n["distance"]) * AU_KM))
        print("    startTime_old - _new       = %.9e d" % (o["startTime"] - n["startTime"]))
        print("    endTime_old   - _new       = %.9e d" % (o["endTime"] - n["endTime"]))
        print("    |endpoint_old - endpoint_new| = %.6e AU  = %.3f km"
              % (vlen(de), vlen(de) * AU_KM))
    for i, n in enumerate(news):
        if i not in used:
            print("new install at frame %d: UNPAIRED (|start| = %.6e AU, "
                  "distance = %.6e AU)" % (n["frame"], vlen(n["start"]), n["distance"]))


if __name__ == "__main__":
    args = [a for a in sys.argv[1:]]
    pw = 8
    if "--pair-window" in args:
        i = args.index("--pair-window")
        pw = int(args[i + 1])
        del args[i:i + 2]
    if not args:
        raise SystemExit(__doc__)
    for p in args:
        report(p, pw)
        print()
