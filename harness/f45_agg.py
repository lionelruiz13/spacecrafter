#!/usr/bin/env python3
"""F45 — the P6 aggregate: is the per-frame cost above this instrument's own floor?

A single empty-vs-full pair CANNOT answer that: the run-to-run drift of the UI_DRAW
phase (measured below) is the same order as the effect. So the legs are run four times
each, INTERLEAVED, and the comparison is made three ways, each of which can fail:

  (1) the effect channel   EXECUTOR_DRAW -> UI_DRAW   contains UI::draw, hence
      drawGravityUi -> getSelectedObjectShortInfo -> convertToSpectralType -> cLog
      (app.cpp:880-884, ui.cpp:278, ui_tuiconf.cpp:126);
  (2) the NEGATIVE CONTROL DRAW_RESOURCE_READY -> EXECUTOR_DRAW, the same frame, the
      same machine, the same drift — and no write in it. A difference that shows up
      here too is drift, not cost;
  (3) rank separation of the 4 v 4 per-run medians. Complete separation of 4 v 4 has
      p = 1/C(8,4) = 1/70 under exchangeability; anything less is reported as such.

It also writes the tail values out, so the statistic can be recomputed without the
6 MB of raw statistics.dat.

Usage: f45_agg.py <outdir> <legdir> [<legdir> ...]
"""
import gzip
import os
import statistics
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import f45_stats as S

EFFECT = ("EXECUTOR_DRAW", "UI_DRAW")
CONTROL = ("DRAW_RESOURCE_READY", "EXECUTOR_DRAW")
TAIL = 1000


def rank_separation(a, b):
    """1 if every value of a exceeds every value of b (or vice versa), else 0."""
    return int(min(a) > max(b) or min(b) > max(a))


def main():
    out = sys.argv[1]
    dirs = sys.argv[2:]
    os.makedirs(out, exist_ok=True)
    rows, csv = {}, ["leg,frame,effect_ms,control_ms"]
    for d in dirs:
        leg = os.path.basename(d)
        frames = S.read_stats(os.path.join(d, "statistics.dat"))
        if not frames:
            print(f"{leg}: NO statistics.dat")
            continue
        tail = frames[-TAIL:]
        e = S.phase(tail, *EFFECT)
        c = S.phase(tail, *CONTROL)
        rows[leg] = {"nframes": len(frames), "n": len(e),
                     "e_p50": statistics.median(e), "e_mean": statistics.mean(e),
                     "c_p50": statistics.median(c), "c_mean": statistics.mean(c)}
        for i, (x, y) in enumerate(zip(e, c)):
            csv.append(f"{leg},{i},{x:.6f},{y:.6f}")
        print(f"{leg:8s} frames={len(frames):5d} tail_n={len(e):4d}  "
              f"EFFECT p50={rows[leg]['e_p50']:.4f} mean={rows[leg]['e_mean']:.4f}  "
              f"CONTROL p50={rows[leg]['c_p50']:.4f} mean={rows[leg]['c_mean']:.4f}  (ms)")
    with gzip.open(os.path.join(out, "tail_phases.csv.gz"), "wt") as f:
        f.write("\n".join(csv) + "\n")

    grp = {}
    for leg in rows:
        grp.setdefault("".join(ch for ch in leg if not ch.isdigit()), []).append(leg)
    if set(grp) != {"empty", "full"}:
        print("\n(no empty/full grouping to compare)")
        return
    print()
    for key, label in [("e_p50", "EFFECT  UI_DRAW phase, per-run p50"),
                       ("e_mean", "EFFECT  UI_DRAW phase, per-run mean"),
                       ("c_p50", "CONTROL exec phase,     per-run p50"),
                       ("c_mean", "CONTROL exec phase,     per-run mean")]:
        E = [rows[l][key] for l in sorted(grp["empty"])]
        F = [rows[l][key] for l in sorted(grp["full"])]
        d = statistics.mean(E) - statistics.mean(F)
        sep = rank_separation(E, F)
        print(f"{label}")
        print(f"   empty {[round(x, 4) for x in E]}  mean={statistics.mean(E):.4f} "
              f"sd={statistics.stdev(E):.4f}")
        print(f"   full  {[round(x, 4) for x in F]}  mean={statistics.mean(F):.4f} "
              f"sd={statistics.stdev(F):.4f}")
        print(f"   delta(empty-full) = {d:+.4f} ms = {d * 100:+.2f}% of the 1 ms/frame "
              f"budget (D11);  rank-separated 4v4: {'YES (p=1/70)' if sep else 'NO'}")


if __name__ == "__main__":
    main()
