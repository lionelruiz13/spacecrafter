#!/usr/bin/env python3
"""F51 (A) — the dim-Moon discriminator: ONE launch, a minutes-long dwell on the
Moon disc, the disc's mean luminance sampled on a fixed cadence, and the
applog's texture events interleaved with the samples by the app's own command
echo (§11.164(e)(1)).

THE SCENE IS NOT A NEW ONE.  It is b3_ladder's moon `base` scene — the very
scene §11.164(c) measured the 2.7x gap in — reproduced by IMPORTING
`b3_ladder` rather than restating it, so the site, the settle discipline
(§5.109) and the free-mode longitude convention (§11.153/F40) have exactly one
authority.  The only departures from `b3_ladder.run_scene`, each deliberate:

  * no composed ladder sections (this is the `base` scene, `sections = ""`);
  * no `wide` shot: the dwell samples at FOV 10 from the first frame, and its
    sample 0 is taken in the SAME app state F48's `terrain_base_zoom.png` was
    (`flag experimental_shadows off`, `zoom fov 10`), so sample 0 is a
    reproduction control against a committed number rather than a new baseline;
  * the dwell, and after it three extra legs (P3 in the prediction file).

WHY THE EXTRA LEGS EXIST, stated before the run and committed with it: the
three shapes §11.164(e)(1) names are NOT three on a luminance series alone —
"the upload never lands" and "the shading changed" predict the SAME flat dim
series, and the three Moon texture events are logged BEFORE the first TCP
command in every 2026-08-29 applog, i.e. before the dwell window can open.  So
the series can refute a minutes-late upload and nothing else.  What separates
the two survivors is whether the image CONTENT of `textures/bodies/moon.jpg`
is on the GPU at all, and the old render path is the instrument for that: it
draws the Moon from the SAME `s_texture` cache entry
[observed: ~/.spacecrafter/ssystem.ini `[moon] tex_map = bodies/moon.jpg`;
modularSystem/SolarSystem.ini.disabled `[Moon] tex_map = bodies/moon.jpg`;
applog `s_texture: already in cache textures/bodies/moon.jpg`], through a
different shader.  An uninitialized image is uninitialized for both.

usage: f51_dwell.py <absOutdir> [--samples N] [--cadence S] [--bin PATH]
       (run it through f51_run.sh, which builds the temp-HOME farm and asserts
        the real ~/.spacecrafter md5 in == out)
"""
import hashlib, json, os, socket, subprocess, sys, time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import b3_ladder as B          # the site + the app-driving plumbing, one authority
import f51_disc as D           # the metric, calibrated against §11.164(c)

AU_KM = 149597870.0
SAMPLES = 80
CADENCE = 5.0
FOV = 10.0                     # F48's zoom frame, the frame the gap was measured on


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def shot(s, png, pause=0.3, timeout=20.0):
    """Screenshot + wait for a COMPLETE file.  Returns (t_cmd, t_ready)."""
    p = Path(png)
    if p.exists():
        p.unlink()
    t0 = time.time()
    B.send(s, f"body action screenshot filename {p}", pause)
    last = -1
    while time.time() - t0 < timeout:
        if p.exists():
            sz = p.stat().st_size
            if sz > 0 and sz == last:
                return t0, time.time()
            last = sz
        time.sleep(0.15)
    return t0, None


def sample(s, png, tag, idx):
    t_cmd, t_ready = shot(s, png)
    rec = {"i": idx, "tag": tag, "t_cmd": round(t_cmd, 3),
           "t_ready": round(t_ready, 3) if t_ready else None,
           "png": Path(png).name}
    if t_ready is None:
        rec["error"] = "screenshot never completed"
        return rec
    rec.update(D.metrics(png))
    rec["md5"] = md5(png)
    rec.pop("png", None)
    rec["png"] = Path(png).name
    return rec


def dump(s, out, name):
    p = out / name
    if p.exists():
        p.unlink()
    B.send(s, f"body action dual_dump filename {p}", 2)
    for _ in range(40):
        if p.exists() and p.stat().st_size > 0:
            break
        time.sleep(0.2)
    return B.load_dump(p) if p.exists() else {}


def main():
    argv = sys.argv[1:]
    n_samples, cadence = SAMPLES, CADENCE
    if "--samples" in argv:
        i = argv.index("--samples"); n_samples = int(argv[i + 1]); del argv[i:i + 2]
    if "--cadence" in argv:
        i = argv.index("--cadence"); cadence = float(argv[i + 1]); del argv[i:i + 2]
    if "--bin" in argv:
        i = argv.index("--bin"); B.SC_BIN = argv[i + 1]; del argv[i:i + 2]
    out = Path(argv[0]).resolve(); out.mkdir(parents=True, exist_ok=True)
    frames = out / "frames"; frames.mkdir(exist_ok=True)

    S = B.S                                     # b3_ladder's moon site
    farmdir = B.FARM / ".spacecrafter"
    twin = B.drop_sections(
        (B.REAL_HOME / ".spacecrafter/modularSystem/SolarSystem.ini.disabled").read_bytes(),
        S["drop_sections"])
    (farmdir / "modularSystem/SolarSystem.ini").write_bytes(twin)   # base scene: no sections
    for f in (farmdir / "log").glob("*.log"):
        f.unlink()

    env = {**os.environ, "HOME": str(B.FARM), "DISPLAY": os.environ.get("DISPLAY", ":2")}
    applog = out / "dwell.applog"
    res = {"binary": B.SC_BIN, "binary_md5": md5(B.SC_BIN),
           "site": "moon(base)", "fov": FOV, "cadence_s": cadence,
           "n_samples": n_samples, "convention": B.CONVENTION,
           "commanded_lon": B.obs_lon_cam(), "nadir_lon": B.nadir_lon(),
           "metric": "f51_disc.metrics: L=PIL convert('L'), Bl=GaussianBlur(4), "
                     "mask r<900 about ((w-1)/2,(h-1)/2) and L>8",
           "t_launch": None, "samples": [], "legs": [], "fails": []}

    def save():
        (out / "f51_dwell.json").write_text(json.dumps(res, indent=1))

    proc = subprocess.Popen([B.SC_BIN], cwd=str(farmdir),
                            stdout=open(applog, "w"), stderr=subprocess.STDOUT, env=env)
    res["t_launch"] = round(time.time(), 3)
    try:
        s = B.wait_port(); time.sleep(10)
        res["t_port"] = round(time.time(), 3)
        B.send(s, "flag experimental_path on"); B.send(s, "timerate rate 0")
        B.send(s, "meteors zhr 0"); B.send(s, f"date jday {S['jd']}")
        B.send(s, f"set home_planet {S['parent']}", 2)
        B.send(s, "camera action free_mode state on")
        B.send(s, "flag atmosphere off"); B.send(s, "flag landscape off")
        for c in S["scale_off"]:
            B.send(s, c, 2)
        B.wait_scale_settled(s, out, "dwell")           # §5.109, by measurement
        B.send(s, f"select planet {S['parent']}")
        B.send(s, f"moveto lat {S['obs_lat']} lon {B.obs_lon_cam()} "
                  f"alt {S['obs_alt_m']} duration 0", 5)
        B.send(s, "flag track_object on", 2)
        B.send(s, f"zoom fov {FOV} duration 0", 2)
        B.send(s, "flag track_object off", 2)
        B.send(s, "flag experimental_shadows off", 2)   # F48's zoom-frame state
        d0 = dump(s, out, "dwell_dump_start.json")
        res["dump_start_moon"] = d0.get(S["parent"], {})
        res["fails"] += list(B.FAILS)
        save()

        t0 = time.time()
        res["t_dwell_start"] = round(t0, 3)
        for k in range(n_samples):
            target = t0 + k * cadence
            dt = target - time.time()
            if dt > 0:
                time.sleep(dt)
            rec = sample(s, frames / f"s{k:03d}.png", "dwell", k)
            res["samples"].append(rec)
            if k % 5 == 0 or k == n_samples - 1:
                save()
                print(f"s{k:03d} t+{rec['t_cmd']-t0:7.1f}s disc_mean "
                      f"{rec.get('disc_mean')} hf {rec.get('hf_mean')}", flush=True)
        res["t_dwell_end"] = round(time.time(), 3)
        d1 = dump(s, out, "dwell_dump_end.json")
        res["dump_end_moon"] = d1.get(S["parent"], {})
        save()

        # ---- P3: the old-path control, same launch --------------------------
        B.send(s, "flag experimental_path off", 3)
        res["legs"].append(sample(s, frames / "old_a.png", "oldpath_fov10", 0))
        B.send(s, "zoom fov 60 duration 0", 2)
        res["legs"].append(sample(s, frames / "old_wide.png", "oldpath_fov60", 1))
        B.send(s, f"zoom fov {FOV} duration 0", 2)
        d2 = dump(s, out, "dwell_dump_old.json")
        res["dump_old_moon"] = d2.get(S["parent"], {})
        time.sleep(10)
        res["legs"].append(sample(s, frames / "old_b.png", "oldpath_fov10_plus10s", 2))
        B.send(s, "flag experimental_path on", 3)
        res["legs"].append(sample(s, frames / "new_after.png", "newpath_after", 3))
        time.sleep(10)
        res["legs"].append(sample(s, frames / "new_after_b.png", "newpath_after_plus10s", 4))
        save()

        B.send(s, "shutdown action now", 1); s.close()
        try:
            proc.wait(timeout=40)
        except subprocess.TimeoutExpired:
            proc.kill()
    finally:
        if proc.poll() is None:
            proc.kill()
        res["fails"] += [f for f in B.FAILS if f not in res["fails"]]
        save()

    print(json.dumps({k: v for k, v in res.items()
                      if k not in ("samples", "legs", "dump_start_moon",
                                   "dump_end_moon", "dump_old_moon")}, indent=1))
    return 1 if res["fails"] else 0


if __name__ == "__main__":
    sys.exit(main())
