#!/usr/bin/env python3
"""§5.63 attribution, wave 1 of F22: the OLD PATH'S OWN VIEW STATE, read back.

§5.63 is owed one probe and it is named in the row: *"a readback of the old
path's own view state (`Navigator::getLocalVision` / `getPrecEquVision` /
`mat_local_to_earth_equ`) on both sides - which does not exist today"*.  It
exists now (INTENT §11.130, code 170c3ad6): the dual dump's header carries an
`oldView` object written by the four owners of the state the old sky is drawn
from - navigator, observer, projector, star pipeline - plus the refraction
decision Core itself makes.

THE INSTRUMENT'S OWN DISCRIMINATION TEST, which this run performs before it
performs anything else: the same three launches §11.129 used

  L1  scene A built by commands            -> the SAVED side (+ the session)
  L2  scene A built by the same commands   -> the in-scene A/A FLOOR
  L3  fresh launch, session RESTORED       -> the RESTORED side

give three dumps of the same fields.  A field is a CANDIDATE only if it
differs L1 vs L3 *and* agrees L1 vs L2 - the floor is what makes the statement
mean anything (§11.80(a)), and a field that moves between two identical
rebuilds cannot attribute anything.  The screen is shot at each stage from the
same launches, so the field table and the pixel table are the same run.

The stars-off leg is the second half of the discrimination: with `flag stars
off` on all three sides the screen agrees (§11.129(b) measured 0 px>8 with the
three sky families off), so any field that still differs there is NOT what the
screen difference is made of, and any field that stops differing is coupled to
the content that carries it.

    cd claude/harness && DISPLAY=:2 ./f22_s563_view.py [outdir]
"""

import json
import os
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f21_s563
from f21_s563 import (App, JD, SESSIONS, USERDIR, FROZEN, md5,
                      assert_no_other_instance, build_scene_a, stats)

import numpy as np

_args = [a for a in sys.argv[1:] if not a.startswith("--")]
OUT = (Path(_args[0]) if _args else Path(__file__).resolve().parent / "artifacts/f22view").resolve()
OUT.mkdir(parents=True, exist_ok=True)
f21_s563.OUT = OUT          # App.shot / App.log write here


# --------------------------------------------------------------------------
# Field flattening: the dump is nested JSON, the comparison is per LEAF.
# --------------------------------------------------------------------------
def flatten(obj, prefix=""):
    out = {}
    if isinstance(obj, dict):
        for k, v in obj.items():
            out.update(flatten(v, f"{prefix}.{k}" if prefix else k))
    elif isinstance(obj, list):
        for i, v in enumerate(obj):
            out.update(flatten(v, f"{prefix}[{i}]"))
    else:
        out[prefix] = obj
    return out


def read_header(path):
    """First JSON line of a dual dump = the header (camera + oldView + gates)."""
    with open(path) as f:
        for line in f:
            line = line.strip()
            if line:
                return json.loads(line)
    raise RuntimeError(f"{path}: empty dump")


def dump(app, tag):
    p = OUT / f"f22_{tag}.json"
    p.unlink(missing_ok=True)
    app.cmd(f"body action dual_dump filename {p}", 2.0)
    for _ in range(30):
        if p.exists() and p.stat().st_size > 0:
            break
        time.sleep(0.3)
    return read_header(p)


def diff(a, b, tol=0.0):
    """Leaf fields present in both and differing by more than tol."""
    fa, fb = flatten(a), flatten(b)
    out = {}
    for k in sorted(set(fa) | set(fb)):
        va, vb = fa.get(k, "<absent>"), fb.get(k, "<absent>")
        if isinstance(va, (int, float)) and isinstance(vb, (int, float)) \
                and not isinstance(va, bool) and not isinstance(vb, bool):
            if abs(va - vb) > tol:
                out[k] = (va, vb, abs(va - vb))
        elif va != vb:
            out[k] = (va, vb, None)
    return out


def report(title, d, limit=200):
    print(f"\n== {title} ({len(d)} differing leaves) ==", flush=True)
    for i, (k, (va, vb, delta)) in enumerate(d.items()):
        if i >= limit:
            print(f"  ... {len(d) - limit} more", flush=True)
            break
        if delta is None:
            print(f"  {k:<48} {va!r:>24} -> {vb!r}", flush=True)
        else:
            print(f"  {k:<48} {va:>24.12g} -> {vb:<24.12g} |d| {delta:.6g}", flush=True)


def main():
    print(f"SC_BIN = {f21_s563.SC_BIN}", flush=True)
    if not assert_no_other_instance():
        return 1
    SESSIONS.mkdir(exist_ok=True)
    for f in SESSIONS.glob("f22v*.ini"):
        f.unlink()
    frozen_in = {n: md5(USERDIR / n) for n in FROZEN if (USERDIR / n).exists()}

    res = {}

    # ---- L1: the saved side -------------------------------------------------
    app = App("v_saved")
    build_scene_a(app)
    app.cmd("session action save filename f22v", 2.5)
    h_saved = dump(app, "saved_base")
    img_saved = app.shot("v_saved_base")
    app.cmd("flag stars off", 2.0)
    h_saved_ns = dump(app, "saved_nostars")
    img_saved_ns = app.shot("v_saved_nostars")
    app.quit()

    # ---- L2: the in-scene A/A floor ----------------------------------------
    app = App("v_aa")
    build_scene_a(app)
    h_aa = dump(app, "aa_base")
    img_aa = app.shot("v_aa_base")
    app.cmd("flag stars off", 2.0)
    h_aa_ns = dump(app, "aa_nostars")
    img_aa_ns = app.shot("v_aa_nostars")
    app.quit()

    # ---- L3: the restored side ---------------------------------------------
    app = App("v_rest")
    app.cmd("session action load filename f22v", 6.0)
    h_rest = dump(app, "rest_base")
    img_rest = app.shot("v_rest_base")
    app.cmd("flag stars off", 2.0)
    h_rest_ns = dump(app, "rest_nostars")
    img_rest_ns = app.shot("v_rest_nostars")
    app.quit()

    frozen_out = {n: md5(USERDIR / n) for n in FROZEN if (USERDIR / n).exists()}
    ok_frozen = frozen_in == frozen_out
    print(f"\nfrozen md5 in == out: {ok_frozen}", flush=True)
    res["frozen_ok"] = ok_frozen

    # ---- the screen, so field table and pixel table are ONE run ------------
    s_rest = stats(img_saved, img_rest)
    s_floor = stats(img_saved, img_aa)
    s_rest_ns = stats(img_saved_ns, img_rest_ns)
    s_floor_ns = stats(img_saved_ns, img_aa_ns)
    print("\n== the screen, same launches ==", flush=True)
    print(f"  base      restored {s_rest['px8']:>8} px>8   floor {s_floor['px8']:>8}   "
          f"lit {s_rest['lit_a']} -> {s_rest['lit_b']}", flush=True)
    print(f"  starsoff  restored {s_rest_ns['px8']:>8} px>8   floor {s_floor_ns['px8']:>8}   "
          f"lit {s_rest_ns['lit_a']} -> {s_rest_ns['lit_b']}", flush=True)
    res["screen"] = {"base": {"restored": s_rest, "floor": s_floor},
                     "nostars": {"restored": s_rest_ns, "floor": s_floor_ns}}

    # ---- the fields --------------------------------------------------------
    d_floor = diff(h_aa, h_saved)
    d_rest = diff(h_saved, h_rest)
    d_floor_ns = diff(h_aa_ns, h_saved_ns)
    d_rest_ns = diff(h_saved_ns, h_rest_ns)

    report("FLOOR: A/A rebuild vs saved  (anything here attributes NOTHING)", d_floor)
    report("RESTORED vs saved", d_rest)

    cand = {k: v for k, v in d_rest.items() if k not in d_floor}
    report("CANDIDATES: differ restored-vs-saved AND agree in the A/A floor", cand)

    cand_ns = {k: v for k, v in d_rest_ns.items() if k not in d_floor_ns}
    report("CANDIDATES with stars off (the screen agrees here)", cand_ns)

    survives = sorted(set(cand) & set(cand_ns))
    print(f"\n== candidates that SURVIVE the stars-off control "
          f"(differ with the screen agreeing -> not what the screen is made of): "
          f"{len(survives)} ==", flush=True)
    for k in survives:
        print(f"  {k}", flush=True)
    only_lit = sorted(set(cand) - set(cand_ns))
    print(f"\n== candidates that COLLAPSE with stars off "
          f"(differ only where the screen differs): {len(only_lit)} ==", flush=True)
    for k in only_lit:
        print(f"  {k}", flush=True)

    res["floor_fields"] = {k: list(v) for k, v in d_floor.items()}
    res["restored_fields"] = {k: list(v) for k, v in d_rest.items()}
    res["candidates"] = {k: list(v) for k, v in cand.items()}
    res["candidates_nostars"] = {k: list(v) for k, v in cand_ns.items()}
    res["survives_control"] = survives
    res["collapses_with_stars_off"] = only_lit
    (OUT / "f22_s563_view.json").write_text(json.dumps(res, indent=1))
    print(f"\nartifacts in {OUT}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
