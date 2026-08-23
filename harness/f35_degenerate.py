#!/usr/bin/env python3
"""F35 / INTENT §5.81 + §5.79 — the two shipped-reachable degenerate-input guards.

    cd claude/harness && export XAUTHORITY=$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.*) \
        && DISPLAY=:2 ./f35_degenerate.py <absOutdir> --bin <binary> --expect pre|post \
           [--legs A,B1,B2,B3]

ONE MANDATE SHAPE FOR BOTH HALVES: enumerate the consumers, establish the
reference answer for the degenerate input, fix only if decision-free. This
script is the MEASUREMENT half — it drives each degenerate input through a
SHIPPED command and records what the app answers, on the pre-fix binary and on
the post-fix one, with a positive control beside each.

--------------------------------------------------------------------- LEG A
§5.81: a body at `distance == 0` gets a NaN `screenPos`. `ModularBody::update`'s
centre-singularity guard covers rq→0 at a FINITE distance; at distance == 0 its
own fallback is `1/(distance·halfFov)` = inf and `mat.r[12]·inf` = NaN
[observed: ModularBody.hpp:486-506]. Newly reachable from the shipped command
`camera action transition_to target point`, which puts the camera AT its anchor
body by design (§11.143(c): `placeAt({0,0,0})`, distance 0 measured < 1e-12).

  A1 (positive control, the SAME launch, before the transition): every body in
     the dump carries a FINITE `screen` pair. This is the non-degenerate scene —
     the guard must not fire in it, on either binary.
  A2 the transition lands: the camera references `temp_point`, distance == 0.
  A3 `temp_point`'s `screen`:  PRE  -> both components non-finite (§11.143(l)'s
     `[-nan,-nan]`);  POST -> exactly [0.0, 0.0], the centre.
  A4 every OTHER body's `screen` stays finite after the transition (both).

  Both binaries write their full dumps into the result JSON, so the INERTNESS of
  the guard is checked by `f35_compare.py` as a field-by-field diff of the two
  runs rather than asserted here.

  NOTE (F34): `dumpTrace`'s `screen` is in normalized [0,1]-style viewport units,
  not pixels; a value outside the unit disc is legitimately off-screen. The
  check is FINITENESS, never magnitude.

--------------------------------------------------------------------- LEG B1
§5.79: `ConstellationMgr::getSelected()` returns `*selected.begin()` on a
possibly-empty vector [observed: constellation_mgr.cpp:717-720], reached from
`select constellation_star` [observed: core.cpp:1063 <- app_command_interface.cpp:3130].
`asterisms->setSelected(id)` does NOTHING when the abbreviation does not resolve
(`findFromAbbreviation` -> nullptr -> `setSelected` returns), so the very next
statement dereferences `selected.begin()` on whatever the vector then is.

  On THIS install 0 constellations load (§5.74), so `selected` was never grown:
  its `_M_start` is null and `*selected.begin()` is a null dereference. The row
  records this half as "not reproducible on this install"; the drive below is
  the discriminating check for that claim.

  B1 drive: `select constellation_star Ori` on a default launch.
     PRE  -> the app dies (no reply, process gone).
     POST -> the app is alive and answers; `get status object` == "EOL",
             `get status constellation` == "EOL" (nothing selected).

--------------------------------------------------------------------- LEG B2
The SECOND manifestation of the same root, and the positive control, both on a
launch where constellations actually exist — F31's fixture pattern (§11.141(f)):
a sky culture loaded from OUTSIDE the frozen field by `sky_culture action load
path <abs dir>`, taking constellations 0 -> 3. Nothing under ~/.spacecrafter is
written; the frozen md5 pair is asserted in == out by the Session.

  After `flag constellation_pick on` (isolateSelected; default false,
  constellation_mgr.hpp:219) a `deselect` CLEARS the selection vector
  (constellation_mgr.cpp:827). A cleared vector keeps its storage, so
  `*selected.begin()` then reads the STALE pointer left in slot 0 rather than
  crashing — a defined-looking WRONG answer instead of a fault.

  B2 sequence: load fixture -> `flag constellation_pick on`
     -> `select constellation_star F3A`      => O1 / C1   (the real answer)
     -> `deselect`                            => C2 == "EOL"
     -> `select constellation_star ZZNOSUCH`  => O3 / C3
          PRE  -> O3 == O1 (the stale slot re-selected F3A's brightest star)
          POST -> O3 == "EOL"
     -> `select constellation_star F3A`       => O4 / C4 == O1 / C1 on BOTH
          binaries. THE POSITIVE CONTROL: with a real selection, nothing changes.

--------------------------------------------------------------------- LEG B3
WHAT THE §5.79 FIX LEAVES BEHIND (added after the PRE run, measured on both).
The guard answers the EMPTY case and, by D8, must not touch any other: with a
NON-empty `selected` an unresolved abbreviation still makes `select
constellation_star` act on the constellation selected BEFORE the command,
because `setSelected(abbrev)` returns without touching anything when
`findFromAbbreviation` gives nullptr. Identical on both binaries by
construction; driven so that the row recording it is a measurement. → §5.87.

--------------------------------------------------------------- PREDICTIONS
Stated here and committed BEFORE the first measuring run (F29/F32 discipline):

  P1  A1/A4 hold on BOTH binaries — the guard is inert off the degenerate input.
  P2  A3 flips exactly once: non-finite PRE, exactly (0.0, 0.0) POST. (0,0) is
      not a chosen value: at distance == 0 the eye-frame x and y are EXACTLY 0
      (distance is their Euclidean norm with z), so `screenPos = (0·f, 0·f)` is
      (0,0) for EVERY finite f — only f = inf made it NaN.
  P3  B1 kills the app PRE and answers "EOL" POST.
  P4  B2's O3 equals O1 PRE (stale) and "EOL" POST, while O1/C1 and O4/C4 are
      identical between the two binaries.
"""

import argparse, json, os, re, shutil, sys, time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f27_reply as f27
import f31_search_drive as f31
import b24_equivalence as b24

HERE = Path(__file__).resolve().parent
DEFAULT_BIN = str(HERE.parents[1] / "build-claude/src/spacecrafter")
JD0 = 2461233.5

CHECKS = []


def chk(ok, label, detail=""):
    CHECKS.append({"ok": bool(ok), "label": label, "detail": detail})
    print(("OK   " if ok else "FAIL ") + label + (("  -- " + detail) if detail else ""),
          flush=True)
    return ok


def finite(x):
    try:
        v = float(x)
    except (TypeError, ValueError):
        return False
    return v == v and abs(v) != float("inf")


def newscreen(b):
    """`dual_dump` nests each path's per-body state under `old`/`new`; §5.81 is
    a NEW-path member, and the old path is the untouched baseline (§11.52(b)),
    so the subject is `new.screen`. A body the old tree does not carry has no
    `old` object at all — that is the case for the point anchor."""
    return b.get("new", {}).get("screen")


def ask(c, cmd, budget=6.0, quiet=0.8):
    """Send and read until the wire is quiet; return the decoded messages."""
    c.sock.sendall((cmd + "\n").encode())
    got, t0, last = b"", time.time(), time.time()
    while time.time() - t0 < budget:
        d = c.read(0.2)
        if d:
            got += d
            last = time.time()
        elif got and time.time() - last > quiet:
            break
    return [m.strip("\n") for m in f27.messages(got)]


def reply(c, cmd):
    """The LAST message the app sent in answer to a `get` — the reply payload."""
    ms = ask(c, cmd)
    return ms[-1] if ms else None


def ident(info):
    """WHICH object `get status object` is describing, with the time-varying
    part dropped. `Core::getSelectedObjectInfo` returns the full info string,
    whose tail carries alt/az and hour angle — quantities that move between two
    reads of the SAME selection, so a full-string comparison could never be an
    identity test. The first two lines are the name and the HP number
    (`StarWrapper::getInfoString`), which do not move."""
    if info is None:
        return None
    return "\n".join(info.split("\n")[:2]).strip()


# ----------------------------------------------------------------- leg A
def dump(sess, drv, tag, outdir, n):
    p = f"/tmp/f35_{n:03d}_{tag}.json"
    drv.send(f"body action dual_dump filename {p}", 1.4)
    shutil.copy(p, outdir / f"f35_dump_{tag}.json")
    with open(p) as f:
        head = json.loads(b24.sanitize_nonfinite(f.readline()))
        bodies = {}
        for line in f:
            line = line.strip()
            if line:
                o = json.loads(b24.sanitize_nonfinite(line))
                if o.get("type") == "body":
                    bodies[o["name"]] = o
    head["_bodies"] = bodies
    return head


def leg_A(out, binary, expect, res):
    sess = f27.Session(out, "f35a", binary)
    drv = sess.client("drv")
    try:
        for c in ("flag atmosphere off", "flag landscape off", "flag fog off",
                  "timerate rate 0"):
            drv.send(c, 0.5)
        drv.send(f"date jday {JD0:.9f}", 1.2)

        base = dump(sess, drv, "base", out, 1)
        bad = {n: newscreen(b) for n, b in base["_bodies"].items()
               if newscreen(b) is not None
               and not (finite(newscreen(b)[0]) and finite(newscreen(b)[1]))}
        chk(not bad and len(base["_bodies"]) > 0,
            "A1 non-degenerate scene: every body's `screen` is finite",
            f"{len(base['_bodies'])} bodies, {len(bad)} non-finite {list(bad)[:4]}")

        drv.send("camera action transition_to target point name Space", 1.5)
        after = dump(sess, drv, "point", out, 2)
        cam = after["camera"]
        chk(cam["reference"] == "temp_point" and abs(float(cam["distance"])) < 1e-12,
            "A2 the transition lands: camera references temp_point AT distance 0",
            f"ref={cam['reference']} distance={cam['distance']}")

        tp = after["_bodies"].get("temp_point")
        res["A_temp_point"] = tp
        if tp is None:
            chk(False, "A3 temp_point is in the dump", "absent")
        else:
            s = newscreen(tp)
            d = tp.get("new", {}).get("dist")
            fin = s is not None and finite(s[0]) and finite(s[1])
            if expect == "pre":
                chk(not fin, "A3 PRE: temp_point's screen is NON-FINITE (the defect)",
                    f"screen={s} dist={d}")
            else:
                chk(fin and float(s[0]) == 0.0 and float(s[1]) == 0.0,
                    "A3 POST: temp_point's screen is exactly the centre (0,0)",
                    f"screen={s} dist={d}")

        bad2 = {n: newscreen(b) for n, b in after["_bodies"].items()
                if n != "temp_point" and newscreen(b) is not None
                and not (finite(newscreen(b)[0]) and finite(newscreen(b)[1]))}
        chk(not bad2,
            "A4 after the transition every OTHER body's `screen` is still finite",
            f"{len(after['_bodies'])} bodies, {len(bad2)} non-finite {list(bad2)[:4]}")

        res["A_base_bodies"] = {n: b for n, b in base["_bodies"].items()}
        res["A_point_bodies"] = {n: b for n, b in after["_bodies"].items()}
        res["A_base_camera"] = base["camera"]
        res["A_point_camera"] = after["camera"]
    finally:
        res["A_exit"] = sess.stop(drv)


# ----------------------------------------------------------------- leg B1
def leg_B1(out, binary, expect, res):
    sess = f27.Session(out, "f35b1", binary)
    drv = sess.client("drv")
    r = {}
    try:
        r["const_before"] = reply(drv, "get status constellation")
        r["object_before"] = reply(drv, "get status object")
        chk(r["const_before"] == "EOL",
            "B1 baseline: nothing selected before the drive",
            f"get status constellation -> {r['const_before']!r}")

        # THE DEGENERATE DRIVE — a shipped command, one line.
        drv.sock.sendall(b"select constellation_star Ori\n")
        time.sleep(3.0)
        r["alive_after"] = sess.proc.poll() is None
        r["returncode"] = sess.proc.poll()
        if r["alive_after"]:
            r["const_after"] = reply(drv, "get status constellation")
            r["object_after"] = reply(drv, "get status object")
        if expect == "pre":
            chk(not r["alive_after"],
                "B1 PRE: `select constellation_star Ori` KILLS the app (the defect)",
                f"returncode={r['returncode']}")
        else:
            chk(r["alive_after"],
                "B1 POST: the app survives `select constellation_star Ori`",
                f"returncode={r['returncode']}")
            chk(r.get("const_after") == "EOL" and r.get("object_after") == "EOL",
                "B1 POST: the defined answer is `nothing selected`",
                f"constellation={r.get('const_after')!r} object={r.get('object_after')!r}")
    finally:
        res["B1"] = r
        if sess.proc.poll() is None:
            res["B1"]["exit"] = sess.stop(drv)
        else:
            for c in sess.clients:
                c.close()
            for n in f27.FROZEN:
                m = f27.md5(f27.SRC_HOME / n)
                if m != sess.md5_in[n]:
                    chk(False, f"B1 frozen {n} md5 in != out",
                        f"{sess.md5_in[n]} -> {m}")


# ----------------------------------------------------------------- leg B2
def leg_B2(out, binary, expect, res):
    fixture = out / "culture_f35"
    res["B2_fixture"] = f31.build_fixture(fixture)
    sess = f27.Session(out, "f35b2", binary)
    drv = sess.client("drv")
    r = {}
    try:
        ask(drv, f"sky_culture action load path {fixture}", budget=25.0, quiet=3.0)
        ask(drv, "flag constellation_pick on")
        r["O1"] = reply(drv, "get status object")
        r["C1"] = reply(drv, "get status constellation")
        ask(drv, "select constellation_star F3A")
        r["O1"] = reply(drv, "get status object")
        r["C1"] = reply(drv, "get status constellation")
        chk(r["C1"] not in (None, "EOL"),
            "B2 the fixture culture gives a REAL constellation selection",
            f"constellation={r['C1']!r} object={str(r['O1'])[:60]!r}")

        ask(drv, "deselect")
        r["C2"] = reply(drv, "get status constellation")
        r["O2"] = reply(drv, "get status object")
        chk(r["C2"] == "EOL",
            "B2 `deselect` with constellation_pick on empties the selection vector",
            f"constellation={r['C2']!r}")

        # THE DEGENERATE DRIVE on a CLEARED (not never-grown) vector.
        drv.sock.sendall(b"select constellation_star ZZNOSUCH\n")
        time.sleep(2.5)
        r["alive"] = sess.proc.poll() is None
        r["returncode"] = sess.proc.poll()
        if r["alive"]:
            r["O3"] = reply(drv, "get status object")
            r["C3"] = reply(drv, "get status constellation")
        r["id1"], r["id3"] = ident(r["O1"]), ident(r.get("O3"))
        if expect == "pre":
            chk(not r["alive"] or r["id3"] == r["id1"],
                "B2 PRE: the cleared slot is re-read — the unknown abbreviation "
                "silently re-selects the previous constellation's star (or faults)",
                f"alive={r['alive']} id3={r['id3']!r} id1={r['id1']!r}")
        else:
            chk(r["alive"] and r.get("O3") == "EOL" and r.get("C3") == "EOL",
                "B2 POST: an unknown abbreviation selects NOTHING",
                f"O3={r.get('O3')!r} C3={r.get('C3')!r}")

        if r["alive"]:
            ask(drv, "select constellation_star F3A")
            r["O4"] = reply(drv, "get status object")
            r["C4"] = reply(drv, "get status constellation")
            r["id4"] = ident(r["O4"])
            chk(r["id4"] == r["id1"] and r["C4"] == r["C1"],
                "B2 POSITIVE CONTROL: with a real selection the answer is unchanged",
                f"id4={r['id4']!r} id1={r['id1']!r} C4={r['C4']!r} C1={r['C1']!r}")
    finally:
        res["B2"] = r
        if sess.proc.poll() is None:
            res["B2"]["exit"] = sess.stop(drv)
        else:
            for c in sess.clients:
                c.close()


# ----------------------------------------------------------------- leg B3
def leg_B3(out, binary, expect, res):
    """WHAT THE §5.79 FIX LEAVES BEHIND, measured rather than derived.

    The guard answers the EMPTY case. It does not — and must not, D8 — touch the
    case where `selected` is NON-empty and the abbreviation does not resolve:
    `ConstellationMgr::setSelected(abbrev)` returns without touching anything
    when `findFromAbbreviation` gives nullptr, so `getSelected()` still answers
    with the constellation selected BEFORE the command, and `select
    constellation_star <unknown>` acts on a constellation it was not asked
    about. Identical on both binaries by construction; measured here so the row
    that records it is a measurement."""
    fixture = out / "culture_f35"
    if not fixture.exists():
        res["B3_fixture"] = f31.build_fixture(fixture)
    sess = f27.Session(out, "f35b3", binary)
    drv = sess.client("drv")
    r = {}
    try:
        ask(drv, f"sky_culture action load path {fixture}", budget=25.0, quiet=3.0)
        ask(drv, "select constellation_star F3A")
        r["O1"], r["C1"] = reply(drv, "get status object"), reply(drv, "get status constellation")
        ask(drv, "select constellation_star ZZNOSUCH")
        r["O2"], r["C2"] = reply(drv, "get status object"), reply(drv, "get status constellation")
        r["id1"], r["id2"] = ident(r["O1"]), ident(r["O2"])
        chk(r["id2"] == r["id1"] and r["C2"] == r["C1"] and r["C1"] == "F3A",
            "B3 an unresolved abbreviation with a NON-empty selection still acts on "
            "the PREVIOUS constellation — unchanged by the fix, on both binaries",
            f"id1={r['id1']!r} id2={r['id2']!r} C1={r['C1']!r} C2={r['C2']!r}")
    finally:
        res["B3"] = r
        if sess.proc.poll() is None:
            res["B3"]["exit"] = sess.stop(drv)
        else:
            for c in sess.clients:
                c.close()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=DEFAULT_BIN)
    ap.add_argument("--expect", choices=("pre", "post"), required=True)
    ap.add_argument("--legs", default="A,B1,B2,B3")
    a = ap.parse_args()
    out = Path(a.outdir)
    out.mkdir(parents=True, exist_ok=True)
    legs = [x.strip() for x in a.legs.split(",") if x.strip()]

    res = {"binary": a.bin, "binary_md5": f27.md5(a.bin), "expect": a.expect,
           "legs": legs, "wall_start": time.strftime("%F %T %Z")}
    print(json.dumps({k: res[k] for k in ("binary", "binary_md5", "expect", "legs")},
                     indent=2), flush=True)

    if "A" in legs:
        leg_A(out, a.bin, a.expect, res)
    if "B1" in legs:
        leg_B1(out, a.bin, a.expect, res)
    if "B2" in legs:
        leg_B2(out, a.bin, a.expect, res)
    if "B3" in legs:
        leg_B3(out, a.bin, a.expect, res)

    res["checks"] = CHECKS
    res["fails"] = [c["label"] for c in CHECKS if not c["ok"]]
    res["wall_end"] = time.strftime("%F %T %Z")
    (out / f"f35_result_{a.expect}.json").write_text(json.dumps(res, indent=1))
    print(f"\n{len(CHECKS) - len(res['fails'])}/{len(CHECKS)} checks pass", flush=True)
    for f in res["fails"]:
        print("  FAIL " + f, flush=True)
    return 1 if res["fails"] else 0


if __name__ == "__main__":
    sys.exit(main())
