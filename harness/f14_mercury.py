#!/usr/bin/env python3
"""F14 — D22's own cheap counterfactual, run in a TEST corpus.

    cd claude/harness && DISPLAY=:2 ./f14_mercury.py <outdir> \
        pre=/abs/pre-fix/binary post=/abs/fixed/binary

§11.101(b) named the cheapest live discriminator for §5.28 and did not run it:
*"add `rot_pole_w0` to Mercury - the surface must visibly rotate 90 deg if this
analysis holds"*. This is that test, and it is a counterfactual in BOTH
directions at once, because Mercury is the one body where the answer is already
known independently: its shipped `rot_rotation_offset` (291.20) registers its
map to the IAU meridian at u = 0.4998 (§11.101(b2)), so converting its FETCHED
W0 must reproduce that number - and the pre-fix conversion produces 201.12,
90.08 deg away.

  PREDICTED, before the run:
    pre-fix binary  : offset 291.20 -> 201.120  => the globe turns ~90 deg
    fixed binary    : offset 291.20 -> 291.120  => the globe does not move
                      (0.080 deg, i.e. sub-pixel on any disc smaller than
                       ~1400 px across)

  The control leg is the SAME binary on the SAME farm without the added key:
  it must be pixel-identical to the shipped-data render, which is what proves
  the farm itself changed nothing.

NOTHING IN THE REAL TREE IS WRITTEN. The farm is `b25_galactic.build_farm`'s
field-state variant (everything symlinked except `config.ini`/`ssystem.ini`,
which are copies); the mutation is applied to the farm's COPY. The md5 of the
real `~/.spacecrafter/ssystem.ini` is asserted before and after.
"""

import os, re, socket, subprocess, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import b25_galactic as b25g
import f14_meridian as F

SRC = Path.home() / ".spacecrafter"
JD = 2451545.0
# Mercury's IAU W0, as already carried by b14_w0_planetscan.py (provenance: the
# §11.86 cited pck00011 BODY_PM fetch, md5 3c0bdc01). No value is introduced.
MERCURY_W0 = "329.5988"
FAILS = []


def fail(m): FAILS.append(m); print(f"FAIL: {m}", flush=True)
def ok(m): print(f"ok:   {m}", flush=True)


def add_key(dst):
    """Insert `rot_pole_w0` into [mercury] of the FARM's ssystem.ini copy."""
    p = dst / "ssystem.ini"
    lines = p.read_text(encoding="iso-8859-1").splitlines(keepends=True)
    out, cur, done = [], None, 0
    for ln in lines:
        s = ln.strip()
        if s.startswith("["):
            cur = s.lower()
        out.append(ln)
        if cur == "[mercury]" and re.match(r"^\s*rot_pole_de\s*=", ln):
            out.append(f"rot_pole_w0 = {MERCURY_W0}\n")
            done += 1
    if done != 1:
        raise RuntimeError(f"expected exactly one [mercury] rot_pole_de line, got {done}")
    p.write_text("".join(out), encoding="iso-8859-1")


def run(binary, dst, out, tag):
    proc = subprocess.Popen([binary], cwd=str(dst),
                            stdout=open(out / f"f14merc_{tag}.applog", "w"),
                            stderr=subprocess.STDOUT,
                            env={**os.environ, "HOME": str(dst.parent),
                                 "DISPLAY": os.environ.get("DISPLAY", ":2")})
    sock = F.wait_port(proc)
    time.sleep(10)
    F.send(sock, "timerate rate 0", 1)
    for f in ("atmosphere", "landscape", "fog", "star_lines", "constellation_art",
              "cardinal_points", "planet_names", "nebula_names"):
        F.send(sock, f"flag {f} off", 0.4)
    F.send(sock, "set home_planet Sun", 4)
    F.send(sock, "select planet Mercury pointer off", 2)
    F.send(sock, "flag track_object on", 4)
    F.send(sock, "zoom fov 0.02 duration 0", 3)
    F.send(sock, f"date jday {JD:.5f}", 3)
    time.sleep(8)                          # settle (§11.120: a fresh jd is black for ~8 s)
    png = out / f"f14merc_{tag}.png"
    dump = out / f"f14merc_{tag}.json"
    F.send(sock, f"body action screenshot filename {png}", 2.5)
    F.send(sock, f"body action dual_dump filename {dump}", 2.5)
    F.send(sock, "shutdown action now", 1)
    sock.close()
    try:
        proc.wait(timeout=40)
    except subprocess.TimeoutExpired:
        proc.kill(); fail(f"{tag}: no exit within 40 s")
    txt = (out / f"f14merc_{tag}.applog").read_text(errors="replace")
    unk = sorted(set(re.findall(r"[\w:]+ is unknown\. Did you mean [\w:]+ \?", txt)))
    if unk:
        fail(f"{tag}: the app REJECTED a command - {unk}")
    _, body, hops = F.read_dump(dump, "Mercury")
    return png, hops[0]["offset"]


def px(a, b, thr=32):
    ia = np.asarray(Image.open(a).convert("L")).astype(int)
    ib = np.asarray(Image.open(b).convert("L")).astype(int)
    return int((np.abs(ia - ib) > thr).sum())


def main(argv):
    out = Path(argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
    bins = {k: v for k, v in (a.split("=", 1) for a in argv[2:] if "=" in a)}
    md5_in = subprocess.run(["md5sum", str(SRC/"ssystem.ini"), str(SRC/"config.ini")],
                            capture_output=True, text=True).stdout
    farm = out / "farm"
    results = {}
    for leg, mutate in (("ctrl", False), ("w0", True)):
        for which, binary in bins.items():
            dst = b25g.build_farm(farm=farm, dotted=False, corpus=None)
            if mutate:
                add_key(dst)
            tag = f"{which}_{leg}"
            results[tag] = run(binary, dst, out, tag)
            print(f"{tag:10s} offset {results[tag][1]:.6f}")
    md5_out = subprocess.run(["md5sum", str(SRC/"ssystem.ini"), str(SRC/"config.ini")],
                             capture_output=True, text=True).stdout
    if md5_in != md5_out:
        fail("the real ~/.spacecrafter data changed during the run")

    for which in bins:
        p_ctrl, o_ctrl = results[f"{which}_ctrl"]
        p_w0, o_w0 = results[f"{which}_w0"]
        turn = ((o_w0 - o_ctrl + 180) % 360) - 180
        moved, moved8 = px(p_ctrl, p_w0), px(p_ctrl, p_w0, 8)
        print(f"{which:5s}: file offset {o_ctrl:.3f} -> converted {o_w0:.3f} "
              f"(turn {turn:+.3f} deg), ctrl-vs-w0 px>32 = {moved}, px>8 = {moved8}")
        results[f"{which}_turn"] = turn
        results[f"{which}_px"] = moved
        results[f"{which}_px8"] = moved8
    if "pre" in bins and "post" in bins:
        if not (abs(abs(results["pre_turn"]) - 90.0) < 0.5):
            fail(f"pre-fix turn {results['pre_turn']:+.3f} deg is not the predicted ~90")
        if not (abs(results["post_turn"]) < 0.5):
            fail(f"fixed turn {results['post_turn']:+.3f} deg is not the predicted ~0")
        # The sensitivity floor is stated on the PRE leg, so a blind scene fails
        # instead of passing quietly. It is deliberately not large: mercury.png
        # is a low-contrast cratered grey, so a 90 deg turn of a 407-px disc
        # moves only ~9k px above 32 grey levels even though it is obvious to
        # the eye - the count measures the MAP's contrast, not the size of the
        # turn, and the turn itself is measured in degrees above.
        if results["pre_px"] < 1000:
            fail(f"pre-fix leg moved only {results['pre_px']} px>32 - the scene "
                 f"cannot see a 90 deg turn, so its null on the fixed binary is worthless")
        if results["post_px"] > max(2, results["pre_px"]/100):
            fail(f"fixed leg moved {results['post_px']} px>32 against the pre-fix "
                 f"{results['pre_px']} - the correction did not converge on the shipped value")
        # the two binaries must agree on the UNMUTATED corpus (nothing else moved)
        same = px(results["pre_ctrl"][0], results["post_ctrl"][0])
        if same > 0:
            fail(f"the two binaries differ by {same} px on the UNMUTATED corpus - "
                 f"the fix reached a body that declares no rot_pole_w0")
        else:
            ok("the two binaries are pixel-identical on the unmutated corpus")
    print("RESULT:", "ALL OK" if not FAILS else f"{len(FAILS)} FAILURE(S)")
    return 0 if not FAILS else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
