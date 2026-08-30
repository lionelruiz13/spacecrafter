#!/usr/bin/env python3
"""F55 — the first-60 s photometric sampler: ONE launch, the Moon disc sampled
from the app's FIRST DRAWN FRAME, on two channels neither of which is the TCP
command path (§11.167(j)(2)).

WHY THIS EXISTS.  F51's dwell series is flat over 355 s and refutes a
minutes-late texture upload, but it opens ~60 s after the port does, and the
Moon's three `creating uninitialized texture` events fire BEFORE the first TCP
command — so §11.167(c) recorded the limit in the same breath as the result:
*"the dwell window opens ~60 s after the only moment a late upload could have
shown"*.  This driver closes that interval.

THE THREE CHANNELS, and why each is outside TCP:

  A  the app's own STARTUP SCRIPT (`<HOME>/.spacecrafter/scripts/fscripts/
     startup.sts`, played at the end of App::init [observed: app.cpp:689,
     script_mgr.cpp:384-388]).  It establishes the scene AND takes the dense
     early photometric samples through `body action screenshot`, which is the
     app's own 2048x2048 readback — i.e. F51's exact frame format and F51's
     exact metric.  No socket exists yet when it starts.
  B  an X-side `ffmpeg -f x11grab -window_id <client>` capture of the app's
     1024x1024 on-screen window, started as soon as the window is mapped and
     running continuously.  A separate process reading the X server; the app is
     never asked for these frames.  It is channel A's independent witness.
     [The obvious X-side channel, the ROOT grab, is MEASURED DEAD on this stack
      — an all-black 2448x1332 frame for a whole run while the window is mapped
      (probe 1) — which is what `app_command_interface.cpp:4035-4039` means by
      "external grabs see black".  The WINDOW grab is not covered by that
      comment and does carry the rendered scene (probe 2).]
  C  the app's own LOG FILES, polled at 20 Hz and stamped with the wall clock
     at which each new line first appears.  `cLog::write` prefixes every log
     FILE line with `SDL_GetTicks()` in ms and flushes per line [observed:
     log.cpp:122-127, 151-156], so `vulkan.log` timestamps every texture event.
     §11.167(c)'s "the applog carries no timestamps" is true of STDOUT only.

THE SCENE is F51's under a similarity of factor 5 — see `f55_predictions.json`,
`scene_definition`: `moon_scaled` is left at its CONFIGURED value (config.ini
`flag_moon_scaled = true`, `moon_scale = 5`), which since `d6aec251` is applied
at init as a state rather than ramped, so there is no §5.109 settle to wait for
and the disc is at its final geometry on the first drawn frame.  `alt` is 5x
F51's, so the observer radius is 5x and every angle — including the disc's
10.28 deg — is F51's.  Phase 2 then reaches F51's scene EXACTLY over TCP, which
is both the butt-join with F51's series and the control on that similarity
argument.

usage: f55_sampler.py <absOutdir> [--burst N] [--no-burst] [--fps N]
       (via f55_run.sh, which asserts the real ~/.spacecrafter md5 in == out)
"""
import glob, hashlib, json, os, re, socket, subprocess, sys, time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import b3_ladder as B          # the site, `send`, `load_dump`, `wait_scale_settled`

FARM = Path(os.environ.get("F55_FARM", "/tmp/f55_farm"))
REAL_HOME = Path.home()
AU_KM = 149597870.0
TICKS_RE = re.compile(r"^(\d{12}): ")
PORT = 7805

FOV = 10.0
ALT_M = 40000000            # 5 x F51's 8 000 000 m; see the module docstring
N_FAST, DT_FAST = 30, 0.02  # the dense burst: ~3 app frames apart at 144 fps
N_MID, DT_MID = 30, 0.5
N_SLOW, DT_SLOW = 40, 1.0   # N_FAST*DT_FAST + N_MID*DT_MID + N_SLOW*DT_SLOW ~ 55 s


def comm_probe():
    out = []
    for c in glob.glob("/proc/[0-9]*/comm"):
        try:
            if open(c).read().strip() == "spacecrafter":
                out.append(c.split("/")[2])
        except OSError:
            pass
    return out


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def frozen():
    return {p: md5(REAL_HOME / ".spacecrafter" / p)
            for p in ("config.ini", "ssystem.ini")}


def client_window():
    """The app's CLIENT window (not mutter's frame): xwininfo reports the frame
    with class `mutter-x11-frames` and the client with class `spacecrafter`."""
    try:
        tree = subprocess.run(["xwininfo", "-root", "-tree"], capture_output=True,
                              text=True, timeout=5).stdout
    except Exception:
        return None
    for line in tree.splitlines():
        if '("spacecrafter" "spacecrafter")' not in line:
            continue
        m = re.search(r"(0x[0-9a-f]+).*?(\d+)x(\d+)\+(-?\d+)\+(-?\d+)\s+\+(-?\d+)\+(-?\d+)", line)
        if m:
            return {"id": m.group(1), "w": int(m.group(2)), "h": int(m.group(3)),
                    "abs_x": int(m.group(6)), "abs_y": int(m.group(7)),
                    "line": line.strip()}
    return None


class LogPoller:
    def __init__(self, logdir):
        self.dir, self.pos, self.lines = Path(logdir), {}, []

    def poll(self):
        now = time.time()
        for p in sorted(self.dir.glob("*.log")) + sorted(self.dir.glob("*.txt")):
            try:
                sz = p.stat().st_size
            except OSError:
                continue
            off = self.pos.get(p.name, 0)
            if sz <= off:
                continue
            with open(p, "rb") as f:
                f.seek(off)
                buf = f.read(sz - off)
            keep = buf.rfind(b"\n") + 1
            self.pos[p.name] = off + keep
            for ln in buf[:keep].decode("utf-8", "replace").splitlines():
                self.lines.append({"t": round(now, 3), "file": p.name, "line": ln})

    def anchor(self):
        pts = [(int(TICKS_RE.match(r["line"]).group(1)) / 1000.0, r["t"])
               for r in self.lines if TICKS_RE.match(r["line"])]
        if len(pts) < 10:
            return {"n": len(pts), "error": "too few ticked lines"}
        offs = sorted(y - x for x, y in pts)
        n = len(offs)
        xs = [p[0] for p in pts]; ys = [p[1] for p in pts]
        mx, my = sum(xs) / n, sum(ys) / n
        sxx = sum((x - mx) ** 2 for x in xs)
        sxy = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
        return {"n": n, "t0_min": round(offs[0], 3), "t0_median": round(offs[n // 2], 3),
                "t0_p90": round(offs[int(0.9 * n)], 3),
                "median_minus_min": round(offs[n // 2] - offs[0], 3),
                "free_slope": round(sxy / sxx, 6) if sxx else None}


def build_sts(shotdir, burst=True):
    """The startup script.  Every command before the first `wait` executes in
    the SAME main-loop batch (the 400 ms deadline loop, script_mgr.cpp:301-303),
    so the scene is established and the first screenshot requested inside the
    first drawn frame or two."""
    L = ["timerate rate 0",
         "meteors zhr 0",
         f"date jday {B.SITES['moon']['jd']}",
         "flag atmosphere off",
         "flag landscape off",
         "flag experimental_path on",
         "set home_planet Moon",
         "camera action free_mode state on",
         f"moveto lat 0.0 lon {B.obs_lon_cam()} alt {ALT_M} duration 0",
         "select planet Moon",
         "flag track_object on",
         f"zoom fov {FOV} duration 0",
         "flag experimental_shadows off"]
    k = 0
    if burst:
        for n, dt in ((N_FAST, DT_FAST), (N_MID, DT_MID), (N_SLOW, DT_SLOW)):
            for _ in range(n):
                L.append(f"body action screenshot filename {shotdir}/e{k:03d}.png")
                L.append(f"wait duration {dt}")
                k += 1
    L.append("flag track_object off")
    return "\n".join(L) + "\n", k


def main():
    argv = sys.argv[1:]
    burst, fps = True, 10
    if "--no-burst" in argv:
        burst = False; argv.remove("--no-burst")
    if "--fps" in argv:
        i = argv.index("--fps"); fps = int(argv[i + 1]); del argv[i:i + 2]
    out = Path(argv[0]).resolve(); out.mkdir(parents=True, exist_ok=True)
    shots = out / "sts"; shots.mkdir(exist_ok=True)
    xdir = out / "x"; xdir.mkdir(exist_ok=True)
    for d in (shots, xdir):
        for f in d.glob("*.png"):
            f.unlink()

    res = {"binary": B.SC_BIN, "binary_md5": md5(B.SC_BIN),
           "predictions_md5": md5(HERE / "f55_predictions.json"),
           "comm_probe_before": comm_probe(), "md5_in": frozen(),
           "fov": FOV, "alt_m": ALT_M, "burst": burst, "x_fps": fps,
           "commanded_lon": B.obs_lon_cam(), "convention": B.CONVENTION,
           "marks": {}, "fails": []}
    if res["comm_probe_before"]:
        print("ABORT: a spacecrafter is already running:", res["comm_probe_before"])
        return 2

    subprocess.run([str(HERE / "f55_farm.sh"), str(FARM)], check=True)
    farmdir = FARM / ".spacecrafter"
    twin = (REAL_HOME / ".spacecrafter/modularSystem/SolarSystem.ini.disabled").read_bytes()
    (farmdir / "modularSystem/SolarSystem.ini").write_bytes(twin)
    res["twin_md5"] = hashlib.md5(twin).hexdigest()
    sts, n_shots = build_sts(shots, burst)
    (farmdir / "scripts/fscripts/startup.sts").write_text(sts)
    (out / "startup.sts").write_text(sts)
    res["n_script_shots"] = n_shots
    res["startup_sts_md5"] = hashlib.md5(sts.encode()).hexdigest()

    poller = LogPoller(farmdir / "log")
    env = {**os.environ, "HOME": str(FARM)}
    proc = subprocess.Popen([B.SC_BIN], cwd=str(farmdir),
                            stdout=open(out / "run.applog", "w"),
                            stderr=subprocess.STDOUT, env=env)
    res["t_launch"] = round(time.time(), 3)
    ff = None
    sock = None
    try:
        # ---- channel B: start the window grab the instant the window exists --
        t0 = time.time()
        win = None
        while time.time() - t0 < 40 and win is None:
            win = client_window()
            poller.poll()
            if win is None:
                time.sleep(0.03)
        res["window"] = win
        res["t_window"] = round(time.time(), 3)
        if win:
            ff = subprocess.Popen(
                ["ffmpeg", "-hide_banner", "-loglevel", "warning", "-f", "x11grab",
                 "-window_id", str(int(win["id"], 16)), "-framerate", str(fps),
                 "-draw_mouse", "0", "-i", os.environ.get("DISPLAY", ":2"),
                 "-t", "300", "-pix_fmt", "rgb24", str(xdir / "x%05d.png")],
                stdout=subprocess.DEVNULL, stderr=open(out / "ffmpeg.log", "w"))
            res["t_xgrab"] = round(time.time(), 3)

        # ---- the port: recorded, NOT used until the script has finished ------
        t_end = res["t_launch"] + 200
        scanned, script_done = 0, False
        while time.time() < t_end:
            poller.poll()
            if sock is None:
                try:
                    sock = socket.create_connection(("127.0.0.1", PORT), timeout=0.2)
                    res["marks"]["t_port"] = round(time.time(), 3)
                except OSError:
                    pass
            while scanned < len(poller.lines):
                if "ScriptMgr: script end" in poller.lines[scanned]["line"] \
                   and time.time() > res["t_launch"] + 30:
                    res["marks"]["t_script_end"] = round(poller.lines[scanned]["t"], 3)
                    script_done = True
                scanned += 1
            if script_done:
                break
            time.sleep(0.05)
        poller.poll()
        if sock is None:
            sock = B.wait_port()
            res["marks"]["t_port"] = round(time.time(), 3)

        # ---- phase 1 close-out: the app-side frame + the dump ---------------
        s = sock
        res["marks"]["t_first_tcp_cmd"] = round(time.time(), 3)
        B.send(s, "flag track_object off", 1.0)
        p = out / "p1_app.png"
        B.send(s, f"body action screenshot filename {p}", 2.5)
        res["marks"]["t_p1_shot"] = round(time.time(), 3)
        d = out / "p1_dump.json"
        B.send(s, f"body action dual_dump filename {d}", 2.0)
        for _ in range(40):
            if d.exists() and d.stat().st_size:
                break
            time.sleep(0.2)
        m1 = (B.load_dump(d) or {}).get("Moon", {})
        res["p1_moon"] = m1
        # the dump's `dist` IS the observer->body-centre distance: F51's
        # 6.50904985e-05 AU * AU_KM = 9737.4 km, the site's own radius.
        res["p1_obs_radius_km"] = round(m1["dist"] * AU_KM, 3) if m1.get("dist") else None
        res["p1_scaledDatumRadius_km"] = (round(m1["scaledDatumRadius"] * AU_KM, 3)
                                          if m1.get("scaledDatumRadius") else None)

        # ---- phase 2: F51 EXACTLY, the settle waited BY MEASUREMENT ----------
        res["marks"]["t_phase2_start"] = round(time.time(), 3)
        B.send(s, "flag moon_scaled off", 2)
        res["p2_settled_km"] = B.wait_scale_settled(s, out, "p2")
        res["marks"]["t_p2_settled"] = round(time.time(), 3)
        B.send(s, "select planet Moon")
        B.send(s, f"moveto lat 0.0 lon {B.obs_lon_cam()} alt 8000000 duration 0", 5)
        B.send(s, "flag track_object on", 2)
        B.send(s, f"zoom fov {FOV} duration 0", 2)
        B.send(s, "flag track_object off", 2)
        p2 = out / "p2_app.png"
        B.send(s, f"body action screenshot filename {p2}", 2.5)
        res["marks"]["t_p2_shot"] = round(time.time(), 3)
        d2 = out / "p2_dump.json"
        B.send(s, f"body action dual_dump filename {d2}", 2.0)
        for _ in range(40):
            if d2.exists() and d2.stat().st_size:
                break
            time.sleep(0.2)
        m2 = (B.load_dump(d2) or {}).get("Moon", {})
        res["p2_moon"] = m2
        res["p2_obs_radius_km"] = round(m2["dist"] * AU_KM, 3) if m2.get("dist") else None
        res["p2_scaledDatumRadius_km"] = (round(m2["scaledDatumRadius"] * AU_KM, 3)
                                          if m2.get("scaledDatumRadius") else None)
        time.sleep(3)
        res["marks"]["t_end"] = round(time.time(), 3)
        poller.poll()
        B.send(s, "shutdown action now", 1)
        s.close()
        try:
            proc.wait(timeout=40)
        except subprocess.TimeoutExpired:
            proc.kill()
    finally:
        if proc.poll() is None:
            proc.kill()
        poller.poll()
        if ff and ff.poll() is None:
            ff.terminate()
            try:
                ff.wait(timeout=30)
            except subprocess.TimeoutExpired:
                ff.kill()

    res["md5_out"] = frozen()
    res["md5_in_eq_out"] = res["md5_in"] == res["md5_out"]
    res["comm_probe_after"] = comm_probe()
    res["anchor"] = poller.anchor()
    res["fails"] += list(B.FAILS)
    (out / "f55_loglines.json").write_text(json.dumps(poller.lines, indent=0))
    # raw file inventories: name + mtime, the wall clock of every sample
    for tag, d in (("sts", shots), ("x", xdir)):
        inv = [{"f": p.name, "mtime": round(p.stat().st_mtime, 3),
                "bytes": p.stat().st_size} for p in sorted(d.glob("*.png"))]
        res[f"n_{tag}"] = len(inv)
        (out / f"f55_inv_{tag}.json").write_text(json.dumps(inv, indent=0))
    (out / "f55_run.json").write_text(json.dumps(res, indent=1))
    print(json.dumps({k: v for k, v in res.items()
                      if k not in ("p1_moon", "p2_moon")}, indent=1))
    return 1 if (res["fails"] or not res["md5_in_eq_out"]) else 0


if __name__ == "__main__":
    sys.exit(main())
