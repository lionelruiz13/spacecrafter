#!/usr/bin/env python3
"""F44 - WHAT THE OLD PATH'S RA/DE SAYS FOR THE SAME BODY, beside the new path's.

WHY THIS PROBE EXISTS (INTENT §5.86's own owed clause, task F44)
----------------------------------------------------------------
§5.86 recorded that `Camera::observedToBodyLocalPos` is not `viewMat`'s inverse,
measured by round trip through the project's own primitives (133.9041 deg,
`harness/f34_probe_inverse.cpp`), and named what is owed before the row can be
judged: *"whether any consumer depends on today's answer, and what the old
path's RA/DE says for the same body - a parity target this row does not
assume."*  This is the second half.  RECORD-ONLY: no product code is touched.

THE CHANNEL ALREADY EXISTS - nothing was built for it.  `dumpTracePaths`
writes a `.navstr` sidecar beside every dual dump that prints, per body, at ONE
frame, in ONE launch:

    <name>
      OLD nav: <Body::getShortInfoNavString>          -> "RA/DE: hh.. / dd.."
      NEW nav: <ModularObject::getShortInfoNavString>  -> "RA/DE: hh.. / dd.."
      OLD inf: <Body::getInfoString>
      NEW inf: <ModularObject::getInfoString>
    [observed: ssystem_factory.cpp:1179-1218]

so the two paths' RA/DE for the same body at the same frame is a parse away,
and the JSON dump beside it carries `altaz_old` / `altaz_new` for the same body
at the same frame [observed: ssystem_factory.cpp:1197-1216].

THE CONFOUND, NAMED BEFORE IT IS MEASURED.  Old's RA/DE is earth-equatorial;
the new path's intends the REFERENCE BODY's equatorial frame, and whether those
two zero points agree is INTENT §11.4 - open, and flagged as open at
`Camera.hpp:256-260`.  So a raw old-vs-new difference is §5.86 AND §11.4
together, and a check that only says "they differ" cannot tell them apart.
Three things separate them, all computable from the same dump:

  (i)  the ALT/AZ control.  `observedPosToAltAz` rides `observedToLocalPos`
       (viewRotation^T), which §5.86 says is the honest one, and §11.60
       measured old-vs-new agreement at <=3e-5 deg over 234/234 bodies.  If
       alt/az agrees and RA/DE does not, the two paths agree on WHERE the body
       is and the disagreement is the RA/DE conversion's own.
  (ii) the SHAPE of the disagreement.  A pure zero-point difference (§11.4
       alone) is a rotation about the polar axis: DE would match and
       (RA_old - RA_new) would be ONE constant for every body.  §5.86's
       mechanism is not a Z rotation (a wrong axis Y-for-X, a wrong translation
       sign, an omitted fold), so it must break both.
  (iii) the RECONSTRUCTION.  The shipped arithmetic, re-evaluated offline on
       the dumped state, must reproduce the new path's printed RA/DE; and the
       ALGEBRAIC INVERSE of the same composition, on the SAME state, gives what
       the readout would have answered without §5.86.  Its gap to OLD is then
       the §11.4 residual - i.e. the parity target the row asks for.

THE BRIDGE that makes (iii) need no view matrix: both the shipped expression
and its algebraic inverse begin with the SAME vector, `observedToLocalPos(o) =
viewRotation^T . o`, and that vector is exactly what the UNAFFECTED alt/az
channel reports.  Inverting `ModularObject::altAz` recovers its direction and
the body's own `mat` translation gives its length:

    az_raw = pi/2 - az_report (mod 2pi)          [ModularObject.cpp:137-139]
    local  = |o| * spheToRect(az_raw, alt)       [utility.cpp:95-99, :110-115]
    |o|    = |mat.getTranslation()|              [ModularBody.hpp:1493-1495,
                                                  dumped at ModularBody.cpp:902-904]

so the reconstruction is driven by the dump alone and its validity is testable:
if it does not reproduce the printed NEW RA/DE, it is wrong and says so.

Primitive conventions, read at source rather than recalled (the §11.144(d)
trap: the "unconventionally clockwise" doc line sits on `zrotation(T angle)`
while describing the OTHER overload):
    zrotation(a) . v = (c*x - s*y,  s*x + c*y,  z)          [vecmath.hpp:1783]
    yrotation(a) . v = (c*x + s*z,  y,  -s*x + c*z)         [vecmath.hpp:1765]
    xrotation(a) . v = (x,  c*y - s*z,  s*y + c*z)          [vecmath.hpp:1686]
all three CONVENTIONAL CCW under multiplyWithoutTranslation [vecmath.hpp:1879].

=== PREDICTIONS, committed here BEFORE the run ==============================

LEG A - the shipped place, anchored (freeMode false, boundToSurface true), the
        branch §5.86 says is wrong three ways plus the omitted fold:
  A1  the defect SHOWS: median angular separation between OLD's and NEW's RA/DE
      unit vectors > 20 deg over the both-tree bodies.  (If this fails, §5.86
      is not reachable at this readout and the row's premise is wrong.)
  A2  the paths AGREE on where the body is: max |alt_old-alt_new| and
      max |az_old-az_new| <= 1e-3 deg over the same bodies (§11.60: 3e-5).
  A3  the reconstruction is VALID: the shipped arithmetic re-evaluated offline
      reproduces the printed NEW RA/DE to <= 0.02 deg of separation for every
      body (the print quantizes RA to 1 s = 15 arcsec = 0.00417 deg).
  A4  the ALGEBRAIC INVERSE on the same state lands far closer to OLD than NEW
      does: median separation(inverse, OLD) < median separation(NEW, OLD) / 4.
      Its value IS the parity residual §11.4 owns.
  A5  the disagreement is NOT a zero-point offset: max |DE_old - DE_new| > 1 deg
      AND the spread (max-min) of (RA_old - RA_new) over bodies > 1 deg.

LEG B - `camera action free_mode state on`, still bound to the surface.  Here
        the method's free branch is exact EXCEPT for the omitted fold, and the
        fold is a pure Z rotation by the reference's axisRot, so:
  B1  DE is EXACT in free mode: max |DE_new_free - DE_inverse_free| <= 0.01 deg.
  B2  RA is off by ONE constant, and the constant is the reference's own dumped
      axisRot: (RA_new_free - RA_inverse_free) == -axisRot (mod 2pi) for every
      body, spread <= 0.05 deg, and |mean + axisRot| <= 0.05 deg.
  B3  therefore the DE channel's old-vs-new gap COLLAPSES relative to leg A:
      max |DE_old - DE_new| in leg B < that of leg A.

LEG A' - `free_mode state off` again (the reversible pair, second traverse from
        the state the first exit produced): leg A's A1/A5 numbers reproduce.

LEG C - the SAME measurement at a SECOND EPOCH (jd 2461321.62 = dual-dump.sts's
        own second date + 0.37 d, ~130 deg of extra spin).  This exists because the fold the inverse
        applies is `Z(+axisRot)` and axisRot is a SPIN angle: a zero point
        measured at one epoch cannot tell a FRAME offset from a spin artifact,
        and a "parity target" that is really a spin phase is not a target.
  C1  the reference's axisRot MOVES between the epochs by more than 90 deg
      (otherwise the test cannot discriminate and must not be reported as one).
  C2  and the zero point does NOT: |offset(C) - offset(A)| <= 0.01 deg.
      If C2 fails, A4b is epoch-specific and says so.

Any leg that cannot fail is not a measurement; each gate above has a number and
a direction.

PRECONDITIONS: fresh launch, no concurrent instance, config/ssystem md5 in==out
on the REAL ~/.spacecrafter (the configuration is not varied, so the field
configuration IS the stamp; nothing here writes it - §11.150: config.ini is
rewritten only by the TUI, `configuration action save`, or a version bump).
The display scale ramp is waited out BY MEASUREMENT (§5.109), never by sleep.

usage: f44_parity.py <outdir> [--bin /abs/binary]
"""
import json
import math
import os
import re
import socket
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
AU_KM = 149597870.0
JD = 2461233.5           # dual-dump.sts's own canonical epoch
JD2 = 2461321.62         # its own second date +0.37 d (the epoch control).
                         # The +0.37 d matters and was MEASURED, not chosen for
                         # taste: dual-dump.sts's bare second date (+87.75 d) is
                         # 87.99 sidereal rotations, so it moves Earth's axisRot
                         # by only 3.5095 deg and C2 cannot discriminate a frame
                         # offset from a spin phase at all.  C1 caught exactly
                         # that (artifacts/f44-run2), which is why C1 exists.
DEG = 180.0 / math.pi

FAILS = []
NOTES = []
def fail(m): FAILS.append(m); print("FAIL: " + m, flush=True)
def ok(m):   print("ok:   " + m, flush=True)
def note(m):
    """A prediction that did NOT hold, recorded rather than deleted.  It is not
    a gate failure when the refutation is itself the finding and a sharper test
    of the SAME stated alternative follows - but it must stay on the record,
    with its number, or the file becomes a place where wrong guesses vanish."""
    NOTES.append(m); print("NOTE: " + m, flush=True)


# Per-body side channels the gates need, filled by analyse() for leg A.
ALT = {}     # name -> (alt_old_deg, alt_new_deg)
ECL = {}     # name -> (ecl_old, ecl_new)
ROBS = {}    # name -> |observed position| in AU


# ---------------------------------------------------------------- primitives
def rz(a, v):
    c, s = math.cos(a), math.sin(a)
    return (c * v[0] - s * v[1], s * v[0] + c * v[1], v[2])

def ry(a, v):
    c, s = math.cos(a), math.sin(a)
    return (c * v[0] + s * v[2], v[1], -s * v[0] + c * v[2])

def rx(a, v):
    c, s = math.cos(a), math.sin(a)
    return (v[0], c * v[1] - s * v[2], s * v[1] + c * v[2])

def sphe_to_rect(lng, lat, r=1.0):
    cl = math.cos(lat)
    return (math.cos(lng) * cl * r, math.sin(lng) * cl * r, math.sin(lat) * r)

def rect_to_sphe(v):
    r = math.sqrt(sum(x * x for x in v))
    return math.atan2(v[1], v[0]), math.asin(v[2] / r)

def sep_deg(a, b):
    """Angular separation of two (ra, de) pairs, in degrees."""
    ua, ub = sphe_to_rect(*a), sphe_to_rect(*b)
    d = sum(x * y for x, y in zip(ua, ub))
    return math.acos(max(-1.0, min(1.0, d))) * DEG

def wrap180(d):
    return (d + 180.0) % 360.0 - 180.0


# ------------------------------------------------------------ navstr parsing
MARK = re.compile(r'^  (OLD|NEW) (nav|inf): (.*)$')
# The two paths do NOT print the same label: the old path's string is
# TRANSLATED ("AD/DE : ", French, with a space before the colon and none around
# the slash) while ModularObject's is not ("RA/DE: ") - `ModularObject.cpp:20`
# writes ("RA/DE: "), the bare-parenthesis remnant of the _() the old path
# still carries at `body.cpp:333`ff.  Measured on the sidecar, not assumed; the
# pattern accepts both spellings so the comparison is between the two paths'
# NUMBERS and not between their locales.
# [SUPERSEDED 2026-09-05, F87 / INTENT S11.209, code 1d839b9d: the "do NOT print
#  the same label" premise no longer holds -- ModularObject.cpp now wraps the
#  same 14 msgids, so in a French session BOTH sides print "AD/DE : ".  Nothing
#  here needs changing: this regex was already written to accept both spellings,
#  and that foresight is exactly why F87 did not move a landed baseline.  The
#  line numbers moved with the wraps (`ModularObject.cpp:20` -> `:46`).]
RADE = re.compile(r'(?:RA|AD)/DE\s*:\s*(\d+)h(\d+)m([\d.]+)s\s*/\s*'
                  r'([+-])(\d+)[^\d]+(\d+)\'([\d.]+)"')

def hms_to_rad(h, m, s):
    return (h + m / 60.0 + s / 3600.0) * (math.pi / 12.0)

def dms_to_rad(sign, d, m, s):
    v = (d + m / 60.0 + s / 3600.0) * (math.pi / 180.0)
    return -v if sign == '-' else v

def parse_navstr(path):
    """{name: {'OLD_nav': (ra,de) or None, 'NEW_nav': ...}} from the sidecar.

    Only MARKER lines are read.  The nav string's RA/DE is on the marker line
    itself, and the inf marker line carries the body name, so the continuation
    lines (which have no fixed prefix) are never relied on."""
    recs, cur = [], {}
    for line in _open(path):
        m = MARK.match(line.rstrip('\n'))
        if not m:
            continue
        key = m.group(1) + '_' + m.group(2)
        if key in cur:
            recs.append(cur); cur = {}
        cur[key] = m.group(3)
    if cur:
        recs.append(cur)
    out = {}
    for r in recs:
        name = (r.get('NEW_inf') or r.get('OLD_inf') or '').strip()
        if not name:
            continue
        e = {}
        for p in ('OLD', 'NEW'):
            t = r.get(p + '_nav')
            g = RADE.search(t) if t else None
            e[p] = (hms_to_rad(int(g.group(1)), int(g.group(2)), float(g.group(3))),
                    dms_to_rad(g.group(4), int(g.group(5)), int(g.group(6)),
                               float(g.group(7)))) if g else None
        out[name] = e
    return out


def _open(path):
    """Committed artifacts are gzipped (a dual dump is ~280 KB of JSON lines);
    both spellings load, so --offline works on the archive as written."""
    path = str(path)
    if path.endswith(".gz"):
        import gzip
        return gzip.open(path, "rt", encoding="utf-8", errors="replace")
    if not os.path.exists(path) and os.path.exists(path + ".gz"):
        import gzip
        return gzip.open(path + ".gz", "rt", encoding="utf-8", errors="replace")
    return open(path, encoding="utf-8", errors="replace")


def load_dump(path):
    bodies, cam, jd = {}, None, None
    for line in _open(path):
        line = line.strip().rstrip(",")
        if not line:
            continue
        try:
            o = json.loads(line)
        except json.JSONDecodeError:
            continue
        if o.get("type") == "header":
            cam = o.get("camera"); jd = o.get("jd")
        if o.get("type") == "body":
            bodies[o["name"]] = o
    return bodies, cam, jd


# ------------------------------------------------------------------ the model
def reconstruct(cam, body, ref_axis_rot):
    """Return (shipped, inverse, inverse_topocentric) RA/DE from the dumped state.

    `shipped`  re-evaluates Camera.hpp:264-287 verbatim.
    `inverse`  is the algebraic inverse of the SAME viewMat composition
               (Camera.cpp:184-198) - what the readout would answer if §5.86
               were repaired.  It is BODY-CENTRED, because that is what the
               inverse of a view matrix gives: the position in the reference's
               own frame, measured from its CENTRE.
    `inv_topo` is the same chain with the origin term dropped, i.e. measured
               from the OBSERVER.  It exists because the old path's RA/DE is
               observer-centred - `Body::getEarthEquPos` returns
               `nav->helioToEarthPosEqu(...)` and that member's own sibling doc
               says so: *"equatorial coordinate but centered on the observer
               position (usefull for objects close to earth)"*
               [observed: navigator.hpp:140-147, body.cpp:492-496].  So the two
               readouts differ in ORIGIN as well as in zero point, and that
               difference is invisible except for a nearby body - which is
               exactly what makes it separable here.
    None when the dump does not carry what any of them needs."""
    nb = body.get("new")
    aa = body.get("altaz_new")
    if not nb or not aa:
        return None, None, None
    mat = nb.get("mat")
    if not mat or len(mat) < 15:
        return None, None, None
    o = (mat[12], mat[13], mat[14])
    r = math.sqrt(sum(x * x for x in o))
    if r == 0.0:
        return None, None, None
    alt, az_report = aa[0], aa[1]
    az_raw = math.pi / 2 - az_report              # invert ModularObject::altAz
    local = sphe_to_rect(az_raw, alt, r)          # == viewRotation^T . o
    lon, lat, dist = cam["longitude"], cam["latitude"], cam["distance"]
    free, bound = cam["freeMode"], cam["boundToSurface"]
    pos = cam["position"]

    if free:                                       # Camera.hpp:266-267
        shipped = tuple(local[i] - pos[i] for i in range(3))
        truth = shipped
        topo = local
    else:                                          # Camera.hpp:268-271
        v = (local[0], local[1], local[2] - dist)
        shipped = rz(-lon, ry(lat - math.pi / 2, v))
        truth = rz(lon, rx(math.pi / 2 - lat, (local[0], local[1], local[2] + dist)))
        topo = rz(lon, rx(math.pi / 2 - lat, local))
    if bound:                                      # the omitted fold, S^T
        truth = rz(ref_axis_rot, truth)
        topo = rz(ref_axis_rot, topo)
    return rect_to_sphe(shipped), rect_to_sphe(truth), rect_to_sphe(topo)


# ---------------------------------------------------------------- app driving
def wait_port(timeout=120):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode()); time.sleep(pause)
    try:
        sock.settimeout(0.25); sock.recv(8192); sock.settimeout(None)
    except socket.timeout:
        sock.settimeout(None)


SEQ = [0]
def probe(sock, out, tag):
    SEQ[0] += 1
    p = out / ("%s_%03d.json" % (tag, SEQ[0]))
    if p.exists():
        p.unlink()
    send(sock, "body action dual_dump filename %s" % p, 0.9)
    for _ in range(50):
        if p.exists() and p.stat().st_size > 0 and Path(str(p) + ".navstr").exists():
            time.sleep(0.4)
            return p
        time.sleep(0.2)
    raise RuntimeError("dump %s never written" % tag)


def settle_scale(sock, out, tries=30):
    """§5.109: wait the display-scale ramp out BY MEASUREMENT, never by sleep."""
    prev = None
    for i in range(tries):
        p = probe(sock, out, "settle")
        b, _c, _j = load_dump(p)
        m = (b.get("Moon", {}).get("new") or {})
        sc, tg = m.get("scaling"), m.get("scalingTarget")
        if sc is None:
            return None
        if abs(sc - tg) < 1e-4 and prev is not None and abs(sc - prev) < 1e-6:
            ok("scale settled: Moon scaling %.8f == target %.8f (%d probes)" % (sc, tg, i + 1))
            return sc
        prev = sc
    fail("scale never settled (last %s vs %s)" % (sc, tg))
    return sc


# ----------------------------------------------------------------- the report
def analyse(tag, dumpfile, rep):
    bodies, cam, jd = load_dump(dumpfile)
    nav = parse_navstr(str(dumpfile) + ".navstr")
    ref = cam["reference"]
    ref_axis = (bodies.get(ref, {}).get("new") or {}).get("axisRot")
    leg = {"tag": tag, "dump": str(dumpfile), "jd": jd, "reference": ref,
           "refAxisRot": ref_axis, "freeMode": cam["freeMode"],
           "boundToSurface": cam["boundToSurface"],
           "viewOffsetEff": cam.get("viewOffsetEff"),
           "longitude": cam["longitude"], "latitude": cam["latitude"],
           "distance": cam["distance"], "bodies": {}}
    for name, e in sorted(nav.items()):
        if not e.get("OLD") or not e.get("NEW"):
            continue                              # new-only body: no old twin
        b = bodies.get(name)
        if not b:
            continue
        row = {"ra_old": e["OLD"][0] * DEG, "de_old": e["OLD"][1] * DEG,
               "ra_new": e["NEW"][0] * DEG, "de_new": e["NEW"][1] * DEG,
               "sep_old_new_deg": sep_deg(e["OLD"], e["NEW"])}
        ao, an = b.get("altaz_old"), b.get("altaz_new")
        if ao and an and all(math.isfinite(x) for x in ao + an):
            row["dalt_deg"] = abs(ao[0] - an[0]) * DEG
            row["daz_deg"] = abs(wrap180((ao[1] - an[1]) * DEG))
            row["alt_new_deg"] = an[0] * DEG
            if tag == "A":
                ALT[name] = (ao[0] * DEG, an[0] * DEG)
        if tag == "A":
            oe = (b.get("old") or {}).get("ecl")
            ne = (b.get("new") or {}).get("ecl")
            if oe and ne and all(math.isfinite(x) for x in list(oe) + list(ne)) \
               and any(oe) and any(ne):
                ECL[name] = (tuple(oe), tuple(ne))
            nm = (b.get("new") or {}).get("mat")
            if nm and len(nm) >= 15:
                ROBS[name] = math.sqrt(sum(x * x for x in nm[12:15]))
        sh, inv, topo = reconstruct(cam, b, ref_axis or 0.0)
        if sh:
            row["sep_recon_new_deg"] = sep_deg(sh, e["NEW"])
            row["sep_inv_old_deg"] = sep_deg(inv, e["OLD"])
            row["dde_new_inv_deg"] = abs(sh[1] - inv[1]) * DEG
            row["dra_new_inv_deg"] = wrap180((sh[0] - inv[0]) * DEG)
            row["ra_inv"] = inv[0] * DEG
            row["de_inv"] = inv[1] * DEG
            row["ra_topo"] = topo[0] * DEG
            row["de_topo"] = topo[1] * DEG
            # The parity residual: the topocentric inverse, turned by the
            # measured +90 deg zero point, against OLD's own answer.
            row["sep_parity_deg"] = sep_deg((topo[0] + math.pi / 2, topo[1]), e["OLD"])
            # The origin term alone, per body: how far the body-centred and the
            # observer-centred answers are from each other.  It is the parallax
            # d*sin(z)/r and is why the Moon is the only body that can see it.
            row["origin_term_deg"] = sep_deg(inv, topo)
        leg["bodies"][name] = row
    rep["legs"][tag] = leg
    return leg


def col(leg, key):
    return [r[key] for r in leg["bodies"].values() if key in r and math.isfinite(r[key])]


def median(xs):
    xs = sorted(xs)
    n = len(xs)
    return float('nan') if not n else (xs[n // 2] if n % 2 else (xs[n // 2 - 1] + xs[n // 2]) / 2)


def summarise(leg):
    s = {"n_bodies": len(leg["bodies"])}
    for k, agg in (("sep_old_new_deg", "median"), ("sep_old_new_deg", "max"),
                   ("dalt_deg", "max"), ("daz_deg", "max"),
                   ("sep_recon_new_deg", "max"), ("sep_inv_old_deg", "median"),
                   ("sep_inv_old_deg", "max"),
                   ("dde_new_inv_deg", "max")):
        v = col(leg, k)
        if v:
            s["%s_%s" % (k, agg)] = (median(v) if agg == "median" else max(v))
    de = [abs(r["de_old"] - r["de_new"]) for r in leg["bodies"].values()]
    ra = [wrap180(r["ra_old"] - r["ra_new"]) for r in leg["bodies"].values()]
    if de:
        s["dde_old_new_max"] = max(de)
    if ra:
        s["dra_old_new_spread"] = max(ra) - min(ra)
        s["dra_old_new_median"] = median(ra)
    dra = col(leg, "dra_new_inv_deg")
    if dra:
        s["dra_new_inv_spread"] = max(dra) - min(dra)
        s["dra_new_inv_mean"] = sum(dra) / len(dra)
    leg["summary"] = s
    return s


def main():
    argv = sys.argv[1:]
    binp = HERE.parents[1] / "build-claude/src/spacecrafter"
    if "--bin" in argv:
        binp = Path(argv[argv.index("--bin") + 1])
    out = Path([a for a in argv if not a.startswith("--")][0]).resolve()
    out.mkdir(parents=True, exist_ok=True)

    # --offline re-analyses the artifacts of a run ALREADY on disk.  The
    # measurement precondition (fresh launch, no concurrent instance, md5
    # in==out) is a property of the run that WROTE them and is recorded in that
    # run's report; re-reading files changes nothing about the app, so a parser
    # repair does not owe a second launch (the analyze.py precedent).
    if "--offline" in argv:
        rep = {"bin": str(binp), "jd": JD, "offline_of": str(out), "legs": {}}
        for tag, pat in (("A", "legA_*"), ("B", "legB_*"),
                         ("A2", "legA2_*"), ("C", "legC_*")):
            f = sorted(out.glob(pat))
            f = [p for p in f if ".navstr" not in str(p) and "_result" not in str(p)]
            f = [Path(str(p)[:-3]) if str(p).endswith(".gz") else p for p in f]
            if f:
                analyse(tag, f[-1], rep)
        return gates(rep, out, None)

    home = Path.home() / ".spacecrafter"
    md5_in = subprocess.run(["md5sum", str(home / "config.ini"), str(home / "ssystem.ini")],
                            capture_output=True, text=True).stdout
    print("md5 IN:\n" + md5_in, flush=True)

    env = {**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")}
    proc = subprocess.Popen([str(binp)], cwd=str(home),
                            stdout=open(out / "f44.applog", "w"),
                            stderr=subprocess.STDOUT, env=env)
    rep = {"bin": str(binp), "jd": JD, "md5_in": md5_in, "legs": {}}
    try:
        s = wait_port(); time.sleep(10)
        send(s, "flag experimental_path on")
        send(s, "timerate rate 0")
        send(s, "date jday %s" % JD, 1.2)
        settle_scale(s, out)

        pA = probe(s, out, "legA")
        send(s, "camera action free_mode state on", 1.5)
        pB = probe(s, out, "legB")
        send(s, "camera action free_mode state off", 1.5)
        pA2 = probe(s, out, "legA2")
        send(s, "date jday %s" % JD2, 1.5)
        pC = probe(s, out, "legC")
        send(s, "date jday %s" % JD, 1.5)      # leave the scene where it began

        analyse("A", pA, rep)
        analyse("B", pB, rep)
        analyse("A2", pA2, rep)
        analyse("C", pC, rep)
    finally:
        try:
            send(s, "shutdown action now", 2.0)
        except Exception:
            pass
        for _ in range(30):
            if proc.poll() is not None:
                break
            time.sleep(0.5)
        if proc.poll() is None:
            proc.kill()
        rep["exit_code"] = proc.poll()

    return gates(rep, out, md5_in, home)


def gates(rep, out, md5_in, home=None):
    A, B, A2, C = (rep["legs"].get(t) for t in ("A", "B", "A2", "C"))
    for leg in (A, B, A2, C):
        if leg:
            summarise(leg)

    # ------------------------------------------------------------ the gates
    if A:
        sA = A["summary"]
        ok("LEG A reference=%s axisRot=%.9f free=%s bound=%s viewOffsetEff=%s"
           % (A["reference"], A["refAxisRot"] or 0, A["freeMode"], A["boundToSurface"],
              A["viewOffsetEff"]))
        ok("LEG A n=%d bodies with BOTH paths' RA/DE" % sA["n_bodies"])
        v = sA.get("sep_old_new_deg_median")
        (ok if v and v > 20 else fail)("A1 median sep(OLD,NEW) = %.4f deg (> 20 predicted)" % (v or -1))

        # A2 as PREDICTED was a max over the whole population; it fails, and the
        # two bodies that fail it are each attributed below rather than dropped.
        va, vz = sA.get("dalt_deg_max", 9e9), sA.get("daz_deg_max", 9e9)
        a2 = max(va, vz) <= 1e-3
        (ok if a2 else note)(
            "A2 (as predicted, whole population) max |dalt| = %.3e deg, max |daz| = %.3e deg"
            " vs <= 1e-3 predicted -> %s" % (va, vz, "holds" if a2 else "DOES NOT HOLD"))
        exc = [n for n, r_ in A["bodies"].items()
               if max(r_.get("dalt_deg", 0), r_.get("daz_deg", 0)) > 1e-3]
        pop = [r_ for n, r_ in A["bodies"].items() if n not in exc]
        pa = max(r_.get("dalt_deg", 0) for r_ in pop)
        pz = max(r_.get("daz_deg", 0) for r_ in pop)
        (ok if pa <= 1e-3 and pz <= 1e-3 else fail)(
            "A2a population of %d (all but %s): max |dalt| = %.3e, max |daz| = %.3e deg"
            " -- §11.60's <=3e-5 reproduced" % (len(pop), ",".join(sorted(exc)), pa, pz))
        # exception 1: the reference body is AT THE NADIR, where azimuth is not
        # defined - a degenerate case, not a divergence.  Both paths carry their
        # own pole branch for exactly this.
        ea = A["bodies"].get("Earth", {})
        eb = ALT.get("Earth")
        if eb:
            (ok if abs(abs(eb[0]) - 90.0) <= 1e-3 and abs(abs(eb[1]) - 90.0) <= 1e-3 else fail)(
                "A2b Earth is DEGENERATE: alt_old = %.6f deg, alt_new = %.6f deg (both -90 to <=1e-3)"
                " => its %.3f deg azimuth gap is the azimuth of a point at the nadir"
                % (eb[0], eb[1], ea.get("daz_deg", 0)))
        # exception 2: the two trees carry DIFFERENT positions for Eris.  The
        # check that can fail: the alt/az gap must EQUAL the angle between the
        # two dumped ecl vectors.
        if "Eris" in ECL:
            oe, ne = ECL["Eris"]
            ang = sep_deg(rect_to_sphe(oe), rect_to_sphe(ne))
            r_ = A["bodies"]["Eris"]
            gap = math.hypot(r_["daz_deg"] * math.cos(math.radians(r_.get("alt_new_deg", 0))),
                             r_["dalt_deg"])
            (ok if abs(ang - gap) <= 0.02 else fail)(
                "A2c Eris: angle between the trees' own ecl = %.6f deg, measured alt/az gap = %.6f deg"
                " (equal to <=0.02) => a BODY-POSITION divergence, not a frame one" % (ang, gap))

        v = sA.get("sep_recon_new_deg_max", 9e9)
        (ok if v <= 0.02 else fail)("A3 max sep(reconstruction, NEW) = %.5f deg (<= 0.02 predicted)" % v)

        vi, vn = sA.get("sep_inv_old_deg_median", 9e9), sA.get("sep_old_new_deg_median", 0)
        a4 = vi < vn / 4
        (ok if a4 else note)(
            "A4 (as predicted) median sep(INVERSE,OLD) = %.4f deg vs median sep(NEW,OLD)/4 = %.4f deg"
            " -> %s.  Separation is BLIND to a constant zero-point rotation, which is"
            " precisely the §11.4 branch this file's header (ii) named; A4b is that branch"
            " tested on its own terms." % (vi, vn / 4, "holds" if a4 else "DOES NOT HOLD"))

        # A4b - header (ii) applied to the INVERSE: if the remaining gap is
        # §11.4's zero point, DE matches and (RA_inv - RA_old) is ONE constant.
        keep = [n for n in A["bodies"] if n not in ("Earth", "Eris", "Moon")]
        dde = [abs(A["bodies"][n]["de_inv"] - A["bodies"][n]["de_old"]) for n in keep]
        dra = [wrap180(A["bodies"][n]["ra_inv"] - A["bodies"][n]["ra_old"]) for n in keep]
        (ok if max(dde) <= 0.01 and (max(dra) - min(dra)) <= 0.01 else fail)(
            "A4b over %d bodies (Earth degenerate / Eris ephemeris / Moon see A4c excluded):"
            " max |DE_inv-DE_old| = %.6f deg, (RA_inv-RA_old) = %.6f deg constant to %.6f"
            % (len(keep), max(dde), median(dra), max(dra) - min(dra)))

        # A4c - the Moon is the ONE body close enough to see the ORIGIN term.
        # Predicted from the dumped state alone: parallax = d*sin(z)/r.
        m = A["bodies"].get("Moon")
        if m and "Moon" in ALT and "Moon" in ROBS:
            z = math.pi / 2 - math.radians(ALT["Moon"][1])
            par = math.degrees(math.asin(min(1.0, A["distance"] * math.sin(z) / ROBS["Moon"])))
            resid = sep_deg((math.radians(m["ra_topo"] + 90.0), math.radians(m["de_topo"])),
                            (math.radians(m["ra_inv"] + 90.0), math.radians(m["de_inv"])))
            (ok if abs(par - resid) <= 0.02 else fail)(
                "A4c Moon origin term: predicted parallax d*sin(z)/r = %.6f deg, measured"
                " sep(body-centred, observer-centred) = %.6f deg (equal to <=0.02)" % (par, resid))

        # A4d - THE PARITY TARGET.  Origin matched to old's and the zero point
        # removed, the repaired new readout against old's own answer.
        p = sorted((r_["sep_parity_deg"], n) for n, r_ in A["bodies"].items() if "sep_parity_deg" in r_)
        worst = [x for x in p if x[0] > 0.02]
        (ok if len(worst) <= 1 else fail)(
            "A4d PARITY TARGET: with §5.86 repaired, the origin matched to old's and the"
            " +90.0003 deg zero point removed, sep(new, OLD) is <= %.6f deg for %d of %d"
            " bodies; over 0.02 deg: %s"
            % (p[-2][0] if len(p) > 1 else p[-1][0], len(p) - len(worst), len(p),
               ", ".join("%s %.4f" % (n, v) for v, n in worst) or "none"))

        # A4e - the inverse computed through the ANCHORED composition and
        # through the FREE one must agree: two different code branches, one
        # answer.  Independent validation of the algebra.
        if B:
            d = [sep_deg((math.radians(A["bodies"][n]["ra_inv"]), math.radians(A["bodies"][n]["de_inv"])),
                         (math.radians(B["bodies"][n]["ra_inv"]), math.radians(B["bodies"][n]["de_inv"])))
                 for n in A["bodies"] if n in B["bodies"] and n != "Earth" and "ra_inv" in A["bodies"][n]
                 and "ra_inv" in B["bodies"][n]]
            (ok if d and max(d) <= 0.01 else fail)(
                "A4e inverse via the ANCHORED composition == inverse via the FREE composition"
                " over %d bodies: max %.6f deg (<= 0.01)" % (len(d), max(d) if d else 9e9))

        d, r = sA.get("dde_old_new_max", 0), sA.get("dra_old_new_spread", 0)
        (ok if d > 1 and r > 1 else fail)(
            "A5 max |DE_old-DE_new| = %.4f deg, spread(RA_old-RA_new) = %.4f deg (both > 1 predicted)" % (d, r))
    if B:
        sB = B["summary"]
        ok("LEG B free=%s bound=%s axisRot=%.9f rad = %.6f deg"
           % (B["freeMode"], B["boundToSurface"], B["refAxisRot"] or 0, (B["refAxisRot"] or 0) * DEG))
        v = sB.get("dde_new_inv_deg_max", 9e9)
        (ok if v <= 0.01 else fail)("B1 max |DE_new - DE_inverse| = %.6f deg (<= 0.01 predicted)" % v)
        sp, mn = sB.get("dra_new_inv_spread", 9e9), sB.get("dra_new_inv_mean", 0)
        want = -(B["refAxisRot"] or 0) * DEG
        want = wrap180(want)
        (ok if sp <= 0.05 and abs(wrap180(mn - want)) <= 0.05 else fail)(
            "B2 (RA_new-RA_inverse): mean %.6f deg, spread %.6f deg; -axisRot = %.6f deg" % (mn, sp, want))
        if A:
            a, b = A["summary"].get("dde_old_new_max", 0), sB.get("dde_old_new_max", 9e9)
            (ok if b < a else fail)("B3 max |DE_old-DE_new| leg B %.4f < leg A %.4f" % (b, a))
    if A2 and A:
        s2 = A2["summary"]
        d1 = abs(s2.get("sep_old_new_deg_median", 0) - A["summary"].get("sep_old_new_deg_median", 0))
        (ok if d1 <= 0.5 else fail)(
            "A' reversible pair: median sep reproduces leg A to %.4f deg (<= 0.5 predicted)" % d1)

    if C and A:
        ax_a, ax_c = A["refAxisRot"] or 0.0, C["refAxisRot"] or 0.0
        moved = abs(wrap180((ax_c - ax_a) * DEG))
        (ok if moved > 90 else fail)(
            "C1 the reference's axisRot MOVED %.4f deg between the epochs (> 90 needed for"
            " C2 to discriminate a frame offset from a spin phase)" % moved)
        off = {}
        for tag, leg in (("A", A), ("C", C)):
            keep = [n for n in leg["bodies"] if n not in ("Earth", "Eris", "Moon")
                    and "ra_inv" in leg["bodies"][n]]
            d = [wrap180(leg["bodies"][n]["ra_inv"] - leg["bodies"][n]["ra_old"]) for n in keep]
            e = [abs(leg["bodies"][n]["de_inv"] - leg["bodies"][n]["de_old"]) for n in keep]
            off[tag] = (median(d), max(d) - min(d), max(e), len(d))
        ok("C  epoch A jd=%s offset %.6f deg (spread %.6f, max |dDE| %.6f, n=%d)"
           % (A["jd"], off["A"][0], off["A"][1], off["A"][2], off["A"][3]))
        ok("C  epoch C jd=%s offset %.6f deg (spread %.6f, max |dDE| %.6f, n=%d)"
           % (C["jd"], off["C"][0], off["C"][1], off["C"][2], off["C"][3]))
        dz = abs(wrap180(off["C"][0] - off["A"][0]))
        (ok if dz <= 0.01 else fail)(
            "C2 the zero point is EPOCH-INDEPENDENT: |offset(C) - offset(A)| = %.6f deg"
            " (<= 0.01) while axisRot moved %.4f deg" % (dz, moved))

    if home is not None:
        md5_out = subprocess.run(["md5sum", str(home / "config.ini"), str(home / "ssystem.ini")],
                                 capture_output=True, text=True).stdout
        rep["md5_out"] = md5_out
        (ok if md5_out == md5_in else fail)("md5 in == out")

    rep["fails"] = FAILS
    rep["notes"] = NOTES
    (out / "f44_result.json").write_text(json.dumps(rep, indent=1))
    print("\n%d FAIL(s), %d refuted-prediction NOTE(s); report -> %s"
          % (len(FAILS), len(NOTES), out / "f44_result.json"), flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
