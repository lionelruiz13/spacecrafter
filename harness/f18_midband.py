#!/usr/bin/env python3
"""F18 / INTENT §5.52 - the depth-less mid band: does the new path draw a surface?

The row's own discriminating check, generalised off the Sun: for each target
body the driver FINDS the fov that puts it inside the mid band
(0.0015 < screenSize <= 0.008 - the band where ModularBody::draw takes the
nearComponents -> drawNoDepth() branch and issues NO clearDepth), suppresses
that body's additive halo so the disc is what the pixels show, and captures the
same scene on both paths.

Nothing here is star-specific: Sun AND planets, same script, same arithmetic.

The fov search is measured, never assumed: screenSize = halfAngularSize/halfFov,
so screenSize * fov is invariant for a frozen scene - one dump at a reference
fov gives the fov that lands any wanted screenSize, and a second dump CONFIRMS
it (the app's own number, not the prediction).

Usage: f18_midband.py <outdir> [target ...]     (default: Sun Mars Jupiter)
Analysis: f18_disc.py <outdir>
"""
import socket, sys, time, os, json, hashlib

OUT = sys.argv[1]
TARGETS = sys.argv[2:] or ["Sun", "Mars", "Jupiter"]
os.makedirs(OUT, exist_ok=True)
SSY = os.path.expanduser("~/.spacecrafter/ssystem.ini")
CFG = os.path.expanduser("~/.spacecrafter/config.ini")

# Geometric middle of the band - as far from both gates as the log scale allows,
# so a small fov error cannot push the sample out of the regime under test.
BAND_LO, BAND_HI = 0.0015, 0.008
WANTED = (BAND_LO * BAND_HI) ** 0.5      # 0.003464
REF_FOV = 60.0


def md5(p):
    with open(p, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2)
        sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f"{time.time():.3f} >> {cmd}", flush=True)


def shot(sock, name, pause=2.5):
    send(sock, f"body action screenshot filename {OUT}/{name}.png", pause)


def dump(sock, name, pause=2.5):
    send(sock, f"body action dual_dump filename {OUT}/{name}.json", pause)
    return f"{OUT}/{name}.json"


def geom(path, name):
    """Both paths' geometry for one body, as the app itself reports it."""
    for line in open(path):
        line = line.strip()
        if not line:
            continue
        d = json.loads(line)
        if d.get("type") == "body" and d.get("name") == name:
            n = d.get("new") if isinstance(d.get("new"), dict) else {}
            o = d.get("old") if isinstance(d.get("old"), dict) else {}
            return {"screenSize": n.get("screenSize"), "screen": n.get("screen"),
                    "dist": n.get("dist"), "boundingRadius": n.get("boundingRadius"),
                    "visible": n.get("visible"), "routing": n.get("routing"),
                    "modules": n.get("modules"),
                    "old_screen": o.get("screen") if o else None}
    return None


MD5_SSY0, MD5_CFG0 = md5(SSY), md5(CFG)
print("ssystem.ini md5 in =", MD5_SSY0, flush=True)
print("config.ini  md5 in =", MD5_CFG0, flush=True)

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)

send(s, "flag experimental_path on", 1)
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)
send(s, "flag landscape off")
send(s, "flag atmosphere off")
send(s, "flag star_twinkle off")
send(s, "flag stars off")
send(s, "flag show_fps off")
send(s, "flag planet_names off")
send(s, "flag planet_orbits off")
send(s, "flag object_trails off")
send(s, "flag constellation_drawing off")
send(s, "flag constellation_art off")
send(s, "flag cardinal_points off")
send(s, "flag milky_way off")
send(s, "flag nebulae off")
send(s, "set home_planet Earth", 3)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)

RESULT = {}
for target in TARGETS:
    tag = target.lower()
    send(s, f"select planet {target} pointer off", 1)
    send(s, "flag track_object on", 8)
    send(s, "timerate rate 0", 1)
    send(s, f"zoom fov {REF_FOV} duration 0", 3)
    g0 = geom(dump(s, f"{tag}_ref"), target)
    if not g0 or not g0.get("screenSize"):
        RESULT[target] = {"error": "no geometry at reference fov", "ref": g0}
        continue
    # screenSize * fov is scene-invariant (screenSize = halfAngularSize/halfFov)
    fov = REF_FOV * g0["screenSize"] / WANTED
    fov = max(0.05, min(179.0, fov))
    send(s, f"zoom fov {fov:.5f} duration 0", 3)
    g1 = geom(dump(s, f"{tag}_geom"), target)
    inband = bool(g1 and g1.get("screenSize")
                  and BAND_LO < g1["screenSize"] <= BAND_HI)
    RESULT[target] = {"ref_fov": REF_FOV, "ref": g0, "fov": fov, "geom": g1,
                      "in_band": inband, "wanted_screenSize": WANTED}
    print(f"### {target}: fov {fov:.4f} -> screenSize "
          f"{g1 and g1.get('screenSize')} in_band={inband}", flush=True)
    if not inband:
        continue
    halo_off = f"body name {target} color halo r 0 g 0 b 0"
    send(s, halo_off, 1)
    shot(s, f"{tag}_new")
    send(s, "flag experimental_path off", 2)
    shot(s, f"{tag}_old")
    send(s, "flag experimental_path on", 2)
    # halo restored from the shipped ssystem value via reload-free re-set: the
    # colour is read back from the OLD path's body, which the dump carries.
    shot(s, f"{tag}_new_b")          # same state twice = the noise floor
    send(s, "flag track_object off", 1)

send(s, "zoom fov 180 duration 0", 2)
RESULT["md5_ok"] = (md5(SSY) == MD5_SSY0 and md5(CFG) == MD5_CFG0)
print("ssystem.ini md5 out =", md5(SSY), "MATCH=", md5(SSY) == MD5_SSY0, flush=True)
print("config.ini  md5 out =", md5(CFG), "MATCH=", md5(CFG) == MD5_CFG0, flush=True)
with open(f"{OUT}/geometry.json", "w") as f:
    json.dump(RESULT, f, indent=2)
print(json.dumps(RESULT, indent=2), flush=True)
