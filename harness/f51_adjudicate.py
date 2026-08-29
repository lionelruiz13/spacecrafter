#!/usr/bin/env python3
"""F51 (A) — adjudication of the dwell against the predictions committed in
`artifacts/f51/f51_predictions.json` (commit `0354ba0`, before the launch).

Reads only committed inputs: the dwell result JSON + its applog, the two frames
§11.164(c) measured, and this run's own frames.  Writes
`artifacts/f51/f51_adjudication.json`.

usage: f51_adjudicate.py [<outjson>]
"""
import gzip, json, re, sys
from pathlib import Path
import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import f51_disc as D

A = HERE / "artifacts"
JULY = A / "b3_ladder/R6_moon_terrain_control/terrain_base_zoom.png"
# TODAY, new path: F48's committed frame.  This run's own `s000.png` is
# BYTE-IDENTICAL to it (md5 0c7de389, recorded per sample in f51_dwell.json for
# all 72 samples), so the file is not duplicated into this task's artifacts -
# one home for one set of bytes.
F48_TODAY = A / "f48/ladder_current/terrain_base_zoom.png"
TODAY_NEW = F48_TODAY
DWELL = A / "f51/dwell"


def read_text(p):
    p = Path(p)
    if p.exists():
        return p.read_text(errors="replace")
    return gzip.open(str(p) + ".gz", "rt", errors="replace").read()
STEP = 5.0          # committed: |Delta| > 5.0 disc_mean units is a step
FLAT = 1.0          # committed: max-min < 1.0 is flat


def strip(s):
    return re.sub(r"\x1b\[[0-9;]*m", "", s)


def series_verdict(res):
    s = res["samples"]
    v = [x["disc_mean"] for x in s]
    diffs = [abs(v[i + 1] - v[i]) for i in range(len(v) - 1)]
    return {"n": len(s),
            "cadence_s": res["cadence_s"],
            "span_s": round(s[-1]["t_cmd"] - s[0]["t_cmd"], 1),
            "disc_mean_first": v[0], "disc_mean_last": v[-1],
            "disc_mean_min": min(v), "disc_mean_max": max(v),
            "range": round(max(v) - min(v), 4),
            "max_abs_successive_delta": round(max(diffs), 4) if diffs else None,
            "hf_mean_min": min(x["hf_mean"] for x in s),
            "hf_mean_max": max(x["hf_mean"] for x in s),
            "distinct_png_md5": sorted({x["md5"] for x in s}),
            "flat_by_committed_criterion": (max(v) - min(v)) < FLAT,
            "step_by_committed_criterion": any(d > STEP for d in diffs)}


def liveness(res):
    """THE OTHER CONTROL A FLAT SERIES NEEDS: 72 byte-identical frames are also
    what a STALLED renderer (or a cached screenshot writer) would produce.  The
    body's own `evalCount` in the dumps that bracket the dwell says how many
    times the Moon was evaluated between them."""
    def bodies(name):
        out = {}
        for line in read_text(DWELL / name).split("\n"):
            line = line.strip().rstrip(",")
            if not line:
                continue
            try:
                o = json.loads(line)
            except json.JSONDecodeError:
                continue
            if o.get("type") == "body":
                out[o["name"]] = o
        return out
    a, b = bodies("dwell_dump_start.json"), bodies("dwell_dump_end.json")
    ma, mb = a["Moon"]["new"], b["Moon"]["new"]
    diff = [k for k in sorted(set(ma) | set(mb))
            if json.dumps(ma.get(k), sort_keys=True) != json.dumps(mb.get(k), sort_keys=True)]
    n = mb.get("evalCount", 0) - ma.get("evalCount", 0)
    lo = res["t_dwell_end"] - res["t_dwell_start"]
    hi = lo + 2.5      # the two dump commands sit just outside the loop
    return {"moon_new_half_fields_differing_start_to_end": diff,
            "moon_old_half_identical": json.dumps(a["Moon"].get("old"), sort_keys=True)
                                       == json.dumps(b["Moon"].get("old"), sort_keys=True),
            "evalCount_start": ma.get("evalCount"), "evalCount_end": mb.get("evalCount"),
            "evalCount_delta": n,
            "interval_bracket_s": [round(lo, 2), round(hi, 2)],
            "evals_per_s_bracket": [round(n / hi, 1), round(n / lo, 1)],
            "note": "compare 11.159(k7)'s discharged H1: cadence = the config cap "
                    "(maximum_fps = 144) on this stack"}


def applog_verdict(path, res):
    lines = [strip(l) for l in read_text(path).split("\n")]
    ev = [(i + 1, l) for i, l in enumerate(lines) if "creating uninitialized texture" in l]
    shots = [(i + 1, l) for i, l in enumerate(lines)
             if "get tcp : body action screenshot" in l]
    tcp = [i + 1 for i, l in enumerate(lines) if "get tcp" in l]
    dwell_shots = [s for s in shots if "/frames/s" in s[1]]
    lo, hi = dwell_shots[0][0], dwell_shots[-1][0]
    return {"applog_lines": len(lines),
            "uninitialized_texture_events_total": len(ev),
            "first_tcp_command_line": tcp[0] if tcp else None,
            "events_before_first_command": sum(1 for e in ev if e[0] < tcp[0]),
            "events_after_first_command": [{"line": e[0], "text": e[1][:120]}
                                           for e in ev if e[0] > tcp[0]],
            "moon_body_texture_events": [{"line": e[0], "text": e[1][:120]} for e in ev
                                         if "bodies/moon" in e[1]],
            "cant_upload_lines": sum(1 for l in lines if "Can't upload" in l),
            "dwell_window_lines": [lo, hi],
            "n_dwell_screenshot_echoes": len(dwell_shots),
            "texture_events_inside_dwell_window": [{"line": e[0], "text": e[1][:120]}
                                                   for e in ev if lo < e[0] < hi],
            "note": "the applog carries no timestamps; the interleaving is exact "
                    "by the app's own command echo (one per named sample)"}


def blobs(ref_png, tgt_png):
    """The dark speckle class visible in the 2026-08-29 frame and absent in the
    2026-07-25 one, counted rather than described, plus the correlation of the
    fine pattern OUTSIDE it (the question P2's diluted r_hf raises)."""
    Lr, Br = D.load(ref_png)
    Lt, Bt = D.load(tgt_png)
    disc = D.disc_mask(Lr) | D.disc_mask(Lt)
    out = {"ref": str(ref_png), "target": str(tgt_png)}
    for t in (8, 16, 32, 64):
        out[f"ref_px_below_{t}"] = int(((Lr < t) & disc).sum())
        out[f"tgt_px_below_{t}"] = int(((Lt < t) & disc).sum())
    blob = disc & (Lt < 32) & (Lr > 100)
    out["blob_px_tgt_lt32_ref_gt100"] = int(blob.sum())
    out["blob_fraction_of_disc"] = round(float(blob.sum()) / float(disc.sum()), 5)
    keep = disc & ~blob & (Lr > 60) & (Lt > 32)
    ha, hb = (Lr - Br)[keep], (Lt - Bt)[keep]
    r = float(((ha - ha.mean()) * (hb - hb.mean())).mean() / (ha.std() * hb.std()))
    a, b = Lr[keep], Lt[keep]
    rl = float(((a - a.mean()) * (b - b.mean())).mean() / (a.std() * b.std()))
    out["n_outside_blobs"] = int(keep.sum())
    out["r_hf_outside_blobs"] = round(r, 4)
    out["r_L_outside_blobs"] = round(rl, 4)
    # percentiles, the shape of the tone change
    qs = [1, 5, 25, 50, 75, 95, 99]
    out["ref_percentiles"] = [round(float(np.percentile(Lr[disc], q)), 1) for q in qs]
    out["tgt_percentiles"] = [round(float(np.percentile(Lt[disc], q)), 1) for q in qs]
    out["percentile_qs"] = qs
    return out


def registration(ref_png, tgt_png, shifts=(0, 4, 8, 16, 32, 64, 128)):
    """THE CONTROL THAT MAKES r_hf READABLE.  A correlation of 0.62 between two
    band-passed frames means nothing until one knows what an UNREGISTERED pair
    of the same two frames scores.  Shifting the target by a few pixels is that
    control: if r collapses, the 0.62 is the same surface features in the same
    places; if it does not, the statistic was measuring texture statistics
    rather than registration."""
    Lr, Br = D.load(ref_png)
    Lt, Bt = D.load(tgt_png)
    m = D.disc_mask(Lr) & D.disc_mask(Lt)
    ha, hb = Lr - Br, Lt - Bt
    lo, hi = 300, 1748

    def corr(dx, dy):
        x = ha[lo:hi, lo:hi]
        y = hb[lo + dy:hi + dy, lo + dx:hi + dx]
        M = m[lo:hi, lo:hi] & m[lo + dy:hi + dy, lo + dx:hi + dx]
        x, y = x[M], y[M]
        return round(float(((x - x.mean()) * (y - y.mean())).mean() / (x.std() * y.std())), 4)

    return {"ref": str(ref_png), "target": str(tgt_png),
            "r_hf_shift_x": {str(d): corr(d, 0) for d in shifts},
            "r_hf_shift_y": {str(d): corr(0, d) for d in shifts}}


def chromaticity(png):
    """Alignment-free, and specific to the COLOUR map rather than to relief: an
    uninitialized image has no reason to carry the source texture's tint."""
    a = np.asarray(Image.open(png).convert("RGB"), dtype=np.float64)
    L, _ = D.load(png)
    m = D.disc_mask(L)
    r, g, b = a[:, :, 0][m].mean(), a[:, :, 1][m].mean(), a[:, :, 2][m].mean()
    return {"png": "/".join(Path(png).parts[-2:]), "R": round(r, 3), "G": round(g, 3), "B": round(b, 3),
            "R_over_B": round(r / b, 4), "G_over_B": round(g / b, 4), "n": int(m.sum())}


def main():
    res = json.loads((DWELL / "f51_dwell.json").read_text())
    legs = {l["tag"]: l for l in res["legs"]}
    out = {
        "task": "F51 (A) adjudication",
        "predictions": "artifacts/f51/f51_predictions.json (commit 0354ba0, pre-run)",
        "binary_md5": res["binary_md5"],
        "P0_scene_reproduction": {
            "sample0": {k: res["samples"][0][k] for k in
                        ("disc_mean", "hf_mean", "hf_sd", "n", "md5")},
            "f48_committed_frame_md5": D.__dict__ and __import__("hashlib").md5(
                F48_TODAY.read_bytes()).hexdigest(),
            "predicted": "disc_mean 61.431 +- 0.10, hf_mean 2.464 +- 0.02, n 2535950 +- 2000",
            "verdict": None},
        "P1_series": series_verdict(res),
        "P1_applog": applog_verdict(DWELL / "dwell.applog", res),
        "P1_liveness": liveness(res),
        "P2_pair_july_vs_today": D.pair(JULY, TODAY_NEW),
        "P2_blobs": blobs(JULY, TODAY_NEW),
        "P3_old_path": {
            "legs": {k: {kk: legs[k][kk] for kk in
                         ("disc_mean", "hf_mean", "hf_sd", "n", "frame_lit_px_gt8", "md5")}
                     for k in legs},
            "positive_control_min_lit_px": 100000,
            "pair_july_vs_oldpath": D.pair(JULY, DWELL / "frames/old_a.png"),
            "pair_newpath_vs_oldpath": D.pair(TODAY_NEW, DWELL / "frames/old_a.png")},
        "P2_registration_control": registration(JULY, TODAY_NEW),
        "P3_registration_control_oldpath": registration(JULY, DWELL / "frames/old_a.png"),
        "chromaticity": [chromaticity(p) for p in
                         (JULY, TODAY_NEW, DWELL / "frames/old_a.png",
                          DWELL / "frames/old_wide.png")],
    }
    s0 = res["samples"][0]
    out["P0_scene_reproduction"]["verdict"] = (
        "BIT-EXACT: sample 0 is byte-identical to F48's committed frame"
        if s0["md5"] == out["P0_scene_reproduction"]["f48_committed_frame_md5"]
        else "within tolerance" if abs(s0["disc_mean"] - 61.431) <= 0.10
        else "FAILED")
    p = Path(sys.argv[1]) if len(sys.argv) > 1 else A / "f51/f51_adjudication.json"
    p.write_text(json.dumps(out, indent=1))
    print(json.dumps(out, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
