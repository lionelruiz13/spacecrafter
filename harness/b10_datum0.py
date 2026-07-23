#!/usr/bin/env python3
# B10-datum0 discriminating case (INTENT 11.75(a) [vixy 2026-07-22], §11.80).
# THE case §11.80 handed the implementation row, now flipped:
#
#   Free-mode `moveto altitude X` at a MilkyWay reference must land at X.
#   Before the class default (datum=ground=0 for any ModularSystem) it landed
#   at 3.2e9 AU + X (MilkyWay datum = radius = 3.2e9 AU) - §11.80 measured live.
#
# Run the SAME driver at HEAD (baseline, class default = radius) and post-change:
#   - milkyway_default_datum0  : |pos| ~= X   (post)   vs  ~= 3.2e9 AU (baseline)
#                                THE flip - fails on baseline, passes post.
#   - milkyway_override_wins   : explicit datum_radius via the §11.84 command
#                                wins over the class default (|pos| = D + X);
#                                passes on BOTH binaries (the command pre-exists),
#                                so it isolates "override channel works" from
#                                "class default changed".
#   - milkyway_reverse_to_0 x2 : reversible pair - datum_radius 0 restores the
#                                centre-relative landing, twice, the 2nd entry
#                                from the 1st exit's state (rare-path discipline).
#
# The reference during the measured moveto is MilkyWay (a ModularSystem), so its
# getAltitudeReference() = scaledDatumRadius drives the landing. MilkyWay is not
# in the dual_dump body list (currentSystem = the OLD SolarSystem), so the datum
# is READ OFF the terminal navigation observable: datum = |pos| - X (the moveto
# free-mode identity |dst| = getAltitudeReference() + altitude).
import socket, time, json, math, sys, os

OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/b10datum0"
os.makedirs(OUT, exist_ok=True)

AU_M = 149597870700.0     # 1 AU in metres  (moveto altitude is in METRES)
AU_KM = 149597870.7       # 1 AU in km      (datum_radius command is in KM)

X_AU = 30000.0            # test altitude: outside the solar AoI (~578 AU), well
X_M = X_AU * AU_M         # inside MilkyWay (3.2e9 AU), and < 1e16 m so the old
                          # observer stays in the solar executor (measurement
                          # unperturbed by an executor flip).
D_AU = 1.0e6              # override datum: 1e6 AU (>> X, clearly separable)
D_KM = D_AU * AU_KM

results = []
def check(name, ok, detail):
    results.append({"name": name, "ok": bool(ok), "detail": detail})
    print(f"{'PASS' if ok else 'FAIL'}  {name}: {detail}", flush=True)

def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(f">> {cmd}", flush=True)

def dump(sock, tag, pause=1.6):
    path = f"{OUT}/b10d_{tag}.json"
    send(sock, f"body action dual_dump filename {path}", pause)
    return tag

def cam(tag):
    with open(f"{OUT}/b10d_{tag}.json") as f:
        return json.loads(f.readline())["camera"]

def plen(c):
    p = c["position"]
    return math.sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2])

s = socket.create_connection(("127.0.0.1", 7805), timeout=10)
send(s, "flag experimental_path on", 1)     # pin new path
send(s, "date jday 2461234.0", 1)
send(s, "timerate rate 0", 1)
send(s, "meteors zhr 0", 1)

# ---- fly to a MilkyWay reference (free mode) --------------------------------
send(s, "set home_planet Earth", 2)
send(s, "moveto lat 0 lon 0 alt 100 duration 0", 1.5)
send(s, "select planet Sun", 1)
send(s, "flag track_object on", 3)
send(s, "camera action free_mode state on", 1)
send(s, f"moveto altitude {int(800*AU_M)} duration 0", 3)   # ~800 AU -> ref=MilkyWay
send(s, "flag track_object off", 1)
c = cam(dump(s, "ref"))
check("reference_is_milkyway", c.get("reference") == "MilkyWay",
      f"reference={c.get('reference')!r} refDist={c.get('refDist')} |pos|={plen(c):.6e} AU")

# ---- THE discriminating case: default (class default 0) --------------------
send(s, f"moveto altitude {int(X_M)} duration 0", 2)
c = cam(dump(s, "default"))
pos = plen(c)
datum = pos - X_AU
check("milkyway_default_datum0", abs(pos - X_AU) < 0.02*X_AU,
      f"|pos|={pos:.6e} AU (want ~{X_AU:.0f}); derived MilkyWay datum={datum:.6e} AU "
      f"(post: ~0; BASELINE would be ~3.2e9 -> |pos|~3.2e9, this check FAILS on baseline)")

# ---- OVERRIDE: explicit datum_radius wins (§11.84 command) -----------------
def measure(tag):
    send(s, f"moveto altitude {int(X_M)} duration 0", 2)
    c = cam(dump(s, tag))
    return plen(c)

send(s, f"body name MilkyWay datum_radius {D_KM:.3f} ground_radius {D_KM:.3f}", 1.5)
p = measure("over1")
check("milkyway_override_wins", abs(p - (D_AU + X_AU)) < 0.02*(D_AU + X_AU),
      f"|pos|={p:.6e} AU (want ~{D_AU+X_AU:.3e}); explicit datum {D_AU:.0e} AU WON over class default 0")

# ---- REVERSE to 0 (reversible pair, twice; 2nd entry from 1st exit's state) -
send(s, "body name MilkyWay datum_radius 0 ground_radius 0", 1.5)
p = measure("rev1")
check("milkyway_reverse_to_0_a", abs(p - X_AU) < 0.02*X_AU,
      f"|pos|={p:.6e} AU (want ~{X_AU:.0f}); datum back to 0")

send(s, f"body name MilkyWay datum_radius {D_KM:.3f} ground_radius {D_KM:.3f}", 1.5)
p = measure("over2")
check("milkyway_override_wins_2", abs(p - (D_AU + X_AU)) < 0.02*(D_AU + X_AU),
      f"|pos|={p:.6e} AU (want ~{D_AU+X_AU:.3e}); 2nd override from the reversed state")

send(s, "body name MilkyWay datum_radius 0 ground_radius 0", 1.5)
p = measure("rev2")
check("milkyway_reverse_to_0_b", abs(p - X_AU) < 0.02*X_AU,
      f"|pos|={p:.6e} AU (want ~{X_AU:.0f}); datum back to 0 (2nd)")

s.close()
json.dump({"results": results, "X_AU": X_AU, "D_AU": D_AU},
          open(f"{OUT}/b10datum0_result.json", "w"), indent=1)
bad = [r for r in results if not r["ok"]]
print(f"=== {len(results)-len(bad)}/{len(results)} PASS ===", flush=True)
sys.exit(1 if bad else 0)
