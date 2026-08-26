#!/usr/bin/env python3
"""F39 / D21 gate: display scaling is presentation, grounded children inherit it.

D21 [vixy 2026-08-22, via INTENT 11.149(c)]: *"Unscaled for physics (orbit,
shadow received and casted), scaled for bounding/rendering, grounded children
inherit scaling (to be visually identical to unscaled, given grounded bodies are
surface-relative, so the referential is the body surface)"*; acceptance criterion
RATIFIED as UNIFORM DILATION - placement AND extent - [vixy 2026-08-26, 11.151(b)].

THE SCENE IS THE 11.78(a) MANDATE SCENE UNDER THE SHIPPED CONFIG: a composed OJM
rover grounded on the Moon with `moon_scale = 5` and `flag_moon_scaled = true`,
i.e. WITHOUT the `flag moon_scaled off` workaround every grounded screen harness
has carried since 5.27 was opened.

Two binaries, one farm, the same commands: `pre=` is the D21 fix's own baseline
(the ASmooth NaN fix ONLY - without it the pre leg dumps NaN and measures
nothing), `post=` carries the two-layer split.

GATES (each states what it discriminates, and what a PRE leg must show for the
gate to be a gate rather than a tautology):

  G1 placement      |eclDisplay| == Moon.scaledDatumRadius for a rover authored
                    at orbit_alt = 0 - the rover is ON the DISPLAYED surface.
                    PRE: |ecl| == Moon.datumRadius, i.e. 6949.6 km UNDER it.
  G2 extent         rover.scaledRadius / rover.radius == Moon display factor,
                    and the SELF-SIMILARITY ratio rover.scaledRadius /
                    Moon.scaledDatumRadius is INVARIANT under the scaling flag.
                    PRE: the rover keeps its true size (ratio drops 5x).
  G3 model truth    rover `ecl` is BIT-IDENTICAL scaled vs unscaled (the D8
                    script-channel leak of 11.101(f)(iii) closed).
                    PRE: `ecl` moves by radius x (scale-1).
  G4 reload         `body action reload` does NOT move the rover.
                    PRE: 11.101(f)'s untested prediction - it MOVES, because the
                    reload bakes the latch at a moment when the ramp is live.
  G5 ramp live      after `flag moon_scaled on`, sampled `inheritedScaling`
                    tracks Moon `scaling` through the 5 s ASmooth ramp and the
                    rover's `eclDisplay` rides it, while `ecl` never moves.
  G6 screen         the rover RENDERS under the shipped scaling. PRE: buried,
                    correctly occluded, no rover pixels.

Usage:
    DISPLAY=:2 ./f39_d21.py <absOutdir> pre=/abs/binary post=/abs/binary
"""
import json, os, shutil, socket, subprocess, sys, time
from pathlib import Path
import numpy as np
from PIL import Image

HOME = Path.home()
FIELD = HOME / ".spacecrafter"
FARM = Path("/tmp/f39home_d21")
AU_KM = 149597870.0

JD = 2461234.0
OBS_LON = 60
ALT_FAR_M = 22000000          # 22 000 km above the DATUM (b24_screen's far view)
ROVER_RADIUS_KM = 800

FAILS = []
def fail(m): FAILS.append(m); print(f"FAIL: {m}", flush=True)
def ok(m):   print(f"ok:   {m}", flush=True)
def note(m): print(f"      {m}", flush=True)


def rover_sections():
    # composed `type=` grammar (11.78(d)/(j)) - the same authoring b24_screen
    # uses, so this scene is that scene with the workaround removed.
    return (f"\n[F39Rover]\nname = F39Rover\nparent = Moon\nrelation = grounded\n"
            f"compose = explicit\ntype = Artificial\ncoord_func = surface_point\n"
            f"orbit_lon = {OBS_LON}\norbit_lat = 0\norbit_alt = 0\n"
            f"radius = {ROVER_RADIUS_KM}\nmodel_name = Curiosity\nhalo = false\n"
            f"[F39Rover:OJM]\nbody = F39Rover\ntype = OJM\n")


def build_farm():
    """Symlink mirror of the field ~/.spacecrafter, with config.ini, log/ and
    modularSystem/ as REAL entries (the twin must be writable). The field pair
    config.ini/ssystem.ini is never touched - asserted by md5 in the caller."""
    if FARM.exists():
        shutil.rmtree(FARM)
    (FARM / ".spacecrafter").mkdir(parents=True)
    for f in FIELD.iterdir():
        if f.name in ("config.ini", "log", "modularSystem"):
            continue
        os.symlink(f, FARM / ".spacecrafter" / f.name)
    (FARM / ".spacecrafter" / "log").mkdir()
    shutil.copy2(FIELD / "config.ini", FARM / ".spacecrafter" / "config.ini")
    md = FARM / ".spacecrafter" / "modularSystem"
    md.mkdir()
    for f in (FIELD / "modularSystem").iterdir():
        shutil.copy2(f, md / f.name)
    twin = md / "SolarSystem.ini.disabled"
    if not twin.exists():
        raise RuntimeError("twin absent in the field modularSystem - launch once on the shipped state first")
    (md / "SolarSystem.ini").write_bytes(twin.read_bytes() + rover_sections().encode("latin-1"))
    return FARM


def concurrent():
    n = 0
    for p in Path("/proc").iterdir():
        try:
            if (p / "comm").read_text().strip() == "spacecrafter":
                n += 1
        except Exception:
            pass
    return n


def wait_port(proc, timeout=120):
    t0 = time.time()
    while time.time() - t0 < timeout:
        if proc.poll() is not None:
            raise RuntimeError("app died before the port opened")
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.25); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)


def load_new(path):
    bodies, cam = {}, None
    for line in open(path, encoding="utf-8", errors="replace"):
        line = line.strip().rstrip(",")
        if not line:
            continue
        try:
            o = json.loads(line)
        except json.JSONDecodeError:
            continue
        if o.get("type") == "header":
            cam = o.get("camera")
        if o.get("type") == "body" and o.get("new") is not None:
            bodies[o["name"]] = o["new"]
    return bodies, cam


def vlen(v):
    return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]) ** 0.5


def shot(sock, out, name, pause=2.2):
    p = out / f"{name}.png"
    send(sock, f"body action screenshot filename {p}", pause)
    for _ in range(25):
        if p.exists() and p.stat().st_size > 0:
            break
        time.sleep(0.3)
    return np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)


def leg(tag, binary, out):
    """One fresh launch; returns {label: (bodies, cam)} plus screenshots on disk."""
    out.mkdir(parents=True, exist_ok=True)
    farm = build_farm()
    assert concurrent() == 0, "another spacecrafter is running"
    env = {**os.environ, "HOME": str(farm), "DISPLAY": os.environ.get("DISPLAY", ":2")}
    proc = subprocess.Popen([binary], cwd=str(farm / ".spacecrafter"),
                            stdout=open(out / f"{tag}.applog", "w"),
                            stderr=subprocess.STDOUT, env=env)
    res = {}
    try:
        s = wait_port(proc)
        time.sleep(12)
        send(s, "flag experimental_path on")
        send(s, "timerate rate 0")
        send(s, "meteors zhr 0")
        send(s, f"date jday {JD}")
        send(s, "set home_planet Moon", 3)
        # FREE MODE, deliberately, and the scene is aimed GEOMETRICALLY rather
        # than by longitude arithmetic. Measured here, both modes, both legs:
        # an observer commanded to `moveto lat 0 lon 60` is NOT above a rover
        # authored at `orbit_lon 60 orbit_lat 0` - it is 60.0 deg away in free
        # mode (cos t = 0.50003 pre / 0.50004 post, from the dumped distances)
        # and 90.0 deg away in surface mode (cos t = -2e-5, the rover at the
        # limb). The two observer-longitude channels and the orbit_lon channel
        # disagree with each other AND with the data key; 5.80/11.144's freeMode
        # converter is one half of that and F40 owns it. Free mode is kept
        # because JD 2461234.0 was calibrated for the LIT hemisphere in that
        # mode (b24_screen), and fov 60 puts the whole disc plus the rover in
        # view; nothing here depends on the longitude being the one commanded.
        send(s, "camera action free_mode state on")
        send(s, "flag atmosphere off")
        send(s, "flag landscape off")
        send(s, "select planet Moon")
        # NEVER `zoom auto in|initial` for scene setup (5.100/5.101): the two
        # paths set up DIFFERENT scenes on those commands.
        send(s, f"moveto lat 0 lon {OBS_LON} alt {ALT_FAR_M} duration 0", 5)
        send(s, "flag track_object on", 2)
        send(s, "zoom fov 60 duration 0", 2)
        send(s, "flag track_object off", 2)     # B30 determinism

        def grab(label, pause=0.0):
            if pause:
                time.sleep(pause)
            p = out / f"{tag}_{label}.json"
            send(s, f"body action dual_dump filename {p}", 1.5)
            res[label] = load_new(p)

        # --- A: the SHIPPED state (moon_scale = 5, flag_moon_scaled = true) ---
        grab("scaled")
        shot(s, out, f"{tag}_scaled")

        # --- B: the same scene UNSCALED, in-run A/B (the causal lever) --------
        send(s, "flag moon_scaled off", 1)
        time.sleep(8)                            # 5 s ASmooth ramp + margin
        grab("unscaled")
        shot(s, out, f"{tag}_unscaled")

        # --- C: RAMP - sample while the ASmooth is transiting -----------------
        send(s, "flag moon_scaled on", 0.2)
        for i, dt in enumerate((0.6, 1.6, 3.0)):
            grab(f"ramp{i}", pause=dt if i == 0 else 1.0)
        time.sleep(8)
        grab("settled")
        shot(s, out, f"{tag}_settled")

        # --- D: RELOAD - 11.101(f)'s pre-fix discriminator --------------------
        send(s, "body action reload", 3)
        time.sleep(8)
        send(s, f"moveto lat 0 lon {OBS_LON} alt {ALT_FAR_M} duration 0", 4)
        send(s, "zoom fov 60 duration 0", 2)
        grab("reload")
        shot(s, out, f"{tag}_reload")

        send(s, "shutdown action now", 1)
        s.close()
        try:
            proc.wait(timeout=40)
        except subprocess.TimeoutExpired:
            proc.kill()
    except Exception as e:
        print(f"LEG {tag} EXCEPTION: {e}", flush=True)
        proc.kill()
    return res


def report(tag, res, out):
    rows = {}
    for label, (bodies, cam) in res.items():
        m = bodies.get("Moon", {})
        r = bodies.get("F39Rover", {})
        if not r:
            note(f"[{tag}/{label}] rover ABSENT from the dump")
            continue
        rows[label] = dict(
            moon_scaledDatum_km=m.get("scaledDatumRadius", float('nan')) * AU_KM,
            moon_scaling=m.get("scaling"),
            rover_ecl_km=vlen(r["ecl"]) * AU_KM,
            rover_eclDisplay_km=(vlen(r["eclDisplay"]) * AU_KM) if "eclDisplay" in r else None,
            rover_inherited=r.get("inheritedScaling"),
            rover_boundingRadius_km=r.get("boundingRadius", float('nan')) * AU_KM,
            rover_dist_km=r.get("dist", float('nan')) * AU_KM,
            rover_screenSize=r.get("screenSize"),
            rover_visible=r.get("visible"),
            cam_dist_km=(cam or {}).get("distance", float('nan')) * AU_KM,
        )
    (out / f"{tag}_rows.json").write_text(json.dumps(rows, indent=1))
    print(f"\n===== {tag} =====", flush=True)
    for label, v in rows.items():
        print(f" {label:9s} moonDatum={v['moon_scaledDatum_km']:10.2f}km "
              f"moonScaling={v['moon_scaling']} rover|ecl|={v['rover_ecl_km']:10.3f}km "
              f"|eclDisp|={('%10.3f' % v['rover_eclDisplay_km']) if v['rover_eclDisplay_km'] is not None else '       n/a'}km "
              f"inh={v['rover_inherited']} bnd={v['rover_boundingRadius_km']:8.2f}km "
              f"vis={v['rover_visible']} ss={v['rover_screenSize']}", flush=True)
    return rows


def rover_px(out, tag, label):
    """Non-black pixel count inside the disc, as a crude 'something is drawn'
    figure; the discriminating number is the PRE/POST diff at the same camera."""
    p = out / f"{tag}_{label}.png"
    if not p.exists():
        return None
    a = np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)
    return int((a.max(axis=2) > 24).sum())


def main():
    out = Path(sys.argv[1]).resolve()
    args = dict(a.split("=", 1) for a in sys.argv[2:] if "=" in a)
    out.mkdir(parents=True, exist_ok=True)
    cfg_in = subprocess.run(["md5sum", str(FIELD / "config.ini")], capture_output=True, text=True).stdout.split()[0]
    ssy_in = subprocess.run(["md5sum", str(FIELD / "ssystem.ini")], capture_output=True, text=True).stdout.split()[0]
    print(f"field config md5 (in)  = {cfg_in}")
    print(f"field ssystem md5 (in) = {ssy_in}")
    print(f"concurrent instances   = {concurrent()}")

    allrows = {}
    for tag in ("pre", "post"):
        if tag not in args:
            continue
        print(f"\n--- leg {tag}: {args[tag]} "
              f"(md5 {subprocess.run(['md5sum', args[tag]], capture_output=True, text=True).stdout.split()[0]})",
              flush=True)
        res = leg(tag, args[tag], out)
        allrows[tag] = report(tag, res, out)

    cfg_out = subprocess.run(["md5sum", str(FIELD / "config.ini")], capture_output=True, text=True).stdout.split()[0]
    ssy_out = subprocess.run(["md5sum", str(FIELD / "ssystem.ini")], capture_output=True, text=True).stdout.split()[0]
    print(f"\nfield config md5 (out)  = {cfg_out}  MATCH={cfg_in == cfg_out}")
    print(f"field ssystem md5 (out) = {ssy_out}  MATCH={ssy_in == ssy_out}")

    print("\n===== SCREEN =====")
    for tag in allrows:
        for label in ("scaled", "unscaled", "settled", "reload"):
            print(f" {tag}/{label}: lit px = {rover_px(out, tag, label)}")

    (out / "rows_all.json").write_text(json.dumps(allrows, indent=1))
    print("\nartifacts:", out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
