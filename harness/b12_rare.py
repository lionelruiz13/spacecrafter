#!/usr/bin/env python3
"""B12 rare paths on the STAR's own surface module (INTENT §11.123).

The photosphere inherits four capabilities from the mesh it replaces, and the
one with a demonstrated operator use is the SKIN SEAM: the shipped Sun's data
carries a commented `#tex_skin = bodies/sun304A-sdo.jpg`, i.e. swapping the Sun
to an SDO 304A image is something shows do. If the new module had dropped it,
nothing else in the battery would have noticed.

Contract (old Body::createTexSkin/switchMapSkin parity, BodyModule.hpp):
  - skin_tex creates/replaces a skin and NEVER activates it;
  - skin_use on requires an existing skin (no-op otherwise);
  - skin_use on/off swaps the drawn map, and the swap REVERSES;
  - every reversible pair entered TWICE, the second entry starting from the
    state the first exit produced.
Also here: hide/show of the star (the module's body leaves and re-enters the
draw sweep) and the big-texture threshold crossed both ways, twice.

Measured on the composed screen: the disc is the observable, and the halo is
suppressed so the disc is what moves. Usage: b12_rare.py <outdir>
"""
import socket, sys, os, time, json, hashlib
import numpy as np
from PIL import Image

OUT = os.path.abspath(sys.argv[1])
os.makedirs(OUT, exist_ok=True)
SSY = os.path.expanduser("~/.spacecrafter/ssystem.ini")
CFG = os.path.expanduser("~/.spacecrafter/config.ini")


def md5(p):
    with open(p, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
MD5_0 = (md5(SSY), md5(CFG))


def send(cmd, pause=1.0):
    s.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        s.settimeout(0.2); s.recv(4096)
    except socket.timeout:
        pass
    s.settimeout(None); print(f">> {cmd}", flush=True)


def shot(tag, pause=3.0):
    send(f"body action screenshot filename {OUT}/{tag}.png", pause)
    return tag


def disc(tag, r=200):
    """Mean RGB over the disc interior (the Sun is tracked, so screen centre)."""
    a = np.asarray(Image.open(f"{OUT}/{tag}.png").convert("RGB"), dtype=float)
    h, w = a.shape[:2]
    yy, xx = np.mgrid[0:h, 0:w]
    m = ((xx - w / 2) ** 2 + (yy - h / 2) ** 2) < r * r
    return [round(float(a[..., c][m].mean()), 2) for c in range(3)]


send("flag experimental_path on", 1)
send("date jday 2461233.5", 1)
send("timerate rate 0", 1)
send("flag landscape off")
send("flag atmosphere off")
send("flag planet_names off")
send("set home_planet Earth", 3)
send("moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send("select planet Sun pointer off", 1)
send("flag track_object on", 8)
send("body name Sun color halo r 0 g 0 b 0", 1)
send("zoom fov 2 duration 0", 3)

R = {}
R["base"] = disc(shot("r_base"))
# --- skin seam, twice ------------------------------------------------------
send("body name Sun skin_use on", 1)                       # no skin yet: no-op
R["use_before_create"] = disc(shot("r_use_nocreate"))
send("body name Sun skin_tex bodies/sun304A-sdo.jpg", 4)   # create, NOT active
R["after_create"] = disc(shot("r_created"))
for i in (1, 2):
    send("body name Sun skin_use on", 3)
    R[f"skin_on_{i}"] = disc(shot(f"r_on_{i}"))
    send("body name Sun skin_use off", 3)
    R[f"skin_off_{i}"] = disc(shot(f"r_off_{i}"))
# replace while active -> deactivates (old parity)
send("body name Sun skin_use on", 3)
send("body name Sun skin_tex bodies/sun304A-sdo.jpg", 4)
R["replace_while_active"] = disc(shot("r_replaced"))
send("body name Sun skin_use off", 2)

# --- hide/show the star, twice --------------------------------------------
for i in (1, 2):
    send("body name Sun hidden true", 3)
    R[f"hidden_{i}"] = disc(shot(f"r_hidden_{i}"))
    send("body name Sun hidden false", 3)
    R[f"shown_{i}"] = disc(shot(f"r_shown_{i}"))

# --- big-texture threshold crossed both ways, twice ------------------------
for i in (1, 2):
    send("zoom fov 20 duration 0", 3)      # screenSize 0.026: base level
    R[f"small_{i}"] = disc(shot(f"r_small_{i}"), r=25)
    send("zoom fov 2 duration 0", 3)       # screenSize 0.262: big level
    R[f"big_{i}"] = disc(shot(f"r_big_{i}"))

send("body name Sun color halo r 1 g 1 b 0.8", 1)
send("zoom fov 180 duration 0", 2)
R["md5_ok"] = (md5(SSY), md5(CFG)) == MD5_0

ok, fails = [], []


def check(name, cond, detail=""):
    (ok if cond else fails).append(f"{name} {detail}")


eq = lambda a, b, t=0.5: all(abs(x - y) <= t for x, y in zip(a, b))
check("skin_use before any skin is a no-op", eq(R["use_before_create"], R["base"]),
      f"{R['use_before_create']} vs {R['base']}")
check("creating a skin does NOT activate it", eq(R["after_create"], R["base"]),
      f"{R['after_create']} vs {R['base']}")
check("skin_use on CHANGES the drawn disc (entry 1)", not eq(R["skin_on_1"], R["base"], 2.0),
      f"{R['skin_on_1']} vs {R['base']}")
check("skin_use off REVERTS (entry 1)", eq(R["skin_off_1"], R["base"]),
      f"{R['skin_off_1']} vs {R['base']}")
check("skin_use on CHANGES again (entry 2, from entry 1's exit state)",
      not eq(R["skin_on_2"], R["base"], 2.0), f"{R['skin_on_2']} vs {R['base']}")
check("entry 2 lands on the same skinned disc as entry 1", eq(R["skin_on_2"], R["skin_on_1"]),
      f"{R['skin_on_2']} vs {R['skin_on_1']}")
check("skin_use off REVERTS (entry 2)", eq(R["skin_off_2"], R["base"]),
      f"{R['skin_off_2']} vs {R['base']}")
check("replacing a skin while active deactivates it", eq(R["replace_while_active"], R["base"]),
      f"{R['replace_while_active']} vs {R['base']}")
check("hidden star draws nothing (entry 1)", sum(R["hidden_1"]) < 1.0, str(R["hidden_1"]))
check("shown star is back (entry 1)", eq(R["shown_1"], R["base"]), f"{R['shown_1']} vs {R['base']}")
check("hidden star draws nothing (entry 2)", sum(R["hidden_2"]) < 1.0, str(R["hidden_2"]))
check("shown star is back (entry 2)", eq(R["shown_2"], R["base"]), f"{R['shown_2']} vs {R['base']}")
check("big-texture threshold reversible (entry 1 == entry 2, big)", eq(R["big_1"], R["big_2"]),
      f"{R['big_1']} vs {R['big_2']}")
check("big-texture threshold reversible (entry 1 == entry 2, small)", eq(R["small_1"], R["small_2"]),
      f"{R['small_1']} vs {R['small_2']}")
check("returning to the big level restores the first reading", eq(R["big_1"], R["base"]),
      f"{R['big_1']} vs {R['base']}")
check("frozen data md5 in == out", R["md5_ok"])

for l in ok:
    print("ok:   " + l)
for l in fails:
    print("FAIL: " + l)
R["ok"], R["fails"] = ok, fails
with open(f"{OUT}/b12_rare.json", "w") as f:
    json.dump(R, f, indent=2)
print(f"\n{len(fails)} failure(s)")
sys.exit(1 if fails else 0)
