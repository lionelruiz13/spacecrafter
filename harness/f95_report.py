#!/usr/bin/env python3
"""F95 - the soak's tables, read back out of what the driver wrote.

    ./f95_report.py <absOutdir> [--hours-bucket 1.0]

Separate from `f95_soak.py` on purpose: the driver's job is to RECORD, and a
driver that also summarised would be deciding what mattered while it was still
happening.  Everything here is derived from `samples.csv`, `cycles.csv`,
`cycle_*.json` and `verdict.json`, so every number in the entry can be
re-derived from the committed artifacts by re-running this.

The LEAK verdict and the dump diff are IMPORTED from `f95_soak.py`, never
re-implemented: they are the criteria that were committed before the launch and
a second copy of them is exactly the desync I2 forbids.
"""

import argparse
import csv
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from f95_soak import (leak_verdict, dump_constant_view, dump_diff,   # noqa: E402
                      HUNG_S, J0)


def fnum(x):
    try:
        return float(x)
    except (TypeError, ValueError):
        return None


def slope_per_hour(ts, vs):
    """Least-squares slope in units per hour.  Stated, never eyeballed."""
    pts = [(t, v) for t, v in zip(ts, vs) if t is not None and v is not None]
    if len(pts) < 2:
        return None
    n = len(pts)
    mt = sum(p[0] for p in pts) / n
    mv = sum(p[1] for p in pts) / n
    den = sum((p[0] - mt) ** 2 for p in pts)
    if den == 0:
        return None
    num = sum((p[0] - mt) * (p[1] - mv) for p in pts)
    return (num / den) * 3600.0


def col(rows, name, cast=fnum):
    return [cast(r.get(name)) for r in rows]


def summarise(rows, name, unit="", scale=1.0):
    ts = col(rows, "t_rel_s")
    vs = [v / scale if v is not None else None for v in col(rows, name)]
    have = [v for v in vs if v is not None]
    if not have:
        return "%-18s no data" % name
    sl = slope_per_hour(ts, vs)
    return ("%-18s first %12.3f  last %12.3f  min %12.3f  max %12.3f  "
            "slope %s%s"
            % (name, have[0], have[-1], min(have), max(have),
               ("%+.3f/h" % sl) if sl is not None else "n/a", unit))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--bucket", type=float, default=3600.0,
                    help="seconds per stall/log bucket (default one hour)")
    a = ap.parse_args()
    out = Path(a.out)

    rows = list(csv.DictReader(open(out / "samples.csv")))
    cyc = list(csv.DictReader(open(out / "cycles.csv")))
    vd = json.loads((out / "verdict.json").read_text()) \
        if (out / "verdict.json").exists() else {}
    st = json.loads((out / "state.json").read_text()) \
        if (out / "state.json").exists() else {}
    cfg = json.loads((out / "config.json").read_text())

    print("=" * 96)
    print("F95 SOAK REPORT   %s" % out)
    print("binary %s (md5 %s)   display %s   started %s"
          % (cfg["bin"], cfg["bin_md5"][:8], cfg["display"], cfg["started_iso"]))
    print("H requested %.3f h   S %.0f s   J0 %.6f   shows %d   criteria md5 %s"
          % (cfg["hours"], cfg["sample"], cfg["J0"], len(cfg["shows"]),
             cfg["criteria_sha"]))
    print("=" * 96)

    # ---- 1. what actually ran
    ts = [t for t in col(rows, "t_rel_s") if t is not None]
    wall = max(ts) if ts else 0
    print("\n--- 1. WHAT RAN ---")
    print("wall (last sample)      : %.1f s = %.3f h" % (wall, wall / 3600.0))
    print("samples                 : %d rows (expected >= %d at S=%.0f over %.3f h)"
          % (len(rows), int(cfg["hours"] * 3600 / cfg["sample"]),
             cfg["sample"], cfg["hours"]))
    print("cycles                  : %d recorded, %d COMPLETE (%d/%d shows)"
          % (len(cyc), sum(1 for c in cyc if c["complete"] == "1"),
             len(cfg["shows"]), len(cfg["shows"])))
    print("playlist dirs / cap     : %s / %s"
          % ("+".join(cfg.get("playlist_dirs") or ["basis", "custom", "deepsky"]),
             ("%.0f s" % cfg["cap"]) if cfg.get("cap") else "none"))
    print("FLAGS                   : %s" % (vd.get("flags") or st.get("flags")
                                            or "none"))

    # ---- 2. the sample series
    print("\n--- 2. THE SAMPLE SERIES (min/max/first/last and a fitted slope) ---")
    for nm, unit, sc in [("vmrss_kb", " kB", 1.0), ("vmsize_kb", " kB", 1.0),
                         ("threads", "", 1.0), ("fds", "", 1.0),
                         ("gpu_mib", " MiB", 1.0),
                         ("scriptlog_b", " B", 1.0), ("applog_b", " B", 1.0),
                         ("scapplog_b", " B", 1.0), ("vulkanlog_b", " B", 1.0),
                         ("probe_rtt_ms", " ms", 1.0), ("loadavg1", "", 1.0)]:
        print("  " + summarise(rows, nm, unit, sc))

    rtt = [v for v in col(rows, "probe_rtt_ms") if v is not None]
    if rtt:
        srt = sorted(rtt)
        print("\n  probe round trip      : n=%d  median %.1f ms  p95 %.1f ms  "
              "max %.1f ms  (F2 bound %.0f ms)  over-bound samples: %d"
              % (len(srt), srt[len(srt) // 2], srt[int(len(srt) * 0.95)],
                 srt[-1], HUNG_S * 1000,
                 sum(1 for v in rtt if v > HUNG_S * 1000)))
    ok = [r for r in rows if r.get("probe_ok") == "1"]
    print("  probe answered        : %d of %d samples" % (len(ok), len(rows)))

    # ---- 3. per-bucket stalls WITH the lock state beside them
    print("\n--- 3. STALLS PER %.2f h, WITH THE SCREEN STATE BESIDE THEM ---"
          % (a.bucket / 3600.0))
    print("  (a lock throttles the engine to 1 Hz, F67 - so a stall count is only "
          "readable\n   next to the lock state; RECORDED, never gating)")
    print("  %-6s %-9s %-9s %-9s %-9s %-22s %-22s"
          % ("bucket", "t_from_s", "t_to_s", "detected", "very_long",
             "screensaver seen", "lockedHint seen"))
    buckets = {}
    for r in rows:
        t = fnum(r["t_rel_s"])
        if t is None:
            continue
        b = int(t // a.bucket)
        buckets.setdefault(b, []).append(r)
    prev_d = prev_v = 0
    stall_rates = []
    for b in sorted(buckets):
        rs = buckets[b]
        d = max([fnum(x["stall_detected"]) or 0 for x in rs])
        v = max([fnum(x["stall_very_long"]) or 0 for x in rs])
        span = (max(fnum(x["t_rel_s"]) for x in rs)
                - min(fnum(x["t_rel_s"]) for x in rs)) or a.bucket
        rate = (d - prev_d) / (span / 3600.0)
        stall_rates.append(rate)
        print("  %-6d %-9.0f %-9.0f %-9d %-9d %-22s %-22s"
              % (b, b * a.bucket, (b + 1) * a.bucket, int(d - prev_d),
                 int(v - prev_v),
                 ",".join(sorted({x["screensaver"] for x in rs})),
                 ",".join(sorted({x["locked_hint"] for x in rs}))))
        prev_d, prev_v = d, v
    if stall_rates:
        print("  -> `Frame stall detected` rate per hour, per bucket: %s"
              % ["%.1f" % x for x in stall_rates])

    # ---- 4. log growth
    print("\n--- 4. LOG GROWTH (Sec.5.115's price for R20's eight-launch window) ---")
    for nm, label in [("scriptlog_b", "script-YY.MM.DD.log (keepHistory=true, "
                                      "APPENDED across launches, uncapped)"),
                      ("scapplog_b", "spacecrafter.log (truncates per launch)"),
                      ("vulkanlog_b", "vulkan log"),
                      ("applog_b", "the process's own stdout+stderr capture")]:
        vs = [v for v in col(rows, nm) if v is not None]
        sl = slope_per_hour(col(rows, "t_rel_s"), col(rows, nm))
        if vs:
            print("  %-58s %10.0f -> %10.0f B   %s"
                  % (label, vs[0], vs[-1],
                     ("%+.0f B/h = %.2f MB/h" % (sl, sl / 1e6))
                     if sl is not None else "n/a"))
    comp = [c for c in cyc if c["complete"] == "1"]
    if comp:
        s0, s1 = fnum(comp[0]["scriptlog_b"]), fnum(comp[-1]["scriptlog_b"])
        n = len(comp)
        if s0 is not None and s1 is not None and n > 1:
            print("  -> per COMPLETE cycle (%d shows): %.0f B/cycle over %d "
                  "cycles" % (len(cfg["shows"]), (s1 - s0) / (n - 1), n))

    # ---- 5. the cycle table
    print("\n--- 5. THE CYCLE TABLE ---")
    print("  %-4s %-4s %-8s %-6s %-7s %-8s %-11s %-9s %-8s %-8s %s"
          % ("cyc", "cmpl", "wall_s", "shows", "t/out", "resumed", "VmRSS kB",
             "VmSize MB", "threads", "fds", "scriptlog B"))
    for c in cyc:
        print("  %-4s %-4s %-8s %-6s %-7s %-8s %-11s %-9.0f %-8s %-8s %s"
              % (c["cycle"], c["complete"], c["wall_s"], c["shows_played"],
                 c["shows_timeout"], c["pauses_resumed"], c["vmrss_kb"],
                 (fnum(c["vmsize_kb"]) or 0) / 1024.0, c["threads"], c["fds"],
                 c["scriptlog_b"]))
    walls = [fnum(c["wall_s"]) for c in comp]
    if len(walls) > 1:
        print("  -> COMPLETE-cycle wall: first %.1f s  last %.1f s  min %.1f  "
              "max %.1f  spread %.1f s  slope %s"
              % (walls[0], walls[-1], min(walls), max(walls),
                 max(walls) - min(walls),
                 ("%+.4f s/cycle" % ((walls[-1] - walls[0]) / (len(walls) - 1)))))

    # ---- 5b. the per-show table (F98: 136 shows make a cycle row too coarse)
    shp = out / "shows.csv"
    if shp.exists():
        srows = list(csv.DictReader(open(shp)))
        print("\n--- 5b. THE PER-SHOW TABLE (one row per show per cycle) ---")
        print("  rows: %d" % len(srows))
        by_show = {}
        for r in srows:
            by_show.setdefault(r["show"], []).append(r)
        capped = sorted({r["show"] for r in srows if r["outcome"] == "CAPPED"})
        touts = sorted({r["show"] for r in srows if r["outcome"] == "SHOW-TIMEOUT"})
        print("  CAPPED  (by design, the model says longer than the cap): %d "
              "show(s)" % len(capped))
        for s in capped:
            rs = by_show[s]
            print("    %-32s %2d/%2d cycle(s)  wall %s"
                  % (s, sum(1 for r in rs if r["outcome"] == "CAPPED"), len(rs),
                     sorted({r["wall_s"] for r in rs})[:6]))
        print("  SHOW-TIMEOUT (a FINDING: no `script end` within modelled + "
              "grace): %d show(s)" % len(touts))
        for s in touts:
            rs = by_show[s]
            r0 = rs[0]
            print("    %-32s %2d/%2d cycle(s)  own %s expanded %s deadline %s "
                  " wall %s" % (s, sum(1 for r in rs
                                       if r["outcome"] == "SHOW-TIMEOUT"),
                                len(rs), r0["own_s"], r0["expanded_s"],
                                r0["deadline_s"],
                                sorted({r["wall_s"] for r in rs})[:6]))
        ended = [r for r in srows if r["outcome"] == "ended"]
        print("  ENDED on their own: %d row(s) over %d distinct shows"
              % (len(ended), len({r["show"] for r in ended})))
        walls = sorted((fnum(r["wall_s"]) or 0, r["show"]) for r in srows)
        print("  slowest ten show-plays: %s"
              % ", ".join("%s %.1fs" % (s, w) for w, s in walls[-10:][::-1]))
        tot = {}
        for r in srows:
            tot.setdefault(r["cycle"], 0.0)
            tot[r["cycle"]] += fnum(r["wall_s"]) or 0.0
        print("  play wall per cycle (s): %s"
              % {k: round(v, 1) for k, v in sorted(tot.items(),
                                                   key=lambda kv: int(kv[0]))})

    # ---- 5c. the authoring instrument
    vdc = vd.get("cycles") or []
    if vdc:
        print("\n--- 5c. THE AUTHORING INSTRUMENT (body counts per path half, "
              "and the named probes) ---")
        print("  %-6s %-26s %-10s %-10s %-9s %-9s"
              % ("cycle", "where", "bodies_old", "bodies_new", "old_only",
                 "new_only"))
        for c in vdc:
            for a in ([(c.get("interlude") or {}).get("authoring")]
                      + list(c.get("authoring_snapshots") or [])):
                if not a:
                    continue
                print("  %-6s %-26s %-10s %-10s %-9s %-9s"
                      % (c["cycle"], a.get("where", "?"), a.get("bodies_old"),
                         a.get("bodies_new"), a.get("old_only"),
                         a.get("new_only")))
                for p in a.get("probes") or []:
                    print("      probe %-16s from %-18s old %-5s new %-5s  "
                          "search %-22s readout %s"
                          % (p["name"], p["from"], p["present_old"],
                             p["present_new"],
                             (p["search_reply"] or "").replace("\n", " ")[:22],
                             (p["get_object"] or "").replace("\n", " | ")[:80]))

    # ---- 6. the leak verdict
    print("\n--- 6. THE LEAK RULE (committed before the launch) ---")
    series = [int(c["vmrss_kb"]) for c in comp if c["vmrss_kb"]]
    lv = leak_verdict(series)
    print("  series (VmRSS kB at each COMPLETE cycle boundary, cycle 1 first):")
    print("    %s" % series)
    print("  VERDICT: %s" % lv["verdict"])
    print("  why    : %s" % lv["why"])
    if "steps" in lv:
        steps = lv["steps"]
        print("  steps (kB, from cycle 2): %s" % steps)
        print("  largest consecutive swing (the FLOOR): %d kB = %.2f MB"
              % (lv["floor"], lv["floor"] / 1024.0))
        print("  span cycle2->last: %d kB = %.2f MB" % (lv["span"],
                                                        lv["span"] / 1024.0))
        if lv["verdict"] == "LEAK":
            hrs = (fnum(comp[-1]["wall_s"]) or 0) * (len(steps)) / 3600.0
            tw = 0.0
            for c in comp[1:]:
                tw += fnum(c["wall_s"]) or 0
            print("  -> %.3f MB/cycle over %d steps; %.1f MB/h "
                  "(%.1f s of cycle wall covered)"
                  % (lv["per_cycle"] / 1024.0, len(steps),
                     (lv["span"] / 1024.0) / (tw / 3600.0) if tw else 0.0, tw))
        else:
            print("  -> last-vs-first STATED AGAINST THE FLOOR: %.2f MB rise "
                  "against a %.2f MB floor => %s"
                  % (lv["span"] / 1024.0, lv["floor"] / 1024.0,
                     "the rise is inside the floor and is not evidence"
                     if abs(lv["span"]) <= lv["floor"] else
                     "the rise exceeds the floor but the sign rule did not hold"))

    # ---- 6b. the finer series: RSS at every SHOW boundary (F98)
    if shp.exists():
        print("\n--- 6b. THE PER-SHOW BOUNDARY SERIES (SECONDARY; the LEAK "
              "VERDICT ABOVE IS THE ONE THAT WAS COMMITTED) ---")
        print("  A cycle over this corpus is an hour or more, so the cycle "
              "series is short.\n  The same playlist point is available 135 "
              "times per cycle instead of once:\n  the RSS after show <i> of "
              "cycle n against the RSS after show <i> of cycle\n  n+1.  That "
              "is a MEDIAN over shows rather than a single difference, and it\n"
              "  is reported as a secondary reading - it cannot overturn the "
              "committed rule.")
        per = {}
        for r in srows:
            v = fnum(r["vmrss_kb"])
            if v is not None:
                per.setdefault(r["show"], {})[int(r["cycle"])] = v
        cycles_seen = sorted({int(r["cycle"]) for r in srows})
        for i in range(len(cycles_seen) - 1):
            c0, c1 = cycles_seen[i], cycles_seen[i + 1]
            d = [per[s][c1] - per[s][c0] for s in per
                 if c0 in per[s] and c1 in per[s]]
            if not d:
                continue
            d.sort()
            pos = sum(1 for x in d if x > 0)
            print("  cycle %d -> %d : n=%3d shows  median %+9.1f kB  "
                  "mean %+9.1f kB  positive %d/%d  min %+d  max %+d"
                  % (c0, c1, len(d), d[len(d) // 2], sum(d) / len(d),
                     pos, len(d), d[0], d[-1]))
        mono = [s for s in per
                if len(per[s]) >= 3
                and all(per[s][b] > per[s][a] for a, b in
                        zip(sorted(per[s])[:-1], sorted(per[s])[1:]))]
        eligible = [s for s in per if len(per[s]) >= 3]
        print("  shows whose own RSS series is strictly increasing over every "
              "cycle: %d of %d with >= 3 cycles" % (len(mono), len(eligible)))

    # ---- 7. the Sec.5.62 dump diff
    print("\n--- 7. THE DUMP DIFF AT THE PINNED CLOCK J0 = %.6f (Sec.5.62) ---" % J0)
    print("  reference = cycle 2 (cycle 1 carries first-touch state).  The")
    print("  expected-constant set was NAMED IN prediction.txt BEFORE cycle 2 ran.")
    files = sorted(out.glob("cycle_*.json"))
    good = [f for f in files if f.stat().st_size > 0]
    print("  dumps written: %d (%d non-empty), sizes %s"
          % (len(files), len(good),
             sorted({f.stat().st_size for f in good})))
    if len(good) >= 3:
        ref = dump_constant_view(good[1])
        print("  reference %s: %d bodies, header %s"
              % (good[1].name, len(ref["bodies"]), ref["header"]))
        total = 0
        worst = []
        for f in good[2:]:
            d = dump_diff(ref, dump_constant_view(f))
            total += len(d)
            if d:
                worst.append((f.name, d))
        print("  compared %d later dumps against it" % len(good[2:]))
        print("  TOTAL DIFFERENCES INSIDE THE EXPECTED-CONSTANT SET: %d" % total)
        if worst:
            for nm, d in worst[:6]:
                print("    %s: %d diff(s)" % (nm, len(d)))
                for x in d[:6]:
                    print("       %-34s a=%s  b=%s"
                          % (x["where"], str(x["a"])[:60], str(x["b"])[:60]))
        else:
            print("    -> every later dump is byte-identical to cycle 2 across "
                  "the whole named set.")
    else:
        print("  fewer than three dumps - the diff needs a reference and at "
              "least one comparison")

    # ---- 8. the quit
    print("\n--- 8. THE QUIT (F3's three numbers) ---")
    q = vd.get("quit") or {}
    print("  exit code       : %s" % q.get("exit_code"))
    print("  wall to exit    : %s s   (bound %.0f s)"
          % (q.get("wall_to_exit_s"), HUNG_S))
    print("  had to be killed: %s" % q.get("killed"))
    print("  teardown faults : %s" % (q.get("teardown_faults") or "none"))
    print("  stalls at quit  : detected %s / very long %s"
          % (q.get("stall_detected_at_quit"), q.get("stall_very_long_at_quit")))
    print("  /proc after     : %s" % (q.get("proc_comm_hits_after") or "clear"))

    # ---- 9. the boundary
    print("\n--- 9. THE BOUNDARY: real HOME md5 in == out ---")
    fi = json.loads((out / "frozen_in.json").read_text())
    fo = json.loads((out / "frozen_out.json").read_text()) \
        if (out / "frozen_out.json").exists() else {}
    moved = [k for k in fi if fi[k] != fo.get(k)]
    print("  %d files recorded in, %d out; MOVED: %s"
          % (len(fi), len(fo), moved or "none"))
    print("  aggregate digest in %s  out %s  (per-file md5s sorted in C order "
          "and hashed - never a concatenation, which is locale-dependent)"
          % ((vd.get("frozen_digest_in") or "?")[:8],
             (vd.get("frozen_digest_out") or "?")[:8]))
    fmi = json.loads((out / "farm_sts_in.json").read_text()) \
        if (out / "farm_sts_in.json").exists() else {}
    fmo = json.loads((out / "farm_sts_out.json").read_text()) \
        if (out / "farm_sts_out.json").exists() else {}
    fmoved = [k for k in fmi if fmi[k] != fmo.get(k)]
    print("  the FARM's own .sts copies: %d in, %d out; rewritten by "
          "`ScriptAnnotator::flush`: %s"
          % (len(fmi), len(fmo), fmoved or "none"))
    print("\n" + "=" * 96)
    return 0


if __name__ == "__main__":
    sys.exit(main())
