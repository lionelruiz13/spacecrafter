#!/usr/bin/env python3
"""F81 - the view offset's TWO couplings, measured.

WHAT THIS INSTRUMENT SEPARATES
------------------------------
The old path applies `view_offset` at TWO sites with DIFFERENT couplings:

  aim   navigator.cpp:162  setLocalVision:
        local_vision = yrotation(-view_offset * M_PI_2 * transition) * _pos
        -> a FIXED 90 deg * offset re-aim, about the OLD LOCAL frame's y axis
  draw  navigator.cpp:324  updateViewMat:
        mat_local_to_eye = xrotation(view_offset * fov/2 * pi/180 * transition)
                           * mat_local_to_eye
        -> an fov-COUPLED pitch, about the EYE frame's x axis

They cancel at fov 180 and nowhere else.  The new path (Camera.cpp:139-155) has
ONE site - `xrotation(offset * ModularBody::halfFov)` in `renderViewRotation()`,
and `Camera::setViewOffset` only stores the scalar: the Camera is never re-aimed.

The ONE sink is `Core::setViewOffset` (core.cpp:2524-2553), reached by the config
key `[navigation] view_offset` at startup AND by `set zoom_offset <v>` at runtime.
It calls `navigation->setLocalVision(InitViewPos)` - so the AIM half only ever
acts on InitViewPos, and only when the transition is already armed.  Hence:

  * the CONFIG channel can never compensate (transition is 0 at startup);
  * the COMMAND channel compensates whenever it is issued armed;
  * for the compensated aim to be MEASURABLE on a body, the body must be AT
    InitViewPos - which is why this instrument writes the measured local-frame
    direction of its target into the FARM config's `init_view_pos`.

THE MERIDIAN CONDITION (no ledger source states it; found reading the two sites)
The aim turns about local EAST; the draw turns about the eye's x axis
s = earthEquToLocal((equ_vision[1], -equ_vision[0], 0)) under viewing_mode=equator.
s is parallel to local East exactly when the view is PERPENDICULAR to East, i.e.
in the MERIDIAN plane.  The `offset * (90 - fov/2)` prediction is a meridian-view
prediction.  Stage `prep` therefore SEARCHES for the target's meridian transit by
measurement (bracket + secant on the local-frame East component) and every later
stage runs at that JD.

MEASUREMENT
Two independent readings of the same thing at every point:
  (1) the engine's own screen position for the body, per path, from
      `body action dual_dump` (old: render px, `rectToRender`; new: normalized,
      radius = theta/halfFov);
  (2) the body's luminance centroid in the frame the app itself wrote
      (`body action screenshot`), per path, in the SAME launch.
Nothing is slept on: the arming transition, the auto-move, the fov ramp and the
new path's tracking follow are each polled in the dump until they settle.

usage:
  f81_offset.py prep    <outdir>                        [--bin PATH]
  f81_offset.py run     <outdir> --channel config|command --jd JD --initview x,y,z
                                                        [--offset 0.3] [--bin PATH]
Run it through f81_run.sh (farm build + real-HOME md5 in==out assert).
"""
import hashlib, json, math, os, re, socket, subprocess, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import b3_ladder as B          # send / wait_port - ONE authority for the plumbing

FARM = Path(os.environ.get("F81_FARM", "/tmp/f81_farm"))
REAL_HOME = Path.home()
FOVS = [180.0, 90.0, 40.0]
TARGET = os.environ.get("F81_TARGET", "Sun")
FAILS = []


def fail(m):
    FAILS.append(m)
    print(f"FAIL: {m}", flush=True)


def ok(m):
    print(f"ok:   {m}", flush=True)


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


# ---- environment asserts (run before every launch) -----------------------
def no_instance():
    """/proc/<pid>/comm probe (F26 11.134(b)); covers every account."""
    hits = []
    for p in Path("/proc").glob("[0-9]*"):
        try:
            if (p / "comm").read_text().strip() == "spacecrafter":
                hits.append(p.name)
        except OSError:
            pass
    return hits


def screensaver_active():
    try:
        out = subprocess.run(
            ["gdbus", "call", "--session", "--dest", "org.gnome.ScreenSaver",
             "--object-path", "/org/gnome/ScreenSaver",
             "--method", "org.gnome.ScreenSaver.GetActive"],
            capture_output=True, text=True, timeout=20,
            env={**os.environ,
                 "DBUS_SESSION_BUS_ADDRESS": "unix:path=/run/user/%d/bus" % os.getuid()})
        return out.stdout.strip()
    except Exception as e:                                    # noqa: BLE001
        return f"probe-error {e}"


# ---- farm ----------------------------------------------------------------
def farm_build():
    subprocess.run([str(HERE / "b3_farm.sh"), str(FARM)], check=True,
                   capture_output=True, text=True)
    return FARM / ".spacecrafter"


def ini_set(path, section, key, value):
    """Rewrite ONE key inside ONE section of a spacecrafter ini, byte-safe.
    Returns the old value (fails loudly if the key is absent - a silent
    no-op here would make the whole channel a lie)."""
    text = Path(path).read_bytes().decode("latin-1")
    lines = text.split("\n")
    cur, old = None, None
    for i, ln in enumerate(lines):
        s = ln.strip()
        if s.startswith("[") and s.endswith("]"):
            cur = s[1:-1]
            continue
        if cur == section and re.match(r"^\s*" + re.escape(key) + r"\s*=", ln):
            old = ln.split("=", 1)[1].strip()
            lines[i] = f"{key:<31}= {value}"
            Path(path).write_bytes("\n".join(lines).encode("latin-1"))
            return old
    raise KeyError(f"{section}/{key} not found in {path}")


# ---- app driving ---------------------------------------------------------
def parse_dump(path):
    """-> (header, {name: {'old':..,'new':..}})"""
    hdr, bodies = {}, {}
    for line in open(path):
        line = line.strip().rstrip(",")
        if not line:
            continue
        try:
            o = json.loads(line)
        except json.JSONDecodeError:
            continue
        if o.get("type") == "header":
            hdr = o
        elif o.get("type") == "body":
            bodies[o["name"]] = o
    return hdr, bodies


class App:
    def __init__(self, out, farmdir, applog):
        self.out = out
        self.farmdir = farmdir
        self.n_dump = 0
        env = {**os.environ, "HOME": str(FARM),
               "DISPLAY": os.environ.get("DISPLAY", ":2")}
        self.proc = subprocess.Popen([B.SC_BIN], cwd=str(farmdir),
                                     stdout=open(applog, "w"),
                                     stderr=subprocess.STDOUT, env=env)
        self.s = B.wait_port()
        time.sleep(8)

    def send(self, cmd, pause=0.7):
        B.send(self.s, cmd, pause)

    def dump(self, tag, pause=1.2, keep=True):
        self.n_dump += 1
        p = self.out / "dumps" / f"{tag}.json"
        p.parent.mkdir(parents=True, exist_ok=True)
        if p.exists():
            p.unlink()
        self.send(f"body action dual_dump filename {p}", pause)
        for _ in range(50):
            if p.exists() and p.stat().st_size > 0:
                break
            time.sleep(0.2)
        if not p.exists():
            fail(f"dump {tag} never appeared")
            return {}, {}
        h, b = parse_dump(p)
        if not keep:
            p.unlink()
        return h, b

    def shot(self, name, pause=1.5, timeout=25.0):
        p = self.out / "frames" / f"{name}.png"
        p.parent.mkdir(parents=True, exist_ok=True)
        if p.exists():
            p.unlink()
        t0 = time.time()
        self.send(f"body action screenshot filename {p}", pause)
        last = -1
        while time.time() - t0 < timeout:
            if p.exists():
                sz = p.stat().st_size
                if sz > 0 and sz == last:
                    return p
                last = sz
            time.sleep(0.2)
        fail(f"screenshot {name} never completed")
        return None

    def close(self):
        try:
            self.send("shutdown action now", 1)
            self.s.close()
            self.proc.wait(timeout=45)
        except Exception:                                     # noqa: BLE001
            pass
        if self.proc.poll() is None:
            self.proc.kill()


# ---- settle-by-measurement ------------------------------------------------
def wait_until(app, tag, pred, describe, tries=40, pause=0.8):
    """Poll the dump until pred(header, bodies).  Returns (ok, n_polls,
    seconds, last header, last bodies) - the poll count IS the evidence that
    this was a measurement and not a sleep."""
    t0 = time.time()
    h, b = {}, {}
    for i in range(tries):
        h, b = app.dump(f"{tag}_poll{i:02d}", pause=0.6, keep=(i == tries - 1))
        if h and pred(h, b):
            dt = time.time() - t0
            ok(f"settled {describe} after {i + 1} poll(s), {dt:.1f}s")
            return True, i + 1, dt, h, b
        time.sleep(pause)
    fail(f"never settled: {describe}")
    return False, tries, time.time() - t0, h, b


def nav(h):
    return h["oldView"]["nav"]


def cam(h):
    return h["camera"]


# ---- frame measurement ----------------------------------------------------
def centroid(png, frac=0.5, win=400):
    """Luminance centroid of the brightest blob.  Window around the brightest
    pixel, weights = (L - t)+ with t = frac*max: unbiased on a symmetric blob,
    immune to labels/stars below the threshold."""
    a = np.asarray(Image.open(png).convert("L"), dtype=np.float64)
    H, W = a.shape
    mx = float(a.max())
    if mx <= 2.0:
        return {"found": False, "max": mx, "w": W, "h": H}
    iy, ix = np.unravel_index(int(np.argmax(a)), a.shape)
    y0, y1 = max(0, iy - win), min(H, iy + win + 1)
    x0, x1 = max(0, ix - win), min(W, ix + win + 1)
    sub = a[y0:y1, x0:x1]
    t = frac * mx
    wgt = np.clip(sub - t, 0, None)
    tot = wgt.sum()
    if tot <= 0:
        return {"found": False, "max": mx, "w": W, "h": H}
    ys, xs = np.mgrid[y0:y1, x0:x1]
    cx = float((wgt * (xs + 0.5)).sum() / tot)
    cy = float((wgt * (ys + 0.5)).sum() / tot)
    return {"found": True, "cx": round(cx, 3), "cy": round(cy, 3),
            "npx": int((wgt > 0).sum()), "max": mx, "w": W, "h": H,
            "argmax": [int(ix), int(iy)]}


def unit(v):
    n = math.sqrt(sum(c * c for c in v))
    return [c / n for c in v] if n else list(v)


def angle_between(a, b):
    a, b = unit(a), unit(b)
    d = max(-1.0, min(1.0, sum(x * y for x, y in zip(a, b))))
    return math.degrees(math.acos(d))


# ---- the per-fov measurement point ---------------------------------------
def measure_point(app, res, tag, fov, target):
    """At the CURRENT app state: set the fov (settle by measurement), then for
    each path in turn take the dump AND the frame while that path is the one
    drawing (each path's screenPos is written by its own draw)."""
    app.send(f"zoom fov {fov} duration 0", 1.0)
    got, npoll, secs, h, _ = wait_until(
        app, f"{tag}_fov",
        lambda H, Bd: abs(H["oldView"]["projector"]["fov"] - fov) < 1e-6
        and abs(math.degrees(H["camera"]["halfFov"]) * 2 - fov) < 1e-3,
        f"fov -> {fov}")
    rec = {"tag": tag, "fov": fov, "fov_settled": got, "fov_polls": npoll,
           "fov_secs": round(secs, 2), "paths": {}}
    R = h.get("oldView", {}).get("projector", {}).get("viewportRadius")
    rec["viewportRadius"] = R
    rec["gatesViewportRadius"] = h.get("gates", {}).get("viewportRadius")

    for path, flag in (("new", "on"), ("old", "off")):
        app.send(f"flag experimental_path {flag}", 1.5)
        # the new path's tracking follow / any residual motion must be still
        h1, b1 = app.dump(f"{tag}_{path}_a", pause=1.0)
        h2, b2 = app.dump(f"{tag}_{path}_b", pause=1.0)
        png = app.shot(f"{tag}_{path}")
        h3, b3 = app.dump(f"{tag}_{path}")
        e = {}
        for k, (hh, bb) in (("a", (h1, b1)), ("b", (h2, b2)), ("c", (h3, b3))):
            if not hh:
                continue
            o = bb.get(target, {})
            e[f"screen_{k}"] = {"old": (o.get("old") or {}).get("screen"),
                                "new": (o.get("new") or {}).get("screen")}
        e["stationary"] = (e.get("screen_b") == e.get("screen_c"))
        e["nav"] = {kk: nav(h3).get(kk) for kk in
                    ("localVision", "equVision", "viewOffset",
                     "viewOffsetTransition", "flagTraking", "viewingMode")} if h3 else {}
        e["navPlans"] = nav(h3).get("plans") if h3 else {}
        e["camera"] = {kk: cam(h3).get(kk) for kk in
                       ("viewOffset", "viewOffsetTransition", "viewOffsetEff",
                        "halfFov", "tracked")} if h3 else {}
        e["control"] = h3.get("control", {}).get("viewOffset") if h3 else None
        e["bodyOld"] = {kk: (b3.get(target, {}).get("old") or {}).get(kk)
                        for kk in ("screen", "dist", "visible", "screenSz")}
        e["bodyNew"] = {kk: (b3.get(target, {}).get("new") or {}).get(kk)
                        for kk in ("screen", "dist", "visible", "screenSize")}
        e["frame"] = str(png.relative_to(app.out)) if png else None
        e["centroid"] = centroid(png) if png else None
        rec["paths"][path] = e
    res["points"].append(rec)
    return rec


# ---- stages ---------------------------------------------------------------
def common_setup(app, jd=None):
    app.send("flag experimental_path on")
    app.send("timerate rate 0")
    app.send("meteors zhr 0")
    app.send("flag atmosphere off")
    app.send("flag landscape off")
    app.send("flag stars off")
    app.send("flag milky_way off")
    app.send("flag nebulae off")
    app.send("flag planet_names off")
    app.send("flag planets_orbits off")
    if jd is not None:
        app.send(f"date jday {jd:.9f}", 1.5)


def stage_prep(out, res):
    farmdir = farm_build()
    app = App(out, farmdir, out / "prep.applog")
    try:
        common_setup(app)
        app.send(f"select planet {TARGET}", 1.0)
        app.send("flag track_object on", 2.0)
        got, npoll, secs, h, b = wait_until(
            app, "prep_arm",
            lambda H, Bd: nav(H)["plans"]["flagAutoMove"] == 0,
            "auto-move complete (tracking aim landed)")
        res["prep_arm"] = {"settled": got, "polls": npoll, "secs": round(secs, 2)}
        jd0 = h["jd"]
        res["jd_start"] = jd0
        ok(f"app clock jd = {jd0}")

        # --- bracket the meridian transit: local-frame East component == 0
        scan = []
        for k in range(25):
            jd = jd0 + k / 24.0
            app.send(f"date jday {jd:.9f}", 0.9)
            h, _ = app.dump(f"prep_scan{k:02d}", pause=0.8, keep=False)
            lv = unit(nav(h)["localVision"])
            scan.append({"jd": jd, "lv": lv, "east": lv[1], "up": lv[2],
                         "jd_dump": h["jd"]})
            if abs(h["jd"] - jd) > 1e-6:
                fail(f"date jday not honoured: asked {jd} got {h['jd']}")
        res["scan"] = scan
        cand = [(i, scan[i], scan[i + 1]) for i in range(len(scan) - 1)
                if scan[i]["east"] * scan[i + 1]["east"] < 0
                and scan[i]["up"] > 0.2 and scan[i + 1]["up"] > 0.2]
        if not cand:
            fail("no meridian crossing with the target above the horizon in 24 h")
            return app, res
        # the crossing whose neighbours are highest = the transit
        i, a1, a2 = max(cand, key=lambda c: c[1]["up"] + c[2]["up"])
        lo, hi = a1["jd"], a2["jd"]
        flo, fhi = a1["east"], a2["east"]
        steps = []
        for _ in range(8):
            jd = lo - flo * (hi - lo) / (fhi - flo)
            app.send(f"date jday {jd:.9f}", 0.9)
            h, _ = app.dump("prep_secant", pause=0.8, keep=False)
            lv = unit(nav(h)["localVision"])
            steps.append({"jd": jd, "east": lv[1], "up": lv[2]})
            if abs(lv[1]) < 1e-6:
                break
            if lv[1] * flo < 0:
                hi, fhi = jd, lv[1]
            else:
                lo, flo = jd, lv[1]
        res["secant"] = steps
        jdstar = steps[-1]["jd"]
        app.send(f"date jday {jdstar:.9f}", 1.2)
        h, b = app.dump("prep_star")
        lv = unit(nav(h)["localVision"])
        res["jd_star"] = jdstar
        res["initview"] = lv
        res["east_residual"] = lv[1]
        res["altitude_deg"] = math.degrees(math.asin(max(-1, min(1, lv[2]))))
        res["azimuth_off_meridian_deg"] = math.degrees(
            math.asin(max(-1, min(1, lv[1] / max(1e-9, math.sqrt(1 - lv[2] ** 2))))))
        ok(f"transit jd {jdstar:.9f}  localVision {lv}  alt "
           f"{res['altitude_deg']:.3f} deg  off-meridian "
           f"{res['azimuth_off_meridian_deg']:.5f} deg")

        # --- P5 zero control: offset 0, both paths, three fovs
        app.send("flag track_object off", 1.5)
        wait_until(app, "prep_hold",
                   lambda H, Bd: nav(H)["flagTraking"] == 0
                   and nav(H)["plans"]["flagAutoMove"] == 0,
                   "tracking released, view held")
        for fov in FOVS:
            measure_point(app, res, f"zero_fov{int(fov)}", fov, TARGET)
    finally:
        pass
    return app, res


def stage_run(out, res, channel, jd, initview, offset):
    farmdir = farm_build()
    cfg = farmdir / "config.ini"
    res["config_edits"] = {}
    res["config_edits"]["init_view_pos"] = ini_set(
        cfg, "navigation", "init_view_pos",
        "%.9g,%.9g,%.9g" % tuple(initview))
    if channel == "config":
        res["config_edits"]["view_offset"] = ini_set(
            cfg, "navigation", "view_offset", "%g" % offset)
    res["farm_config_md5"] = md5(cfg)
    app = App(out, farmdir, out / f"{channel}.applog")
    try:
        common_setup(app, jd=jd)
        h, _ = app.dump("startup")
        res["startup_nav"] = {k: nav(h)[k] for k in
                              ("localVision", "viewOffset", "viewOffsetTransition")}
        res["startup_camera"] = {k: cam(h)[k] for k in
                                 ("viewOffset", "viewOffsetTransition", "viewOffsetEff")}
        app.send(f"select planet {TARGET}", 1.0)
        app.send("flag track_object on", 2.0)      # arms BOTH paths (11.92(a))
        got, npoll, secs, h, b = wait_until(
            app, "arm",
            lambda H, Bd: nav(H)["plans"]["flagAutoMove"] == 0
            and abs(nav(H)["viewOffsetTransition"] - 1.0) < 1e-9
            and abs(cam(H)["viewOffsetTransition"] - 1.0) < 1e-9,
            "view_offset_transition == 1 on BOTH paths, auto-move complete")
        res["arm"] = {"settled": got, "polls": npoll, "secs": round(secs, 2),
                      "old_transition": nav(h)["viewOffsetTransition"],
                      "new_transition": cam(h)["viewOffsetTransition"]}
        app.send("flag track_object off", 1.5)
        got2, np2, s2, h, b = wait_until(
            app, "hold",
            lambda H, Bd: nav(H)["flagTraking"] == 0
            and nav(H)["plans"]["flagAutoMove"] == 0,
            "tracking released, aim held")
        # the aim the launch inherited must BE the config's init_view_pos
        lv = unit(nav(h)["localVision"])
        res["aim_before"] = {"localVision": lv,
                             "angle_to_initview_deg": angle_between(lv, initview)}
        res["transition_after_release"] = {
            "old": nav(h)["viewOffsetTransition"],
            "new": cam(h)["viewOffsetTransition"]}

        if channel == "command":
            h0, _ = app.dump("pre_command")
            lv0 = unit(nav(h0)["localVision"])
            app.send(f"set zoom_offset {offset}", 1.5)
            h1, _ = app.dump("post_command")
            lv1 = unit(nav(h1)["localVision"])
            res["P1_aim_compensation"] = {
                "localVision_before": lv0, "localVision_after": lv1,
                "angle_deg": angle_between(lv0, lv1),
                "predicted_deg": offset * 90.0,
                "mutation45_deg": offset * 45.0,
                "old_transition": nav(h1)["viewOffsetTransition"],
                "old_viewOffset": nav(h1)["viewOffset"],
                "camera_viewOffset": cam(h1)["viewOffset"],
                "control": h1.get("control", {}).get("viewOffset")}
            ok(f"P1 aim compensation measured {res['P1_aim_compensation']['angle_deg']:.4f} deg "
               f"(predicted {offset * 90.0})")

        for fov in FOVS:
            measure_point(app, res, f"{channel}_fov{int(fov)}", fov, TARGET)

        if channel == "command":
            # P6: the 90-deg coefficient read as a SLOPE, at the fov where the
            # 0.3 leg is predicted OFF the disc
            for o in (0.1, 0.2, 0.3):
                app.send(f"set zoom_offset {o}", 1.5)
                h, _ = app.dump(f"sweep_o{o}")
                res.setdefault("P6_sweep", []).append(
                    {"offset": o,
                     "localVision": unit(nav(h)["localVision"]),
                     "angle_from_initview_deg":
                         angle_between(nav(h)["localVision"], initview)})
                measure_point(app, res, f"sweep_o{int(o * 10)}_fov40", 40.0, TARGET)
    finally:
        pass
    return app, res


def main():
    argv = sys.argv[1:]
    if "--bin" in argv:
        i = argv.index("--bin"); B.SC_BIN = argv[i + 1]; del argv[i:i + 2]
    kw = {}
    for k in ("--channel", "--jd", "--initview", "--offset"):
        if k in argv:
            i = argv.index(k); kw[k[2:]] = argv[i + 1]; del argv[i:i + 2]
    stage, outdir = argv[0], Path(argv[1]).resolve()
    outdir.mkdir(parents=True, exist_ok=True)

    hits = no_instance()
    if hits:
        fail(f"another spacecrafter is running: pids {hits}")
        print(json.dumps({"fails": FAILS}))
        return 2
    gact = screensaver_active()
    real_md5_in = {p: md5(REAL_HOME / ".spacecrafter" / p)
                   for p in ("config.ini", "ssystem.ini")}

    res = {"stage": stage, "binary": B.SC_BIN, "binary_md5": md5(B.SC_BIN),
           "target": TARGET, "fovs": FOVS,
           "t_start": round(time.time(), 3),
           "display": os.environ.get("DISPLAY"),
           "screensaver_GetActive": gact,
           "no_instance_before_launch": True,
           "real_home_md5_in": real_md5_in,
           "points": [], "fails": []}
    if stage == "run":
        res["channel"] = kw["channel"]
        res["offset"] = float(kw.get("offset", 0.3))
        res["jd"] = float(kw["jd"])
        res["initview"] = [float(x) for x in kw["initview"].split(",")]

    app = None
    try:
        if stage == "prep":
            app, res = stage_prep(outdir, res)
        else:
            app, res = stage_run(outdir, res, res["channel"], res["jd"],
                                 res["initview"], res["offset"])
    finally:
        if app:
            app.close()
        res["real_home_md5_out"] = {p: md5(REAL_HOME / ".spacecrafter" / p)
                                    for p in ("config.ini", "ssystem.ini")}
        if res["real_home_md5_out"] != real_md5_in:
            fail("real ~/.spacecrafter md5 CHANGED across the launch")
        res["fails"] = FAILS
        res["t_end"] = round(time.time(), 3)
        (outdir / f"f81_{stage}{'_' + kw.get('channel', '') if stage == 'run' else ''}.json"
         ).write_text(json.dumps(res, indent=1))
    print(json.dumps({k: v for k, v in res.items()
                      if k not in ("points", "scan")}, indent=1)[:4000])
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
