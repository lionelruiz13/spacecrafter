#!/usr/bin/env python3
"""F45 — the analyzer: the empty-vs-populated comparison, on every predicted channel.

Reads each leg's artifacts and answers P1..P6 with a value or an explicit NONE. Every
comparison here is one that CAN fail: P1 is a predicted null, so its instrument is
checked against a case where it does move (the A/A floor and the two legs' own
per-frame counts), and a null is only reported after the instrument has been shown
to be alive.

statistics.dat is a packed stream of `struct TimePoint {int type; float datetime;}`
(EntityCore/Tools/CaptureMetrics.hpp:44-47), written 2048 points at a time. `datetime`
is seconds since the frame's own FRAME_START (type 0), because `capture()` resets its
origin on type 0 (CaptureMetrics.cpp:47) — so intervals WITHIN a frame are exact and
frames cannot be aligned to a wall clock. The dwell is therefore taken as the LAST N
frames of the run, which phase D + the immediate shutdown make the dwell by construction.

Usage: f45_stats.py <legdir> [<legdir> ...]
"""
import json
import os
import re
import struct
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
CAPTURE_HPP = os.path.join(HERE, "..", "..", "src", "capture.hpp")
DWELL_FRAMES = 1000   # tail window; phase D dwells 30 s, so this is safely inside it


def capture_names():
    """The type->name map, read from the enum in the source (the authority)."""
    src = open(CAPTURE_HPP).read()
    body = src.split("enum class Capture : int {", 1)[1].split("}", 1)[0]
    return [t.strip() for t in body.split(",") if t.strip()]


NAMES = capture_names()
IDX = {n: i for i, n in enumerate(NAMES)}


def read_stats(path):
    """-> list of frames; each frame is a dict name -> datetime (s since FRAME_START)."""
    if not os.path.exists(path):
        return None
    raw = open(path, "rb").read()
    n = len(raw) // 8
    pts = struct.unpack("<" + "if" * n, raw[: n * 8])
    frames, cur = [], None
    for i in range(n):
        t, dt = pts[2 * i], pts[2 * i + 1]
        if t == 0:
            if cur:
                frames.append(cur)
            cur = {}
            continue
        if cur is None:
            continue
        if 0 <= t < len(NAMES):
            cur[NAMES[t]] = dt
    if cur:
        frames.append(cur)
    return frames


def phase(frames, a, b):
    """The a->b interval per frame, in MILLISECONDS, for frames that carry both."""
    out = []
    for f in frames:
        if a in f and b in f:
            out.append((f[b] - f[a]) * 1000.0)
    return out


def stat(v):
    if not v:
        return None
    s = sorted(v)
    n = len(s)
    return {
        "n": n,
        "min_ms": s[0],
        "p50_ms": s[n // 2],
        "p90_ms": s[int(n * 0.9)],
        "p99_ms": s[min(n - 1, int(n * 0.99))],
        "max_ms": s[-1],
        "mean_ms": sum(s) / n,
    }


def fmt(st):
    if st is None:
        return "NONE"
    return (f"n={st['n']:5d} min={st['min_ms']:.4f} p50={st['p50_ms']:.4f} "
            f"p90={st['p90_ms']:.4f} p99={st['p99_ms']:.4f} max={st['max_ms']:.4f} "
            f"mean={st['mean_ms']:.4f}  (ms)")


def load_leg(d):
    leg = {"dir": d, "name": os.path.basename(d)}
    pj = os.path.join(d, "probe.json")
    leg["probe"] = json.load(open(pj)) if os.path.exists(pj) else None
    leg["frames"] = read_stats(os.path.join(d, "statistics.dat"))
    lg = os.path.join(d, "spacecrafter.log")
    leg["log"] = open(lg, "rb").read().decode("utf-8", "replace") if os.path.exists(lg) else ""
    shot = os.path.join(d, "sky.png")
    if os.path.exists(shot):
        b = open(shot, "rb").read()
        leg["shot_bytes"] = len(b)
        leg["shot_crc"] = format(zlib.crc32(b) & 0xFFFFFFFF, "08x")
    else:
        leg["shot_bytes"] = None
        leg["shot_crc"] = None
    return leg


def main():
    legs = [load_leg(d) for d in sys.argv[1:]]
    print("=" * 78)
    print("F45 — empty vs populated spectral array")
    print("=" * 78)

    for L in legs:
        p = L["probe"]
        print(f"\n### leg {L['name']}")
        if p is None:
            print("  no probe.json — leg did not complete")
            continue
        spec = {h: v["spectral"] for h, v in p["hp"].items()}
        present = [h for h, v in spec.items() if v is not None]
        nonempty = [h for h, v in spec.items() if v]
        cids = {h: v["compids"] for h, v in p["hp"].items()}
        cids_nonempty = [h for h, v in cids.items() if v]
        print(f"  P2  HP sampled                    : {len(p['hp'])}")
        print(f"  P2  spectral-type line PRESENT    : {len(present)}")
        print(f"  P2  ... with a NON-EMPTY value    : {len(nonempty)}")
        vals = sorted({v for v in spec.values() if v})
        print(f"  P2  distinct spectral values      : {len(vals)}  {vals[:12]}")
        print(f"  P4  HP line carrying a component id (positive control): "
              f"{len(cids_nonempty)}  {sorted(set(v for v in cids.values() if v))}")
        bad = len(re.findall(r"convertToSpectralType: bad index", L["log"]))
        badc = len(re.findall(r"convertToComponentIds: bad index", L["log"]))
        maxes = sorted(set(re.findall(r"convertToSpectralType: bad index: \d+, max: (\d+)", L["log"])))
        print(f"  P3  log lines 'convertToSpectralType: bad index' : {bad}   max= {maxes}")
        print(f"  P3  log lines 'convertToComponentIds: bad index' : {badc}")
        print(f"  P3  log lines 'ERROR while loading data'         : "
              f"{len(re.findall(r'ERROR while loading data', L['log']))}")
        ph = p["phases"]
        if "D_start_badindex" in ph:
            d = ph["D_end_badindex"] - ph["D_start_badindex"]
            print(f"  P5  per-frame lines during the {ph['D_dwell_s']:.1f}s dwell : {d}"
                  f"   ({d / ph['D_dwell_s']:.1f} lines/s)  target hp={p['perframe_target_hp']}")
        print(f"  P1  screenshot: {L['shot_bytes']} bytes  crc32={L['shot_crc']}")
        fr = L["frames"]
        if fr:
            tail = fr[-DWELL_FRAMES:]
            print(f"  frames captured: {len(fr)}   (tail window = {len(tail)})")
            for a, b in [("EXECUTOR_DRAW", "UI_DRAW"),
                         ("DRAW_RESOURCE_READY", "EXECUTOR_DRAW"),
                         ("FRAME_ACQUIRE", "ASYNC_FRAME_SUBMIT")]:
                print(f"  P6  {a:>19s} -> {b:<19s} TAIL  {fmt(stat(phase(tail, a, b)))}")
            print(f"  P6  {'EXECUTOR_DRAW':>19s} -> {'UI_DRAW':<19s} ALL   "
                  f"{fmt(stat(phase(fr, 'EXECUTOR_DRAW', 'UI_DRAW')))}")
        else:
            print("  frames captured: NONE (no statistics.dat)")

    # ---------------- pairwise ----------------
    print("\n" + "=" * 78)
    print("PAIRWISE")
    print("=" * 78)
    for i in range(len(legs)):
        for j in range(i + 1, len(legs)):
            A, B = legs[i], legs[j]
            print(f"\n--- {A['name']}  vs  {B['name']} ---")
            if A["shot_crc"] and B["shot_crc"]:
                print(f"  P1 screenshot crc32 : {A['shot_crc']} vs {B['shot_crc']}  "
                      f"{'IDENTICAL' if A['shot_crc'] == B['shot_crc'] else 'DIFFERENT'}")
            if A["probe"] and B["probe"]:
                sa = {h: v["spectral"] for h, v in A["probe"]["hp"].items()}
                sb = {h: v["spectral"] for h, v in B["probe"]["hp"].items()}
                same = sum(1 for h in sa if h in sb and sa[h] == sb[h])
                diff = [h for h in sa if h in sb and sa[h] != sb[h]]
                print(f"  P2 spectral readout: {same} of {len(sa)} identical, "
                      f"{len(diff)} differ")
                for h in sorted(diff, key=int)[:8]:
                    print(f"       hp {h:>6s}: {sa[h]!r}  ->  {sb[h]!r}")
                ca = {h: v["compids"] for h, v in A["probe"]["hp"].items()}
                cb = {h: v["compids"] for h, v in B["probe"]["hp"].items()}
                cdiff = [h for h in ca if h in cb and ca[h] != cb[h]]
                print(f"  P4 component-id readout (control): {len(cdiff)} differ "
                      f"(expected 0 — same file in both legs)")
            fa, fb = A["frames"], B["frames"]
            if fa and fb:
                for a, b in [("EXECUTOR_DRAW", "UI_DRAW")]:
                    va = stat(phase(fa[-DWELL_FRAMES:], a, b))
                    vb = stat(phase(fb[-DWELL_FRAMES:], a, b))
                    if va and vb:
                        d50 = vb["p50_ms"] - va["p50_ms"]
                        dme = vb["mean_ms"] - va["mean_ms"]
                        print(f"  P6 {a}->{b} tail: p50 {va['p50_ms']:.4f} -> "
                              f"{vb['p50_ms']:.4f} ms  (delta {d50:+.4f} ms, "
                              f"{d50 / 1.0 * 100:+.2f}% of the 1 ms/frame budget)")
                        print(f"     mean {va['mean_ms']:.4f} -> {vb['mean_ms']:.4f} ms "
                              f"(delta {dme:+.4f} ms, {dme * 100:+.2f}% of budget)")


if __name__ == "__main__":
    main()
