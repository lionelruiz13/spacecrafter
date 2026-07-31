#!/usr/bin/env python3
"""B12 - the near-surface star family, slice 1: the limb-darkened emissive disc.

Design note: claude/b12-design.md. This driver produces the captures; b12_limb.py
does the radial arithmetic. It is BINARY-AGNOSTIC on purpose: run it once with
the pre-change binary and once with the post-change binary into two directories,
so the same commands produce the pre/post pair the inertness check needs.

Captures
  S0 noise floor        same state, twice          -> the diff threshold's floor
  S1 disc, halo OFF     Sun tracked, zoom fov 2    -> the LAW's measurement
     (new + old, path-pinned; the additive big halo is removed by setting its
      colour to 0,0,0 - it feeds StarModule live - so the disc is alone)
  S2 disc, halo ON      same, halo restored        -> the COMPOSED screen
  S3 near approach      home_planet Sun, free-mode descent, twice (rare path)
  N1..N3 standard scenes (Earth surface fisheye 180 / Moon close / Mars close)
                                                    -> star-gated inertness
  N4 Sun at fisheye 180 (the drawNoDepth mid band)  -> expected to change

Geometry for the prediction comes from the dual_dump (dist, radius, screenSize,
screen position), never from an assumption about where the disc landed.

Usage: b12_photosphere.py <outdir>
"""
import socket, sys, time, os, json, hashlib

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
SSY = os.path.expanduser("~/.spacecrafter/ssystem.ini")
CFG = os.path.expanduser("~/.spacecrafter/config.ini")
AU_M = 149597870700.0
R_SUN_AU = 696000.0 / (AU_M / 1000.0)   # the shipped radius, in AU


def md5(p):
    with open(p, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f"{time.time():.3f} >> {cmd}", flush=True)


def shot(sock, name, pause=2.5):
    send(sock, f"body action screenshot filename {OUT}/{name}.png", pause)


def dump(sock, name, pause=2.5):
    send(sock, f"body action dual_dump filename {OUT}/{name}.json", pause)
    return f"{OUT}/{name}.json"


MD5_SSY0, MD5_CFG0 = md5(SSY), md5(CFG)
print("ssystem.ini md5 in =", MD5_SSY0, flush=True)
print("config.ini  md5 in =", MD5_CFG0, flush=True)

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)

# ---- common scene --------------------------------------------------------
send(s, "flag experimental_path on", 1)
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)
send(s, "flag landscape off")
send(s, "flag atmosphere off")
send(s, "flag star_twinkle off")
send(s, "flag show_fps off")
send(s, "flag planet_names off")
send(s, "flag constellation_drawing off")
send(s, "flag constellation_art off")
send(s, "flag cardinal_points off")
send(s, "set home_planet Earth", 3)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "select planet Sun pointer off", 1)
send(s, "flag track_object on", 8)
send(s, "timerate rate 0", 1)

HALO_OFF = "body name Sun color halo r 0 g 0 b 0"       # dual-write: both paths
HALO_ON = "body name Sun color halo r 1 g 1 b 0.8"      # the shipped `color` value


def ab(tag, geom=True):
    """One A/B pair with the additive big halo removed: the disc, alone, on both
    paths. Halo restored afterwards so the composed captures stay honest."""
    if geom:
        dump(s, f"{tag}_geom")
    send(s, HALO_OFF, 1)
    shot(s, f"{tag}_new")
    send(s, "flag experimental_path off", 2)
    shot(s, f"{tag}_old")
    send(s, "flag experimental_path on", 2)
    send(s, HALO_ON, 1)


# ---- N4: the Sun in the mid band (native fisheye 180), composed ----------
send(s, "zoom fov 180 duration 0", 2)
dump(s, "n4_geom")
shot(s, "n4_sun_fisheye")

# ---- S0 noise floor + S1 the law, big-texture band (screenSize > 0.2) ----
send(s, "zoom fov 2 duration 0", 3)
dump(s, "s1_geom")
shot(s, "s0_floor_a")
shot(s, "s0_floor_b")            # same state twice
shot(s, "s2_disc_halo_new")      # composed screen, NEW
send(s, "flag experimental_path off", 2)
shot(s, "s2_disc_halo_old")      # composed screen, OLD
send(s, "flag experimental_path on", 2)
send(s, HALO_OFF, 1)
shot(s, "s1_disc_new")           # disc alone, NEW
send(s, "flag experimental_path off", 2)
shot(s, "s1_disc_old")           # disc alone, OLD
send(s, "flag experimental_path on", 2)
send(s, HALO_ON, 1)

# ---- S4: the plain near band (0.008 < screenSize < 0.2, no big texture) --
send(s, "zoom fov 20 duration 0", 3)
ab("s4")
# ---- S5: the depth-less mid band (0.0015 < screenSize < 0.008) -----------
#      the drawNoDepth hook - a separate code path in every mesh module.
send(s, "zoom fov 100 duration 0", 3)
ab("s5")

# ---- S3 near-Sun approach, entered TWICE (rare-path discipline) ----------
send(s, "zoom fov 180 duration 0", 2)
for leg in ("a", "b"):
    send(s, "set home_planet Sun", 4)
    send(s, "camera action free_mode state on", 1)
    # altitude is measured from the surface here (measured: altitude 2.5 R gave
    # dist 3.5 R), so 1.5 R lands the camera at d = 2.5 R - inside the near
    # list (d > 2 R, else the empty grounded list draws) and above screenSize
    # 0.2, so BOTH paths bind their big texture. That matters: the reduced and
    # full levels of the same map do NOT render alike (§5 finding), and the two
    # paths switch level at different sizes (old at 180 px diameter, new at
    # 409), so a size between the two thresholds compares different textures.
    send(s, f"moveto altitude {int(1.5 * R_SUN_AU * AU_M)} duration 0", 5)
    send(s, "select planet Sun pointer off", 1)
    send(s, "flag track_object on", 3)
    dump(s, f"s3_geom_{leg}")
    shot(s, f"s3_composed_new_{leg}")          # the terminal observable (halo on)
    ab(f"s3_{leg}", geom=False)                # the law, halo removed
    send(s, "camera action free_mode state off", 1)
    send(s, "set home_planet Earth", 4)
    send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)

# ---- N1..N3 standard scenes (star-gated inertness) -----------------------
send(s, "flag track_object off", 1)
send(s, "zoom fov 180 duration 0", 2)
send(s, "select planet Earth pointer off", 1)
shot(s, "n1_earth_surface")
send(s, "select planet Moon pointer off", 1)
send(s, "flag track_object on", 6)
send(s, "zoom fov 5 duration 0", 3)
shot(s, "n2_moon")
send(s, "select planet Mars pointer off", 1)
send(s, "flag track_object on", 6)
shot(s, "n3_mars")
send(s, "flag track_object off", 1)
send(s, "zoom fov 180 duration 0", 2)

print("ssystem.ini md5 out =", md5(SSY), "MATCH=", md5(SSY) == MD5_SSY0, flush=True)
print("config.ini  md5 out =", md5(CFG), "MATCH=", md5(CFG) == MD5_CFG0, flush=True)


def geom(path):
    """Sun geometry as the app itself reports it (never assumed)."""
    for line in open(path):
        line = line.strip()
        if not line:
            continue
        d = json.loads(line)
        if d.get("type") == "body" and d.get("name") == "Sun" and isinstance(d.get("new"), dict):
            n = d["new"]
            return {"dist": n.get("dist"), "screen": n.get("screen"),
                    "screenSize": n.get("screenSize"),
                    "boundingRadius": n.get("boundingRadius"),
                    "visible": n.get("visible"),
                    "modules": n.get("modules"), "routing": n.get("routing")}
    return None


G = {t: geom(f"{OUT}/{t}.json") for t in
     ("s1_geom", "s4_geom", "s5_geom", "n4_geom", "s3_geom_a", "s3_geom_b")}
G["md5_ok"] = (md5(SSY) == MD5_SSY0 and md5(CFG) == MD5_CFG0)
with open(f"{OUT}/geometry.json", "w") as f:
    json.dump(G, f, indent=2)
print(json.dumps(G, indent=2), flush=True)
