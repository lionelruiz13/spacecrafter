#!/usr/bin/env python3
"""§5.63 attribution probe (F21 step 0, BOUNDED).

§5.63 says: a restored session reproduces every field of the camera dump and the
composed screen still differs by 3185 px>8, photometrically (+7.7/8-bit over the
differing pixels, lit>32 441 saved vs 625 restored, no displacement structure).
The row's own owed check is written into it: *"hold the restored scene and the
rebuilt scene side by side with the sky content off, which separates a
photometric sky term from everything else in one run."*  This is that run, made
into a BISECTION: the same scene is shot at a ladder of stages, each stage
turning off one content family, on three launches:

  L1  the scene built by commands  -> the SAVED side (and the session file)
  L2  the same scene built by the same commands, fresh launch -> the A/A FLOOR
  L3  a fresh launch that RESTORES the session -> the RESTORED side

For each stage the probe reports  |restored - saved|  and the floor
|A/A - saved|.  The stage at which the difference COLLAPSES names the content
family that carries §5.63's residual; the floor at the same stage is what makes
that statement mean anything (§11.80(a): the floor is measured in-scene).

Two riders the row also needs:
  * TRANSIENCE.  The restored side is shot twice more, 15 s and 40 s after the
    restore, without touching anything.  A term that DECAYS is a settling state
    (D32 class); a term that does not is persistent state.
  * PHOTOMETRY.  Per stage, the mean signed difference over the differing pixels
    and the lit>32 counts on both sides - the numbers §5.63 itself quotes, so the
    comparison with the row is direct.

    cd claude/harness && DISPLAY=:2 ./f21_s563.py [outdir]
"""

import hashlib
import json
import os
import socket
import subprocess
import sys
import time
from pathlib import Path

import numpy as np
from PIL import Image

HOME = Path.home()
USERDIR = HOME / ".spacecrafter"
SESSIONS = USERDIR / "sessions"
SC_BIN = os.environ.get("SC_BIN", str(Path(__file__).resolve().parents[2] / "build-claude/src/spacecrafter"))
_args = [a for a in sys.argv[1:] if not a.startswith("--")]
OUT = (Path(_args[0]) if _args else Path(__file__).resolve().parent / "artifacts/f21s563").resolve()
OUT.mkdir(parents=True, exist_ok=True)

FROZEN = ["config.ini", "ssystem.ini", "galactic.ini", "anchor.ini"]
JD = 2461233.5

# The ladder. Cumulative: each stage removes one more content family, so the
# DROP at a stage is that family's contribution.
STAGES = [
    ("base", None),
    ("nostars", "flag stars off"),
    ("nomw", "flag milky_way off"),
    ("noneb", "flag nebulae off"),
    ("noatm", "flag atmosphere off"),
    ("noland", "flag landscape off"),
    ("noplanets", "flag planets off"),
]


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def assert_no_other_instance():
    r = subprocess.run(["ps", "-e", "-o", "pid=,args="], capture_output=True, text=True)
    live = [l.strip() for l in r.stdout.splitlines()
            if l.split(maxsplit=1)[1:] and
            l.split(maxsplit=1)[1].split()[0].endswith("/spacecrafter")]
    if live:
        print("FAIL: another spacecrafter process is running: " + " | ".join(live))
        return False
    print("ok:   no other spacecrafter instance")
    return True


def wait_port(timeout=90):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


class App:
    def __init__(self, tag):
        self.tag = tag
        self.log = OUT / f"f21s_{tag}.applog"
        self.proc = subprocess.Popen([SC_BIN], cwd=str(USERDIR),
                                     stdout=open(self.log, "w"), stderr=subprocess.STDOUT,
                                     env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
        self.sock = wait_port()
        time.sleep(10)

    def cmd(self, c, pause=0.7):
        self.sock.sendall((c + "\n").encode())
        time.sleep(pause)
        try:
            self.sock.settimeout(0.3)
            self.sock.recv(8192)
        except socket.timeout:
            pass
        self.sock.settimeout(None)

    def shot(self, name, pause=2.5):
        p = OUT / f"f21s_{name}.png"
        p.unlink(missing_ok=True)
        self.cmd(f"body action screenshot filename {p}", pause)
        for _ in range(25):
            if p.exists() and p.stat().st_size > 0:
                break
            time.sleep(0.3)
        return np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)

    def quit(self):
        self.cmd("shutdown action now", 1.0)
        self.sock.close()
        try:
            self.proc.wait(timeout=30)
        except subprocess.TimeoutExpired:
            self.proc.kill()
            print(f"FAIL: {self.tag}: app did not exit within 30 s")


def build_scene_a(app):
    """f20_session.py's scene A, verbatim - the scene §5.63 was measured in."""
    app.cmd("timerate rate 0", 1.0)
    app.cmd(f"date jday {JD}", 1.5)
    app.cmd("set home_planet Mars", 3.0)
    app.cmd("select planet Mars", 1.0)
    app.cmd("moveto lat 12 lon 34 alt 500000 duration 0", 2.5)
    app.cmd("zoom fov 45 duration 0", 2.0)
    app.cmd("set view_offset 0.25", 1.5)
    app.cmd("flag lock_sky_position on", 2.0)


def stats(a, b, thr=8):
    """b against a: px>8, px>32, the signed mean over the differing pixels, and
    the lit>32 count on each side - §5.63's own quantities."""
    if a.shape != b.shape:
        return {"px8": -1}
    d = b - a
    m = np.abs(d).max(axis=2) > thr
    lit_a = int((a.max(axis=2) > 32).sum())
    lit_b = int((b.max(axis=2) > 32).sum())
    return {
        "px8": int(m.sum()),
        "px32": int((np.abs(d).max(axis=2) > 32).sum()),
        "signed_mean": float(d[m].mean()) if m.any() else 0.0,
        "lit_a": lit_a, "lit_b": lit_b,
        "mean_a": float(a.mean()), "mean_b": float(b.mean()),
    }


def ladder(app, prefix, extra_first=()):
    """Walk the stage ladder, shooting at each stage. Returns {stage: image}."""
    out = {}
    for name, cmd in STAGES:
        if cmd:
            app.cmd(cmd, 2.0)
        out[name] = app.shot(f"{prefix}_{name}")
    return out


def main():
    print(f"SC_BIN = {SC_BIN}", flush=True)
    if not assert_no_other_instance():
        return 1
    SESSIONS.mkdir(exist_ok=True)
    for f in SESSIONS.glob("f21s*.ini"):
        f.unlink()
    frozen_in = {n: md5(USERDIR / n) for n in FROZEN if (USERDIR / n).exists()}

    # ---- L1: the saved side ----
    app = App("saved")
    build_scene_a(app)
    app.cmd("session action save filename f21s", 2.5)
    saved = ladder(app, "saved")
    app.quit()

    # ---- L2: the A/A floor ----
    app = App("aa")
    build_scene_a(app)
    aa = ladder(app, "aa")
    app.quit()

    # ---- L3: the restored side ----
    app = App("rest")
    app.cmd("session action load filename f21s", 6.0)
    rest_t0 = app.shot("rest_t0")
    time.sleep(15)
    rest_t15 = app.shot("rest_t15")
    time.sleep(25)
    rest_t40 = app.shot("rest_t40")
    rest = ladder(app, "rest")
    app.quit()

    frozen_out = {n: md5(USERDIR / n) for n in FROZEN if (USERDIR / n).exists()}
    print(f"\nfrozen md5 in == out: {frozen_in == frozen_out}", flush=True)

    print("\n== the ladder: |restored - saved| against the in-scene A/A floor ==", flush=True)
    print(f"{'stage':<12} {'restored px>8':>14} {'floor px>8':>12} {'px>32':>8} "
          f"{'signed mean':>12} {'lit saved':>10} {'lit rest':>9}", flush=True)
    res = {}
    for name, _ in STAGES:
        s = stats(saved[name], rest[name])
        f = stats(saved[name], aa[name])
        res[name] = {"restored": s, "floor": f}
        print(f"{name:<12} {s['px8']:>14} {f['px8']:>12} {s['px32']:>8} "
              f"{s['signed_mean']:>12.3f} {s['lit_a']:>10} {s['lit_b']:>9}", flush=True)

    print("\n== transience: the restored scene, untouched, over 40 s ==", flush=True)
    for tag, img in (("t0", rest_t0), ("t15", rest_t15), ("t40", rest_t40)):
        s = stats(saved["base"], img)
        res[f"transient_{tag}"] = s
        print(f"  {tag:<4} {s['px8']:>8} px>8   signed mean {s['signed_mean']:>8.3f}   "
              f"lit {s['lit_b']}", flush=True)

    (OUT / "f21_s563.json").write_text(json.dumps(res, indent=1))
    print(f"\nartifacts in {OUT}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
