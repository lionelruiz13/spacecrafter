#!/usr/bin/env python3
"""F14 — what the fix does to the 17 landed moons that carry a PLACEHOLDER map.

    cd claude/harness && DISPLAY=:2 ./f14_placeholder.py <outdir> \
        pre=/abs/pre-fix/binary post=/abs/fixed/binary

THE PREMISE THIS LEG EXISTS TO TEST. The task was written expecting these 17 to
be observably unchanged, *"the placeholder textures are featureless, so the
channel is the numeric orientation, not pixels"*. MEASURED, they are not
featureless: over the equatorial band this file scores (v in 0.35..0.65) the
longitudinal brightness profile of `generic.png` has sigma = 8.15 grey levels
and `asteroid.png` sigma = 13.33, against `iapetus.png`'s 51.51 on the same band
- smaller, not absent. So the prediction is the
opposite of the premise: their pixels DO move, by the same 90 deg as everything
else, and the reason that is not a defect is that a placeholder asserts no
registration - there is no correct orientation for it to be turned away from.

  PREDICTED: offset +90.000 deg (already measured for all 20 in the dump-level
  only-movers table), and a NON-ZERO pixel difference between the binaries on
  both placeholder families - measured here so "unchanged" is never claimed for
  something that was not looked at.

Two Saturn moons, one per placeholder family, in one launch per binary:
Janus (`generic.png`) and Pandora (`asteroid.png`).
"""

import json, os, re, subprocess, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import f14_meridian as F

JD = 2451545.0
MOONS = [("Saturn", "Janus", 0.05), ("Saturn", "Pandora", 0.05)]
# Candidate epochs per moon, one orbital period sampled: the FIRST one whose
# disc is lit enough is chosen on the pre-fix pass and REUSED verbatim on the
# fixed one, so the two legs are the same scene and the difference between them
# is the binary and nothing else. Pandora at J2000 is a near-new crescent (6 lit
# px, measured) - a leg that shot it there would have compared two black frames
# and called the result "unchanged".
CANDIDATES = {"Janus":   [JD + 0.6945*k/6 for k in range(6)],
              "Pandora": [JD + 0.6285*k/6 for k in range(6)]}
LIT_FLOOR = 20000
FAILS = []
# The two LIT_FLOOR uses' own measured values, kept so the run leaves them in an
# artifact instead of only in a printed line or (for the selector) nowhere at all
# (§11.167(i): both existed NOWHERE in the repo). Observation only - nothing
# reads these, no gate consults them. Module level, like FAILS, because the
# selector runs inside run() while the artifact is written in main().
#   SELECTOR_LIT[tag][moon] = [{"jd":…, "lit_px_gt8":…}, …]   the :78-81 use
#   GATE_LIT[moon]          = lit_px_gt8                      the :127 use
SELECTOR_LIT = {}
GATE_LIT = {}


def fail(m): FAILS.append(m); print(f"FAIL: {m}", flush=True)
def ok(m): print(f"ok:   {m}", flush=True)


def run(binary, out, tag, chosen=None):
    applog = out / f"f14ph_{tag}.applog"
    proc = subprocess.Popen([binary], cwd=str(F.USERDIR),
                            stdout=open(applog, "w"), stderr=subprocess.STDOUT,
                            env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
    sock = F.wait_port(proc)
    time.sleep(10)
    F.send(sock, "timerate rate 0", 1)
    for f in ("atmosphere", "landscape", "fog", "star_lines", "constellation_art",
              "cardinal_points", "planet_names", "nebula_names"):
        F.send(sock, f"flag {f} off", 0.4)
    made, picked = {}, {}
    for parent, moon, fov in MOONS:
        F.send(sock, f"set home_planet {parent}", 4)
        F.send(sock, f"select planet {moon} pointer off", 2)
        F.send(sock, "flag track_object on", 4)
        F.send(sock, f"zoom fov {fov} duration 0", 3)
        png = out / f"f14ph_{tag}_{moon}.png"
        dump = out / f"f14ph_{tag}_{moon}.json"
        for jd in ([chosen[moon]] if chosen else CANDIDATES[moon]):
            F.send(sock, f"date jday {jd:.5f}", 3)
            time.sleep(8)
            F.send(sock, f"body action screenshot filename {png}", 2.5)
            lit = int((np.asarray(Image.open(png).convert("L")) > 8).sum())
            SELECTOR_LIT.setdefault(tag, {}).setdefault(moon, []).append(
                {"jd": jd, "lit_px_gt8": lit})   # RECORD (observation only, F54)
            picked[moon] = jd
            if lit >= LIT_FLOOR or chosen:
                break
        F.send(sock, f"body action dual_dump filename {dump}", 2.5)
        made[moon] = (png, dump)
    F.send(sock, "shutdown action now", 1)
    sock.close()
    try:
        proc.wait(timeout=40)
    except subprocess.TimeoutExpired:
        proc.kill(); fail(f"{tag}: no exit within 40 s")
    unk = sorted(set(re.findall(r"[\w:]+ is unknown\. Did you mean [\w:]+ \?",
                                applog.read_text(errors="replace"))))
    if unk:
        fail(f"{tag}: the app REJECTED a command - {unk}")
    return made, picked


def main(argv):
    out = Path(argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
    bins = dict(a.split("=", 1) for a in argv[2:] if "=" in a)
    ini = F.ini_bodies()
    md5_in = subprocess.run(["md5sum", str(F.SSYSTEM)], capture_output=True, text=True).stdout
    caps, chosen = {}, None
    for k in ("pre", "post"):
        caps[k], picked = run(bins[k], out, k, chosen)
        chosen = chosen or picked
    print("epochs used (chosen on the pre leg, reused on the fixed one):",
          {m: round(j, 5) for m, j in chosen.items()})
    md5_out = subprocess.run(["md5sum", str(F.SSYSTEM)], capture_output=True, text=True).stdout
    if md5_in != md5_out:
        fail("the frozen ssystem corpus changed during the run")
    for _, moon, _ in MOONS:
        tex = F.TEXDIR / ini[moon.lower()]["tex_map"]
        a = np.asarray(Image.open(tex).convert("L")).astype(float)
        prof = a[int(a.shape[0]*0.35):int(a.shape[0]*0.65)].mean(axis=0)
        pa = np.asarray(Image.open(caps["pre"][moon][0]).convert("L")).astype(int)
        pb = np.asarray(Image.open(caps["post"][moon][0]).convert("L")).astype(int)
        d32, d8 = int((np.abs(pa-pb) > 32).sum()), int((np.abs(pa-pb) > 8).sum())
        _, _, hp = F.read_dump(caps["pre"][moon][1], moon)
        _, _, hq = F.read_dump(caps["post"][moon][1], moon)
        turn = ((hq[0]["offset"] - hp[0]["offset"]) % 360.0)
        lit = int((pa > 8).sum())
        GATE_LIT[moon] = lit                 # RECORD (observation only, F54)
        print(f"{moon:9s} {Path(tex).name:13s} eq-profile sigma {prof.std():5.2f} | "
              f"offset {hp[0]['offset']:.4f} -> {hq[0]['offset']:.4f} (+{turn:.4f} deg) | "
              f"pre-vs-post px>32 {d32}, px>8 {d8} | lit px {lit} | jd {chosen[moon]:.5f}")
        if abs(turn - 90.0) > 2e-4:
            fail(f"{moon}: offset moved {turn:.6f} deg, not the predicted 90")
        if lit < LIT_FLOOR:
            fail(f"{moon}: only {lit} lit px - the body did not reach the frame, "
                 f"so its pixel difference means nothing")
        elif d8 == 0:
            fail(f"{moon}: 0 px changed. Its map has sigma {prof.std():.2f} of "
                 f"longitudinal structure, so a 90 deg turn MUST move pixels - "
                 f"either the body did not turn or the leg is not looking at it")
        else:
            ok(f"{moon}: turned 90 deg and the placeholder map moved with it "
               f"({d8} px>8) - featureless was the wrong word, unregistered is "
               f"the right one")
    # This file had no artifact record at all, so neither LIT_FLOOR value had
    # anywhere to land; both are written here (F54, §11.167(i)). Observation
    # only: written after every gate above has already decided.
    (out / "f14ph_values.json").write_text(json.dumps(
        {"file": "f14_placeholder.py", "lit_floor": LIT_FLOOR,
         "selector_lit_px_gt8": SELECTOR_LIT,      # the :78-81 use, per tried jd
         "chosen_jd": chosen,                      # which jd the selector picked
         "gate_lit_px_gt8": GATE_LIT,              # the :127 use, on the pre frame
         "bins": bins, "fails": FAILS}, indent=1))
    print("RESULT:", "ALL OK" if not FAILS else f"{len(FAILS)} FAILURE(S)")
    return 0 if not FAILS else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
