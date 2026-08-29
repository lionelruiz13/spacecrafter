#!/usr/bin/env python3
"""F43 - the ONE gate the 28-file audit could not settle by reading.

`f23_b33_control.py` (L212) and `f24_b34_seams.py` (L381) both command, IN FREE
MODE, `moveto lat 0 lon 270 alt 40000000` on Earth at jd 2461233.5, fov 90, and
both then guard their screen legs with a LIT-FRAME check - f23's
`if P["A0_lit"] < 20000: fail("the ladder's frames are EMPTY ...")` and f24's
`if min(lits) < 1000: fail("POSITION: an empty frame in the scene ...")`.
Every other gate in those two files is either radial or an in-run comparison at
one camera position, so the lit guard is their ONLY coupling to the free-mode
observer longitude - and F40 moved that observer's sub-point from `180 - 270`
= -90 to `270 - 90` = 180, i.e. 90 deg along the equator, which is exactly the
kind of move that can put a frame on the night side.

Reading the code cannot answer whether the frame is still lit.  This does:
the same scene, the same commands, the current binary, counting the same
quantity the two guards count.

PREDICTION, before the run: at 40 000 km above a 6378.14 km Earth the whole
globe subtends 2*asin(6378.14/46378.14) = 15.9 deg inside a 90 deg field, so
unless the sub-point is within a few degrees of the anti-solar point some lit
crescent is always in frame; the guards need 20 000 and 1 000 px of 4 194 304.
The measurement decides it, not this sentence.

usage: f43_litguard.py <outdir> [--bin /abs/binary]
"""
import json, math, os, socket, subprocess, sys, time
from pathlib import Path
import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
FARM = Path("/tmp/f43_lit_farm")
JD = 2461233.5          # [observed: f23_b33_control.py:69, f24_b34_seams.py:67]
LON, LAT, ALT_M = 270, 0, 40000000
F23_MIN, F24_MIN = 20000, 1000
RENDER = 2048           # render side [measured: existing artifacts]

FAILS = []
def fail(m): FAILS.append(m); print(f"FAIL: {m}", flush=True)
def ok(m):   print(f"ok:   {m}", flush=True)


def wait_port(timeout=90):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.25); sock.recv(8192); sock.settimeout(None)
    except socket.timeout:
        sock.settimeout(None)


def shot(sock, out, name, pause=2.5):
    p = out / f"{name}.png"
    send(sock, f"body action screenshot filename {p}", pause)
    for _ in range(25):
        if p.exists() and p.stat().st_size > 0:
            break
        time.sleep(0.3)
    return np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)


def main():
    argv = sys.argv[1:]
    binp = HERE.parents[1] / "build-claude/src/spacecrafter"
    if "--bin" in argv:
        binp = Path(argv[argv.index("--bin") + 1])
    out = Path([a for a in argv if not a.startswith("--")][0]).resolve()
    out.mkdir(parents=True, exist_ok=True)

    subprocess.run(["bash", str(HERE / "b3_farm.sh"), str(FARM)], check=True)
    env = {**os.environ, "HOME": str(FARM), "DISPLAY": os.environ.get("DISPLAY", ":2")}
    proc = subprocess.Popen([str(binp)], cwd=str(FARM / ".spacecrafter"),
                            stdout=open(out / "lit.applog", "w"),
                            stderr=subprocess.STDOUT, env=env)
    rep = {"bin": str(binp), "jd": JD, "place": [LAT, LON, ALT_M]}
    try:
        s = wait_port(); time.sleep(10)
        send(s, "flag experimental_path on"); send(s, "timerate rate 0")
        send(s, "meteors zhr 0"); send(s, f"date jday {JD}", 1)
        send(s, "set home_planet Earth", 2)
        # f23/f24's own order: free mode first, then the commanded place.
        send(s, "camera action free_mode state on", 1.5)
        send(s, f"moveto lat {LAT} lon {LON} alt {ALT_M} duration 0", 4)
        send(s, "zoom fov 90 duration 0", 2)
        img = shot(s, out, "lit_lon270")
        lum = img.max(axis=2)
        lit = int((lum > 8).sum())
        rep["lit_px_gt8"] = lit
        rep["lit_px_gt32"] = int((lum > 32).sum())
        rep["total_px"] = int(lum.size)
        print(f"lit px>8 = {lit} of {lum.size} (px>32 = {rep['lit_px_gt32']})", flush=True)
        if lit >= F23_MIN:
            ok(f"f23_b33_control's lit guard holds at the POST-F40 place: {lit} px>8 "
               f">= its {F23_MIN} threshold - the free-mode longitude move did not "
               f"empty its frame")
        else:
            fail(f"f23_b33_control's lit guard would FIRE: {lit} px>8 < {F23_MIN}")
        if lit >= F24_MIN:
            ok(f"f24_b34_seams' lit guard holds: {lit} px>8 >= its {F24_MIN} threshold")
        else:
            fail(f"f24_b34_seams' lit guard would FIRE: {lit} px>8 < {F24_MIN}")
        # THE CONTROL, and its first spelling was wrong.  "Is the frame lit" is
        # a number with no scale until something says what number the GLOBE
        # would produce, so the control is that PREDICTION:
        #   half-angle a = asin(R / (R + alt)) = asin(6378.14 / 46378.14)
        #   the projection is radially linear in theta (custom_project.glsl, the
        #   b3_ladder derivation), so px radius = (a / halfFov) * viewportRadius
        #   and the disc area = pi * that^2; the atmosphere shell adds
        #   atmosphere_radius_factor = 1.03 of radius, i.e. x1.06 of area.
        # A lit count that matches this IS the globe; one that does not is
        # background, and the guard it feeds would be vacuous.
        R, halfFov, vpR = 6378.14, 45.0, RENDER / 2
        a = math.degrees(math.asin(R / (R + ALT_M / 1000.0)))
        px_r = (a / halfFov) * vpR
        pred_lo, pred_hi = math.pi * px_r ** 2, math.pi * (1.03 * px_r) ** 2
        rep["disc_halfangle_deg"], rep["pred_px_area"] = a, [pred_lo, pred_hi]
        if pred_lo * 0.8 <= lit <= pred_hi * 1.2:
            ok(f"control: {lit} px>8 is the GLOBE - predicted disc area "
               f"{pred_lo:.0f}-{pred_hi:.0f} px (half-angle {a:.3f} deg at fov 90, "
               f"atmosphere shell included), so the guard is counting content")
        else:
            fail(f"control: {lit} px>8 against a predicted globe area of "
                 f"{pred_lo:.0f}-{pred_hi:.0f} px - the lit count is NOT the globe, "
                 f"so f23/f24's lit guard can be satisfied by background")
        # RECORDED, not gated: `flag planets off` does not remove the reference
        # body from this frame - measured 2026-08-29, identical px>8 to the
        # pixel before and after. Reported so the next reader does not use that
        # flag as a control the way this file's first spelling did.
        send(s, "flag planets off", 2)
        rep["planets_off_px_gt8"] = int((shot(s, out, "lit_planets_off").max(axis=2) > 8).sum())
        print(f"note: `flag planets off` leaves {rep['planets_off_px_gt8']} px>8 "
              f"against {lit} (recorded, not gated)", flush=True)
    finally:
        try:
            send(s, "shutdown action now", 1); s.close(); proc.wait(timeout=40)
        except Exception:
            pass
        if proc.poll() is None:
            proc.kill()
    rep["fails"] = FAILS
    (out / "f43_litguard_result.json").write_text(json.dumps(rep, indent=1))
    print(f"\n{'F43-LITGUARD GREEN' if not FAILS else f'{len(FAILS)} FAILURES'}", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
