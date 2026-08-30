#!/usr/bin/env python3
"""F55 PROBE 2 — which capture channel can actually SEE the app on this stack.

PROBE 1's result, which forced this one: `ffmpeg -f x11grab -i :2+0,0` returns
an all-black 2448x1332 frame for the WHOLE run (mean 0.0000, max 0 while the
app's window is mapped at +687+129) — the X11 ROOT window of an Xwayland server
under a Wayland compositor carries no composited output.  So the root-grab
channel, which is the obvious X-side one, is measured DEAD here rather than
assumed alive: that measurement is itself the channel's failure-proof running
in the failing direction.

This probe tries, in one launch and against a scene it does not measure, the
four remaining capture channels, and records for each whether it returns the
app's pixels:

  M1  ffmpeg x11grab -window_id <client window>   (XGetImage on a redirected
      window returns its backing pixmap, which is how `xwd -id` works)
  M2  ffmpeg x11grab -window_id <mutter frame window>
  M3  org.gnome.Shell.Screenshot.Screenshot over the session bus
  M4  org.gnome.Shell.Screenshot.ScreenshotWindow over the session bus

usage: f55_probe2.py <absOutdir>
"""
import glob, hashlib, json, os, re, subprocess, sys, time
from pathlib import Path

HERE = Path(__file__).resolve().parent
FARM = Path(os.environ.get("F55_FARM", "/tmp/f55_farm"))
SC_BIN = os.environ.get("SC_BIN", str(HERE.parents[1] / "build-claude/src/spacecrafter"))
REAL_HOME = Path.home()
DBUS = "unix:path=/tmp/dbus-nfWAoZGoXc"   # gnome-shell 147372's own, from /proc
PROBE_STS = "timerate rate 0\nflag atmosphere off\n"


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


def tree():
    return subprocess.run(["xwininfo", "-root", "-tree"], capture_output=True,
                          text=True, timeout=10).stdout


def windows():
    """Every window in the tree whose name or class mentions spacecrafter,
    with its geometry.  The mutter frame and the client are both matched; they
    are distinguished by which one xwininfo reports as the child."""
    out = []
    for line in tree().splitlines():
        if "spacecrafter" not in line.lower():
            continue
        m = re.search(r"(0x[0-9a-f]+).*?(\d+)x(\d+)\+(-?\d+)\+(-?\d+)\s+\+(-?\d+)\+(-?\d+)", line)
        if m:
            out.append({"id": m.group(1), "w": int(m.group(2)), "h": int(m.group(3)),
                        "abs_x": int(m.group(6)), "abs_y": int(m.group(7)),
                        "indent": len(line) - len(line.lstrip()),
                        "line": line.strip()})
    return out


def stats(png):
    import numpy as np
    from PIL import Image
    try:
        a = np.asarray(Image.open(png).convert("L"), dtype=np.float64)
    except Exception as e:
        return {"error": str(e)}
    return {"shape": list(a.shape), "mean": round(float(a.mean()), 5),
            "max": int(a.max()), "nonzero": int((a > 0).sum())}


def ffmpeg_win(win_id, png):
    t0 = time.time()
    r = subprocess.run(
        ["ffmpeg", "-hide_banner", "-loglevel", "error", "-f", "x11grab",
         "-window_id", str(int(win_id, 16)), "-framerate", "10",
         "-i", os.environ.get("DISPLAY", ":2"), "-frames:v", "1", "-y", str(png)],
        capture_output=True, text=True, timeout=30)
    return {"rc": r.returncode, "secs": round(time.time() - t0, 3),
            "stderr": r.stderr.strip()[:400],
            **({} if r.returncode else stats(png))}


def gdbus_shot(method, args, png):
    t0 = time.time()
    env = {**os.environ, "DBUS_SESSION_BUS_ADDRESS": DBUS,
           "XDG_RUNTIME_DIR": "/tmp/rt-claude"}
    r = subprocess.run(
        ["gdbus", "call", "--session", "--dest", "org.gnome.Shell.Screenshot",
         "--object-path", "/org/gnome/Shell/Screenshot",
         "--method", "org.gnome.Shell.Screenshot." + method] + args + [str(png)],
        capture_output=True, text=True, timeout=30, env=env)
    return {"rc": r.returncode, "secs": round(time.time() - t0, 3),
            "stdout": r.stdout.strip()[:200], "stderr": r.stderr.strip()[:300],
            **(stats(png) if Path(png).exists() else {})}


def main():
    out = Path(sys.argv[1]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    res = {"comm_probe_before": comm_probe(), "md5_in": frozen(),
           "binary_md5": md5(SC_BIN)}
    if res["comm_probe_before"]:
        print("ABORT: spacecrafter already running")
        return 2

    # channels tried with NO app running: the failure-proof direction
    res["no_app"] = {"M3_Screenshot": gdbus_shot("Screenshot", ["true", "false"],
                                                 out / "noapp_M3.png")}

    subprocess.run([str(HERE / "f55_farm.sh"), str(FARM)], check=True)
    farmdir = FARM / ".spacecrafter"
    (farmdir / "scripts/fscripts/startup.sts").write_text(PROBE_STS)
    (farmdir / "modularSystem/SolarSystem.ini").write_bytes(
        (REAL_HOME / ".spacecrafter/modularSystem/SolarSystem.ini.disabled").read_bytes())

    proc = subprocess.Popen([SC_BIN], cwd=str(farmdir),
                            stdout=open(out / "probe2.applog", "w"),
                            stderr=subprocess.STDOUT,
                            env={**os.environ, "HOME": str(FARM)})
    res["t_launch"] = round(time.time(), 3)
    try:
        # wait for the window, then let the app finish init and draw
        w = []
        t0 = time.time()
        while time.time() - t0 < 60 and not w:
            w = windows()
            time.sleep(0.3)
        res["windows"] = w
        time.sleep(25)                     # well past the ~10.5 s init
        res["windows_settled"] = windows()
        ids = [x["id"] for x in res["windows_settled"]]
        res["M1_first_window"] = ffmpeg_win(ids[0], out / "M1.png") if ids else None
        res["M2_second_window"] = ffmpeg_win(ids[1], out / "M2.png") if len(ids) > 1 else None
        res["M3_Screenshot"] = gdbus_shot("Screenshot", ["true", "false"], out / "M3.png")
        res["M4_ScreenshotWindow"] = gdbus_shot("ScreenshotWindow",
                                                ["true", "true", "false"], out / "M4.png")
        # repeat M3 to time the channel's cadence
        t = []
        for k in range(5):
            t0 = time.time()
            gdbus_shot("Screenshot", ["true", "false"], out / f"M3_c{k}.png")
            t.append(round(time.time() - t0, 3))
        res["M3_cadence_secs"] = t
    finally:
        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(timeout=30)
            except subprocess.TimeoutExpired:
                proc.kill()
    res["md5_out"] = frozen()
    res["md5_in_eq_out"] = res["md5_in"] == res["md5_out"]
    res["comm_probe_after"] = comm_probe()
    (out / "f55_probe2.json").write_text(json.dumps(res, indent=1))
    print(json.dumps(res, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
