#!/usr/bin/env python3
"""F55 PROBE — the instrument-chain probe for the first-60 s photometric
sampler (§11.167(j)(2)).  It measures the MECHANICS and deliberately measures
NO Moon disc: it runs a startup script that establishes no scene, so nothing it
records can pre-empt the predictions committed in `f55_predictions.json`.

WHAT IT ESTABLISHES, each of which the sampler needs and none of which is
guessable:

  1. the app's on-screen window RECT on `:2` (config `screen_w/h = 1024`,
     `fullscreen = false` [observed: ~/.spacecrafter/config.ini:26-31], so the
     window is placed by the compositor and the crop cannot be assumed);
  2. the wall clock at which the screen STOPS being the no-app state — the
     X-side channel's own failure-proof runs here: every frame captured before
     the app draws must show the empty desktop;
  3. the SDL_GetTicks -> wall-clock anchor.  `cLog::write` prefixes every LOG
     FILE line with `SDL_GetTicks()` in ms when isDebug is set, and flushes
     after every line [observed: log.cpp:122-127, 151-156]; §11.167(c) recorded
     "the applog carries no timestamps", which is true of STDOUT (writeConsole
     appends no prefix) and false of the log files.  A poller that stamps each
     newly-appeared line with wall clock therefore yields (ticks, wall) pairs;
     the fit's slope must be 1.000 and its residual spread is the anchor's
     uncertainty;
  4. that a startup.sts written into the F55 farm is LOADED AND EXECUTED, and
     at what ticks — the sampler's whole scene-establishment channel.

usage: f55_probe.py <absOutdir>
"""
import glob, hashlib, json, os, re, socket, subprocess, sys, time
from pathlib import Path

HERE = Path(__file__).resolve().parent
FARM = Path(os.environ.get("F55_FARM", "/tmp/f55_farm"))
SC_BIN = os.environ.get("SC_BIN", str(HERE.parents[1] / "build-claude/src/spacecrafter"))
REAL_HOME = Path.home()
SCREEN = "2448x1332"
TICKS_RE = re.compile(r"^(\d{12}): ")

# The probe's startup script: no scene, no body, no view change.  Two commands
# that are observable in the script log and change nothing photometric.
PROBE_STS = "timerate rate 0\nflag atmosphere off\n"


def comm_probe():
    """/proc/<pid>/comm probe (§11.134(b)) — every account, no self-match."""
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


def frozen_pair():
    return {p: md5(REAL_HOME / ".spacecrafter" / p)
            for p in ("config.ini", "ssystem.ini")}


def win_rect():
    """The app's client-area rect on the screen, or None.  Parsed from
    `xwininfo -root -tree`; the app's window carries the binary's name."""
    try:
        tree = subprocess.run(["xwininfo", "-root", "-tree"], capture_output=True,
                              text=True, timeout=10).stdout
    except Exception:
        return None
    for line in tree.splitlines():
        if "spacecrafter" not in line.lower():
            continue
        m = re.search(r"(0x[0-9a-f]+).*?(\d+)x(\d+)\+(-?\d+)\+(-?\d+)\s+\+(-?\d+)\+(-?\d+)", line)
        if m and int(m.group(2)) >= 256:
            return {"id": m.group(1), "w": int(m.group(2)), "h": int(m.group(3)),
                    "abs_x": int(m.group(6)), "abs_y": int(m.group(7)),
                    "line": line.strip()}
    return None


class LogPoller:
    """Stamps every newly-appeared log-file line with the wall clock at which
    it was first SEEN.  cLog flushes per line, so first-seen is an upper bound
    on the write time, tight to one poll interval."""

    def __init__(self, logdir, period=0.05):
        self.dir, self.period, self.pos, self.lines = Path(logdir), period, {}, []

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

    def sleep_poll(self, until):
        while time.time() < until:
            self.poll()
            time.sleep(self.period)
        self.poll()

    def anchor(self):
        """Least-squares fit wall = a + b*ticks over every stamped line that
        carries a ticks prefix.  b is forced to 1.0 (the app's clock and ours
        are the same monotonic second); the free fit is reported beside it as
        the check that they are."""
        pts = [(int(TICKS_RE.match(r["line"]).group(1)) / 1000.0, r["t"])
               for r in self.lines if TICKS_RE.match(r["line"])]
        if len(pts) < 10:
            return {"n": len(pts), "error": "too few ticked lines"}
        xs = [p[0] for p in pts]
        ys = [p[1] for p in pts]
        n = len(pts)
        mx, my = sum(xs) / n, sum(ys) / n
        sxx = sum((x - mx) ** 2 for x in xs)
        sxy = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
        b = sxy / sxx if sxx else float("nan")
        a = my - b * mx
        # forced-slope-1 offset: wall = ticks + t0.  Each line's own offset is
        # an UPPER bound (first-seen lags the write), so the MINIMUM offset is
        # the least-contaminated estimate; the median is reported beside it.
        offs = sorted(y - x for x, y in pts)
        return {"n": n, "free_slope": round(b, 6), "free_intercept": round(a, 3),
                "t0_min": round(offs[0], 3), "t0_median": round(offs[n // 2], 3),
                "t0_p90": round(offs[int(0.9 * n)], 3),
                "spread_min_to_median": round(offs[n // 2] - offs[0], 3)}


def main():
    out = Path(sys.argv[1]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    frames = out / "screen"
    frames.mkdir(exist_ok=True)
    for f in frames.glob("*.png"):
        f.unlink()

    res = {"binary": SC_BIN, "binary_md5": md5(SC_BIN),
           "comm_probe_before": comm_probe(), "md5_in": frozen_pair(),
           "display": os.environ.get("DISPLAY"), "screen": SCREEN}
    if res["comm_probe_before"]:
        print("ABORT: a spacecrafter is already running", res["comm_probe_before"])
        return 2

    subprocess.run([str(HERE / "f55_farm.sh"), str(FARM)], check=True)
    farmdir = FARM / ".spacecrafter"
    (farmdir / "scripts/fscripts/startup.sts").write_text(PROBE_STS)
    res["startup_sts"] = PROBE_STS
    # The composed twin, exactly as b3_ladder/f51 write it for the moon site
    # (drop_sections = () there), so this probe's LOAD is the sampler's load
    # and the timings it measures transfer.
    twin = (REAL_HOME / ".spacecrafter/modularSystem/SolarSystem.ini.disabled").read_bytes()
    (farmdir / "modularSystem/SolarSystem.ini").write_bytes(twin)
    res["twin_md5"] = hashlib.md5(twin).hexdigest()

    # --- the X-side channel, started BEFORE the app exists -------------------
    ff = subprocess.Popen(
        ["ffmpeg", "-hide_banner", "-loglevel", "warning", "-f", "x11grab",
         "-framerate", "5", "-video_size", SCREEN, "-i",
         os.environ.get("DISPLAY", ":2") + "+0,0", "-t", "70", "-pix_fmt", "rgb24",
         str(frames / "f%05d.png")],
        stdout=subprocess.DEVNULL, stderr=open(out / "ffmpeg.log", "w"))
    res["t_ffmpeg"] = round(time.time(), 3)
    time.sleep(3.0)                      # >= 10 no-app frames, the failure-proof
    res["win_before_launch"] = win_rect()

    poller = LogPoller(farmdir / "log")
    applog = out / "probe.applog"
    proc = subprocess.Popen([SC_BIN], cwd=str(farmdir),
                            stdout=open(applog, "w"), stderr=subprocess.STDOUT,
                            env={**os.environ, "HOME": str(FARM)})
    res["t_launch"] = round(time.time(), 3)
    res["window_appeared"] = None
    try:
        t_end = time.time() + 45
        while time.time() < t_end:
            poller.sleep_poll(min(t_end, time.time() + 0.5))
            if res["window_appeared"] is None:
                w = win_rect()
                if w:
                    res["window_appeared"] = {"t": round(time.time(), 3), **w}
        # the TCP port: the moment F51's series could first have opened
        t0 = time.time()
        s = None
        while time.time() - t0 < 60 and s is None:
            try:
                s = socket.create_connection(("127.0.0.1", 7805), timeout=1)
            except OSError:
                poller.poll()
                time.sleep(0.25)
        res["t_port"] = round(time.time(), 3) if s else None
        poller.poll()
        if s:
            s.sendall(b"shutdown action now\n")
            s.close()
        try:
            proc.wait(timeout=40)
        except subprocess.TimeoutExpired:
            proc.kill()
    finally:
        if proc.poll() is None:
            proc.kill()
        poller.poll()
        try:
            ff.wait(timeout=80)
        except subprocess.TimeoutExpired:
            ff.terminate()

    res["md5_out"] = frozen_pair()
    res["md5_in_eq_out"] = res["md5_in"] == res["md5_out"]
    res["comm_probe_after"] = comm_probe()
    res["anchor"] = poller.anchor()
    res["log_files"] = sorted(p.name for p in (farmdir / "log").iterdir())
    (out / "f55_probe_loglines.json").write_text(json.dumps(poller.lines, indent=0))

    # frame inventory: mtime series + mean luminance, the no-app control first
    import numpy as np
    from PIL import Image
    inv = []
    for p in sorted(frames.glob("*.png")):
        a = np.asarray(Image.open(p).convert("L"), dtype=np.float64)
        inv.append({"f": p.name, "mtime": round(p.stat().st_mtime, 3),
                    "mean": round(float(a.mean()), 5), "max": int(a.max()),
                    "bytes": p.stat().st_size})
    res["n_frames"] = len(inv)
    (out / "f55_probe_frames.json").write_text(json.dumps(inv, indent=0))
    (out / "f55_probe.json").write_text(json.dumps(res, indent=1))
    print(json.dumps(res, indent=1))
    return 0 if res["md5_in_eq_out"] else 3


if __name__ == "__main__":
    sys.exit(main())
