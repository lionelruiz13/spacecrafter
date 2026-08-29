#!/usr/bin/env python3
"""F51 (A) — the disc photometry metric, and the frame-pair discriminators.

THE METRIC IS NOT NEW: it is §11.164(c)'s, restated here as executable code so
that every number this task produces is on the same scale as the ones F48
committed.  Its definition, in full, so a reader never has to guess:

    L  = PIL `Image.open(p).convert("L")`      (ITU-R 601-2 luma, integer)
    Bl = the same image through `ImageFilter.GaussianBlur(4)`
    M  = { (x, y) : (x - 1023.5)^2 + (y - 1023.5)^2 < 900^2  AND  L > 8 }
    disc_mean = mean(L[M])
    hf_mean   = mean(|L - Bl|[M])       hf_sd = std((L - Bl)[M])   n = |M|

`--calib` re-derives F48's four committed numbers from the two committed
frames; it MATCHES to the last printed digit on both, which is what licenses
comparing anything measured here against §11.164(c):

    JULY  (artifacts/b3_ladder/R6_moon_terrain_control/terrain_base_zoom.png)
          165.258 / 6.644 / 9.513 / 2544661   vs committed 165.258 / 6.644 / 9.514
    TODAY (artifacts/f48/ladder_current/terrain_base_zoom.png)
           61.431 / 2.464 / 4.230 / 2535950   vs committed  61.431 / 2.464 / 4.230

(The mask centre is the FRAME centre (w-1)/2, not w/2 — recovered from F48's
own committed pixel count 2544661, which w/2 does not reproduce (2544530).)

`--pair A B` answers the question the luminance SERIES cannot: whether the dim
frame is the bright frame put through a tone map (the albedo texture's content
is on the GPU and the shading changed) or a different image altogether (the
content is not there).  Two statistics, both over the intersection of the two
masks:

  * `r_L`   — Pearson r of the two frames' L.  Weak on its own: the large-scale
              shading structure is common to both hypotheses and dominates it.
  * `r_hf`  — Pearson r of the two frames' HIGH-FREQUENCY parts (L - Bl).  This
              is the discriminating one.  A smooth tone map g sends
              hf -> g'(L)*hf, so it PRESERVES the fine pattern up to a local
              scale factor; an image whose albedo content never arrived has no
              reason to carry the same fine pattern at all.
  * `r2_pointwise` — how well a per-bin median map g (256 bins on the reference)
              explains the target: 1 - var(target - g(ref))/var(target).

usage: f51_disc.py --calib
       f51_disc.py --metrics <png> [<png> ...]
       f51_disc.py --pair <reference.png> <target.png> [--out <json>]
"""
import json, sys
from pathlib import Path
import numpy as np
from PIL import Image, ImageFilter

MASK_R = 900.0
BLUR = 4
LMIN = 8


def load(png):
    im = Image.open(png).convert("L")
    L = np.asarray(im, dtype=np.float64)
    Bl = np.asarray(im.filter(ImageFilter.GaussianBlur(BLUR)), dtype=np.float64)
    return L, Bl


def disc_mask(L):
    h, w = L.shape
    yy, xx = np.mgrid[0:h, 0:w]
    r2 = (xx - (w - 1) / 2.0) ** 2 + (yy - (h - 1) / 2.0) ** 2
    return (r2 < MASK_R * MASK_R) & (L > LMIN)


def metrics(png):
    L, Bl = load(png)
    m = disc_mask(L)
    d = (L - Bl)[m]
    return {"png": str(png),
            "disc_mean": round(float(L[m].mean()), 3),
            "hf_mean": round(float(np.abs(d).mean()), 3),
            "hf_sd": round(float(d.std()), 3),
            "n": int(m.sum()),
            "disc_sd": round(float(L[m].std()), 3),
            "frame_mean": round(float(L.mean()), 3),
            "frame_lit_px_gt8": int((L > LMIN).sum())}


def pair(ref_png, tgt_png):
    """Reference = the frame we ask 'is the target a tone map of this?'."""
    Lr, Br = load(ref_png)
    Lt, Bt = load(tgt_png)
    if Lr.shape != Lt.shape:
        return {"error": f"shape {Lr.shape} vs {Lt.shape}"}
    m = disc_mask(Lr) & disc_mask(Lt)
    a, b = Lr[m], Lt[m]
    ha, hb = (Lr - Br)[m], (Lt - Bt)[m]

    def r(x, y):
        sx, sy = x.std(), y.std()
        if sx == 0 or sy == 0:
            return None
        return round(float(((x - x.mean()) * (y - y.mean())).mean() / (sx * sy)), 4)

    # per-bin median map g on the reference's integer levels
    g = np.full(256, np.nan)
    ai = a.astype(np.int32)
    for v in np.unique(ai):
        g[v] = np.median(b[ai == v])
    pred = g[ai]
    good = ~np.isnan(pred)
    r2 = 1.0 - float(((b[good] - pred[good]) ** 2).mean()) / float(b[good].var())
    # local slope of the hf relation (least squares through origin)
    slope = float((ha * hb).sum() / (ha * ha).sum()) if (ha * ha).sum() else None
    return {"ref": str(ref_png), "target": str(tgt_png),
            "n_common": int(m.sum()),
            "ref_disc_mean": round(float(a.mean()), 3),
            "tgt_disc_mean": round(float(b.mean()), 3),
            "ratio_disc_mean": round(float(b.mean() / a.mean()), 4) if a.mean() else None,
            "r_L": r(a, b),
            "r_hf": r(ha, hb),
            "hf_slope_through_origin": round(slope, 4) if slope is not None else None,
            "r2_pointwise_median_map": round(r2, 4),
            "ref_hf_mean": round(float(np.abs(ha).mean()), 3),
            "tgt_hf_mean": round(float(np.abs(hb).mean()), 3)}


CALIB = [("JULY_F1P2_R6", "artifacts/b3_ladder/R6_moon_terrain_control/terrain_base_zoom.png",
          {"disc_mean": 165.258, "hf_mean": 6.644, "hf_sd": 9.514, "n": 2544661}),
         ("TODAY_F48_current", "artifacts/f48/ladder_current/terrain_base_zoom.png",
          {"disc_mean": 61.431, "hf_mean": 2.464, "hf_sd": 4.230, "n": 2535950})]


def main():
    here = Path(__file__).resolve().parent
    argv = sys.argv[1:]
    if not argv:
        print(__doc__); return 2
    if argv[0] == "--calib":
        out = {"method": "L=PIL convert('L'); Bl=GaussianBlur(4); "
                         "mask r<900 about ((w-1)/2,(h-1)/2) and L>8",
               "arms": {}}
        bad = 0
        for tag, rel, committed in CALIB:
            got = metrics(here / rel)
            delta = {k: round(got[k] - v, 4) for k, v in committed.items()}
            out["arms"][tag] = {"measured": got, "committed_11_164": committed,
                                "delta": delta}
            for k, v in delta.items():
                if abs(v) > (1.0 if k == "n" else 0.002):
                    bad += 1
                    print(f"MISMATCH {tag}.{k}: {got[k]} vs {committed[k]}")
        print(json.dumps(out, indent=1))
        return 1 if bad else 0
    if argv[0] == "--metrics":
        print(json.dumps([metrics(p) for p in argv[1:]], indent=1)); return 0
    if argv[0] == "--pair":
        res = pair(argv[1], argv[2])
        if "--out" in argv:
            Path(argv[argv.index("--out") + 1]).write_text(json.dumps(res, indent=1))
        print(json.dumps(res, indent=1)); return 0
    print(__doc__); return 2


if __name__ == "__main__":
    sys.exit(main())
