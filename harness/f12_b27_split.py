#!/usr/bin/env python3
"""B27 / D27 counterfactual battery — `light_source` splits into `light_source`
+ `primary` (INTENT §11.113(f), delivered §11.118).

WHAT IT IS FOR. Until this change one key carried two unrelated promises: "this
body emits light" and "this body is the thing its subsystem is built around".
D27 [vixy]: *"Split light_source and primary flags, the first one for light
purpose and the second one for isStar() purpose minus light source."* The
per-site assignment is DERIVED, not given, so every moved consumer needs a leg
that shows the two keys now act independently — the §11.107 nine-leg pattern.

    cd claude/harness && DISPLAY=:2 ./f12_b27_split.py <outdir> [--bin B] [--prebin B]

PHASE A — the capability legs (dump observables, one fresh launch each):

  W0  composed twin as generated ............. control; both keys on the Sun
  W1  composed, `primary` stripped (Sun) ..... the STRUCTURAL half alone moves:
                                               planets become satellites, so
                                               Earth loses its TRAIL module,
                                               while the Sun stays a star
                                               (bodyType 64, day-length line,
                                               StarLoader routing)
  W2  composed, `light_source` stripped ...... the LUMINOUS half alone moves:
                                               bodyType 64->7, the CUSTOM slot
                                               goes to GridLoader, the
                                               day-length line disappears -
                                               and Earth KEEPS its trail,
                                               which is what could not be
                                               expressed before the split
  W3  composed, BOTH stripped ................ reproduces §11.107's V1 exactly
                                               (the un-split behaviour), so the
                                               split is shown to be a
                                               refinement and not a change
  W4  legacy control ......................... D9/D14: `type = Sun` still
                                               grants the whole bundle
  W5  composed, `primary = true` on EARTH .... a PRIMARY THAT DOES NOT SHINE -
                                               the capability the split exists
                                               for: the Moon stops being a
                                               satellite and gains a TRAIL,
                                               with Earth's bodyType untouched
  W2pre  the W2 corpus on the PRE-SPLIT binary  same bytes, older engine: it
                                               has no `primary`, so Earth
                                               loses its trail. Same data, two
                                               answers = the split is what is
                                               being measured.

PHASE B — the HINT gate at the SCREEN (the site where this implementation
diverges from §11.113(f)'s provisional list; see the source comment in
HintModule.cpp). The Sun's hint is drawn only because the angular-separation
gate is skipped for it. W0 vs W1 is an A/B of the SAME scene under two corpora
that differ in one key: the hint must appear in W0 and vanish in W1, with a
measured same-corpus floor, and the measurement box is asserted to contain no
other body (no foreign halo can paint into it).
"""

import json, os, re, shutil, socket, subprocess, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import b25_galactic as b25g
import b24_equivalence as b24

HERE = Path(__file__).resolve().parent
SRC = Path.home() / ".spacecrafter"
JD = "2461233.5"
DEFAULT_BIN = str(HERE.parents[1] / "build-claude/src/spacecrafter")

FAILS = []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


# ------------------------------------------------------------- corpus editing
def author(text, section, edits):
    """edits: {key: value or None(delete)} inside [section]. Verbatim elsewhere."""
    lines, out, cur, done = text.split("\n"), [], None, set()
    for ln in lines:
        s = ln.strip()
        if s.startswith("[") and s.endswith("]"):
            if cur == section:
                for k, v in edits.items():
                    if v is not None and k not in done:
                        out.append(f"{k} = {v}")
            cur = s[1:-1]
        elif cur == section and "=" in s and not s.startswith("#"):
            k = s.split("=", 1)[0].strip()
            if k in edits:
                done.add(k)
                if edits[k] is None:
                    continue
                out.append(f"{k} = {edits[k]}")
                continue
        out.append(ln)
    if cur == section:
        for k, v in edits.items():
            if v is not None and k not in done:
                out.append(f"{k} = {v}")
    return "\n".join(out)


# ------------------------------------------------------------------ app cycle
def farm(out, tag):
    return b25g.build_farm(farm=out / f"farm_{tag}", dotted=False, corpus=None)


def launch(dst, farmroot, binary, applog):
    proc = subprocess.Popen([binary], cwd=str(dst),
                            stdout=open(applog, "w"), stderr=subprocess.STDOUT,
                            env={**os.environ, "HOME": str(farmroot),
                                 "DISPLAY": os.environ.get("DISPLAY", ":2")})
    sock, t0 = None, time.time()
    while time.time() - t0 < 90:
        if proc.poll() is not None:
            raise RuntimeError("app died before opening its port")
        try:
            sock = socket.create_connection(("127.0.0.1", 7805), timeout=1)
            break
        except OSError:
            time.sleep(1)
    if sock is None:
        proc.kill()
        raise RuntimeError("port 7805 never opened")
    time.sleep(10)
    return proc, sock


def send(sock, cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2)
        sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)


def stop(proc, sock):
    send(sock, "shutdown action now")
    sock.close()
    try:
        return proc.wait(timeout=40)
    except subprocess.TimeoutExpired:
        proc.kill()
        return -1


def warnings_of(applog):
    txt = Path(applog).read_text(errors="replace")
    return [re.sub(r"\s+", " ", m).strip()
            for m in re.findall(r"[^\n]*type-as-identity is retired[^\n]*", txt)]


def nonfinite(dump):
    """Bodies carrying a non-finite float in this dump. The §11.18 ASmooth
    class still fires intermittently on a COLD HOME (measured 2026-07-30: 2 of
    13 launches, always on the Moon or the Sun - the two bodies config scales at
    init, so `scaling` goes NaN and every radius derived from it follows). It
    does not touch the integer/enum observables below, but it DOES corrupt the
    screen phase, so it is named per leg rather than left to be noticed."""
    _, bodies = b24.load_dump(dump)
    bad = {}
    for n, v in bodies.items():
        hit = [k for k, x in v.items()
               if isinstance(x, float) and (x != x or x in (float("inf"), float("-inf")))]
        if hit:
            bad[n] = hit
    return bad


def observe(dump):
    _, bodies = b24.load_dump(dump)
    out = {}
    for n in ("Sun", "Earth", "Moon", "Pallas"):
        v = bodies.get(n)
        if v is None:
            out[n] = None
            continue
        tr = v.get("trail") or []
        out[n] = {
            "bodyType": v.get("bodyType"), "primary": v.get("primary"),
            "modules": v.get("modules"), "routing": v.get("routing"),
            "trailLength": v.get("trailLength"),
            "maxTrail": tr[0].get("maxTrail") if tr else None,
            "screen": v.get("screen"), "composedDecl": v.get("composedDecl"),
        }
    return out, len(bodies)


def daylength(dump):
    p = Path(str(dump) + ".navstr")
    if not p.exists():
        return None
    nav = p.read_bytes().decode("latin-1")
    m = re.search(r"^Sun\n  OLD nav:.*?\n  NEW nav: (.*?)\n  OLD inf:", nav, re.S | re.M)
    return ("Day length" in m.group(1)) if m else None


# --------------------------------------------------------------------- legs
def deduce(text, body):
    """Opt ONE body back into G6 deduction (§11.107(e)): `compose = deduced`
    plus the removal of its explicit `[<body>:<FAMILY>]` module sections. Needed
    because in an `explicit` twin the module SET is declared, so isSatellite's
    deduction consumer is inert there - the only place a dump can see the
    structural half move."""
    out = []
    for sec in re.split(r"\n(?=\[)", text):
        if sec.startswith(f"[{body}:"):
            continue
        if sec.startswith(f"[{body}]"):
            sec = sec.replace("compose = explicit", "compose = deduced")
        out.append(sec)
    return "\n".join(out)


LEGS = {
    #  tag: (composed?, [(section, edits) ...], deduced body)
    "W0": (True, [], None),
    "W1": (True, [("Sun", {"primary": None})], None),
    "W2": (True, [("Sun", {"light_source": None})], None),
    "W3": (True, [("Sun", {"light_source": None, "primary": None})], None),
    "W4": (False, [], None),
    "W5": (True, [("Earth", {"primary": "true"})], None),
    "W6": (True, [], "Earth"),
    "W7": (True, [("Sun", {"primary": None})], "Earth"),
}


def build_corpus(twin_base, edits, deduced):
    twin = twin_base
    for section, e in edits:
        twin = author(twin, section, e)
    if deduced:
        twin = deduce(twin, deduced)
    return twin


def prepare(out, tag, twin_base, composed, edits, deduced=None):
    dst = farm(out, tag)
    if composed:
        (dst / "modularSystem").mkdir(exist_ok=True)
        (dst / "modularSystem/SolarSystem.ini").write_bytes(
            build_corpus(twin_base, edits, deduced).encode("latin-1"))
    return dst


def run_leg(out, tag, binary, dst):
    dump = out / f"f12b27_{tag}.json"
    applog = out / f"f12b27_{tag}.applog"
    dump.unlink(missing_ok=True)
    proc, sock = launch(dst, dst.parent, binary, applog)
    send(sock, "timerate rate 0")
    send(sock, f"date jday {JD}")
    time.sleep(3)
    send(sock, f"body action dual_dump filename {dump}", 2.5)
    stop(proc, sock)
    if not dump.exists():
        raise RuntimeError(f"{tag}: no dump")
    obs, nbodies = observe(dump)
    bad = nonfinite(dump)
    if bad:
        print(f"    NOTE {tag}: non-finite fields present (§11.18 ASmooth cold-launch "
              f"class, pre-existing - measured on BOTH binaries): {bad}", flush=True)
    return {"bodies": nbodies, "obs": obs, "warnings": warnings_of(applog),
            "daylength_on_sun": daylength(dump), "nonfinite": bad,
            "satellite": satellite_table(dump)}


def satellite_table(dump):
    """isSatellite() recomputed in the harness from the dump's own `parent` and
    `primary` fields - `parent && !(parent->isPrimary() || parent->isSystem())`,
    ModularBody.hpp. The engine does not expose the predicate directly, so this
    is the CLAIM being checked against the drawn consequences in phase B, not a
    substitute for them."""
    _, bodies = b24.load_dump(dump)
    sysnodes = {n for n in bodies if n.endswith("System")} | {"Universe", "MilkyWay"}
    out = {}
    for n, v in bodies.items():
        p = v.get("parent")
        if not p or p not in bodies:
            out[n] = False
            continue
        out[n] = not (bool(bodies[p].get("primary")) or p in sysnodes)
    return out


# ------------------------------------------------------------------ phase B
def img(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.int32)


def run_screen(out, scene, tag, binary, dst, shots=3):
    """One rendered scene, `shots` frames apart (the floor is measured, never
    assumed) plus a dump of the same state. `scene` is "hint" or "orbit"."""
    dump = out / f"f12scr_{scene}_{tag}.json"
    applog = out / f"f12scr_{scene}_{tag}.applog"
    dump.unlink(missing_ok=True)
    proc, sock = launch(dst, dst.parent, binary, applog)
    send(sock, "flag experimental_path on", 1)
    send(sock, "timerate rate 0", 1)
    send(sock, f"date jday {JD}", 1)
    send(sock, "flag landscape off", 1)             # never compare masked content
    send(sock, "flag atmosphere off", 1)
    send(sock, "flag stars off", 1)
    send(sock, "flag nebulae off", 1)
    send(sock, "flag constellation_drawing off", 1)
    send(sock, "flag object_trails off", 1)
    send(sock, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2.5)
    if scene == "hint":
        send(sock, "flag planet_orbits off", 1)     # routing changes must not paint
        send(sock, "flag satellites_orbits off", 1)
        send(sock, "select planet Sun pointer off", 1)
        send(sock, "flag track_object on", 5)
        send(sock, "flag track_object off", 1.5)
        send(sock, "zoom fov 8 duration 0", 2)
        send(sock, "flag planet_names on", 1.5)
    else:                                            # "orbit"
        send(sock, "flag planet_names off", 1)
        send(sock, "flag planet_orbits on", 1)       # non-satellites draw an orbit
        send(sock, "flag satellites_orbits off", 1)  # satellites do not
        send(sock, "select planet Moon pointer off", 1)
        send(sock, "flag track_object on", 5)
        send(sock, "flag track_object off", 1.5)
        send(sock, "zoom fov 40 duration 0", 2)
    time.sleep(6)                                    # faders + luminance adaptation
    paths = []
    for i in range(shots):
        p = out / f"f12scr_{scene}_{tag}_{i}.png"
        send(sock, f"body action screenshot filename {p}", 2.0)
        paths.append(p)
    ctrl = None
    if scene == "orbit":
        # POSITIVE CONTROL, same launch (§11.111(i2): a null on an un-aimed
        # scene is not a null): flip the SATELLITE master on and the Moon's
        # orbit must appear here, proving the line is in frame at all and that
        # the subject leg measures the ROUTING, not an empty scene.
        send(sock, "flag satellites_orbits on", 2)
        time.sleep(4)
        ctrl = out / f"f12scr_{scene}_{tag}_ctrl.png"
        send(sock, f"body action screenshot filename {ctrl}", 2.5)
        send(sock, "flag satellites_orbits off", 1.5)
    send(sock, f"body action dual_dump filename {dump}", 2.5)
    stop(proc, sock)
    unknown = sorted(set(re.findall(r"[\w:]+ is unknown\. Did you mean [\w:]+ \?",
                                    Path(applog).read_text(errors="replace"))))
    if unknown:
        fail(f"screen/{scene} {tag}: the app REJECTED a command - {unknown}. "
             f"Every flag in this scene must land or the leg measures nothing.")
    return paths, dump, ctrl


def floor_of(paths, sl=None):
    """Worst same-corpus pairwise difference over the shot series."""
    worst = 0
    for i in range(len(paths)):
        for j in range(i + 1, len(paths)):
            a, b = img(paths[i]), img(paths[j])
            if sl is not None:
                x0, y0, x1, y1 = sl
                a, b = a[y0:y1, x0:x1], b[y0:y1, x0:x1]
            worst = max(worst, int((np.abs(a - b).max(axis=2) > 8).sum()))
    return worst


def box_of(screen, w, h, half):
    """NDC screen pos -> pixel box (x0, y0, x1, y1)."""
    cx = (screen[0] * 0.5 + 0.5) * w
    cy = (1.0 - (screen[1] * 0.5 + 0.5)) * h
    return (max(0, int(cx - half)), max(0, int(cy - half)),
            min(w, int(cx + half)), min(h, int(cy + half)))


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    out = Path(args[0]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    binary = DEFAULT_BIN
    prebin = None
    for a in sys.argv[1:]:
        if a.startswith("--bin="):
            binary = a.split("=", 1)[1]
        if a.startswith("--prebin="):
            prebin = a.split("=", 1)[1]

    src_md5 = b25g.real_tree_md5()
    report = {}

    # ---- generate the twin with THIS binary (it must carry the new key) -----
    gen = farm(out, "gen")
    proc, sock = launch(gen, gen.parent, binary, out / "f12b27_gen.applog")
    stop(proc, sock)
    twin_path = gen / "modularSystem/SolarSystem.ini.disabled"
    if not twin_path.exists():
        fail("no twin generated - cannot run the composed legs")
        return 1
    twin_base = twin_path.read_bytes().decode("latin-1")
    sun_keys = {k: v for k, v in re.findall(r"^(\w+) = (.*)$",
                author(twin_base, "Sun", {}).split("[Sun]", 1)[1].split("\n[", 1)[0],
                re.M)}
    if sun_keys.get("primary") == "true" and sun_keys.get("light_source") == "true":
        ok("twin emits BOTH keys on the legacy star (light_source = true, primary = true)")
    else:
        fail(f"twin does not emit both keys on the Sun: {sun_keys}")
    census = {k: len(re.findall(rf"^{k} = ", twin_base, re.M))
              for k in ("light_source", "primary", "shadow_exempt",
                        "surface_model", "trail_length")}
    print(f"    twin key census: {census}", flush=True)
    report["twin_census"] = census

    # ---- phase A -----------------------------------------------------------
    for tag, (composed, edits, deduced) in LEGS.items():
        dst = prepare(out, tag, twin_base, composed, edits, deduced)
        report[tag] = run_leg(out, tag, binary, dst)
        o = report[tag]["obs"]
        print(f"[{tag}] Sun bodyType={o['Sun']['bodyType']} primary={o['Sun']['primary']} "
              f"routing={o['Sun']['routing']['far']}/{o['Sun']['routing']['near']} | "
              f"Earth TRAIL={'TRAIL' in o['Earth']['modules']} trail={o['Earth']['routing']['trail']} | "
              f"Earth satellite={report[tag]['satellite'].get('Earth')} "
              f"Moon satellite={report[tag]['satellite'].get('Moon')} | "
              f"daylen={report[tag]['daylength_on_sun']} warns={len(report[tag]['warnings'])}",
              flush=True)
    if prebin:
        # The W7 corpus is the cross-binary one: it is the leg where `primary`
        # ALONE decides, so an engine that does not know the key must answer
        # differently on the same bytes.
        dst = prepare(out, "W7pre", twin_base, True, LEGS["W7"][1], LEGS["W7"][2])
        report["W7pre"] = run_leg(out, "W7pre", prebin, dst)
        o = report["W7pre"]["obs"]
        print(f"[W7pre] Sun bodyType={o['Sun']['bodyType']} primary={o['Sun']['primary']} | "
              f"Earth TRAIL={'TRAIL' in o['Earth']['modules']}", flush=True)

    # ---- phase A assertions (predictions written before the run) ------------
    def T(tag, name, field):
        return report[tag]["obs"][name][field]

    def has_trail(tag, name):
        return "TRAIL" in report[tag]["obs"][name]["modules"]

    def sat(tag, name):
        return report[tag]["satellite"].get(name)

    checks = [
        ("W0 control: Sun is star AND primary", T("W0", "Sun", "bodyType") == 64 and T("W0", "Sun", "primary") is True),
        ("W0 control: no retired-capability warning on a generated twin", len(report["W0"]["warnings"]) == 0),
        ("W0 control: Earth is not a satellite, the Moon is",
         sat("W0", "Earth") is False and sat("W0", "Moon") is True),
        ("W4 legacy: identical capabilities to the composed control",
         T("W4", "Sun", "bodyType") == 64 and T("W4", "Sun", "primary") is True
         and sat("W4", "Earth") is False and sat("W4", "Moon") is True),

        # --- the LUMINOUS half moves alone ---------------------------------
        ("W2 light_source stripped: the Sun stops being a star (bodyType 7)", T("W2", "Sun", "bodyType") == 7),
        ("W2 light_source stripped: the day-length line disappears", report["W2"]["daylength_on_sun"] is False),
        ("W2 light_source stripped: the Sun's CUSTOM slot changes hands (routing moves)",
         T("W2", "Sun", "routing") != T("W0", "Sun", "routing")),
        ("W2 light_source stripped: Earth STAYS a non-satellite (the structural "
         "half did not move with it - impossible before the split)",
         sat("W2", "Earth") is False),
        ("W2 light_source stripped: exactly one warning, naming `light_source`",
         len(report["W2"]["warnings"]) == 1 and "light_source" in report["W2"]["warnings"][0]),

        # --- the STRUCTURAL half moves alone -------------------------------
        ("W1 primary stripped: the Sun STAYS a light source (bodyType 64)", T("W1", "Sun", "bodyType") == 64),
        ("W1 primary stripped: the day-length line survives", report["W1"]["daylength_on_sun"] is True),
        ("W1 primary stripped: Sun routing unchanged (StarLoader still bids)",
         T("W1", "Sun", "routing") == T("W0", "Sun", "routing")),
        ("W1 primary stripped: Earth BECOMES a satellite", sat("W1", "Earth") is True),
        ("W1 primary stripped: exactly one warning, naming `primary`",
         len(report["W1"]["warnings"]) == 1 and "primary" in report["W1"]["warnings"][0]),

        ("W3 both stripped: reproduces the un-split behaviour (§11.107 V1)",
         T("W3", "Sun", "bodyType") == 7 and sat("W3", "Earth") is True
         and report["W3"]["daylength_on_sun"] is False),
        ("W3 both stripped: TWO warnings, one per capability the type would grant",
         len(report["W3"]["warnings"]) == 2),

        ("W5 dark primary: Earth is primary WITHOUT being a light source",
         T("W5", "Earth", "primary") is True and T("W5", "Earth", "bodyType") == 7),
        ("W5 dark primary: the Moon stops being a satellite", sat("W5", "Moon") is False),
        ("W5 dark primary: nothing else moves (the Sun is untouched)",
         T("W5", "Sun", "bodyType") == 64 and sat("W5", "Earth") is False),

        # --- the deduction consumer, which needs `compose = deduced` --------
        ("W6 deduced control: Earth deduces its own module set, TRAIL included",
         has_trail("W6", "Earth") and T("W6", "Earth", "routing")["trail"] == 1),
        ("W7 deduced + primary stripped: Earth is a satellite and DEDUCES NO TRAIL",
         not has_trail("W7", "Earth") and T("W7", "Earth", "routing")["trail"] == 0
         and sat("W7", "Earth") is True),
        ("W7: and the Sun is still a light source while that happens",
         T("W7", "Sun", "bodyType") == 64 and report["W7"]["daylength_on_sun"] is True),
    ]
    if prebin:
        checks.append(("W7pre: the SAME corpus on the pre-split binary KEEPS Earth's "
                       "trail - the key is what the two engines disagree about",
                       has_trail("W7pre", "Earth")))
        checks.append(("W7pre: and the pre-split binary reports no `primary` field at all",
                       T("W7pre", "Sun", "primary") is None))
    for text, cond in checks:
        (ok if cond else fail)(text)

    # ---- phase B: the moved consumers at the SCREEN -------------------------
    # The two consumers this change MOVES are draw-time, so a dump cannot see
    # them: the hint's angular-separation skip and the orbit line's
    # planet-vs-satellite master flag. Both are measured on live renders.
    report["screen"] = {}

    def screen_pair(scene, tagA, tagB, boxed_on=None, label=""):
        runs = {}
        for tag in (tagA, tagB):
            dst = prepare(out, f"scr{scene}{tag}", twin_base, True,
                          LEGS[tag][1], LEGS[tag][2])
            runs[tag] = run_screen(out, scene, tag, binary, dst)
        (pa, da, ca), (pb, db, cb) = runs[tagA], runs[tagB]
        h, w, _ = img(pa[0]).shape
        obsA, _ = observe(da)
        obsB, _ = observe(db)
        sl = None
        if boxed_on:
            sA, sB = obsA[boxed_on]["screen"], obsB[boxed_on]["screen"]
            if sA != sB:
                fail(f"screen/{scene}: {boxed_on} moved between legs ({sA} vs {sB}) "
                     f"- the A/B is not of one variable")
            sl = box_of(sA, w, h, 160)
            # PRECONDITION: nothing else DRAWN may paint into the box (a halo
            # whose size depends on isSatellite() would read as a hint).
            _, allb = b24.load_dump(da)
            intruders = [n for n, v in allb.items()
                         if n != boxed_on and v.get("visible") and (v.get("screenSize") or 0) > 0
                         and v.get("screen") and sl[0] - 60 <= box_of(v["screen"], w, h, 0)[0] <= sl[2] + 60
                         and sl[1] - 60 <= box_of(v["screen"], w, h, 0)[1] <= sl[3] + 60]
            if intruders:
                fail(f"screen/{scene}: other DRAWN bodies inside the box: {intruders}")
            else:
                ok(f"screen/{scene}: box {sl} contains no drawn body but {boxed_on}")
        # The §11.18 cold-launch NaN only invalidates this comparison if the
        # affected body is DRAWN in the region being compared - named either way.
        for tag, d in ((tagA, da), (tagB, db)):
            bad = nonfinite(d)
            if not bad:
                continue
            _, allb = b24.load_dump(d)
            painting = [n for n in bad if allb.get(n, {}).get("visible")
                        and (sl is None or (allb[n].get("screen") and
                             sl[0] <= box_of(allb[n]["screen"], w, h, 0)[0] <= sl[2] and
                             sl[1] <= box_of(allb[n]["screen"], w, h, 0)[1] <= sl[3]))]
            if painting:
                fail(f"screen/{scene} {tag}: the §11.18 cold-launch NaN fired on a body "
                     f"that PAINTS into the compared region ({painting}) - RE-RUN")
            else:
                print(f"    NOTE screen/{scene} {tag}: §11.18 NaN on {sorted(bad)}, "
                      f"none of them drawn in the compared region - leg stands",
                      flush=True)
        floor = max(floor_of(pa, sl), floor_of(pb, sl))
        d = np.abs(img(pa[0]) - img(pb[0])).max(axis=2)
        if sl:
            d = d[sl[1]:sl[3], sl[0]:sl[2]]
        diff = int((d > 8).sum())
        print(f"    [{scene}] same-corpus floor: {floor} px>8   {tagA} vs {tagB}: "
              f"{diff} px>8", flush=True)
        report["screen"][scene] = {"box": sl, "floor": floor, "diff": diff}
        if diff > max(10 * max(floor, 1), 50):
            ok(f"screen/{scene}: {label} - {diff} px>8 over a measured {floor} px floor")
        else:
            fail(f"screen/{scene}: no difference - {diff} px>8 over a {floor} px floor")
        if ca is not None:
            cd = int((np.abs(img(ca) - img(pa[0])).max(axis=2) > 8).sum())
            report["screen"][scene]["positive_control"] = cd
            if cd > max(10 * max(floor, 1), 50):
                ok(f"screen/{scene}: POSITIVE CONTROL - in {tagA} the same orbit line "
                   f"appears under the SATELLITE master ({cd} px>8), so the subject "
                   f"leg measures routing and not an empty scene")
            else:
                fail(f"screen/{scene}: positive control saw nothing ({cd} px>8) - the "
                     f"orbit line is not in this frame, the leg proves nothing")

    print("\n--- phase B1: the HINT gate (moved to `primary`) ---", flush=True)
    screen_pair("hint", "W0", "W1", boxed_on="Sun",
                label="the Sun's hint is controlled by `primary` alone "
                      "(light_source identical in both legs)")
    print("\n--- phase B2: the ORBIT master flag (isSatellite) ---", flush=True)
    screen_pair("orbit", "W0", "W5",
                label="`primary` on Earth alone moves the Moon from the satellite "
                      "orbit flag to the planet one, on the drawn frame")

    if b25g.real_tree_md5() != src_md5:
        fail("the real ~/.spacecrafter tree was written by this run")
    else:
        ok("real tree md5 in == out")

    (out / "f12_b27_split.json").write_text(json.dumps(report, indent=1, default=str))
    print(f"\n{'OK' if not FAILS else str(len(FAILS)) + ' FAILS'} -> {out}/f12_b27_split.json",
          flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
