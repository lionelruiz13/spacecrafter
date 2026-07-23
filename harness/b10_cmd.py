#!/usr/bin/env python3
# B10-cmd: runtime command follow-through for datum_radius/ground_radius
# (INTENT 11.71 landing + 11.79(e) D9key answer). The scalars already flow
# from the ssystem.ini keys; this proves the RUNTIME COMMAND drives the SAME
# scalars and the SAME discriminating cases - now on a body LOADED WITH THE
# DEFAULTS (datum=ground=radius) and then COMMANDED, so the behavior FLIP is
# attributable to the command, not to the load.
#
# Command spelling per D9key (verbatim §11.79(e)): "data keys stay
# datum_radius/ground_radius, the COMMAND matches the data word order" =>
#   body name <X> datum_radius <km>
#   body name <X> ground_radius <km>
# (datum/ground FIRST - NOT Q12's `radius datum`/`radius ground`).
#
# Two 2(c) channels, both exercised on this one fresh launch:
#   channel 1 (live/TCP):  the commands sent directly below
#   channel 2 (script):    `script action play filename b10_cmd_script.sts`
#
# Observables:
#   - camera `distance` (AU, anchored) / `position` (AU, free): the BEHAVIORAL
#     discriminators (moveto altitude 0 -> centre / surface / ground hold)
#   - body `new.scaledDatumRadius` / `new.scaledGroundRadius` (AU): the DIRECT
#     numeric proof the live scalars change when commanded (B10-cmd instrument)
import socket, time, json, math, sys, os

OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/b10cmd"
os.makedirs(OUT, exist_ok=True)

AU_KM = 149597870.691   # sc_const.hpp AU (km), the value the code converts with
R_KM = 6000.0           # test-body radius
CLR_KM = 6012.0         # radius * 1.002 (terrain clearance)

def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

def dump(sock, tag, pause=1.5):
    path = f"{OUT}/b10c_{tag}.json"
    send(sock, f"body action dual_dump filename {path}", pause)
    return path

def load_dump(tag):
    with open(f"{OUT}/b10c_{tag}.json") as f:
        lines = f.read().splitlines()
    header = json.loads(lines[0])
    bodies = {}
    for ln in lines[1:]:
        ln = ln.strip()
        if not ln:
            continue
        try:
            o = json.loads(ln)
        except Exception:
            continue
        if o.get("type") == "body":
            bodies[o["name"]] = o
    return header["camera"], bodies

def cam_dist_km(tag):
    c, _ = load_dump(tag)
    return c["distance"] * AU_KM

def cam_pos_km(tag):
    c, _ = load_dump(tag)
    p = c["position"]
    return math.sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]) * AU_KM

def body_scaled_km(tag, name):
    _, bodies = load_dump(tag)
    b = bodies.get(name)
    if b is None or b.get("new") is None:
        return None, None
    n = b["new"]
    return n["scaledDatumRadius"] * AU_KM, n["scaledGroundRadius"] * AU_KM

# ---------------------------------------------------------------------------
s = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(s, "flag experimental_path on", 1)   # pin new path
send(s, "date jday 2461233.5", 1)
send(s, "timerate rate 0", 1)

COMMON = ("parent Sun type Planet oblateness 0.0 albedo 0.3 halo false "
          "color 0.6,0.6,0.9 tex_map bodies/moon.png coord_func still_orbit")

# ============================================================================
# CHANNEL 1 (live/TCP): body loaded with DEFAULT radii, then COMMANDED.
# ============================================================================
send(s, f"body action load name CmdTest radius {int(R_KM)} {COMMON} "
        f"orbit_x 900 orbit_y 0 orbit_z 0", 2)
send(s, "set home_planet CmdTest", 3)

# (0) DEFAULT (solid): datum=ground=radius; moveto altitude 0 -> SURFACE.
send(s, "moveto lat 0 lon 0 alt 100 duration 0", 1.5)
send(s, "moveto altitude 0 duration 0", 1.5); dump(s, "c1_default_a0")

# (1) COMMAND datum=ground=0 (enterable). moveto altitude 0 -> CENTRE.
send(s, "body name CmdTest datum_radius 0", 1.0)
send(s, "body name CmdTest ground_radius 0", 1.5); dump(s, "c1_enter_scaled")
send(s, "moveto altitude 0 duration 0", 1.5); dump(s, "c1_enter_a0")
send(s, "moveto altitude 100000 duration 0", 1.5); dump(s, "c1_enter_a100k")

# (2) REVERSE: command back to the file value (radius). moveto altitude 0 -> SURFACE.
send(s, "body name CmdTest datum_radius 6000", 1.0)
send(s, "body name CmdTest ground_radius 6000", 1.5); dump(s, "c1_restore_scaled")
send(s, "moveto altitude 0 duration 0", 1.5); dump(s, "c1_restore_a0")

# (3) CLEARANCE: command ground=radius*1.002 (datum stays radius).
#     Free-mode descent HOLDS at ground_radius (hard stop), reversible x2.
send(s, "body name CmdTest ground_radius 6012", 1.5); dump(s, "c1_clear_scaled")
send(s, "camera action free_mode state on", 1.5)
send(s, "moveto altitude 0 duration 0", 2); dump(s, "c1_clear_f_desc1")
send(s, "moveto altitude 100000 duration 0", 2); dump(s, "c1_clear_f_asc1")
send(s, "moveto altitude 0 duration 0", 2); dump(s, "c1_clear_f_desc2")
send(s, "moveto altitude 100000 duration 0", 2); dump(s, "c1_clear_f_asc2")
send(s, "camera action free_mode state off", 1.5)

# (4) 2(f) diagnostics: negative value REFUSED (scalar unchanged from step 3
#     ground=6012), missing body a silent no-op (no crash).
send(s, "body name CmdTest datum_radius -5", 1.5); dump(s, "c1_neg")
send(s, "body name NoSuchBody datum_radius 100", 1.5); dump(s, "c1_missing")

# ============================================================================
# CHANNEL 2 (script): the SAME command reached from a played .sts file.
# ============================================================================
send(s, f"body action load name CmdScript radius {int(R_KM)} {COMMON} "
        f"orbit_x 0 orbit_y 900 orbit_z 0", 2)
send(s, "set home_planet CmdScript", 3)
send(s, "moveto lat 0 lon 0 alt 100 duration 0", 1.5)
send(s, "moveto altitude 0 duration 0", 1.5); dump(s, "c2_default_a0")
# The script issues: body name CmdScript datum_radius 0 / ground_radius 0
send(s, "script action play filename b10_cmd_script.sts", 6); dump(s, "c2_script_scaled")
send(s, "moveto altitude 0 duration 0", 1.5); dump(s, "c2_script_a0")
s.close()

# ---------------------------------------------------------------------------
# REPORT + ASSERTS
# ---------------------------------------------------------------------------
fails = []
def check(name, cond, detail):
    print(f"[{'PASS' if cond else 'FAIL'}] {name}: {detail}", flush=True)
    if not cond:
        fails.append(name)

print("\n==== B10-cmd numeric layer ====", flush=True)

# Channel 1 direct-scalar proof
d0, g0 = body_scaled_km("c1_default_a0", "CmdTest")
de, ge = body_scaled_km("c1_enter_scaled", "CmdTest")
dr, gr = body_scaled_km("c1_restore_scaled", "CmdTest")
dc, gc = body_scaled_km("c1_clear_scaled", "CmdTest")
print(f"scaled datum/ground km: default=({d0},{g0}) enter=({de},{ge}) "
      f"restore=({dr},{gr}) clear=({dc},{gc})", flush=True)

check("scalar_default_solid", d0 is not None and abs(d0-R_KM) < 2 and abs(g0-R_KM) < 2,
      f"datum={d0} ground={g0} (expect ~{R_KM})")
check("scalar_command_enter", de is not None and abs(de) < 1 and abs(ge) < 1,
      f"datum={de} ground={ge} (expect ~0 after command)")
check("scalar_command_restore", dr is not None and abs(dr-R_KM) < 2 and abs(gr-R_KM) < 2,
      f"datum={dr} ground={gr} (expect ~{R_KM} after reverse command)")
check("scalar_command_clearance", gc is not None and abs(gc-CLR_KM) < 2 and abs(dc-R_KM) < 2,
      f"datum={dc} ground={gc} (expect datum~{R_KM} ground~{CLR_KM})")

# Channel 1 behavioral discriminators
bd = cam_dist_km("c1_default_a0")
be = cam_dist_km("c1_enter_a0")
br = cam_dist_km("c1_restore_a0")
print(f"anchored moveto-alt-0 distance km: default={bd:.3f} enter={be:.3f} restore={br:.3f}", flush=True)
check("behav_default_surface", abs(bd-R_KM) < 2, f"{bd:.3f} km (expect ~{R_KM}, surface)")
check("behav_command_centre", abs(be) < 1, f"{be:.3f} km (expect ~0, CENTRE - command made it enterable)")
check("behav_command_restore", abs(br-R_KM) < 2, f"{br:.3f} km (expect ~{R_KM}, surface restored)")

# Channel 1 clearance free-descent hold + reversible x2
fd1 = cam_pos_km("c1_clear_f_desc1"); fa1 = cam_pos_km("c1_clear_f_asc1")
fd2 = cam_pos_km("c1_clear_f_desc2"); fa2 = cam_pos_km("c1_clear_f_asc2")
print(f"free clearance |pos| km: desc1={fd1:.3f} asc1={fa1:.3f} desc2={fd2:.3f} asc2={fa2:.3f}", flush=True)
# escape target = datum(6000) + altitude 100000 m (=100 km) = 6100 km, ABOVE
# the 6012 ground clamp (the §11.71 "asc->6100 escape" figure); the discriminator
# is hold(6012) != escape(6100), repeatable.
ESC_KM = R_KM + 100.0
check("clear_hold1", abs(fd1-CLR_KM) < 2, f"desc1={fd1:.3f} (expect hold ~{CLR_KM})")
check("clear_escape1", fa1 > CLR_KM + 50 and abs(fa1-ESC_KM) < 3,
      f"asc1={fa1:.3f} (expect escape ~{ESC_KM}, above clamp {CLR_KM})")
check("clear_hold2", abs(fd2-CLR_KM) < 2, f"desc2={fd2:.3f} (expect hold ~{CLR_KM}, 2nd entry)")
check("clear_escape2", fa2 > CLR_KM + 50 and abs(fa2-ESC_KM) < 3,
      f"asc2={fa2:.3f} (expect escape ~{ESC_KM}, above clamp {CLR_KM}, 2nd)")

# 2(f): negative refused (scalar stays at clearance value 6012), missing no-op
dn, gn = body_scaled_km("c1_neg", "CmdTest")
check("neg_refused", dn is not None and abs(dn-R_KM) < 2,
      f"datum after `datum_radius -5` = {dn} (expect UNCHANGED ~{R_KM}, refused)")
cm, _ = load_dump("c1_missing")
check("missing_no_crash", cm is not None, "app answered after missing-body command (no crash)")

# Channel 2 (script)
d2d, g2d = body_scaled_km("c2_default_a0", "CmdScript")
d2s, g2s = body_scaled_km("c2_script_scaled", "CmdScript")
b2d = cam_dist_km("c2_default_a0"); b2s = cam_dist_km("c2_script_a0")
print(f"CHANNEL2 script: scaled default=({d2d},{g2d}) after=({d2s},{g2s}) "
      f"moveto-alt-0 default={b2d:.3f} after={b2s:.3f}", flush=True)
check("script_scalar_enter", d2s is not None and abs(d2s) < 1 and abs(g2s) < 1,
      f"datum={d2s} ground={g2s} (expect ~0 via SCRIPT channel)")
check("script_behav_centre", abs(b2d-R_KM) < 2 and abs(b2s) < 1,
      f"default={b2d:.3f} after={b2s:.3f} (expect surface->centre via SCRIPT)")

print(f"\n==== B10-cmd: {'ALL PASS' if not fails else 'FAILURES: '+','.join(fails)} ====", flush=True)
sys.exit(0 if not fails else 1)
