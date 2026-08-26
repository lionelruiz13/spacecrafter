#!/usr/bin/env python3
"""F40 — the ANCHORED regression proper: no free mode is ever entered.

    cd claude/harness && DISPLAY=:2 ./f40_anchored.py <absOutdir> --bin B
    ./f40_cmp.py <preOutdir> <postOutdir>        # the comparison (same shape)

WHY THIS EXISTS SEPARATELY FROM f40_inverse's L9.  L9 walks the same itinerary
at the END of a run that has been through every free-mode transition, and the
two binaries legitimately arrive there with DIFFERENT ORIENTATIONS: the
deduce-identical-view rule holds the composed body->eye rotation across each
transition, so when the pre binary's observer is teleported 16 700 km its LOCAL
(alt, az, heading) must change to keep the same absolute view, and the post
binary's does not.  Measured: identical lat/lon/alt at all five places, heading
55.402987 vs -46.992559 - a difference that is the FIX working, not a
regression, but which makes a screen comparison across the two runs meaningless.

So this run touches no free-mode member at all.  Every place is reached by an
anchored `moveto`, and the view is aimed DETERMINISTICALLY (select the
reference, track on, track off) so that the composed orientation is a function
of the place and not of the history.  Then pre and post must agree bit for bit,
including the composed screen - and that is the claim the DoD's "anchored places
bit-identical" makes.  `set home_planet Mars` is in the itinerary on purpose: it
is the shipped route into `placeAt`, the fifth member this task rerouted.
"""

import argparse, json, sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
from f27_reply import Session
import dumpread

HERE = Path(__file__).resolve().parent
JD0 = 2461233.5

PLACES = [("Earth", 0.0, 0.0, 100.0),
          ("Earth", 43.3, 5.3667, 75.0),
          ("Earth", -33.9, 151.2, 1000.0),
          ("Earth", 89.9, -179.9, 500000.0),
          ("Earth", 0.0, 60.0, 2000.0),
          ("Mars", 12.0, 47.0, 55000000.0),
          ("Mars", 0.0, -25.0, 55000000.0),
          ("Moon", 0.0, 60.0, 8000000.0)]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=str(HERE.parents[1] / "build-claude/src/spacecrafter"))
    a = ap.parse_args()
    out = Path(a.outdir).resolve()
    out.mkdir(parents=True, exist_ok=True)

    sess = Session(out, "f40anch", a.bin)
    drv = sess.client("drv")
    n = [0]

    def cmd(c, p=0.8):
        drv.send(c, p)

    for c in ("flag atmosphere off", "flag landscape off", "flag fog off",
              "timerate rate 0"):
        cmd(c, 0.5)
    cmd(f"date jday {JD0:.9f}", 1.2)

    home = None
    rows = []
    for i, (body, la, lo, alt) in enumerate(PLACES):
        if body != home:
            cmd(f"set home_planet {body}", 6.0)      # the shipped route into placeAt
            home = body
        cmd(f"moveto lat {la} lon {lo} alt {alt} duration 0", 1.8)
        cmd(f"select planet {body} pointer off", 1.0)
        cmd("flag track_object on", 5.0)             # deterministic aim
        cmd("flag track_object off", 1.5)
        n[0] += 1
        p = out / f"anchdump_{n[0]:03d}.json"
        drv.send(f"body action dual_dump filename {p}", 1.4)
        head, pairs, _, _ = dumpread.load_dump(p)
        cam = head["camera"]
        m = np.array(cam["mat"], dtype=float).reshape(4, 4).T
        E = -m[:3, :3].T @ m[:3, 3]
        shot = out / f"shot_L9_anchored_{i}.png"
        drv.send(f"body action screenshot filename {shot}", 2.5)
        drv.sock.sendall(b"get status position\n")
        place, _, _ = drv.poll_for_reply(4.0)
        st = {"ref": cam["reference"], "free": cam["freeMode"],
              "bound": cam["boundToSurface"], "lon": cam["longitude"],
              "lat": cam["latitude"], "dist": cam["distance"],
              "position": list(cam["position"]), "mat": list(cam["mat"]),
              "E": E.tolist(), "alt": cam["alt"], "az": cam["az"],
              "heading": cam["heading"], "refDist": cam["refDist"]}
        rows.append({"cmd": [body, la, lo, alt], "dump": st, "place": place,
                     "shot": str(shot)})
        print(f"  {i} {body:6s} lat {la:>7} lon {lo:>7} alt {alt:>10}  "
              f"|E| = {np.linalg.norm(E):.9e} AU  heading {cam['heading']:.6f}",
              flush=True)

    (out / "f40_result.json").write_text(json.dumps(
        {"data": {"L9": rows}, "binary": a.bin}, indent=1, default=str))
    sess.stop(drv)
    return 0


if __name__ == "__main__":
    sys.exit(main())
