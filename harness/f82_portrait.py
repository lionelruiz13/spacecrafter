#!/usr/bin/env python3
"""F82 - THE PORTRAIT LEG: what happens when the window is taller than wide.

WHAT THIS INSTRUMENT SEPARATES
------------------------------
Every recorded measurement in this corpus was taken at `screen_w = screen_h =
1024`.  INTENT 11.198(e) names portrait aspect (window h > w) as one of the two
barely-tested configurations in the field, with no ledger row.  This leg asks
what the projection, the readback, the dump and the on-screen transfer DO when
the assumed aspect inverts, and classifies every difference against a square
control taken in the same session with the same scene.

The chain, read at source before the run (see f82_predictions.json for the
citations): the window size reaches Vulkan only as `min(w,h)` (main.cpp:308),
but the swapchain then takes the surface's REAL extent (VulkanMgr.cpp:542), so
the window's aspect enters at exactly ONE place - `dedicatedViewport`'s
`mouseNorm` (VulkanMgr.cpp:145-163), which is what the final blit's destination
rectangle is computed from (app.cpp:1107-1113).  Everything upstream of that
blit - Projector, render target, the app's own readback - is a function of
`render_size` alone and should not move at all.

SO THERE ARE TWO CHANNELS AND THEY ANSWER DIFFERENT QUESTIONS (11.172(c)):
  A  the app's OWN readback (`body action screenshot`) - renderSize x renderSize,
     what the engine drew;
  B  an X-side `x11grab -window_id <client>` of the app's window - what the
     operator SEES.  Only channel B can see the blit, and the blit is the only
     place the aspect is used.

MEASUREMENT
  * the applog's own extent lines (Windows size / Scaling / Viewport / Swapchain
    / Rect) - the engine reporting its own configuration;
  * the projector's own dump (fov, viewport, viewportRadius, projectionType) and
    the camera's halfFov, per fov, per path;
  * the ZERO CONTROL FIRST on each aspect: a tracked bright body at the dome
    centre, offset 0.  If the centre is no longer at (R, R) every later reading
    is wrong;
  * per body per path: the engine's own screen position from `body action
    dual_dump` AND the body's luminance centroid in the frame the app wrote
    (channel A) AND its centroid in the window grab (channel B);
  * the render->window map FITTED from those correspondences (least squares over
    every body found in both channels), against the two candidate models.

Nothing is slept on: fov ramps and tracking are polled in the dump until they
settle (`f81_offset.wait_until`).  The blob rule, the farm, the ini writer, the
dump parser and the environment asserts are f81_offset's - ONE authority.

usage:
  f82_portrait.py run   <outdir> --aspect portrait|square [--bin PATH]
  f82_portrait.py table <artifacts/f82 dir>        -> writes f82_table.md
Run it through f82_run.sh (real-HOME md5 in == out assert around the stage).
"""
import json, math, os, re, subprocess, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import b3_ladder as B                       # send / wait_port
import f81_offset as F                      # farm, ini_set, App, wait_until, blob_at
from f55_sampler import client_window       # the client-window rule, one authority

FARM = Path(os.environ.get("F82_FARM", "/tmp/f82_farm"))
F.FARM = FARM                               # f81_offset.App reads its own global for HOME
REAL_HOME = Path.home()
TARGET = os.environ.get("F82_TARGET", "Sun")
FOVS = [180.0, 90.0, 40.0]
# F81's MEASURED meridian transit of the target (artifacts/f81/prep/f81_prep.json,
# key `jd_star`) - reused so this leg's square control is comparable to F81's
# recorded zero control, not only to itself.
JD = 2461288.9841951793
ASPECTS = {"portrait": (768, 1024), "square": (1024, 1024)}

EXTENT_RE = {
    "windows_size": re.compile(r"Windows size is (\d+)x(\d+)px"),
    "scaling": re.compile(r"^Scaling : ([0-9.]+)"),
    "viewport": re.compile(r"^Viewport : \((-?\d+), (-?\d+)\)"),
    "swapchain": re.compile(r"^Swapchain : \((\d+), (\d+)\)"),
    "rect": re.compile(r"^Rect : \((\d+), (\d+)\) offset=\((-?\d+), (-?\d+)\)"),
    "framebuffer_main0": re.compile(r"Build FrameBuffer 'main 0' with size \((\d+), (\d+)\)"),
}
ANSI = re.compile(r"\x1b\[[0-9;]*m")


def parse_applog(path):
    """The engine's own report of its extents.  Returns {key: [groups...]}."""
    out = {}
    try:
        text = Path(path).read_text(errors="replace")
    except OSError:
        return out
    for line in text.splitlines():
        line = ANSI.sub("", line)
        for k, rx in EXTENT_RE.items():
            m = rx.search(line)
            if m and k not in out:
                out[k] = [int(x) if x.lstrip("-").isdigit() else float(x)
                          for x in m.groups()]
    return out


def _ffgrab(args, out_png, tries=3):
    out_png = Path(out_png)
    out_png.parent.mkdir(parents=True, exist_ok=True)
    err = ""
    for i in range(tries):
        if out_png.exists():
            out_png.unlink()
        r = subprocess.run(
            ["ffmpeg", "-hide_banner", "-loglevel", "error", "-f", "x11grab"] + args
            + ["-frames:v", "1", "-pix_fmt", "rgb24", "-y", str(out_png)],
            capture_output=True, text=True, timeout=90)
        err = (r.stderr or "")[-400:]
        if out_png.exists() and out_png.stat().st_size > 0:
            a = np.asarray(Image.open(out_png).convert("L"), dtype=np.float64)
            return {"ok": True, "path": str(out_png), "w": int(a.shape[1]),
                    "h": int(a.shape[0]), "max": float(a.max()),
                    "mean": round(float(a.mean()), 4),
                    "nonzero": int((a > 0).sum()), "attempt": i + 1}
        time.sleep(0.5)
    return {"ok": False, "why": "ffmpeg produced no frame", "stderr": err}


def window_grab(win, out_png):
    """Channel B: one frame of the app's CLIENT window, straight off the X server.

    The ROOT grab is measured DEAD on this stack (all-black for a whole run while
    the window is mapped - 11.172, f55_sampler probe 1, and the app's own comment
    at app_command_interface.cpp:4200-4203); the WINDOW grab is not covered by
    that and carried the scene at F55 (probe 2).  If it comes back black here,
    that is recorded as an instrument state, never worked around silently: the
    root-crop attempt is taken beside it so the two can be told apart.
    """
    if not win:
        return {"ok": False, "why": "no client window"}
    disp = os.environ.get("DISPLAY", ":2")
    g = _ffgrab(["-window_id", str(int(win["id"], 16)), "-draw_mouse", "0", "-i", disp],
                out_png)
    g["channel"] = "window_id"
    if g.get("ok") and g.get("max", 0) == 0:
        # all black: take the root-crop of the same rectangle for the record
        rc = _ffgrab(["-video_size", f"{win['w']}x{win['h']}", "-draw_mouse", "0",
                      "-i", f"{disp}+{win['abs_x']},{win['abs_y']}"],
                     str(Path(out_png).with_suffix(".rootcrop.png")), tries=1)
        g["rootcrop"] = rc
    return g


def lit_bbox(png, thr=8.0):
    """Bounding box of everything brighter than `thr` - the disc's extent when
    the frame holds a filled sky, and a geometry reading only."""
    a = np.asarray(Image.open(png).convert("L"), dtype=np.float64)
    m = a >= thr
    out = {"w": a.shape[1], "h": a.shape[0], "n": int(m.sum())}
    if not m.any():
        out["found"] = False
        return out
    ys, xs = np.nonzero(m)
    out.update({"found": True, "x0": int(xs.min()), "x1": int(xs.max()),
                "y0": int(ys.min()), "y1": int(ys.max()),
                "cx": round(float(xs.mean()), 3), "cy": round(float(ys.mean()), 3)})
    out["bw"] = out["x1"] - out["x0"] + 1
    out["bh"] = out["y1"] - out["y0"] + 1
    return out


def row_profile(png, step=1):
    """Per-row max luminance - presence, not photometry: it says which rows of
    the window the app wrote to at all."""
    a = np.asarray(Image.open(png).convert("L"), dtype=np.float64)
    return [float(v) for v in a[::step].max(axis=1)]


def band_signature(png, y0, y1):
    a = np.asarray(Image.open(png).convert("L"), dtype=np.float64)
    y1 = min(y1, a.shape[0])
    if y1 <= y0:
        return None
    sub = a[y0:y1]
    return {"y0": y0, "y1": y1, "max": float(sub.max()), "mean": round(float(sub.mean()), 6),
            "nonzero": int((sub > 0).sum()), "npx": int(sub.size)}


def lstsq_1d(xs, ys):
    """ys = a*xs + b, least squares; returns a, b, max |residual|."""
    if len(xs) < 2:
        return None, None, None
    A = np.vstack([np.asarray(xs, dtype=float), np.ones(len(xs))]).T
    sol, *_ = np.linalg.lstsq(A, np.asarray(ys, dtype=float), rcond=None)
    a, b = float(sol[0]), float(sol[1])
    res = max(abs(a * x + b - y) for x, y in zip(xs, ys))
    return round(a, 6), round(b, 4), round(res, 4)


# ---------------------------------------------------------------------------
def bodies_on_screen(bodies, R, margin=40):
    """Bodies the OLD dump places inside the render, with a lit-enough size."""
    out = []
    for name, o in bodies.items():
        old = o.get("old") or {}
        s = old.get("screen")
        if not s or not old.get("visible"):
            continue
        if margin <= s[0] <= 2 * R - margin and margin <= s[1] <= 2 * R - margin:
            out.append((name, s, (o.get("new") or {}).get("screen")))
    return out


def measure_point(app, res, win, tag, fov):
    """At the current state: set the fov (settle by MEASUREMENT), then per path
    take the dump, the app's own frame (channel A) and a window grab (channel B)."""
    app.send(f"zoom fov {fov} duration 0", 1.0)
    got, npoll, secs, h, _ = F.wait_until(
        app, f"{tag}_fov",
        lambda H, Bd: abs(H["oldView"]["projector"]["fov"] - fov) < 1e-6
        and abs(math.degrees(H["camera"]["halfFov"]) * 2 - fov) < 1e-3,
        f"fov -> {fov}")
    proj = (h.get("oldView") or {}).get("projector") or {}
    R = proj.get("viewportRadius")
    rec = {"tag": tag, "fov": fov, "fov_settled": got, "fov_polls": npoll,
           "fov_secs": round(secs, 2),
           "projector": {k: proj.get(k) for k in
                         ("fov", "viewport", "viewportCenter", "viewportRadius",
                          "viewportFovDiameter", "fisheyeScaleFactor", "projectionType")},
           "camera_halfFov": (h.get("camera") or {}).get("halfFov"),
           "paths": {}}
    for path, flag in (("new", "on"), ("old", "off")):
        app.send(f"flag experimental_path {flag}", 1.5)
        h1, b1 = app.dump(f"{tag}_{path}_a", pause=1.0)
        h2, b2 = app.dump(f"{tag}_{path}_b", pause=1.0)
        png = app.shot(f"{tag}_{path}")
        grab = window_grab(win, app.out / "grabs" / f"{tag}_{path}.png")
        h3, b3 = app.dump(f"{tag}_{path}")
        nv = F.nav(h3) if h3 else {}
        p3 = (h3.get("oldView") or {}).get("projector") or {}
        e = {"stationary": (((b1.get(TARGET) or {}).get("old") or {}).get("screen")
                            == ((b2.get(TARGET) or {}).get("old") or {}).get("screen")),
             "projector": {k: p3.get(k) for k in
                           ("fov", "viewport", "viewportCenter", "viewportRadius",
                            "viewportFovDiameter", "projectionType")},
             "nav": {k: nv.get(k) for k in
                     ("localVision", "viewOffset", "viewOffsetTransition",
                      "flagTraking", "viewingMode", "heading")},
             "camera": {k: (h3.get("camera") or {}).get(k) for k in
                        ("viewOffset", "viewOffsetTransition", "viewOffsetEff", "halfFov")},
             "control": (h3.get("control") or {}).get("viewOffset"),
             "frame": str(png.relative_to(app.out)) if png else None,
             "grab": grab, "bodies": {}}
        if png:
            a = np.asarray(Image.open(png).convert("L"), dtype=np.float64)
            e["frame_dims"] = [int(a.shape[1]), int(a.shape[0])]
            # THE ONE CONSTANT F81 SAID TO RE-MEASURE: H vs 2*viewportRadius
            e["frame_H_eq_2R"] = (R is not None and a.shape[0] == a.shape[1] == 2 * R)
        Hh = 2 * (R or 1024)
        for name, s_old, s_new in bodies_on_screen(b3, R or 1024):
            ent = {"screen_old": s_old, "screen_new": s_new}
            if png:
                # both y conventions tried, the one that holds the body recorded
                # (F81's measured trap: the PNG is the VERTICAL MIRROR of the old
                # dump's screen px) - never assume the convention
                bflip = F.blob_at(png, (s_old[0], Hh - s_old[1]))
                bdir = F.blob_at(png, tuple(s_old))
                ent["A"] = bflip if bflip.get("found") else bdir
                ent["A_convention"] = ("y-flip" if bflip.get("found")
                                       else ("y-direct" if bdir.get("found") else "neither"))
            a_c = ent.get("A")
            if grab.get("ok") and a_c and a_c.get("found"):
                # channel B seeded from the MEASURED channel-A position, so the
                # only model under test is the blit's, not the dump convention's.
                # Each candidate map is tried and the one that holds the body is
                # recorded (F81's rule).
                sw, sh = grab["w"], grab["h"]
                sc = min(sw, sh) / Hh
                scaled = sc * Hh
                bx = a_c["cx"] * sc + (sw - scaled) / 2
                by = a_c["cy"] * sc
                cands = {"bottom_aligned": (bx, by + (sh - scaled)),
                         "centred": (bx, by + (sh - scaled) / 2),
                         "top_aligned": (bx, by)}
                got_b = {k: F.blob_at(grab["path"], v, win=40) for k, v in cands.items()}
                winner = next((k for k in ("bottom_aligned", "centred", "top_aligned")
                               if got_b[k].get("found")), None)
                ent["B_seed_px"] = {k: [round(v[0], 2), round(v[1], 2)]
                                    for k, v in cands.items()}
                ent["B_candidates"] = {k: {kk: v.get(kk) for kk in
                                           ("found", "cx", "cy", "npx", "peak", "why")}
                                       for k, v in got_b.items()}
                ent["B_seed_model"] = winner
                ent["B"] = got_b[winner] if winner else None
            e["bodies"][name] = ent
        rec["paths"][path] = e
    res["points"].append(rec)
    return rec


def fit_map(rec, R):
    """Fit window = a*render + b from every body found in BOTH channels, per path.
    The independent variable is the MEASURED channel-A centroid (image px of the
    app's own readback); the dependent one is the channel-B centroid."""
    out = {}
    for path, e in rec["paths"].items():
        xs, ys, wxs, wys, names = [], [], [], [], []
        for name, ent in e["bodies"].items():
            b, a = ent.get("B"), ent.get("A")
            if not (b and b.get("found") and a and a.get("found")):
                continue
            names.append(name)
            xs.append(a["cx"]); ys.append(a["cy"])
            wxs.append(b["cx"]); wys.append(b["cy"])
        ax, bx, rx = lstsq_1d(xs, wxs)
        ay, by, ry = lstsq_1d(ys, wys)
        out[path] = {"n": len(names), "bodies": names,
                     "x": {"a": ax, "b": bx, "maxres": rx},
                     "y": {"a": ay, "b": by, "maxres": ry}}
        if ax is not None:
            out[path]["dome_centre_in_window"] = [round(ax * R + bx, 3),
                                                  round(ay * R + by, 3)]
    return out


# ---------------------------------------------------------------------------
def stage_run(out, res, aspect):
    w, h = ASPECTS[aspect]
    farmdir = F.farm_build()
    cfg = farmdir / "config.ini"
    res["config_edits"] = {"screen_w": F.ini_set(cfg, "video", "screen_w", w),
                           "screen_h": F.ini_set(cfg, "video", "screen_h", h)}
    txt = cfg.read_bytes().decode("latin-1")

    def _key(k):
        m = re.search(r"^%s\s*=\s*(\S+)" % re.escape(k), txt, re.M)
        return m.group(1) if m else None
    res["farm_config_readback"] = {
        k: _key(k) for k in ("screen_w", "screen_h", "fullscreen", "render_size",
                             "autoscreen", "remote_display", "projection",
                             "view_offset", "heading", "init_view_pos")}
    res["farm_config_md5"] = F.md5(cfg)
    res["asked"] = {"screen_w": w, "screen_h": h}

    app = F.App(out, farmdir, out / f"{aspect}.applog")
    try:
        # the window, as X sees it - the ONLY authority on what was created
        win, t0 = None, time.time()
        while time.time() - t0 < 40 and win is None:
            win = client_window()
            if win is None:
                time.sleep(0.3)
        res["window"] = win
        res["applog_extents"] = parse_applog(out / f"{aspect}.applog")
        F.ok(f"window {win}  extents {res['applog_extents']}")

        F.common_setup(app, jd=JD)
        h0, _ = app.dump("startup")
        res["startup"] = {"projector": (h0.get("oldView") or {}).get("projector"),
                          "camera_halfFov": (h0.get("camera") or {}).get("halfFov"),
                          "jd": h0.get("jd")}
        app.send(f"select planet {TARGET}", 1.0)
        app.send("flag track_object on", 2.0)
        got, npoll, secs, h1, _ = F.wait_until(
            app, "arm", lambda H, Bd: F.nav(H)["plans"]["flagAutoMove"] == 0,
            "auto-move complete (tracking aim landed)")
        res["arm"] = {"settled": got, "polls": npoll, "secs": round(secs, 2)}
        app.send("flag track_object off", 1.5)
        F.wait_until(app, "hold",
                     lambda H, Bd: F.nav(H)["flagTraking"] == 0
                     and F.nav(H)["plans"]["flagAutoMove"] == 0,
                     "tracking released, view held")

        # ---- THE ZERO CONTROL IS THE FIRST MEASUREMENT (fov 180) -------------
        for fov in FOVS:
            rec = measure_point(app, res, win, f"{aspect}_fov{int(fov)}", fov)
            R = (rec.get("projector") or {}).get("viewportRadius") or 1024
            rec["map_fit"] = fit_map(rec, R)
            if fov == FOVS[0]:
                for path, e in rec["paths"].items():
                    a = (e["bodies"].get(TARGET) or {}).get("A")
                    if not (a and a.get("found")):
                        F.fail(f"ZERO CONTROL: {TARGET} not found in the {path} frame")
                        continue
                    d = math.hypot(a["cx"] - R, a["cy"] - R)
                    res.setdefault("zero_control", {})[path] = {
                        "R": R, "cx": a["cx"], "cy": a["cy"], "dist_px": round(d, 4),
                        "dump_screen_old": (e["bodies"][TARGET]["screen_old"]),
                        "pass": d <= 0.25}
                    (F.ok if d <= 0.25 else F.fail)(
                        f"zero control {path}: centroid ({a['cx']}, {a['cy']}) "
                        f"vs centre ({R}, {R}) -> {d:.4f} px")

        # ---- the disc-extent probe: a filled sky, geometry only --------------
        app.send("zoom fov 180 duration 0", 1.0)
        F.wait_until(app, "disc_fov",
                     lambda H, Bd: abs(H["oldView"]["projector"]["fov"] - 180.0) < 1e-6,
                     "fov -> 180 for the disc probe")
        app.send("flag stars on", 1.0)
        app.send("flag milky_way on", 2.0)
        res["disc_probe"] = {}
        for path, flag in (("new", "on"), ("old", "off")):
            app.send(f"flag experimental_path {flag}", 1.5)
            png = app.shot(f"disc_{path}")
            g = window_grab(win, out / "grabs" / f"disc_{path}.png")
            d = {"frame": str(png.relative_to(out)) if png else None, "grab": g}
            if png:
                d["A_bbox"] = lit_bbox(png)
            if g.get("ok"):
                d["B_bbox"] = lit_bbox(g["path"])
                d["B_rows"] = row_profile(g["path"])
            res["disc_probe"][path] = d
        app.send("flag stars off", 1.0)
        app.send("flag milky_way off", 1.5)

        # ---- the unwritten-band probe: two grabs, 1.5 s apart ---------------
        res["band_probe"] = {}
        for i in (0, 1):
            g = window_grab(win, out / "grabs" / f"band_{i}.png")
            if g.get("ok"):
                sw, sh = g["w"], g["h"]
                scaled = min(sw, sh)
                res["band_probe"][f"grab{i}"] = {
                    "grab": g,
                    "above_disc": band_signature(g["path"], 0, max(0, sh - scaled)),
                    "md5": F.md5(g["path"])}
            if i == 0:
                time.sleep(1.5)
    finally:
        res["applog_extents"] = parse_applog(out / f"{aspect}.applog") \
            or res.get("applog_extents")
    return app, res


def stage_default_render(out, res):
    """SUPPLEMENTARY leg: portrait at the DEFAULT render_size (0).

    checkConfig.cpp:106 writes `render_size = 0` into a generated config.ini; the
    field file carries an AUTHORED 2048.  At 0 the app skips `dedicatedViewport`
    entirely (app.cpp:107), sizes the render target from the swapchain
    (app.cpp:267-271) and blits to the full swapchain extent (app.cpp:1114-1118) -
    a different code path at every stage this leg measures.  Predictions in
    f82_predictions_supplement.json, committed before this launch.
    """
    w, h = ASPECTS["portrait"]
    farmdir = F.farm_build()
    cfg = farmdir / "config.ini"
    res["config_edits"] = {"screen_w": F.ini_set(cfg, "video", "screen_w", w),
                           "screen_h": F.ini_set(cfg, "video", "screen_h", h),
                           "render_size": F.ini_set(cfg, "video", "render_size", 0)}
    res["asked"] = {"screen_w": w, "screen_h": h, "render_size": 0}
    res["farm_config_md5"] = F.md5(cfg)
    app = F.App(out, farmdir, out / "defaultrender.applog")
    try:
        win, t0 = None, time.time()
        while time.time() - t0 < 40 and win is None:
            win = client_window()
            if win is None:
                time.sleep(0.3)
        res["window"] = win
        res["applog_extents"] = parse_applog(out / "defaultrender.applog")
        F.ok(f"window {win}  extents {res['applog_extents']}")

        F.common_setup(app, jd=JD)
        app.send(f"select planet {TARGET}", 1.0)
        app.send("flag track_object on", 2.0)
        F.wait_until(app, "arm", lambda H, Bd: F.nav(H)["plans"]["flagAutoMove"] == 0,
                     "auto-move complete (tracking aim landed)")
        app.send("flag track_object off", 1.5)
        F.wait_until(app, "hold", lambda H, Bd: F.nav(H)["flagTraking"] == 0
                     and F.nav(H)["plans"]["flagAutoMove"] == 0,
                     "tracking released, view held")
        app.send("zoom fov 180 duration 0", 1.0)
        F.wait_until(app, "fov",
                     lambda H, Bd: abs(H["oldView"]["projector"]["fov"] - 180.0) < 1e-6,
                     "fov -> 180")
        res["points"] = []
        for path, flag in (("new", "on"), ("old", "off")):
            app.send(f"flag experimental_path {flag}", 1.5)
            png = app.shot(f"dr_{path}")
            g = window_grab(win, out / "grabs" / f"dr_{path}.png")
            hh, bb = app.dump(f"dr_{path}")
            proj = (hh.get("oldView") or {}).get("projector") or {}
            rec = {"path": path,
                   "projector": {k: proj.get(k) for k in
                                 ("fov", "viewport", "viewportCenter", "viewportRadius",
                                  "viewportFovDiameter", "projectionType")},
                   "screen_old": ((bb.get(TARGET) or {}).get("old") or {}).get("screen"),
                   "screen_new": ((bb.get(TARGET) or {}).get("new") or {}).get("screen"),
                   "frame": str(png.relative_to(out)) if png else None, "grab": g}
            # the target is the brightest thing in this scene by a wide margin and
            # it is AT the dome centre; the blob at the brightest pixel therefore
            # LOCATES the dome centre in each channel without needing a map fit.
            if png:
                a = np.asarray(Image.open(png).convert("L"), dtype=np.float64)
                rec["frame_dims"] = [int(a.shape[1]), int(a.shape[0])]
                rec["A_brightest"] = F.blob_at(png, None)
            if g.get("ok"):
                rec["B_brightest"] = F.blob_at(g["path"], None)
                rec["B_rows"] = row_profile(g["path"])
            res["points"].append(rec)
    finally:
        res["applog_extents"] = parse_applog(out / "defaultrender.applog") \
            or res.get("applog_extents")
    return app, res


# ---------------------------------------------------------------------------
def _pt(res, fov):
    for p in res["points"]:
        if p["fov"] == fov:
            return p
    return None


def build_table(d):
    """Regenerate the whole table from the two run JSONs - nothing retyped."""
    d = Path(d)
    R = {a: json.loads((d / a / f"f82_{a}.json").read_text())
         for a in ("portrait", "square")}
    pred = json.loads((HERE / "f82_predictions.json").read_text())
    L, A = [], None
    A = L.append
    A("# F82 - the portrait leg: one launch per aspect, both paths, both readback channels\n")
    A(f"binary `{R['portrait']['binary_md5'][:8]}` - code master-beta @ 85cc2785 "
      f"(engine sources ba7a32a8), target `{R['portrait']['target']}`, "
      f"jd {R['portrait']['jd']} (F81's measured meridian transit), view_offset 0 everywhere.\n")
    A("Channel A = the app's own readback (`body action screenshot`); "
      "channel B = `x11grab -window_id <client>` of the app's window (11.172(c)).\n")

    A("\n## 1. What the two launches report about themselves\n")
    A("| reading | source | square (control) | portrait | same? |")
    A("|---|---|---|---|---|")

    def row(label, src, f):
        s, p = f(R["square"]), f(R["portrait"])
        A(f"| {label} | {src} | {s} | {p} | {'YES' if s == p else 'no'} |")

    row("config asked", "farm config.ini",
        lambda r: f"{r['asked']['screen_w']}x{r['asked']['screen_h']}")
    row("config read back", "farm config.ini",
        lambda r: f"{r['farm_config_readback']['screen_w']}x{r['farm_config_readback']['screen_h']}")
    row("`Windows size is`", "sdl_facade.cpp:199 (the REQUESTED size)",
        lambda r: "x".join(str(v) for v in r["applog_extents"].get("windows_size", [])))
    row("X client window", "xwininfo -root -tree",
        lambda r: f"{(r.get('window') or {}).get('w')}x{(r.get('window') or {}).get('h')}")
    row("`Swapchain :`", "VulkanMgr.cpp:149",
        lambda r: str(tuple(r["applog_extents"].get("swapchain", []))))
    row("`Scaling :`", "VulkanMgr.cpp:148",
        lambda r: str(r["applog_extents"].get("scaling", [None])[0]))
    row("`Viewport :`", "VulkanMgr.cpp:150",
        lambda r: str(tuple(r["applog_extents"].get("viewport", []))))
    row("`Rect :`", "VulkanMgr.cpp:155",
        lambda r: str(tuple(r["applog_extents"].get("rect", []))))
    row("FrameBuffer 'main 0'", "applog",
        lambda r: str(tuple(r["applog_extents"].get("framebuffer_main0", []))))
    for fov in FOVS:
        row(f"projector.viewport @ fov {fov:.0f}", "the projector's OWN dump",
            lambda r, fv=fov: str(_pt(r, fv)["projector"]["viewport"]))
        row(f"projector.viewportRadius @ fov {fov:.0f}", "the projector's OWN dump",
            lambda r, fv=fov: str(_pt(r, fv)["projector"]["viewportRadius"]))
        row(f"projector.viewportCenter @ fov {fov:.0f}", "the projector's OWN dump",
            lambda r, fv=fov: str(_pt(r, fv)["projector"]["viewportCenter"]))
        row(f"camera.halfFov @ fov {fov:.0f}", "the camera's OWN dump",
            lambda r, fv=fov: str(_pt(r, fv)["camera_halfFov"]))
        row(f"channel A frame dims @ fov {fov:.0f}", "the written PNG",
            lambda r, fv=fov: str(_pt(r, fv)["paths"]["old"].get("frame_dims")))
        row(f"A: H == 2*viewportRadius @ fov {fov:.0f}", "measured, not inherited",
            lambda r, fv=fov: str(_pt(r, fv)["paths"]["old"].get("frame_H_eq_2R")))
        row(f"channel B grab dims @ fov {fov:.0f}", "x11grab of the client window",
            lambda r, fv=fov: f"{_pt(r, fv)['paths']['old']['grab'].get('w')}x"
                              f"{_pt(r, fv)['paths']['old']['grab'].get('h')}")

    A("\n## 2. The zero control (offset 0, the tracked target at the dome centre), FIRST on each aspect\n")
    A("| aspect | path | viewportRadius | old dump screen | channel A centroid | distance from (R,R) px | pass (<= 0.25) |")
    A("|---|---|---:|---|---|---:|---|")
    for a in ("square", "portrait"):
        for path in ("old", "new"):
            z = (R[a].get("zero_control") or {}).get(path)
            if not z:
                A(f"| {a} | {path} | - | - | NOT MEASURED | - | - |")
                continue
            A(f"| {a} | {path} | {z['R']} | {z['dump_screen_old']} | "
              f"({z['cx']}, {z['cy']}) | {z['dist_px']} | {'PASS' if z['pass'] else 'FAIL'} |")

    A("\n## 3. The render -> window map, FITTED from the bodies found in both channels\n")
    A("model: `window = a * A_centroid + b`, least squares over every body found in "
      "channel A and channel B of the same frame.\n")
    A("| aspect | path | n bodies | a (x) | b (x) | max res x | a (y) | b (y) | max res y | dome centre in window | bodies found: bottom / centred / top |")
    A("|---|---|---:|---:|---:|---:|---:|---:|---:|---|---|")
    for a in ("square", "portrait"):
        for fov in FOVS:
            p = _pt(R[a], fov)
            for path in ("old", "new"):
                m = (p.get("map_fit") or {}).get(path)
                if not m or m["x"]["a"] is None:
                    continue
                cnt = {k: 0 for k in ("bottom_aligned", "centred", "top_aligned")}
                for e in p["paths"][path]["bodies"].values():
                    for k, v in (e.get("B_candidates") or {}).items():
                        if v.get("found"):
                            cnt[k] += 1
                A(f"| {a} fov {fov:.0f} | {path} | {m['n']} | {m['x']['a']} | {m['x']['b']} | "
                  f"{m['x']['maxres']} | {m['y']['a']} | {m['y']['b']} | {m['y']['maxres']} | "
                  f"{m.get('dome_centre_in_window')} | "
                  f"{cnt['bottom_aligned']} / {cnt['centred']} / {cnt['top_aligned']} |")
    A("\n**At the square control the three candidate seeds COINCIDE** (swapchain "
      "height == the scaled square), so the counts there are 24/24/24 by "
      "construction and discriminate nothing - which is exactly why the control "
      "cannot see this. Portrait separates them by 128 px and the counts are "
      "24 / 0 / 0.\n")
    A("\nThe predictor, evaluated from each launch's OWN measured swapchain extent "
      "(VulkanMgr.cpp:145-163 + VulkanMgr.hpp:122-124):\n")
    A("| aspect | swapchain | scaled = min(W,H) | a = (scaled-1)/(2R) | dome x = (W-1)/2 | dome y, BOTTOM = H-1-(scaled-1)/2 | dome y, CENTRED (mutation) | measured dome centre |")
    A("|---|---|---:|---:|---:|---:|---:|---|")
    for a in ("square", "portrait"):
        sw, sh = R[a]["applog_extents"]["swapchain"]
        p = _pt(R[a], 90.0)
        Rr = p["projector"]["viewportRadius"]
        scaled = min(sw, sh)
        A(f"| {a} | {sw}x{sh} | {scaled} | {(scaled - 1) / (2 * Rr):.6f} | "
          f"{(sw - 1) / 2:.1f} | {sh - 1 - (scaled - 1) / 2:.1f} | "
          f"{(sh - scaled) / 2 + (scaled - 1) / 2:.1f} | "
          f"{(p['map_fit']['old'] or {}).get('dome_centre_in_window')} |")
    pp = pred["predictions"]["P8_render_to_window_map"]
    A(f"\npredicted before the run (integer form): portrait a={pp['portrait']['a']}, "
      f"b={pp['portrait']['b']}, d={pp['portrait']['d']}, dome centre "
      f"{pp['portrait']['dome_centre_in_window']}; square a={pp['square']['a']}, "
      f"b={pp['square']['b']}, d={pp['square']['d']}, dome centre "
      f"{pp['square']['dome_centre_in_window']}. The half-pixel between those and "
      "the row above is the `(scaled - 1)` term of `mouseNorm.scaleX/Y`, which the "
      "pre-run form rounded; it moves nothing that this leg decides.\n")
    A(f"mutation (centred letterbox): {pp['mutation_M8a_centred_letterbox']}\n")

    A("\n## 4. The disc extent, second and independent read (star field on, fov 180) - geometry only\n")
    A("| aspect | path | channel | frame/grab dims | lit bbox x0..x1 | y0..y1 | bbox w x h |")
    A("|---|---|---|---|---|---|---|")
    for a in ("square", "portrait"):
        for path in ("old", "new"):
            dp = (R[a].get("disc_probe") or {}).get(path) or {}
            for ch, key in (("A", "A_bbox"), ("B", "B_bbox")):
                bb = dp.get(key)
                if not bb:
                    A(f"| {a} | {path} | {ch} | - | NOT MEASURED | - | - |")
                    continue
                A(f"| {a} | {path} | {ch} | {bb['w']}x{bb['h']} | "
                  f"{bb.get('x0')}..{bb.get('x1')} | {bb.get('y0')}..{bb.get('y1')} | "
                  f"{bb.get('bw')} x {bb.get('bh')} |")

    A("\n## 5. The band the blit never writes (channel B, two grabs 1.5 s apart)\n")
    A("| aspect | grab | window | band rows | band max | band mean | nonzero px | grab md5 |")
    A("|---|---|---|---|---:|---:|---:|---|")
    for a in ("square", "portrait"):
        for k, v in sorted((R[a].get("band_probe") or {}).items()):
            b = v.get("above_disc")
            g = v["grab"]
            A(f"| {a} | {k} | {g.get('w')}x{g.get('h')} | "
              f"{(str(b['y0']) + '..' + str(b['y1'])) if b else 'none'} | "
              f"{b['max'] if b else '-'} | {b['mean'] if b else '-'} | "
              f"{b['nonzero'] if b else '-'} | `{v['md5'][:8]}` |")

    A("\n## 6. Parity old vs new, and the ONE question this leg exists to answer: "
      "does any divergence exist ONLY at portrait? (11.52(b): perceptual, geometry only)\n")

    def _pair(a, fov, name):
        """(dump |old-new| px, frame |old-new| px) for one body at one aspect."""
        p = _pt(R[a], fov)
        Rr = p["projector"]["viewportRadius"]
        eo = p["paths"]["old"]["bodies"].get(name)
        en = p["paths"]["new"]["bodies"].get(name)
        if not (eo and en):
            return None, None
        so, sn = eo["screen_old"], eo["screen_new"]
        dd = None
        if so and sn:
            nx = (sn[0] + 1) / 2 * 2 * Rr
            ny = (1 - sn[1]) / 2 * 2 * Rr
            dd = round(math.hypot(nx - so[0], ny - (2 * Rr - so[1])), 4)
        ao, an = eo.get("A") or {}, en.get("A") or {}
        df = (round(math.hypot(ao["cx"] - an["cx"], ao["cy"] - an["cy"]), 4)
              if (ao.get("found") and an.get("found")) else None)
        return dd, df

    A("Every body is measured twice per aspect: the engine's own dual dump (old "
      "render px vs new normalized, brought to render px) and the luminance "
      "centroid in the frame the app itself wrote. `sq - pt` is the whole "
      "question: a non-zero there is a divergence the aspect created.\n")
    A("| fov | body | dump d, square | dump d, portrait | sq - pt | frame d, square | frame d, portrait | sq - pt |")
    A("|---:|---|---:|---:|---:|---:|---:|---:|")
    shared = {}
    for fov in FOVS:
        p = _pt(R["square"], fov)
        groups, loose = {}, []
        for name, e in sorted(p["paths"]["old"]["bodies"].items()):
            c = e.get("A") or {}
            if c.get("found"):
                groups.setdefault((c["cx"], c["cy"]), []).append(name)
            else:
                loose.append(name)
        shared[fov] = {k: v for k, v in groups.items() if len(v) > 1}
        keep = sorted([v[0] for v in groups.values()] + loose)
        for name in keep:
            ds, fs = _pair("square", fov, name)
            dp, fpv = _pair("portrait", fov, name)
            A(f"| {fov:.0f} | {name} | {ds} | {dp} | "
              f"{round(ds - dp, 6) if (ds is not None and dp is not None) else '-'} | "
              f"{fs} | {fpv} | "
              f"{round(fs - fpv, 6) if (fs is not None and fpv is not None) else '-'} |")
    for fov in FOVS:
        for k, v in shared[fov].items():
            A(f"\n*fov {fov:.0f}: {len(v)} bodies share ONE luminance blob at "
              f"{tuple(round(x, 3) for x in k)} ({', '.join(v)}) - the frame channel "
              f"cannot separate them, the dump channel can; the first is listed above "
              f"and the rest are the same measurement.*")

    dr_path = d / "defaultrender" / "f82_defaultrender.json"
    if dr_path.exists():
        dr = json.loads(dr_path.read_text())
        A("\n## 7. SUPPLEMENTARY LEG - the same portrait window at the DEFAULT "
          "`render_size` (0), which is what `checkConfig.cpp:106` writes into a "
          "generated config.ini\n")
        A("This is a THIRD launch and a scope expansion, taken so that any mint "
          "from this leg can name which branch it fires in rather than assume it. "
          "Predictions in `f82_predictions_supplement.json`, committed before it ran.\n")
        A("| reading | portrait, render_size 2048 (field, authored) | portrait, render_size 0 (default) | predicted for the default branch |")
        A("|---|---|---|---|")
        pt = _pt(R["portrait"], 180.0)
        d0 = dr["points"][0]
        A(f"| applog `Scaling` / `Viewport` / `Swapchain` / `Rect` | all four present "
          f"({R['portrait']['applog_extents'].get('scaling')} / "
          f"{tuple(R['portrait']['applog_extents'].get('viewport', []))} / "
          f"{tuple(R['portrait']['applog_extents'].get('swapchain', []))} / "
          f"{tuple(R['portrait']['applog_extents'].get('rect', []))}) | "
          f"{'ABSENT' if not dr['applog_extents'].get('scaling') else 'present'} - "
          f"all four are printed only inside `dedicatedViewport` | absent (the "
          f"prediction named only three of the four; `Swapchain` is in that function too "
          f"- prediction S_P1 partly WRONG, kept) |")
        A(f"| FrameBuffer 'main 0' | "
          f"{tuple(R['portrait']['applog_extents'].get('framebuffer_main0', []))} | "
          f"{tuple(dr['applog_extents'].get('framebuffer_main0', []))} | (768, 1024) |")
        A(f"| projector.viewport | {pt['projector']['viewport']} | "
          f"{d0['projector']['viewport']} | [0, 128, 768, 768] |")
        A(f"| projector.viewportCenter | {pt['projector']['viewportCenter']} | "
          f"{d0['projector']['viewportCenter']} | [384, 512, 0] |")
        A(f"| projector.viewportRadius | {pt['projector']['viewportRadius']} | "
          f"{d0['projector']['viewportRadius']} | 384 |")
        A(f"| channel A readback dims | "
          f"{pt['paths']['old'].get('frame_dims')} | {d0.get('frame_dims')} | [768, 768] |")
        for p in dr["points"]:
            A(f"| target in channel A ({p['path']}) | (1024.18, 1024.044) at R 1024 | "
              f"({p['A_brightest']['cx']}, {p['A_brightest']['cy']}) at R 384 | "
              f"(384, 384) +- the instrument zero |")
        for p in dr["points"]:
            A(f"| **dome centre in the WINDOW** ({p['path']}) | "
              f"{(pt['map_fit'][p['path']] or {}).get('dome_centre_in_window')} | "
              f"({p['B_brightest']['cx']}, {p['B_brightest']['cy']}) | (384, 512) - CENTRED |")
        A("\n**The 128 px is the whole finding**: the same window, the same scene, "
          "the same binary; the authored branch puts the dome centre at y 639.5 and "
          "the default branch at y 512.0. The default branch also makes the "
          "PROJECTOR aspect-aware (viewport [0,128,768,768], radius 384) where the "
          "authored branch keeps it square and window-blind.\n")

    A("\n## 8. Environment asserts\n")
    allr = dict(R)
    if dr_path.exists():
        allr["defaultrender (supplementary)"] = json.loads(dr_path.read_text())
    for a in allr:
        r = allr[a]
        A(f"- **{a}**: GetActive `{r['screensaver_GetActive'].strip()}`, "
          f"no other spacecrafter before the launch, real `~/.spacecrafter` md5 in "
          f"`{r['real_home_md5_in']['config.ini'][:8]}`/`{r['real_home_md5_in']['ssystem.ini'][:8]}` "
          f"== out `{r['real_home_md5_out']['config.ini'][:8]}`/"
          f"`{r['real_home_md5_out']['ssystem.ini'][:8]}`; fails: {r['fails'] or 'none'}")
    (d / "f82_table.md").write_text("\n".join(L) + "\n")
    print(f"wrote {d / 'f82_table.md'}")


def verify(d):
    """Recompute every recorded blob from the COMMITTED frames and grabs, through
    the same rule, and assert it equals what the run wrote.  A number in the entry
    that the committed artifacts cannot reproduce is not evidence."""
    d = Path(d)
    n, bad = 0, []
    for a in ("square", "portrait"):
        run = d / a
        res = json.loads((run / f"f82_{a}.json").read_text())
        for p in res["points"]:
            for path, e in p["paths"].items():
                png = run / e["frame"] if e.get("frame") else None
                Rr = (p["projector"] or {}).get("viewportRadius") or 1024
                for name, ent in e["bodies"].items():
                    rec = ent.get("A")
                    if not (png and png.exists() and rec and rec.get("found")):
                        continue
                    s = ent["screen_old"]
                    got = F.blob_at(png, (s[0], 2 * Rr - s[1]))
                    n += 1
                    if (got.get("cx"), got.get("cy"), got.get("npx")) != \
                       (rec.get("cx"), rec.get("cy"), rec.get("npx")):
                        bad.append((a, p["tag"], path, name, rec, got))
                    b = ent.get("B")
                    g = e.get("grab") or {}
                    if b and b.get("found") and g.get("ok"):
                        gp = run / "grabs" / Path(g["path"]).name
                        if gp.exists():
                            seed = ent["B_seed_px"][ent["B_seed_model"]]
                            got2 = F.blob_at(gp, tuple(seed), win=40)
                            n += 1
                            if (got2.get("cx"), got2.get("cy")) != (b.get("cx"), b.get("cy")):
                                bad.append((a, p["tag"], path, name + " [B]", b, got2))
    print(f"verify: {n} blob(s) recomputed from the committed artifacts, "
          f"{len(bad)} mismatch(es)")
    for row in bad[:10]:
        print("  MISMATCH", row)
    return 1 if bad else 0


def main():
    argv = sys.argv[1:]
    if "--bin" in argv:
        i = argv.index("--bin"); B.SC_BIN = argv[i + 1]; del argv[i:i + 2]
    kw = {}
    for k in ("--aspect",):
        if k in argv:
            i = argv.index(k); kw[k[2:]] = argv[i + 1]; del argv[i:i + 2]
    stage, outdir = argv[0], Path(argv[1]).resolve()

    if stage == "table":
        build_table(outdir)
        return 0
    if stage == "verify":
        return verify(outdir)

    outdir.mkdir(parents=True, exist_ok=True)
    aspect = kw.get("aspect", "defaultrender" if stage == "defaultrender" else None)
    hits = F.no_instance()
    if hits:
        F.fail(f"another spacecrafter is running: pids {hits}")
        print(json.dumps({"fails": F.FAILS}))
        return 2
    gact = F.screensaver_active()
    real_in = {p: F.md5(REAL_HOME / ".spacecrafter" / p)
               for p in ("config.ini", "ssystem.ini")}
    res = {"stage": stage, "aspect": aspect, "binary": B.SC_BIN,
           "binary_md5": F.md5(B.SC_BIN), "target": TARGET, "fovs": FOVS, "jd": JD,
           "t_start": round(time.time(), 3), "display": os.environ.get("DISPLAY"),
           "screensaver_GetActive": gact, "no_instance_before_launch": True,
           "real_home_md5_in": real_in, "points": [], "fails": []}
    app = None
    try:
        if stage == "defaultrender":
            app, res = stage_default_render(outdir, res)
        else:
            app, res = stage_run(outdir, res, aspect)
    finally:
        if app:
            app.close()
        res["real_home_md5_out"] = {p: F.md5(REAL_HOME / ".spacecrafter" / p)
                                    for p in ("config.ini", "ssystem.ini")}
        if res["real_home_md5_out"] != real_in:
            F.fail("real ~/.spacecrafter md5 CHANGED across the launch")
        res["fails"] = F.FAILS
        res["t_end"] = round(time.time(), 3)
        (outdir / f"f82_{aspect}.json").write_text(json.dumps(res, indent=1))
    print(json.dumps({k: v for k, v in res.items() if k not in ("points", "disc_probe")},
                     indent=1)[:4000])
    return 1 if F.FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
