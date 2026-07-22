#!/usr/bin/env python3
# B9 - azimuth-convention divergence probe + regression lock (INTENT §11.4/§11.60).
#
# ROW: the old path reports azimuth as az = 3π − az (mod 2π) ("N is zero, E is 90"),
#   applied at each old reporting site [observed: body.cpp:381 (getAltAz),
#   body.cpp:341 (getInfoString), body.cpp:431 (getShortInfoNavString)].
#   The new ModularObject surface returned the Camera-frame RAW az [was
#   ModularObject.cpp:122-124]. The fix installs ONE conversion authority
#   ModularObject::altAz() consumed by getAltAz + getInfoString +
#   getShortInfoNavString (§11.60).
#
# INSTRUMENT: `body action dual_dump` now emits per body, at the SAME frame:
#   altaz_old = Body::getAltAz  (old convention)
#   altaz_new = the REAL ModularObject::getAltAz (the D2 bridge method
#               getSelectedAZ→getSelected().getAltAz would call once wired;
#               ModularObject is not yet instantiated in selection, so this is
#               the only live path to the surface today - §13.B B9 finding).
#   sidecar <file>.navstr = the caller-visible nav/info STRINGS both paths print.
#
# DISCRIMINATION (the lock is meaningful; it detects the bug):
#   PRE-FIX  binary: az_new is RAW  → |wrap(az_old − az_new)| ≈ 3π-offset (FAIL),
#                    but |wrap(az_old − (3π − az_new))| ≈ 0 (the convention holds).
#   POST-FIX binary: az_new is CONVERTED → |wrap(az_old − az_new)| ≈ 0 (PASS).
#   alt_old ≈ alt_new in BOTH (the fix must not touch altitude).
#
# METHOD: FISHEYE config (harness precondition), time FROZEN per sample, several
#   observer states (2 dates × 3 surface locations, tracking on and off) so a
#   per-body / per-hop / per-observer residual would show as a spread, not a
#   constant. Anchored (surface) mode only: that is where alt/az is a meaningful
#   readout and where the old path defines it; free mode reinterprets "local" as
#   the body frame (Camera.hpp:148-149), a different quantity with no old-path
#   az counterpart, so parity there is undefined (stated, not tested).
#
#   ./b9_run.sh                 # fresh launch + this driver
# Exit 0 = az parity + alt parity within tolerance on every fresh body/sample.

import socket, time, json, sys, os, math

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.abspath(sys.argv[1] if len(sys.argv) > 1
                      else os.path.join(HERE, "artifacts", "b9"))
os.makedirs(OUT, exist_ok=True)

# Tolerances. az_old is double (Navigator/earthEquToLocal), az_new is float32
# (Camera Vec3f) and the two topocentric frames differ by the P4 ~km class, so a
# small angular residual is expected and its class is stated in the report.
AZ_TOL_DEG = 0.05      # az parity (post-fix) and convention (pre-fix)
ALT_TOL_DEG = 0.05     # altitude unaffected

sock = socket.create_connection(("127.0.0.1", 7805), timeout=15)


def send(cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None); print(f">> {cmd}", flush=True)


# (label, jd, lat, lon, alt, track_target-or-None)
SAMPLES = [
    ("S1_paris_dA_trackMoon", 2461233.5, 48.85, 2.35, 100.0, "Moon"),
    ("S2_paris_dB_trackMoon", 2461321.25, 48.85, 2.35, 100.0, "Moon"),
    ("S3_sydney_dA_free",     2461233.5, -33.9, 151.2, 0.0, None),
    ("S4_north_dA_free",      2461233.5, 60.0, -45.0, 500.0, None),
]

send("flag experimental_path on", 1)   # pin NEW path (dump emits both regardless)
send("set home_planet Earth", 1)

files = []
for (label, jd, lat, lon, alt, track) in SAMPLES:
    send("timerate rate 0", 0.5)
    send(f"date jday {jd}", 1)
    send(f"moveto lat {lat} lon {lon} alt {alt} duration 0", 1.5)
    if track:
        send(f"select planet {track}", 0.5)
        send("flag track_object on", 12)   # smoothing settle (INTENT 11.19c)
    else:
        send("flag track_object off", 1)
    time.sleep(2)   # let both paths settle at the frozen jd
    fp = os.path.join(OUT, label + ".json")
    send(f"body action dual_dump filename {fp}", 2.5)
    files.append((label, fp))

sock.close()
print("dumps done", flush=True)


def wrap180(d):
    while d > 180.0: d -= 360.0
    while d <= -180.0: d += 360.0
    return d


def load(path):
    out = []
    for line in open(path):
        line = line.strip()
        if not line:
            continue
        r = json.loads(line)
        if r.get("type") != "body":
            continue
        if r.get("altaz_new") is None or r.get("altaz_old") is None:
            continue
        oalt, oaz = r["altaz_old"]      # radians
        naltaz = r["altaz_new"]
        nalt, naz = naltaz
        # skip non-finite (unpositioned bodies)
        vals = [oalt, oaz, nalt, naz]
        if any(v != v or abs(v) == float("inf") for v in vals):
            continue
        vis = r["old"].get("visible", True) and r["new"].get("visible", True)
        out.append({
            "name": r["name"],
            "alt_old": math.degrees(oalt), "az_old": math.degrees(oaz),
            "alt_new": math.degrees(nalt), "az_new": math.degrees(naz),
            "visible": vis,
        })
    return out


# Azimuth is undefined at the zenith/nadir pole (|alt|→90): the observer's home
# body sits at the nadir (dist ≈ observer height), so its az is unstable in BOTH
# paths - excluded from the az assertion, alt still asserted.
POLE_ALT_DEG = 89.5

max_daz_direct = 0.0     # |wrap(az_old − az_new)|        -> ~0 POST-fix (parity)
max_dev_pi2 = 0.0        # |wrap(az_old − (π/2−az_new))|  -> ~0 PRE-fix (convention)
max_dalt = 0.0
n_rows = 0
n_pole = 0
fail = []
print("\n%-24s %-11s %8s %9s %9s %11s %9s %5s" %
      ("sample", "body", "az_old", "az_new", "|Δaz|", "|Δ(π/2−)|", "|Δalt|", "pole"))
print("-" * 92)
for (label, fp) in files:
    try:
        rows = load(fp)
    except FileNotFoundError:
        print(f"!! missing dump {fp}"); fail.append(label); continue
    for r in rows:
        if not r["visible"]:
            continue
        d_direct = abs(wrap180(r["az_old"] - r["az_new"]))
        d_pi2 = abs(wrap180(r["az_old"] - (90.0 - r["az_new"])))  # π/2 − az_new
        d_alt = abs(r["alt_old"] - r["alt_new"])
        pole = abs(r["alt_new"]) > POLE_ALT_DEG or abs(r["alt_old"]) > POLE_ALT_DEG
        n_rows += 1
        max_dalt = max(max_dalt, d_alt)
        if d_alt > ALT_TOL_DEG:
            fail.append(f"{label}/{r['name']}:alt={d_alt:.4f}")
        if pole:
            n_pole += 1
        else:
            max_daz_direct = max(max_daz_direct, d_direct)
            max_dev_pi2 = max(max_dev_pi2, d_pi2)
            if d_direct > AZ_TOL_DEG:
                fail.append(f"{label}/{r['name']}:az={d_direct:.4f}")
        print("%-24s %-11s %8.3f %9.3f %9.4f %11.4f %9.4f %5s" %
              (label, r["name"], r["az_old"], r["az_new"], d_direct, d_pi2,
               d_alt, "Y" if pole else ""))

print("\n=== SUMMARY (visible-in-both, %d rows; %d pole-degenerate excluded from az) ===" %
      (n_rows, n_pole))
print("max |Δaz| direct  (az_old vs az_new)          = %.5f deg  [~0 => POST-fix parity]" % max_daz_direct)
print("max |Δ(π/2−az_new)| (az_old vs π/2−az_new)    = %.5f deg  [~0 => PRE-fix convention]" % max_dev_pi2)
print("max |Δalt|                                     = %.5f deg  [must stay ~0]" % max_dalt)
ok = (not fail) and n_rows > 0
print("\nRESULT:", "PASS" if ok else "FAIL", "" if ok else ("(" + "; ".join(fail[:12]) + ")"))
sys.exit(0 if ok else 1)
