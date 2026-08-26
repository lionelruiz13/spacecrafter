#!/usr/bin/env python3
"""F40 — §11.152(o)'s FREE DISCRIMINATOR, measured on a real authored rover.

    cd claude/harness && DISPLAY=:2 ./f40_disc.py <absOutdir> --bin B --tag pre|post

§11.152(o) [measured, F39]: an observer commanded to `moveto lat 0 lon 60` sits
exactly **60.0 deg** from a rover authored at `orbit_lon 60 orbit_lat 0` in FREE
mode and exactly **90.0 deg** in SURFACE mode.  F39 recorded it as a
discriminator F40 could use for free, and named the surface-mode half a THIRD
channel (§5.49's observer-longitude origin) that F40 must not touch.

WHAT THIS INSTRUMENT ASKS, and why the question is the toggle and not the number:
the two readings are the SAME command answered twice, so after the converter
becomes the composer's exact inverse they must COINCIDE - and they coincide on
the anchored answer, because the anchored place is the baseline (§11.52(b)) and
is bit-identical by construction here.  So "60 stays 60" cannot be a target of a
`moveto lon 60`: it is the free-mode reading of a command whose two answers
differ, i.e. the defect.  What the ratification asks for (§11.151(a)) is that
the TOGGLE move nothing - the angle the observer stands at, whatever it is, is
the same before and after.  Both statements are measured here:

  D1  place ANCHORED at lon 60, toggle free, toggle back
      pre  90.0 -> 60.0 -> 90.0     post  90.0 -> 90.0 -> 90.0
  D2  place FREE at lon 60, toggle anchored, toggle back
      pre  60.0 -> 90.0 -> 60.0     post  90.0 -> 90.0 -> 90.0
  D3  the LITERAL "60.0 before and after, both directions": on the fixed
      binary the place whose angle IS 60.0 deg is `moveto lon 210`
      (azimuth = lon - 90 = 120, rover at 60), and the toggle holds it.

THE ANGLE IS MEASURED FRAME-FREE, by the law of cosines on three dumped
distances - |E| (observer from the Moon's centre, from the DRAWN matrix),
|rover.ecl| and rover.dist - which is F39's own method, so the numbers are
directly comparable to §11.152(o)'s.  It is CROSS-CHECKED against the closed
form (the azimuth of Z(-theta).E against the authored orbit_lon), and the two
must agree: one instrument, two derivations.

`flag moon_scaled off` is a SCENE DECLARATION here (F39/§11.152's reading): the
subject is real geometry, and model == display keeps the law of cosines honest.
"""

import argparse, json, math, os, shutil, sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
from f27_reply import Session
import dumpread

HERE = Path(__file__).resolve().parent
FIELD = Path.home() / ".spacecrafter"
JD = 2461234.0
ROVER_LON = 60.0
AU_KM = 149597870.691

CHECKS = []


def chk(ok, label, detail=""):
    CHECKS.append({"ok": bool(ok), "label": label, "detail": detail})
    print(("OK   " if ok else "FAIL ") + label + (("  -- " + detail) if detail else ""),
          flush=True)


def rover_sections():
    # b24_screen's own authoring (11.78(d)/(j)), one body, on the equator.
    return (f"\n[DiscRover]\nname = DiscRover\nparent = Moon\nrelation = grounded\n"
            f"compose = explicit\ntype = Artificial\ncoord_func = surface_point\n"
            f"orbit_lon = {ROVER_LON}\norbit_lat = 0\norbit_alt = 0\nradius = 800\n"
            f"model_name = Curiosity\nhalo = false\n"
            f"[DiscRover:OJM]\nbody = DiscRover\ntype = OJM\n")


def prepare(dst):
    """`build_farm` leaves modularSystem/ empty; the composed body must be on
    disc BEFORE the load (F32's lesson)."""
    md = Path(dst) / "modularSystem"
    for f in (FIELD / "modularSystem").iterdir():
        shutil.copy2(f, md / f.name)
    twin = md / "SolarSystem.ini.disabled"
    (md / "SolarSystem.ini").write_bytes(twin.read_bytes()
                                         + rover_sections().encode("latin-1"))


def zrot(a):
    c, s = math.cos(a), math.sin(a)
    return np.array([[c, -s, 0.0], [s, c, 0.0], [0.0, 0.0, 1.0]])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=str(HERE.parents[1] / "build-claude/src/spacecrafter"))
    ap.add_argument("--tag", default="post")
    a = ap.parse_args()
    out = Path(a.outdir).resolve()
    out.mkdir(parents=True, exist_ok=True)

    sess = Session(out, f"f40disc{a.tag}", a.bin, prepare=prepare)
    drv = sess.client("drv")
    n = [0]

    def cmd(c, p=0.8):
        drv.send(c, p)

    def angle(name):
        """-> (law-of-cosines angle, closed-form angle, dict) for the CURRENT state."""
        n[0] += 1
        p = out / f"disc_{n[0]:03d}_{name}.json"
        drv.send(f"body action dual_dump filename {p}", 1.4)
        head, pairs, mnew, _ = dumpread.load_dump(p)
        cam = head["camera"]
        bodies = {r["name"]: r for r in pairs}
        rover = bodies["DiscRover"]["new"] if "DiscRover" in bodies else None
        if rover is None:                       # composed body has no old twin
            for line in open(p, encoding="utf-8", errors="replace"):
                try:
                    rec = dumpread.loads(line)
                except Exception:
                    continue
                if rec.get("name") == "DiscRover":
                    rover = rec["new"]
                    break
        m = np.array(cam["mat"], dtype=float).reshape(4, 4).T
        E = -m[:3, :3].T @ m[:3, 3]
        moon = bodies["Moon"]["new"]
        r = float(np.linalg.norm(rover["eclDisplay"]))
        d = float(rover["dist"])
        Eo = float(np.linalg.norm(E))
        cosa = (Eo * Eo + r * r - d * d) / (2 * Eo * r)
        law = math.degrees(math.acos(max(-1.0, min(1.0, cosa))))
        # closed form: the eye's azimuth in the reference's SURFACE frame
        th = moon["axisRot"] + math.pi / 2
        Es = zrot(-th) @ E
        az = math.degrees(math.atan2(Es[1], Es[0]))
        closed = abs(((az - ROVER_LON) + 180) % 360 - 180)
        rec = {"tag": name, "free": cam["freeMode"], "bound": cam["boundToSurface"],
               "law_deg": law, "closed_deg": closed, "obs_km": Eo * AU_KM,
               "rover_km": r * AU_KM, "sep_km": d * AU_KM, "azimuth_deg": az,
               "lon": cam["longitude"], "lat": cam["latitude"],
               "position": list(cam["position"]), "file": str(p)}
        print(f"     {name:16s} free={cam['freeMode']!s:5s} law={law:8.4f} deg "
              f"closed={closed:8.4f} deg  obs={rec['obs_km']:.1f} km "
              f"rover={rec['rover_km']:.1f} km sep={rec['sep_km']:.1f} km", flush=True)
        return rec

    for c in ("flag atmosphere off", "flag landscape off", "flag fog off",
              "timerate rate 0"):
        cmd(c, 0.5)
    cmd(f"date jday {JD}", 1.2)
    cmd("set home_planet Moon", 6.0)
    cmd("flag moon_scaled off", 2.0)          # scene declaration, F39/§11.152

    DATA = {"tag": a.tag, "binary": a.bin, "rover_orbit_lon": ROVER_LON}

    # ---- D1: placed ANCHORED, toggled ------------------------------------
    cmd("camera action free_mode state off", 1.5)
    cmd("moveto lat 0 lon 60 alt 8000000 duration 0", 2.0)
    d1 = [angle("D1_anchored")]
    cmd("camera action free_mode state on", 1.5)
    d1.append(angle("D1_free"))
    cmd("camera action free_mode state off", 1.5)
    d1.append(angle("D1_anchored_2"))
    DATA["D1"] = d1

    # ---- D2: placed FREE, toggled ----------------------------------------
    cmd("camera action free_mode state on", 1.5)
    cmd("moveto lat 0 lon 60 alt 8000000 duration 0", 2.0)
    d2 = [angle("D2_free")]
    cmd("camera action free_mode state off", 1.5)
    d2.append(angle("D2_anchored"))
    cmd("camera action free_mode state on", 1.5)
    d2.append(angle("D2_free_2"))
    DATA["D2"] = d2

    # ---- D3: the place whose angle IS 60 deg on the fixed binary ---------
    cmd("camera action free_mode state off", 1.5)
    cmd("moveto lat 0 lon 210 alt 8000000 duration 0", 2.0)
    d3 = [angle("D3_anchored")]
    cmd("camera action free_mode state on", 1.5)
    d3.append(angle("D3_free"))
    cmd("camera action free_mode state off", 1.5)
    d3.append(angle("D3_anchored_2"))
    DATA["D3"] = d3

    # ---- the gates -------------------------------------------------------
    for tag, legs in (("D1", d1), ("D2", d2), ("D3", d3)):
        for r in legs:
            chk(abs(r["law_deg"] - r["closed_deg"]) < 0.05,
                f"{tag}/{r['tag']}: the two derivations of the angle agree",
                f"law of cosines {r['law_deg']:.4f} vs closed form {r['closed_deg']:.4f} deg")
        span = max(r["law_deg"] for r in legs) - min(r["law_deg"] for r in legs)
        if a.tag == "post":
            chk(span < 0.05, f"{tag}: the TOGGLE does not move the observer",
                " -> ".join(f"{r['law_deg']:.4f}" for r in legs) + f" deg (span {span:.4f})")
        else:
            chk(span > 20.0, f"{tag}: the toggle swings the observer (the defect)",
                " -> ".join(f"{r['law_deg']:.4f}" for r in legs) + f" deg (span {span:.4f})")
    if a.tag == "post":
        chk(all(abs(r["law_deg"] - 90.0) < 0.05 for r in d1 + d2),
            "D1+D2: `moveto lon 60` names ONE place in both modes — the anchored 90.0 deg",
            " ; ".join(f"{r['tag']}={r['law_deg']:.4f}" for r in d1 + d2))
        chk(all(abs(r["law_deg"] - 60.0) < 0.05 for r in d3),
            "D3: 60.0 deg before AND after the toggle, both directions",
            " -> ".join(f"{r['law_deg']:.4f}" for r in d3) + " deg")
    else:
        chk(abs(d1[0]["law_deg"] - 90.0) < 0.05 and abs(d1[1]["law_deg"] - 60.0) < 0.05,
            "D1: §11.152(o)'s two numbers are the SAME command's two answers",
            f"anchored {d1[0]['law_deg']:.4f} deg -> free {d1[1]['law_deg']:.4f} deg")

    res = {"data": DATA, "checks": CHECKS,
           "failures": [c for c in CHECKS if not c["ok"]]}
    (out / f"f40_disc_{a.tag}.json").write_text(json.dumps(res, indent=1, default=str))
    n_ok = sum(1 for c in CHECKS if c["ok"])
    print(f"\nf40_disc [{a.tag}]: {n_ok}/{len(CHECKS)} checks OK", flush=True)
    sess.stop(drv)
    return 0 if n_ok == len(CHECKS) else 1


if __name__ == "__main__":
    sys.exit(main())
