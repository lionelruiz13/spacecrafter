#!/usr/bin/env python3
"""B29 - runtime per-body COLOR seam: measure old, reproduce on the new path.

One fresh launch (new path pinned by default, §11.53). Every command spelling
is verified from the running process (the gdb probe counts setBodyColor /
setDefaultBodyColor hits; a bogus spelling = 0 hits, the §11.54(j) class).

Numeric (state) instrument: the dual_dump now carries new-path HALO color
(ModularBody.haloColor) and TRAIL color (TrailModule.dumpState), so the recolor
and its reload behaviour are read as RGB values, not adjectives (INTENT §11.65).

Sections:
  A scene          frozen, Earth surface, tracked Moon, atm/landscape off.
  B live per-body  body name Venus color halo/trail  -> dump asserts the RGB.
  C broadcast      body name all color halo          -> every body's halo set.
  D override       Venus, then all, then Venus       -> broadcast overrides,
                                                        per-body re-overrides.
  E default        color property planet_trails      -> existing body UNCHANGED
                                                        (old parity: default hits
                                                        future bodies only).
  F reload         set colors, body action reload    -> new RESETS to file
                                                        (measured, both entries of
                                                        the reversible pair).
  G screen A/B     recolor-all-halo-red, per-path
                   base->after delta                 -> the terminal observable,
                                                        both paths respond.
  H old persist    pin old, recolor, body action     -> old KEEPS the color (old
                   initial                              has no reset that touches
                                                        color; reinitParam = radius).

Usage: b29_color.py <outdir>
"""
import socket, sys, time, os, json, hashlib

OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
SSY = os.path.expanduser("~/.spacecrafter/ssystem.ini")
CFG = os.path.expanduser("~/.spacecrafter/config.ini")


def md5(p):
    with open(p, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


def send(sock, cmd, pause=0.5):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f"{time.time():.3f} >> {cmd}", flush=True)


def dump(sock, name, pause=3):
    p = f"{OUT}/{name}.json"
    send(sock, f"body action dual_dump filename {p}", pause)
    return p


def shot(sock, name, pause=3):
    p = f"{OUT}/{name}.png"
    send(sock, f"body action screenshot filename {p}", pause)
    return p


def read_new(path):
    """name -> new-path body dict (has haloColor + trail[])."""
    out = {}
    for line in open(path):
        line = line.strip()
        if not line:
            continue
        d = json.loads(line)
        if d.get("type") == "header":
            out["__jd__"] = d["jd"]
            continue
        # The dual_dump also emits dumpHops lines (name-keyed, "new" is a LIST);
        # the body-trace we want has "new" as a dict. Take only the dict.
        if isinstance(d.get("new"), dict):
            out[d["name"]] = d["new"]
    return out


MD5_SSY0, MD5_CFG0 = md5(SSY), md5(CFG)
print("ssystem.ini md5 in =", MD5_SSY0, flush=True)
print("config.ini  md5 in =", MD5_CFG0, flush=True)

s = socket.create_connection(("127.0.0.1", 7805), timeout=15)

# --- A. scene -------------------------------------------------------------
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)
send(s, "flag landscape off")
send(s, "flag atmosphere off")
send(s, "flag star_twinkle off")
send(s, "flag show_fps off")
send(s, "flag planet_names off")
send(s, "set home_planet Earth", 3)
send(s, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(s, "select planet Moon pointer off", 1)
send(s, "flag track_object on", 12)
send(s, "flag experimental_path on", 2)   # pin NEW explicitly (default, made explicit)
send(s, "timerate rate 0", 2)

# --- B. live per-body recolor (halo + trail channels, numeric) ------------
p_base = dump(s, "b29_base")
send(s, "body name Venus color halo r 1 g 0 b 0", 1)
send(s, "body name Venus color trail r 0 g 1 b 0", 1)
p_perbody = dump(s, "b29_perbody")

# --- C. broadcast "all" ---------------------------------------------------
send(s, "body name all color halo r 0 g 0 b 1", 1)
p_all = dump(s, "b29_all")

# --- D. broadcast override precedence -------------------------------------
send(s, "body name Venus color halo r 1 g 0 b 0", 1)      # Venus red
p_ov1 = dump(s, "b29_ov1")
send(s, "body name all color halo r 0.2 g 0.2 b 0.2", 1)  # all grey (incl Venus)
p_ov2 = dump(s, "b29_ov2")
send(s, "body name Venus color halo r 1 g 0 b 0", 1)      # Venus red again
p_ov3 = dump(s, "b29_ov3")

# --- E. runtime DEFAULT color: existing body UNCHANGED (old parity) -------
send(s, "color property planet_trails r 0 g 0 b 1", 1)    # default trail = blue
p_def = dump(s, "b29_default")                            # Venus trail must be UNCHANGED

# --- F. reload behaviour + reversible pair --------------------------------
# Set a distinctive per-body color, reload, and read whether the new path
# keeps it (old persists; new rebuilds from file -> resets, per B16 §11.55(i)).
send(s, "body name Venus color halo r 0.9 g 0.1 b 0.05", 1)
send(s, "body name Mars  color halo r 0.1 g 0.9 b 0.05", 1)
p_rpre1 = dump(s, "b29_reload_pre1")
send(s, "body action reload", 5)
p_rpost1 = dump(s, "b29_reload_post1")
# second entry of the reversible pair, from the state the first produced
send(s, "body name Venus color halo r 0.05 g 0.1 b 0.9", 1)
send(s, "body name Mars  color halo r 0.9 g 0.05 b 0.1", 1)
p_rpre2 = dump(s, "b29_reload_pre2")
send(s, "body action reload", 5)
p_rpost2 = dump(s, "b29_reload_post2")

# --- G. screen A/B: recolor-all-halo-red, per-path base->after ------------
send(s, "flag track_object on", 2)
send(s, "flag experimental_path off", 2)      # pin OLD
shot(s, "g_base_old")
send(s, "flag experimental_path on", 2)       # pin NEW
shot(s, "g_ctrl_new")                         # noise floor (same state twice)
shot(s, "g_base_new")
send(s, "body name all color halo r 1 g 0 b 0", 1)   # recolor BOTH paths
shot(s, "g_after_new")
send(s, "flag experimental_path off", 2)      # pin OLD
shot(s, "g_after_old")

# --- H. old persist across `body action initial` (the only old reset) -----
# reinitParam resets RADIUS, not color -> old KEEPS the recolor. Measured on
# the pinned-old screen: after_old (red) vs initial_old must be ~equal.
send(s, "body action initial", 3)
shot(s, "h_initial_old")
send(s, "flag experimental_path on", 2)       # restore new pin (shipped default)

print("ssystem.ini md5 out =", md5(SSY), "MATCH=", md5(SSY) == MD5_SSY0, flush=True)
print("config.ini  md5 out =", md5(CFG), "MATCH=", md5(CFG) == MD5_CFG0, flush=True)

# ---- numeric assertions (state instrument) -------------------------------
def close(a, b, tol=0.02):
    return all(abs(x - y) <= tol for x, y in zip(a, b))


def halo(bodies, name):
    return bodies[name]["haloColor"]


def trailcol(bodies, name):
    t = bodies[name].get("trail") or []
    return t[0]["color"] if t else None


base, perbody, allc = read_new(p_base), read_new(p_perbody), read_new(p_all)
ov1, ov2, ov3 = read_new(p_ov1), read_new(p_ov2), read_new(p_ov3)
defd = read_new(p_def)
rpre1, rpost1 = read_new(p_rpre1), read_new(p_rpost1)
rpre2, rpost2 = read_new(p_rpre2), read_new(p_rpost2)

R = {}
R["venus_file_halo"] = halo(base, "Venus")
R["venus_file_trail"] = trailcol(base, "Venus")
# B: per-body
R["B_venus_halo_set"] = close(halo(perbody, "Venus"), [1, 0, 0])
R["B_venus_trail_set"] = close(trailcol(perbody, "Venus"), [0, 1, 0])
R["B_venus_halo"] = halo(perbody, "Venus")
R["B_venus_trail"] = trailcol(perbody, "Venus")
# C: broadcast all
R["C_venus_all"] = close(halo(allc, "Venus"), [0, 0, 1])
R["C_mars_all"] = close(halo(allc, "Mars"), [0, 0, 1])
R["C_jupiter_all"] = close(halo(allc, "Jupiter"), [0, 0, 1])
# D: override precedence
R["D_ov1_venus_red"] = close(halo(ov1, "Venus"), [1, 0, 0])
R["D_ov2_venus_grey"] = close(halo(ov2, "Venus"), [0.2, 0.2, 0.2])   # broadcast overrode
R["D_ov2_mars_grey"] = close(halo(ov2, "Mars"), [0.2, 0.2, 0.2])
R["D_ov3_venus_red"] = close(halo(ov3, "Venus"), [1, 0, 0])          # per-body re-override
R["D_ov3_mars_grey"] = close(halo(ov3, "Mars"), [0.2, 0.2, 0.2])     # Mars kept the grey
# E: default -> existing body UNCHANGED (old parity)
R["E_venus_trail_unchanged"] = close(trailcol(defd, "Venus"), trailcol(base, "Venus"))
R["E_venus_trail_after_default"] = trailcol(defd, "Venus")
# F: reload resets to file
R["F_rpre1_venus"] = halo(rpre1, "Venus")
R["F_rpost1_venus"] = halo(rpost1, "Venus")
R["F_reset1_to_file"] = close(halo(rpost1, "Venus"), R["venus_file_halo"])
R["F_rpre2_venus"] = halo(rpre2, "Venus")
R["F_rpost2_venus"] = halo(rpost2, "Venus")
R["F_reset2_to_file"] = close(halo(rpost2, "Venus"), R["venus_file_halo"])
R["jd_all_equal"] = len({round(read_new(p)["__jd__"], 6)
                         for p in (p_base, p_perbody, p_all, p_rpre1, p_rpost1,
                                   p_rpre2, p_rpost2)}) == 1
R["md5_ssystem_preserved"] = md5(SSY) == MD5_SSY0
R["md5_config_preserved"] = md5(CFG) == MD5_CFG0

with open(f"{OUT}/b29_numeric.json", "w") as f:
    json.dump(R, f, indent=2)
print(json.dumps(R, indent=2), flush=True)
print("b29 done", flush=True)
s.close()
