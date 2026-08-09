#!/usr/bin/env python3
"""F33 smoke — validate the new-path position readback BEFORE any port claim.

Three questions, one launch, all read-only except the named commands:

  P1  does `camera.rootPos` (the new path's observer position in the ROOT
      frame, §11.141) agree with the OLD path's observer heliocentric position?
      Old's is recovered from the dump's `helioToEye` the same way the new one
      is derived from viewMat: for an affine map [R|t] taking a frame to the
      eye, the eye's position in that frame is -Rᵀ·t. Agreement is BOTH the
      frame-math check and the "the root frame is heliocentric on this data"
      check the fixed-point anchors already assume.
  P2  is the free-mode round trip position-preserving? (getReferenceRelative-
      Position is derived from viewMat and is convention-proof, but
      Camera::setFreeMode converts the pose with its OWN spherical convention —
      if the two disagree the toggle teleports, which would be a defect to
      RECORD, and it would also poison every measurement below.)
  P3  the chain above the Sun: Sun / SolarSystem / MilkyWay `ecl`, to show that
      root == heliocentric is a MEASURED fact on this data, not an assumption.
"""

import argparse, json, sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import numpy as np
from f27_reply import Session, JD

HERE = Path(__file__).resolve().parent


def eye_pos(mat16):
    """-Rᵀ·t of a column-major 4x4 affine map (frame -> eye)."""
    m = np.array(mat16, dtype=float).reshape(4, 4).T  # column-major -> rows
    R, t = m[:3, :3], m[:3, 3]
    return -R.T @ t


def dump(sess, drv, tag):
    p = f"/tmp/f33_{tag}.json"
    drv.send(f"body action dual_dump filename {p}", 1.5)
    head = None
    with open(p) as f:
        head = json.loads(f.readline())
    bodies = {}
    with open(p) as f:
        f.readline()
        for line in f:
            line = line.strip()
            if not line:
                continue
            o = json.loads(line)
            if o.get("type") == "body":
                bodies[o["name"]] = o
    return head, bodies


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=str(HERE.parents[1] / "build-claude/src/spacecrafter"))
    a = ap.parse_args()
    out = Path(a.outdir); out.mkdir(parents=True, exist_ok=True)

    sess = Session(out, "smoke", a.bin)
    drv = sess.client("drv")
    drv.send("flag atmosphere off", 0.5)
    drv.send(f"date jday {JD}", 1.0)
    drv.send("timerate rate 0", 0.5)

    res = {}
    head, bodies = dump(sess, drv, "p0")
    cam = head["camera"]
    old = eye_pos(head["helioToEye"])
    new = np.array(cam["rootPos"], dtype=float)
    res["P1"] = {"oldHelio": old.tolist(), "newRootPos": new.tolist(),
                 "delta_AU": float(np.linalg.norm(old - new)),
                 "reference": cam["reference"], "distance": cam["distance"],
                 "boundToSurface": cam["boundToSurface"], "freeMode": cam["freeMode"]}

    # P3 - the chain above the Sun, from the app
    res["P3"] = {n: bodies[n]["new"]["ecl"] for n in ("Sun", "SolarSystem", "MilkyWay")
                 if n in bodies and bodies[n].get("new")}

    # P2 - free-mode round trip
    seq = []
    for cmd in ("free_mode state on", "free_mode state off", "free_mode state on", "free_mode state off"):
        drv.send(f"camera action {cmd}", 0.8)
        h, _ = dump(sess, drv, "p2_" + cmd.replace(" ", "_"))
        seq.append({"cmd": cmd,
                    "rootPos": h["camera"]["rootPos"],
                    "oldHelio": eye_pos(h["helioToEye"]).tolist(),
                    "freeMode": h["camera"]["freeMode"],
                    "lon": h["camera"]["longitude"], "lat": h["camera"]["latitude"],
                    "distance": h["camera"]["distance"],
                    "position": h["camera"]["position"]})
    base = np.array(res["P1"]["newRootPos"], dtype=float)
    for s in seq:
        s["moved_AU"] = float(np.linalg.norm(np.array(s["rootPos"], dtype=float) - base))
    res["P2"] = seq

    sess.stop(drv)
    (out / "f33_smoke.json").write_text(json.dumps(res, indent=1))
    print(json.dumps(res, indent=1))


if __name__ == "__main__":
    main()
