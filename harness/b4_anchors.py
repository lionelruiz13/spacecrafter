#!/usr/bin/env python3
# B4 - the three camera-anchor kinds (INTENT §11.111, task F7; row §13.B B4,
# §12 row 19).  Locks R3's answer [§11.70(b)]: (3) on-orbit is the PRIMARY kind,
# (2) body-attached-keeping-its-angle is the NEEDED GAP, (1) fixed-point is
# Universe-scoped - each in BOTH §2(c) channels (authored anchor.ini AND the
# `camera action create/switch/drop/follow_rotation` commands).
#
# PRECONDITION: run through b4_anchors_run.sh - it builds the temp-HOME farm
# (11.103(a)) and authors the anchor file (shipped bytes + harness/b4_anchors.ini)
# so channel 1 is exercised through the production load path and the real
# ~/.spacecrafter is never written.  FRESH LAUNCH per run (this driver owns the
# app lifecycle).  FISHEYE; no init_fov requirement - every observable here is
# mat/position layer, no screen px.
#
# ------------------------------------------------------------------ PREDICTIONS
# Committed before the run (all arithmetic from the AUTHORED values in
# b4_anchors.ini, which are synthetic and chosen to make each number exact):
#
#  P1  ON-ORBIT holds through body motion.  The anchor rides a circle of
#      a = 200000 km, P = 0.5 d around the Moon, so at any date |ecl| = a, and
#      over dt = P/8 = 0.0625 d the anchor moves a chord of exactly
#      2a sin(pi dt/P) = 153073.373 km, while the MOON itself moves (its own
#      ephemeris, > 1000 km over the interval).  The camera anchored to it must
#      keep its relation to the anchor EXACTLY: distance delta == 0.
#  P2  KEEP-ANGLE.  Anchored to Mars with follow_rotation = false the camera's
#      frame (`camera.mat`) is INVARIANT under a date advance - bit-identical,
#      because nothing in its composition depends on jd once the surface bind is
#      dropped.  With follow_rotation = true the same frame rotates by EXACTLY
#      the body's spin advance (Mars' dumped axisRot difference), which is the
#      in-scene discriminator: the two legs differ by that whole angle.
#      NOTE on the metric: the relative-rotation angle is read with the
#      Frobenius small-angle form, never acos((tr-1)/2) - at 9 printed digits
#      the latter turns ulp noise into ~0.026 deg of phantom rotation (measured
#      on a BIT-IDENTICAL pair here; the same caveat orientation_check.py
#      carries, harness/README.md).
#  P3  FIXED POINT.  Declared at (10,0,0) AU, it is read in the ROOT frame, so
#      its `ecl` is bit-identical at both dates (nothing moves there - R3's own
#      justification) while the on-orbit anchor's moved 153073 km under the same
#      instrument.  Its parent is the Universe node, and switching to it from a
#      solar-system reference SUCCEEDS and says so in the log (the designed,
#      D12-logged out-of-Universe behaviour: the switch takes you to the
#      Universe frame rather than refusing).
#  P4  CHANNEL PARITY.  An anchor created by `camera action create` with the
#      SAME parameters as the authored one produces the SAME anchor state: the
#      two anchor bodies' `ecl` are equal AS STRINGS, and the camera state after
#      switching to each from the same start is equal on
#      reference-independent fields (longitude/latitude/distance/mat/bound).
#      Shown able to fail in-run by a THIRD anchor commanded with a different
#      semi-major axis (300000 km), whose ecl must differ.
#  P5  REVERSIBLE PAIRS, each traversed TWICE, the second entry starting from
#      the state the first exit produced: switch A->B->A->B, anchor->un-anchor
#      twice, create->drop->create->drop.  Split by observable, because the two
#      halves of the camera state have different exactness:
#        - POSE (longitude/latitude/distance/boundToSurface/freeMode/reference)
#          must be EXACTLY reproduced - no float path touches it on a switch;
#        - VIEW ORIENTATION (alt/az/heading, and `mat` with them) goes through
#          Camera::recoverParams' ZXZ Euler extraction once per switch, whose
#          float floor §11.61 already records (~6e-6 deg for one switch).  The
#          bound is therefore N x F with N = the number of switches between the
#          two compared dumps (counted from the scene, not fitted) and F the
#          per-switch floor MEASURED IN THIS RUN by the floor leg below: four
#          consecutive switches to the SAME anchor, each physically a no-op.
#          The measurement is also its own check - the drift must be LINEAR in
#          the number of extractions, which is what attributes it.
#  P7  SCREEN WITNESS (verification height: the terminal observable is the
#      composed screen, which no dump can stand in for).  Three shots, all from
#      an ANCHOR as the camera reference: the frame must carry lit content (an
#      anchor is a viewpoint that actually renders - a dump-only run cannot see
#      a black or broken frame), and the two on-orbit shots taken 1/8 of the
#      anchor's period apart must DIFFER, because the viewpoint swung 45 deg
#      around the Moon between them.  The fixed-point shot is the in-run control
#      for "renders at all" from the other kind.
#  P6  INSTRUMENT CHAIN.  14 anchors declared (10 shipped + 4 B4 - a load that
#      silently dropped the shipped corpus would pass everything else); every
#      OWNED anchor body carries bodyType 1 (ANCHOR) and relation < 3 (hidden,
#      i.e. outside the draw and pick sweeps).
#
# Exit 0 = all pass; artifacts + b4_result.json in <outdir>.

import socket, time, json, math, sys, os, subprocess, re, signal
import numpy as np
from PIL import Image

AU_KM = 149597870.7
J0 = 2461233.5
DT_ORBIT = 0.0625          # P/8 of the authored 0.5 d anchor orbit
DT_SPIN = 0.25             # Mars-spin interval for the keep-angle legs
A_KM = 200000.0            # authored semi-major axis
P_DAYS = 0.5               # authored period

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "artifacts", "b4")
FARM = os.environ.get("B4_FARM", "/tmp/b4_farm")
SC_BIN = os.environ.get("SC_BIN", os.path.join(HERE, "..", "..", "build-claude", "src", "spacecrafter"))
os.makedirs(OUT, exist_ok=True)

NAN = re.compile(r'(?<=[:,\[])-?nan\b')


def load(tag):
    """-> (header, {name: entry}).  The OLD path emits bare `nan` screen
    coordinates for a far fixed-point observer (pre-existing, old-path only -
    verified 0 NaN on the new side); tolerate it rather than lose the file."""
    header, bodies = None, {}
    with open(os.path.join(OUT, "b4_%s.json" % tag)) as f:
        for line in f:
            d = json.loads(NAN.sub("NaN", line))
            if d.get("type") == "header":
                header = d
            elif d.get("type") == "body":
                bodies[d["name"]] = d
    return header, bodies


def rot3(m):
    return [[m[0], m[1], m[2]], [m[4], m[5], m[6]], [m[8], m[9], m[10]]]


def rel_angle_deg(A, B):
    """Angle of B.A^T through the Frobenius small-angle form (see P2)."""
    C = [[sum(B[i][k] * A[j][k] for k in range(3)) for j in range(3)] for i in range(3)]
    f2 = sum((C[i][j] - (1.0 if i == j else 0.0)) ** 2 for i in range(3) for j in range(3))
    s = math.sqrt(max(0.0, f2)) / (2 * math.sqrt(2))
    return math.degrees(2 * math.asin(min(1.0, s)))


def dist_km(a, b):
    return math.sqrt(sum((a[i] - b[i]) ** 2 for i in range(3))) * AU_KM


# ------------------------------------------------------------------ launch ----
env = dict(os.environ)
env["HOME"] = FARM
env["DISPLAY"] = env.get("DISPLAY", ":2")
applog = open(os.path.join(OUT, "app.log"), "w")
app = subprocess.Popen([os.path.abspath(SC_BIN)], cwd=FARM, env=env,
                       stdout=applog, stderr=subprocess.STDOUT)
sock = None
for _ in range(60):
    time.sleep(1)
    if app.poll() is not None:
        print("FAIL: app exited during startup (rc=%s)" % app.returncode)
        sys.exit(2)
    try:
        sock = socket.create_connection(("127.0.0.1", 7805), timeout=5)
        break
    except OSError:
        continue
if sock is None:
    app.kill()
    print("FAIL: no TCP interface after 60 s")
    sys.exit(2)
time.sleep(3)


def send(cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)
    print(">> %s" % cmd, flush=True)


def dump(tag):
    send("body action dual_dump filename %s/b4_%s.json" % (OUT, tag), 2.5)


def shot(tag):
    send("body action screenshot filename %s/b4_%s.png" % (OUT, tag), 3.0)


def img(tag):
    return np.asarray(Image.open("%s/b4_%s.png" % (OUT, tag)).convert("RGB"), dtype=np.int16)


ORBIT_KEYS = ("type orbit parent Moon coord_func ell_orbit orbit_period 0.5 "
              "orbit_epoch 2451545.0 orbit_eccentricity 0.0 orbit_inclination 0.0 "
              "orbit_ascendingnode 0.0 orbit_longofpericenter 0.0 "
              "orbit_meanlongitude 0.0 orbit_semimajoraxis ")

send("flag experimental_path on", 1)
send("timerate rate 0", 1)                 # BEFORE the epoch (b19 lesson)
send("date jday %.9f" % J0, 1)
dump("base")

# ---- floor leg: four no-op switches to the SAME anchor (P5's instrument) ----
for k in range(5):
    send("camera action switch name b4_orbit_moon", 1.2)
    dump("floor%d" % k)

# ---- P1 on-orbit (+ P7 screen witness) -------------------------------------
send("camera action switch name b4_orbit_moon", 1.2)
# Aim at the Moon, then RELEASE tracking before any shot (11.80(c)/11.94(e)):
# tracking would re-centre the Moon at t1 and hide the very motion under test.
# Without this the Moon sits outside the frame (measured: screenPos x = 1.14,
# i.e. |NDC| > 1) and the shots differ only by the star field.
send("select planet Moon", 1.2)
send("flag track_object on", 3.0)
send("flag track_object off", 1.5)
dump("orb_t0")
shot("orb_t0")
send("date jday %.9f" % (J0 + DT_ORBIT), 1.5)
dump("orb_t1")
shot("orb_t1")

# ---- P3 fixed point (same instrument, same interval) -----------------------
send("date jday %.9f" % J0, 1.2)
send("camera action switch name b4_fixed", 1.2)
dump("fix_t0")
shot("fix_t0")
send("date jday %.9f" % (J0 + DT_ORBIT), 1.5)
dump("fix_t1")

# ---- P2 keep-angle vs follow ----------------------------------------------
for entry in (1, 2):                        # reversible pair, twice (P5)
    for kind in ("keepangle", "follow"):
        send("date jday %.9f" % J0, 1.2)
        send("camera action switch name b4_mars_%s" % kind, 1.2)
        dump("%s%d_t0" % (kind, entry))
        send("date jday %.9f" % (J0 + DT_SPIN), 1.5)
        dump("%s%d_t1" % (kind, entry))

# ---- P4 channel parity + P5 create/drop twice ------------------------------
send("date jday %.9f" % J0, 1.2)
for entry in (1, 2):
    send("camera action switch name Earth", 1.2)          # common start state
    send("camera action create name b4_cmd_orbit %s200000.0" % ORBIT_KEYS, 1.2)
    send("camera action create name b4_cmd_other %s300000.0" % ORBIT_KEYS, 1.2)
    send("camera action switch name b4_cmd_orbit", 1.2)
    dump("cmd%d" % entry)
    send("camera action switch name b4_orbit_moon", 1.2)
    dump("auth%d" % entry)
    send("camera action switch name b4_cmd_other", 1.2)
    dump("other%d" % entry)
    send("camera action switch name Earth", 1.2)          # leave before dropping
    dump("unanchor%d" % entry)
    send("camera action drop name b4_cmd_orbit", 1.2)
    send("camera action drop name b4_cmd_other", 1.2)
    dump("dropped%d" % entry)

sock.close()
app.send_signal(signal.SIGTERM)
try:
    app.wait(timeout=30)
except subprocess.TimeoutExpired:
    app.kill()
applog.close()

# ---------------------------------------------------------------- evaluate ----
fail = 0
report = {"predictions": {}, "checks": []}


def check(ok, text, **kv):
    global fail
    fail += not ok
    print("%s %s" % ("OK  " if ok else "FAIL", text), flush=True)
    report["checks"].append(dict(ok=bool(ok), text=text, **kv))
    return ok


D = {}
for tag in ("base", "floor0", "floor1", "floor2", "floor3", "floor4",
            "orb_t0", "orb_t1", "fix_t0", "fix_t1",
            "keepangle1_t0", "keepangle1_t1", "follow1_t0", "follow1_t1",
            "keepangle2_t0", "keepangle2_t1", "follow2_t0", "follow2_t1",
            "cmd1", "auth1", "other1", "unanchor1", "dropped1",
            "cmd2", "auth2", "other2", "unanchor2", "dropped2"):
    D[tag] = load(tag)

print("\n--- P6: instrument chain (the authored file actually loaded) ---")
anc = D["base"][0]["anchors"]
check(anc["count"] == 14, "14 anchors declared (10 shipped + 4 B4), got %d" % anc["count"],
      count=anc["count"], declared=anc["declared"])
for n in ("b4_orbit_moon", "b4_mars_keepangle", "b4_mars_follow", "b4_fixed"):
    check(n in anc["declared"], "authored anchor '%s' declared" % n)
for n in ("b4_orbit_moon", "b4_fixed"):
    e = D["orb_t0"][1].get(n)
    check(e is not None and e["old"] is None and e["new"]["bodyType"] == 1,
          "%s is a new-path-only body of bodyType ANCHOR(1)" % n)
    check(e is not None and e["new"]["relation"] < 3,
          "%s is HIDDEN (relation %s < 3) - outside the draw and pick sweeps"
          % (n, e["new"]["relation"] if e else "?"))

print("\n--- P5 instrument: the per-switch recoverParams Euler floor, measured ---")
VIEW = ("alt", "az", "heading")
POSE = ("longitude", "latitude", "distance", "boundToSurface", "freeMode")


def view_delta(a, b):
    return max(abs(D[a][0]["camera"][k] - D[b][0]["camera"][k]) for k in VIEW)


steps = [view_delta("floor%d" % k, "floor%d" % (k + 1)) for k in range(4)]
F = max(steps) if max(steps) > 0 else 1e-9
print("    per-switch view deltas (rad): %s" % ["%.2e" % s for s in steps])
for k in range(5):
    check(all(D["floor%d" % k][0]["camera"][f] == D["floor0"][0]["camera"][f] for f in POSE),
          "floor leg %d: POSE bit-identical across a no-op switch" % k)
cum = view_delta("floor0", "floor4")
check(cum <= 4 * F * 1.0001,
      "floor drift is LINEAR in the extraction count: 4 switches give %.2e rad <= 4 x F (%.2e)"
      % (cum, 4 * F), F=F, cumulative=cum)
report["predictions"]["per_switch_euler_floor_rad"] = F
print("    F = %.3e rad = %.2e deg per switch (§11.61 recoverParams floor)"
      % (F, math.degrees(F)))

print("\n--- P1: on-orbit anchor holds through body motion ---")
h0, b0 = D["orb_t0"]; h1, b1 = D["orb_t1"]
e0 = b0["b4_orbit_moon"]["new"]["ecl"]; e1 = b1["b4_orbit_moon"]["new"]["ecl"]
r0 = dist_km(e0, [0, 0, 0]); r1 = dist_km(e1, [0, 0, 0])
chord = dist_km(e0, e1)
pred_chord = 2 * A_KM * math.sin(math.pi * DT_ORBIT / P_DAYS)
moon = dist_km(b0["Moon"]["new"]["ecl"], b1["Moon"]["new"]["ecl"])
check(h0["camera"]["reference"] == "b4_orbit_moon", "camera reference is the anchor")
check(abs(r0 - A_KM) < 1.0 and abs(r1 - A_KM) < 1.0,
      "anchor radius |ecl| = %.3f / %.3f km (authored %.1f)" % (r0, r1, A_KM), r0=r0, r1=r1)
check(abs(chord - pred_chord) < 0.001 * pred_chord,
      "anchor moved %.3f km, predicted 2a.sin(pi.dt/P) = %.3f km (%.2e rel)"
      % (chord, pred_chord, abs(chord - pred_chord) / pred_chord), chord=chord, predicted=pred_chord)
check(moon > 1000.0, "the MOON itself moved %.1f km over the same interval" % moon, moon_km=moon)
dd = h1["camera"]["distance"] - h0["camera"]["distance"]
check(dd == 0.0, "camera-to-anchor distance delta = %.3e AU (must be exactly 0)" % dd, delta=dd)

print("\n--- P3: fixed point is fixed in the Universe frame (same instrument) ---")
hf0, bf0 = D["fix_t0"]; hf1, bf1 = D["fix_t1"]
f0 = bf0["b4_fixed"]["new"]["ecl"]; f1 = bf1["b4_fixed"]["new"]["ecl"]
check(f0 == f1, "fixed-point ecl bit-identical across the date advance: %s" % f0, ecl=f0)
check(f0 == [10.0, 0.0, 0.0], "fixed-point ecl == the authored (10,0,0) AU")
check(hf0["camera"]["refParent"] == "Universe",
      "reference parent is the Universe node (got '%s')" % hf0["camera"]["refParent"])
check(chord > 1000.0 and dist_km(f0, f1) == 0.0,
      "DISCRIMINATION: orbit anchor moved %.1f km, fixed point 0.000 km, same instrument" % chord)
log = open(os.path.join(FARM, ".spacecrafter", "log", "spacecrafter.log"),
           errors="replace").read()
check("fixed-point anchors are read in the Universe frame" in log,
      "the designed out-of-Universe behaviour is LOGGED at the switch (D12)")
check(D["orb_t0"][0]["camera"]["reference"] == "b4_orbit_moon",
      "the switch to the fixed point came FROM a solar-system reference")

print("\n--- P2: keep-angle vs follow-rotation ---")
angles = {}
for entry in (1, 2):
    for kind in ("keepangle", "follow"):
        a, b = D["%s%d_t0" % (kind, entry)], D["%s%d_t1" % (kind, entry)]
        ang = rel_angle_deg(rot3(a[0]["camera"]["mat"]), rot3(b[0]["camera"]["mat"]))
        spin = math.degrees((b[1]["Mars"]["new"]["axisRot"] - a[1]["Mars"]["new"]["axisRot"]) % (2 * math.pi))
        angles[(kind, entry)] = (ang, spin, a[0]["camera"]["mat"] == b[0]["camera"]["mat"])
        check(a[0]["camera"]["reference"] == "Mars",
              "%s entry %d: reference is Mars" % (kind, entry))
        check(a[0]["camera"]["boundToSurface"] == (kind == "follow"),
              "%s entry %d: boundToSurface = %s (the kind's own state)"
              % (kind, entry, a[0]["camera"]["boundToSurface"]))
        if kind == "keepangle":
            check(angles[(kind, entry)][2],
                  "keepangle entry %d: camera frame BIT-IDENTICAL across %.2f d (angle %.6f deg) "
                  "while Mars spun %.4f deg" % (entry, DT_SPIN, ang, spin), angle=ang, spin=spin)
        else:
            check(abs(ang - spin) < 0.01,
                  "follow entry %d: camera frame rotated %.6f deg == Mars spin %.6f deg (d=%.2e)"
                  % (entry, ang, spin, abs(ang - spin)), angle=ang, spin=spin)
for entry in (1, 2):
    ka, fa = angles[("keepangle", entry)][0], angles[("follow", entry)][0]
    check(fa > 1000 * max(ka, 1e-9),
          "DISCRIMINATION entry %d: follow %.4f deg vs keepangle %.6f deg" % (entry, fa, ka))
FLOOR_DEG = math.degrees(F)
check(abs(angles[("keepangle", 1)][0] - angles[("keepangle", 2)][0]) <= FLOOR_DEG
      and abs(angles[("follow", 1)][0] - angles[("follow", 2)][0]) <= FLOOR_DEG,
      "REVERSIBLE: the second entry of the keep/follow pair reproduces the first within the "
      "measured per-switch floor (%.2e deg): keep %.3e, follow %.3e"
      % (FLOOR_DEG, abs(angles[("keepangle", 1)][0] - angles[("keepangle", 2)][0]),
         abs(angles[("follow", 1)][0] - angles[("follow", 2)][0])))

print("\n--- P4: both channels produce the same anchor ---")
for entry in (1, 2):
    hc, bc = D["cmd%d" % entry]; ha, ba = D["auth%d" % entry]; ho, bo = D["other%d" % entry]
    ec = bc["b4_cmd_orbit"]["new"]["ecl"]; ea = ba["b4_orbit_moon"]["new"]["ecl"]
    eo = bo["b4_cmd_other"]["new"]["ecl"]
    check(ec == ea, "entry %d: commanded and authored anchor ecl identical: %s" % (entry, ec))
    check(eo != ea, "entry %d: DISCRIMINATION - a differently-parametrized commanded anchor "
                    "differs (%s vs %s)" % (entry, eo, ea))
    same = [k for k in POSE if hc["camera"][k] == ha["camera"][k]]
    check(len(same) == len(POSE),
          "entry %d: camera POSE identical (%s) after switching to either channel's anchor"
          % (entry, ",".join(POSE)), matched=same)
    # ONE switch separates the two dumps -> one Euler extraction of headroom.
    dv = view_delta("cmd%d" % entry, "auth%d" % entry)
    check(dv <= 1 * F,
          "entry %d: camera VIEW identical to within 1 x the measured switch floor "
          "(%.2e rad <= %.2e)" % (entry, dv, F), delta=dv)
    check(hc["anchors"]["kind"] == ha["anchors"]["kind"] == "orbit",
          "entry %d: both channels report kind 'orbit'" % entry)

print("\n--- P5: reversible pairs, second entry from the first exit's state ---")
# Switches between the two entries of each pair, counted from the scene above
# (Earth, cmd, auth, other, Earth): five, hence five Euler extractions.
N_SWITCH = 5
for tag in ("cmd", "auth", "other", "unanchor", "dropped"):
    h1_, h2_ = D[tag + "1"][0], D[tag + "2"][0]
    check(all(h1_["camera"][k] == h2_["camera"][k] for k in POSE)
          and h1_["camera"]["reference"] == h2_["camera"]["reference"],
          "%s: entry 2 POSE + reference EXACTLY == entry 1" % tag)
    dv = view_delta(tag + "1", tag + "2")
    check(dv <= N_SWITCH * F,
          "%s: entry 2 VIEW == entry 1 within %d x the measured switch floor (%.2e <= %.2e rad)"
          % (tag, N_SWITCH, dv, N_SWITCH * F), delta=dv)
for entry in (1, 2):
    hd, bd = D["dropped%d" % entry]
    check("b4_cmd_orbit" not in hd["anchors"]["declared"],
          "entry %d: dropped anchor is gone from the registry" % entry)
    check("b4_cmd_orbit" not in bd,
          "entry %d: the dropped anchor's body is gone from the tree" % entry)
    hu = D["unanchor%d" % entry][0]
    check(hu["camera"]["reference"] == "Earth",
          "entry %d: un-anchoring reverses (reference back to Earth)" % entry)
    hc = D["cmd%d" % entry][1]
    check("b4_cmd_orbit" in hc, "entry %d: re-created anchor body is back in the tree" % entry)

print("\n--- P7: screen witness (the composed screen, from an anchor) ---")
lit = {}
# Floors are per-scene predictions, not one number: from the on-orbit anchor the
# lit Moon disc + Earth + Sun are in frame (>1000 px); from a fixed point 10 AU
# out the frame carries the star field and the distant Sun, so the honest claim
# is only "it renders at all" (>0) - the measured value is reported either way.
for tag, floor in (("orb_t0", 1000), ("orb_t1", 1000), ("fix_t0", 0)):
    a = img(tag)
    lit[tag] = int((a.max(axis=2) > 16).sum())
    check(lit[tag] > floor,
          "%s: the anchored camera RENDERS - %d lit px (>16/255), floor %d"
          % (tag, lit[tag], floor), lit=lit[tag])
d = np.abs(img("orb_t0") - img("orb_t1"))
moved = int((d > 32).any(axis=2).sum())
print("    frame-wide difference between the two on-orbit shots: %d px>32 (reported)" % moved)
report["predictions"]["orb_px32"] = moved

# THE screen witness proper: the composed screen must show the Moon WHERE THE
# DUMP SAYS at both ends of the interval - a 2x2 contrast test with its own
# in-run control (the other date's position is empty sky at this date).  The
# window radius is 2x the disc radius PREDICTED from measured quantities
# (atan(R/d) / halfFov x H/2), never a tuned number; R and d come from the dump.
H = img("orb_t0").shape[0]
hf = D["orb_t0"][0]["camera"]["halfFov"]
Rm = D["orb_t0"][1]["Moon"]["new"]["scaledDatumRadius"]
dm = D["orb_t0"][1]["Moon"]["new"]["dist"]
rpx = math.atan(Rm / dm) / hf * (H / 2)
win = max(8, int(2 * rpx))
P = {t: D["orb_%s" % t][1]["Moon"]["new"]["screen"] for t in ("t0", "t1")}
sep_px = math.dist(P["t0"], P["t1"]) * (H / 2)
print("    Moon disc radius %.1f px (from R=%.3e AU, d=%.3e AU, halfFov=%.4f); dumped screen "
      "separation %.1f px; window +-%d px" % (rpx, Rm, dm, hf, sep_px, win))


def bright(shot_tag, ndc):
    a = img(shot_tag).max(axis=2)
    x = int((ndc[0] * 0.5 + 0.5) * a.shape[1])
    y = int((1.0 - (ndc[1] * 0.5 + 0.5)) * a.shape[0])   # screen y up -> image row down
    if not (0 <= x < a.shape[1] and 0 <= y < a.shape[0]):
        return None
    return int(a[max(0, y - win):y + win, max(0, x - win):x + win].max())


cells = {(s, p): bright("orb_%s" % s, P[p]) for s in ("t0", "t1") for p in ("t0", "t1")}
print("    window max luminance: %s" % {("%s@%s" % k): v for k, v in cells.items()})
check(sep_px > 4 * rpx,
      "the dumped Moon position moved %.1f px, more than the %.1f px disc - the two windows are "
      "disjoint, so the test below has a real control" % (sep_px, 2 * rpx))
check(all(v is not None for v in cells.values()),
      "both dumped Moon positions are inside the frame (the shot can witness them)")
if all(v is not None for v in cells.values()):
    check(cells[("t0", "t0")] > 64 and cells[("t1", "t1")] > 64,
          "SCREEN WITNESS: the Moon is BRIGHT where the dump puts it at each date "
          "(%d and %d / 255)" % (cells[("t0", "t0")], cells[("t1", "t1")]))
    check(cells[("t0", "t1")] < cells[("t0", "t0")] and cells[("t1", "t0")] < cells[("t1", "t1")],
          "SCREEN WITNESS control: the OTHER date's position is darker in each shot "
          "(%d < %d and %d < %d) - the disc moved on the composed screen exactly as the anchor's "
          "orbit says" % (cells[("t0", "t1")], cells[("t0", "t0")],
                          cells[("t1", "t0")], cells[("t1", "t1")]))

report["summary"] = {"failures": fail}
with open(os.path.join(OUT, "b4_result.json"), "w") as f:
    json.dump(report, f, indent=1)
print("\n%d failure(s); report %s/b4_result.json" % (fail, OUT))
sys.exit(1 if fail else 0)
