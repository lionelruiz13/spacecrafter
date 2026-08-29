#!/usr/bin/env python3
"""B3 / INTENT 5.30 + 5.33 collateral A/B: what ELSE moved when Earth's ray
row started writing depth and the atmosphere shell stopped.

Two questions, three scenes, one state per invocation (the caller swaps the
binary AND the deployed .spv between invocations - both changed, so a
shader-only swap would not be a valid A/B here):

  O_disc / O_limb   OLD render path (`flag experimental_path off`), Earth as
                    the old CoI in its ray regime.  The old path's own
                    my_earth_shadow row and its AtmExt pipeline are untouched
                    by construction; this is the MEASUREMENT of that claim.
                    Criterion: 0 changed px, with content asserted present.
  N_axis            NEW path with `flag planets_axis on` - which arms BOTH
                    AxisModule and PlanetGridModule (the old flag coupling,
                    body.cpp:229-234).  These are the two depth-TESTING
                    consumers of Earth's bucket depth that are OFF at shipped
                    defaults (AxisModule::show / PlanetGridModule::show are
                    false initializers and no config key sets them), so they
                    cannot change a stock scene - but they DO change here,
                    and the direction is the point: their own headers state
                    "hidden by its own disc exactly" / "depth-tested, drawn
                    after the body surface", contracts Earth could not honour
                    while it wrote no depth.  CHARACTERIZATION, not a gate.

usage: b3_earth_ab.py <outdir> <pre|post>     (SC_BIN selects the binary)
"""
import json, os, socket, subprocess, sys, time
from pathlib import Path
import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
SC_BIN = os.environ.get("SC_BIN", str(HERE.parents[1] / "build-claude/src/spacecrafter"))
FARM = Path(os.environ.get("B3_FARM", "/tmp/b3_farm"))
REAL_HOME = Path.home()
JD = 2461234.0
OBS_SITE_LON = 270.0     # the lit site (b3_ladder SITES["earth"]) - the site is
                         # the SUB-OBSERVER point, not the commanded longitude.
# The command that lands on that site.  F40 (§11.153) changed the free-mode
# sub-point of `moveto lon L` from `180 - L` to `L - 90`, so holding the site
# fixed means L' = 270 - L (mod 360) - b3_ladder.obs_lon_cam()'s arithmetic,
# restated here rather than imported because this driver imports nothing.
# 270 -> 0.0, sub-point -90 == 180 - 270, unchanged.
OBS_LON = (270.0 - OBS_SITE_LON) % 360.0     # = 0.0
OBS_ALT_M = 10000000
RENDER = 2048


def wait_port(timeout=120):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.25); sock.recv(8192); sock.settimeout(None)
    except socket.timeout:
        sock.settimeout(None)


def shot(sock, out, name, pause=2.5):
    p = out / f"{name}.png"
    send(sock, f"body action screenshot filename {p}", pause)
    for _ in range(30):
        if p.exists() and p.stat().st_size > 0:
            break
        time.sleep(0.3)
    return np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)


def run(out, state):
    farmdir = FARM / ".spacecrafter"
    (farmdir / "modularSystem/SolarSystem.ini").write_bytes(
        (REAL_HOME / ".spacecrafter/modularSystem/SolarSystem.ini.disabled").read_bytes())
    for f in (farmdir / "log").glob("*.log"):
        f.unlink()
    env = {**os.environ, "HOME": str(FARM), "DISPLAY": os.environ.get("DISPLAY", ":2")}
    proc = subprocess.Popen([SC_BIN], cwd=str(farmdir),
                            stdout=open(out / f"{state}.applog", "w"),
                            stderr=subprocess.STDOUT, env=env)
    imgs = {}
    try:
        s = wait_port(); time.sleep(10)
        send(s, "timerate rate 0"); send(s, "meteors zhr 0")
        send(s, f"date jday {JD}")
        send(s, "set home_planet Earth", 2)
        send(s, "camera action free_mode state on")
        send(s, "flag atmosphere off"); send(s, "flag landscape off")
        send(s, "select planet Earth")
        send(s, f"moveto lat 0 lon {OBS_LON} alt {OBS_ALT_M} duration 0", 5)
        send(s, "flag track_object on", 2)
        send(s, "zoom fov 20 duration 0", 2)
        send(s, "flag track_object off", 2)     # B30 determinism
        # --- OLD path legs -------------------------------------------------
        send(s, "flag experimental_path off", 3)
        imgs["O_disc"] = shot(s, out, f"{state}_O_disc")
        send(s, "zoom fov 90 duration 0", 2)
        imgs["O_limb"] = shot(s, out, f"{state}_O_limb")
        # --- NEW path, axis + grid armed ------------------------------------
        send(s, "flag experimental_path on", 3)
        send(s, "flag planets_axis on", 2)
        imgs["N_axis"] = shot(s, out, f"{state}_N_axis")
        send(s, "flag planets_axis off", 2)
        imgs["N_plain"] = shot(s, out, f"{state}_N_plain")
        send(s, "shutdown action now", 1); s.close()
        try:
            proc.wait(timeout=40)
        except subprocess.TimeoutExpired:
            proc.kill()
    finally:
        if proc.poll() is None:
            proc.kill()
    return imgs


def main():
    out = Path(sys.argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
    state = sys.argv[2]
    imgs = run(out, state)
    rep = {"state": state, "sc_bin": SC_BIN, "views": {}}
    for k, im in imgs.items():
        lum = im.max(axis=2)
        rep["views"][k] = {"lit_px_gt32": int((lum > 32).sum()),
                           "lit_px_gt8": int((lum > 8).sum()),
                           "total_luma": float(lum.mean())}
        print(f"[{state}] {k:8s} lit>32 {rep['views'][k]['lit_px_gt32']:8d} "
              f"lit>8 {rep['views'][k]['lit_px_gt8']:8d} mean {rep['views'][k]['total_luma']:7.2f}",
              flush=True)
    (out / f"b3_earth_ab_{state}.json").write_text(json.dumps(rep, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
