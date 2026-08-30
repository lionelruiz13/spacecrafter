#!/usr/bin/env python3
"""F55 — the series, the event join and the verdict, computed from a run's
committed artifacts alone (so the verdict is recomputable without the launch).

METRIC AUTHORITY is `f51_disc.py`, unchanged; the only F55 addition is that the
mask radius scales with the frame width (900 px on a 2048 app screenshot, 450
on a 1024 X-side window grab), and that a SECOND mean is taken over the bare
geometric mask.  That second one exists because F51's mask carries `L > 8`, so
an all-black disc — the thing "the texture never arrived" would actually draw —
gives an EMPTY mask and no `disc_mean` at all.  `disc_mean_geom` is the
quantity in which that state is visible.

usage: f55_series.py <run-dir> [--out <json>]
"""
import json, re, sys
from pathlib import Path
import numpy as np
from PIL import Image, ImageFilter

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import f51_disc as D

TICKS_RE = re.compile(r"^(\d{12}): ")
SHOT_RE = re.compile(r"Execute_command body action screenshot filename .*?/(e\d{3})\.png")
LMIN, BLUR, MASK_R_AT_2048 = 8, 4, 900.0


def metrics(png):
    """f51_disc's numbers, plus the two F55 needs (geometric mean, md5)."""
    import hashlib
    im = Image.open(png).convert("L")
    L = np.asarray(im, dtype=np.float64)
    Bl = np.asarray(im.filter(ImageFilter.GaussianBlur(BLUR)), dtype=np.float64)
    h, w = L.shape
    r = MASK_R_AT_2048 * w / 2048.0
    yy, xx = np.mgrid[0:h, 0:w]
    geom = ((xx - (w - 1) / 2.0) ** 2 + (yy - (h - 1) / 2.0) ** 2) < r * r
    lit = geom & (L > LMIN)
    d = (L - Bl)[lit]
    out = {"f": Path(png).name, "w": w, "mask_r": round(r, 1),
           "n_geom": int(geom.sum()), "n_lit": int(lit.sum()),
           "fill": round(float(lit.sum()) / float(geom.sum()), 4),
           "disc_mean_geom": round(float(L[geom].mean()), 3),
           "disc_sd_geom": round(float(L[geom].std()), 3),
           "frame_mean": round(float(L.mean()), 3),
           "md5": hashlib.md5(Path(png).read_bytes()).hexdigest()[:8]}
    if lit.sum():
        out.update({"disc_mean_lit": round(float(L[lit].mean()), 3),
                    "disc_sd_lit": round(float(L[lit].std()), 3),
                    "hf_mean": round(float(np.abs(d).mean()), 3)})
    else:
        out.update({"disc_mean_lit": None, "disc_sd_lit": None, "hf_mean": None})
    return out


def main():
    run = Path(sys.argv[1]).resolve()
    R = json.loads((run / "f55_run.json").read_text())
    lines = json.loads((run / "f55_loglines.json").read_text())
    tl = R["t_launch"]
    t0 = R["anchor"]["t0_min"]          # wall = ticks + t0 ; see f55_predictions

    def tk(s):
        m = TICKS_RE.match(s)
        return int(m.group(1)) / 1000.0 if m else None

    # ---- events, on the app's own clock, expressed as t - t_launch ----------
    events = []
    shot_ticks = {}
    for r in lines:
        t = tk(r["line"])
        rel = round((t + t0 - tl), 3) if t is not None else None
        m = SHOT_RE.search(r["line"])
        if m and t is not None:
            shot_ticks[m.group(1)] = rel
        txt = r["line"]
        if any(k in txt for k in ("uninitialized texture", "is ready for use",
                                  "Loading big", "Created image support",
                                  "End of loading SC", "ScriptMgr: load",
                                  "ScriptMgr: script end", "blocking texture",
                                  "Can't upload")):
            events.append({"t_rel": rel, "t_seen_rel": round(r["t"] - tl, 3),
                           "file": r["file"], "line": txt})
    moon_ev = [e for e in events if "bodies/moon" in e["line"]]

    # ---- the two series -----------------------------------------------------
    series = {}
    for tag, sub in (("sts", "sts"), ("x", "x")):
        rows = []
        for p in sorted((run / sub).glob("*.png")):
            m = metrics(p)
            m["mtime_rel"] = round(p.stat().st_mtime - tl, 3)
            m["t_cmd_rel"] = shot_ticks.get(p.stem)
            m["t_rel"] = m["t_cmd_rel"] if m["t_cmd_rel"] is not None else m["mtime_rel"]
            rows.append(m)
        rows.sort(key=lambda r: r["t_rel"])
        series[tag] = rows

    # ---- the committed criteria --------------------------------------------
    def evaluate(rows, label):
        est = [r for r in rows if r["fill"] >= 0.90]
        pre = [r for r in rows if r["fill"] < 0.90]
        v = {"label": label, "n": len(rows), "n_established": len(est),
             "first_established_t_rel": est[0]["t_rel"] if est else None,
             "first_established": est[0] if est else None,
             "pre_established_n": len(pre),
             "pre_established_max_disc_mean_geom":
                 round(max((r["disc_mean_geom"] for r in pre), default=0.0), 3),
             "dark_disc_samples": [r["f"] for r in rows
                                   if r["fill"] < 0.90 and r["disc_mean_geom"] < 2.0]}
        steps = []
        for a, b in zip(est, est[1:]):
            if a["disc_mean_lit"] is None or b["disc_mean_lit"] is None:
                continue
            d = b["disc_mean_lit"] - a["disc_mean_lit"]
            if d > 5.0:
                steps.append({"from": a["f"], "to": b["f"],
                              "t_rel": b["t_rel"], "delta": round(d, 3),
                              "from_val": a["disc_mean_lit"], "to_val": b["disc_mean_lit"]})
        v["STEPS_up_gt5"] = steps
        vals = [r["disc_mean_lit"] for r in est if r["disc_mean_lit"] is not None]
        if vals:
            v["est_min"], v["est_max"] = round(min(vals), 3), round(max(vals), 3)
            v["est_range"] = round(max(vals) - min(vals), 3)
            v["est_median"] = round(float(np.median(vals)), 3)
            v["first_vs_median_pct"] = round(100.0 * (vals[0] - v["est_median"])
                                             / v["est_median"], 2)
            tail = [r["disc_mean_lit"] for r in est
                    if r["t_rel"] >= est[-1]["t_rel"] - 30.0 and r["disc_mean_lit"] is not None]
            v["last30s_range"] = round(max(tail) - min(tail), 3) if tail else None
        return v

    out = {"run": str(run), "t_launch": tl, "anchor_t0": t0,
           "marks_rel": {k: round(v - tl, 3) for k, v in R["marks"].items()},
           "t_window_rel": round(R["t_window"] - tl, 3),
           "p1_obs_radius_km": R.get("p1_obs_radius_km"),
           "p2_obs_radius_km": R.get("p2_obs_radius_km"),
           "moon_texture_events": moon_ev,
           "events": events,
           "verdict_sts": evaluate(series["sts"], "channel A (app 2048 readback)"),
           "verdict_x": evaluate(series["x"], "channel B (X-side 1024 window grab)")}
    for name in ("p1_app.png", "p2_app.png"):
        p = run / name
        if p.exists():
            out[name] = metrics(p)
    out["f51_committed"] = {"disc_mean": 61.431, "hf_mean": 2.464,
                            "frame_md5": "0c7de389",
                            "july_disc_mean": 165.258}
    (run / "f55_series.json").write_text(json.dumps(
        {**out, "series": series}, indent=1))
    print(json.dumps(out, indent=1)[:12000])
    return 0


if __name__ == "__main__":
    sys.exit(main())
