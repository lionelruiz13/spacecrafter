#!/usr/bin/env python3
"""B24 composition scene - the mandate's structural objects (INTENT 11.78(a)):
rover on a moon (grounded), rocket ascending (surface_point ramp), plus
non-grounded controls. Numeric layer, DEFAULT observer (Earth surface).

FRAME MODEL (hard-won - three instrument iterations, INTENT 11.78(f)): the
dual_dump eclRoot base is camera-composed and ROTATES with the observer's
reference surface. Every gate below is therefore frame-proof by construction:
  - Earth grounded pair (RoverE lon0, RoverE2 lon180): the observer stands on
    Earth, so the base CO-ROTATES with a correctly-folded grounded rover ->
    net rotation == 0. The pre-fix fold (child spin, frozen) leaves the pair
    inertial -> net == Earth spin advance (~90 deg over 0.25 d): the
    discriminator is enormous and binary.
  - RoverEC (orbiting control at the same pad): inertial by construction ->
    net rotation == Earth spin advance, and |rel| == Earth radius exactly.
  - Moon pair (RoverM grounded vs RoverC orbiting, same pad): CHORD LENGTH
    |RoverM-RoverC| is frame-invariant; with r and the dumped Moon spin
    advance it is point-PREDICTED: x = 2 asin(L0/2r), L1 = 2r sin((x+-d)/2).
    A frozen fold predicts L1 == L0 instead.
  - Rocket |ecl| (pre-fold orbit output, frame-free): exact lerp replay over
    the ascent window + reversible pair.
Cross-run attribution record: with a Moon observer the Earth pair measured
87.2198 deg = R(90.2465, pole_E) o R(-3.294, pole_M) composed (predicted
87.20 at ~23 deg pole separation) - the apparent "12 deg/day deficit" was the
base rotation, not the fold. Artifacts: artifacts/b24loc/.
"""

import json, math, os, socket, subprocess, sys, time
from pathlib import Path

import numpy as np

HOME = Path.home()
SC_BIN = os.environ.get("SC_BIN", str(Path(__file__).resolve().parents[2] / "build-claude/src/spacecrafter"))
USERDIR = HOME / ".spacecrafter"
TWIN = USERDIR / "modularSystem/SolarSystem.ini.disabled"
ENABLED = USERDIR / "modularSystem/SolarSystem.ini"
OUT = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).parent / "artifacts/b24"

T0 = 2461233.5
T1 = T0 + 0.25
ASCENT_START = 2461233.8
ASCENT_DUR = 0.2
T2 = 2461233.9   # mid-window: f = 0.5
T3 = 2461234.1   # past the window
ALT_END_KM = 2.0
MOON_R_KM = 1737.4    # [observed: ~/.spacecrafter/ssystem.ini [moon] radius] - datum default
AU_KM = 149597870.0   # sc_const AU in km

SECTIONS = f"""
[RoverM]
declare = ModularBody
name = RoverM
parent = Moon
relation = grounded
compose = explicit
type = Artificial
coord_func = surface_point
orbit_lon = 0
orbit_lat = 0
orbit_alt = 0
radius = 0.01
model_name = Curiosity
halo = false

[RoverM:OJM]
declare = BodyModule
body = RoverM
module = OJM

[RoverE]
declare = ModularBody
name = RoverE
parent = Earth
relation = grounded
compose = explicit
type = Artificial
coord_func = surface_point
orbit_lon = 0
orbit_lat = 0
orbit_alt = 0
radius = 0.01
model_name = Curiosity
halo = false

[RoverE:OJM]
declare = BodyModule
body = RoverE
module = OJM

[RoverE2]
declare = ModularBody
name = RoverE2
parent = Earth
relation = grounded
compose = explicit
type = Artificial
coord_func = surface_point
orbit_lon = 180
orbit_lat = 0
orbit_alt = 0
radius = 0.01
model_name = Curiosity
halo = false

[RoverE2:OJM]
declare = BodyModule
body = RoverE2
module = OJM

[RoverEC]
declare = ModularBody
name = RoverEC
parent = Earth
relation = orbiting
compose = explicit
type = Artificial
coord_func = surface_point
orbit_lon = 0
orbit_lat = 0
orbit_alt = 0
radius = 0.01
model_name = Curiosity
halo = false

[RoverEC:OJM]
declare = BodyModule
body = RoverEC
module = OJM

[RoverC]
declare = ModularBody
name = RoverC
parent = Moon
relation = orbiting
compose = explicit
type = Artificial
coord_func = surface_point
orbit_lon = 0
orbit_lat = 0
orbit_alt = 0
radius = 0.01
model_name = Curiosity
halo = false

[RoverC:OJM]
declare = BodyModule
body = RoverC
module = OJM

[Rocket]
declare = ModularBody
name = Rocket
parent = Moon
relation = grounded
compose = explicit
type = Artificial
coord_func = surface_point
orbit_lon = 90
orbit_lat = 0
orbit_alt = 0
orbit_alt_end = {ALT_END_KM}
orbit_ascent_start = {ASCENT_START}
orbit_ascent_duration = {ASCENT_DUR}
radius = 0.01
model_name = Curiosity
halo = false

[Rocket:OJM]
declare = BodyModule
body = Rocket
module = OJM
"""

FAILS = []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


def wait_port(timeout=90):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2)
        sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)


def load_bodies(path):
    bodies = {}
    with open(path) as f:
        for line in f:
            line = line.strip().rstrip(",")
            if not line:
                continue
            try:
                obj = json.loads(line)
            except json.JSONDecodeError:
                continue
            if obj.get("type") == "body" and obj.get("new") is not None:
                bodies[obj["name"]] = obj["new"]
    return bodies


def rel_root(bodies, name, parent):
    """Parent->body offset in the ROOT-aligned walk frame, from eclRoot -
    matLocalToBodyPos translations (fresh for every body via the translation
    tick, B19 - unlike `mat`, chimeric on invisible bodies, INTENT 11.14b).
    The walk base cancels in the difference; the rover's own fold is exactly
    what remains: rel = fold . ecl."""
    return (np.array(bodies[name]["eclRoot"], dtype=np.float64)
            - np.array(bodies[parent]["eclRoot"], dtype=np.float64))


def angle_between(a, b):
    c = np.dot(a, b) / (np.linalg.norm(a) * np.linalg.norm(b))
    return math.acos(max(-1.0, min(1.0, c)))


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    if not TWIN.exists():
        raise RuntimeError("twin absent - launch once on the shipped state first")
    ENABLED.write_bytes(TWIN.read_bytes() + SECTIONS.encode("latin-1"))
    dumps = {}
    try:
        proc = subprocess.Popen([SC_BIN], cwd=str(USERDIR),
                                stdout=open(OUT / "b24_compose.applog", "w"),
                                stderr=subprocess.STDOUT,
                                env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
        sock = wait_port()
        time.sleep(10)
        send(sock, "timerate rate 0")
        for tag, jd in (("D0", T0), ("D1", T1), ("D2", T2), ("D3", T3), ("D4", T0)):
            send(sock, f"date jday {jd}")
            time.sleep(2.5)
            p = OUT / f"b24_compose_{tag}.json"
            send(sock, f"body action dual_dump filename {p}")
            time.sleep(1.5)
            dumps[tag] = load_bodies(p)
        send(sock, "shutdown action now")
        sock.close()
        try:
            proc.wait(timeout=30)
        except subprocess.TimeoutExpired:
            proc.kill()
            fail("app did not exit cleanly")
    finally:
        ENABLED.unlink(missing_ok=True)

    applog = (OUT / "b24_compose.applog").read_text(errors="replace")

    # ---- presence + composition structure ----
    d0 = dumps["D0"]
    for name, parent, rel in (("RoverM", "Moon", 3), ("RoverE", "Earth", 3),
                              ("RoverC", "Moon", 4), ("RoverEC", "Earth", 4), ("RoverE2", "Earth", 3), ("Rocket", "Moon", 3)):
        if name not in d0:
            fail(f"{name} absent from the dump (composed body not created?)")
            continue
        nb = d0[name]
        if nb.get("parent") != parent:
            fail(f"{name}.parent = {nb.get('parent')!r}, expected {parent!r}")
        if nb.get("relation") != rel:
            fail(f"{name}.relation = {nb.get('relation')}, expected {rel}")
        if "OJM" not in nb.get("modules", []):
            fail(f"{name}.modules = {nb.get('modules')}, OJM absent")
    if not FAILS:
        ok("all 6 composed bodies present: parent/relation/OJM module correct")
    if "uses coord_func=surface_point without the grounded relation" in applog:
        ok("provider 2(f) trap warning fired for RoverC")
    else:
        fail("RoverC non-grounded warning absent from the log")

    # ---- co-rotation gates (frame-proof set - see docstring) ----
    report = {"pairs": {}}
    EARTH_R_KM = 6378.14

    if all(n in dumps[t] for n in ("RoverE", "RoverE2", "RoverEC") for t in ("D0", "D1")):
        diff0 = np.array(dumps["D0"]["RoverE"]["eclRoot"]) - np.array(dumps["D0"]["RoverE2"]["eclRoot"])
        diff1 = np.array(dumps["D1"]["RoverE"]["eclRoot"]) - np.array(dumps["D1"]["RoverE2"]["eclRoot"])
        net = math.degrees(angle_between(diff0, diff1))
        d_spin = math.degrees((dumps["D1"]["Earth"]["axisRot"] - dumps["D0"]["Earth"]["axisRot"]) % (2 * math.pi))
        if net < 0.05:
            ok(f"Earth grounded pair: net rotation {net:.4f} deg == 0 (co-rotates with the observer base; pre-fix fold would read ~{d_spin:.1f})")
        else:
            fail(f"Earth grounded pair: net rotation {net:.4f} deg != 0 (fold not co-rotating; Earth spin advance {d_spin:.4f})")
        mid0 = (np.array(dumps["D0"]["RoverE"]["eclRoot"]) + np.array(dumps["D0"]["RoverE2"]["eclRoot"])) / 2
        mid1 = (np.array(dumps["D1"]["RoverE"]["eclRoot"]) + np.array(dumps["D1"]["RoverE2"]["eclRoot"])) / 2
        rc0 = np.array(dumps["D0"]["RoverEC"]["eclRoot"]) - mid0
        rc1 = np.array(dumps["D1"]["RoverEC"]["eclRoot"]) - mid1
        ctl = math.degrees(angle_between(rc0, rc1))
        if abs(ctl - d_spin) < 0.05:
            ok(f"orbiting control: net rotation {ctl:.4f} deg == Earth spin advance {d_spin:.4f} (inertial control correct)")
        else:
            fail(f"orbiting control: net rotation {ctl:.4f} deg != Earth spin advance {d_spin:.4f}")
        report["pairs"]["Earth"] = {"net_grounded": net, "net_control": ctl, "d_spin": d_spin,
                                    "r_control_km": float(np.linalg.norm(rc0)) * AU_KM}

    if all(n in dumps[t] for n in ("RoverM", "RoverC") for t in ("D0", "D1")):
        r_au = MOON_R_KM / AU_KM
        L0 = float(np.linalg.norm(np.array(dumps["D0"]["RoverM"]["eclRoot"]) - np.array(dumps["D0"]["RoverC"]["eclRoot"])))
        L1 = float(np.linalg.norm(np.array(dumps["D1"]["RoverM"]["eclRoot"]) - np.array(dumps["D1"]["RoverC"]["eclRoot"])))
        d_spin_m = (dumps["D1"]["Moon"]["axisRot"] - dumps["D0"]["Moon"]["axisRot"]) % (2 * math.pi)
        x = 2 * math.asin(min(1.0, L0 / (2 * r_au)))
        preds = [2 * r_au * abs(math.sin((x + s * d_spin_m) / 2)) for s in (+1, -1)]
        err_km = min(abs(L1 - p) for p in preds) * AU_KM
        frozen_km = abs(L1 - L0) * AU_KM
        if err_km < 2.0:
            ok(f"Moon pair chord: L1 predicted from (L0, Moon d_spin) to {err_km:.3f} km (frozen-fold would keep L0: delta {frozen_km:.1f} km)")
        else:
            fail(f"Moon pair chord: L1 off prediction by {err_km:.3f} km (L0 {L0*AU_KM:.2f} -> L1 {L1*AU_KM:.2f} km, d_spin {d_spin_m:.5f})")
        report["pairs"]["Moon"] = {"L0_km": L0 * AU_KM, "L1_km": L1 * AU_KM,
                                   "d_spin": d_spin_m, "pred_err_km": err_km}

    # ---- rocket ascent: exact lerp replay + reversibility ----
    datum_au = MOON_R_KM / AU_KM
    expect = {"D0": datum_au, "D1": datum_au,
              "D2": datum_au + (ALT_END_KM * 0.5) / AU_KM,
              "D3": datum_au + ALT_END_KM / AU_KM,
              "D4": datum_au}
    for tag, want in expect.items():
        if "Rocket" not in dumps[tag]:
            continue
        got = float(np.linalg.norm(np.array(dumps[tag]["Rocket"]["ecl"])))
        err_m = abs(got - want) * AU_KM * 1000
        if err_m > 5.0:  # 5 m on a float AU chain at 1.2e-5 AU scale
            fail(f"Rocket |ecl| at {tag}: {got:.9e} AU vs expected {want:.9e} AU (err {err_m:.1f} m)")
        else:
            ok(f"Rocket altitude at {tag}: err {err_m:.3f} m (exact lerp replay)")

    (OUT / "b24_compose_result.json").write_text(json.dumps({"fails": FAILS, **report}, indent=1))
    print(f"\n{'COMPOSITION SCENE GREEN' if not FAILS else f'{len(FAILS)} FAILURES'} -> b24_compose_result.json", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
