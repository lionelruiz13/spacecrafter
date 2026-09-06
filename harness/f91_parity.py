#!/usr/bin/env python3
"""F91 -- the new path's RA/DE readout against the old path's, on the SAME frame,
through the channel F44 found (INTENT §11.213; rows §5.86 + §5.19).

WHAT THIS MEASURES, AND WHY IT IS ONE INSTRUMENT READ TWICE.  Every claim is
scored against BOTH expressions of `Camera::observedToBodyLocalPos` --

    H_pre   the shipped one (Camera.hpp:264-274 at 0b46a63f): Y(lat-pi/2),
            Z(-lon), `-= distance`, no surface fold          -> §5.86's defect
    H_post  the exact inverse of viewMat, observer-centred:  Z(+lon),
            X(pi/2-lat), `+= distance` dropped for the topocentric origin,
            and the FULL fold Z(axisRotation + pi/2)         -> F91's fix

-- and `--expect pre|post` chooses which one the GATES ask for.  The printed
numbers are identical either way, so a pre-fix binary and a post-fix binary are
one measurement read twice rather than two harnesses (the F40 shape,
harness/README.md).  Gate G3 requires the named hypothesis to reproduce the
binary's own printed RA/DE AND the other one to fail to: a model that cannot be
wrong about which binary it is looking at is not evidence.

THE CHANNEL was found, not built (§11.158(e)): `dumpTracePaths` already writes a
`.navstr` sidecar carrying, per body, at ONE frame, in ONE launch, the OLD path's
`Body::getShortInfoNavString` and the NEW path's `ModularObject::getShortInfoNavString`
[observed: ssystem_factory.cpp:1179-1218].  Nothing is written into `src/`.

TWO PARSING FACTS THIS FILE OWES ITS SUCCESSOR.
  1. The NEW path's nav string contains a `std::endl` after its RA/DE line
     [observed: ModularObject.cpp:74], so `SA/GHA/LHA` land on a CONTINUATION
     line with no marker prefix.  `f44_parity.parse_navstr` reads marker lines
     only and therefore cannot see the nav fields at all; this file accumulates
     the continuation lines into the field's value.
  2. The angle tokens are read POSITIONALLY, not by label.  `printAngleDMS`
     writes `<sign><ddd>°<mm>'<ss>"` [observed: utility.cpp:254-308], and the
     nav string's DMS tokens are, in order: DE, SA, GHA, LHA, Az, Alt, coAlt,
     LPA -- on BOTH paths.  Reading them by label would make the instrument
     depend on the translation catalogue, which F87 has just moved (§11.209).

PRECONDITIONS: a PRIVATE farm (nothing writes the field), fresh launch, no
concurrent instance (/proc/<pid>/comm, §11.134(b)), config/ssystem md5 in == out
on the REAL ~/.spacecrafter, the display-scale ramp waited out BY MEASUREMENT
(§5.109) and never by sleep.  The wrapper `f91_run.sh` owns the canary and the
frozen-file assert.

usage: f91_parity.py <absOutdir> [--bin PATH] [--expect pre|post]
                                 [--locale fr|en] [--offline]
"""
import json
import math
import os
import re
import shutil
import socket
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import f44_parity as F44                      # noqa: E402  (parsing primitives)

DEG = 180.0 / math.pi
PORT = 7805
JD = "2461233.5"          # F44's own frame, so the two runs are comparable
JD_PRE2000 = "2440000.0"  # a date BEFORE J2000, where the mean-sidereal
                          # polynomial goes NEGATIVE: old normalises its hour
                          # angles into [0,360) with `while` loops
                          # [body.cpp:416-421], the new path takes a bare
                          # `fmod` [ModularObject.cpp:82-83].  A frame at a
                          # post-2000 date cannot discriminate the two.
PARITY_TARGET = 0.002014  # §11.158(f), the number that entry measured
# F96: overridable so a later task can keep its runs inside its own scratch
# root (the default is unchanged, so F91's own invocation is untouched).
FARM_ROOT = Path(os.environ.get("F91_FARM_ROOT", "/home/claude/sc-f91"))
REAL_HOME = Path.home() / ".spacecrafter"

FAILS, NOTES = [], []
def fail(m): FAILS.append(m); print("FAIL: " + m, flush=True)
def ok(m):   print("ok:   " + m, flush=True)
def note(m): NOTES.append(m); print("NOTE: " + m, flush=True)


# --------------------------------------------------------------- nav parsing
MARK = re.compile(r'^  (OLD|NEW) (nav|inf): (.*)$')
DMS_TOK = re.compile(r'([+-])(\d+)[^\d\'"]{1,4}(\d{2})\'(\d{2})"')
HMS_TOK = re.compile(r'(\d+)h(\d+)m([\d.]+)s')


def dms_deg(sign, d, m, s):
    v = int(d) + int(m) / 60.0 + int(s) / 3600.0
    return -v if sign == '-' else v


def parse_navstr_full(path):
    """{name: {'OLD_nav': <full multi-line str>, 'NEW_nav': ..., ...}}.

    Unlike f44_parity.parse_navstr this KEEPS the continuation lines, because
    the new path's nav string is two lines and its SA/GHA/LHA are on the
    second one."""
    recs, cur, key = [], {}, None
    for line in F44._open(path):
        line = line.rstrip('\n')
        m = MARK.match(line)
        if m:
            key = m.group(1) + '_' + m.group(2)
            if key in cur:
                recs.append(cur); cur = {}
            cur[key] = m.group(3)
        elif key is not None and key in cur:
            cur[key] += '\n' + line
    if cur:
        recs.append(cur)
    out = {}
    for r in recs:
        # the inf string's FIRST line is the body's english name
        raw = (r.get('NEW_inf') or r.get('OLD_inf') or '')
        name = raw.split('\n')[0].strip()
        if name:
            out[name] = r
    return out


def nav_fields(s):
    """(ra_deg, [DE, SA, GHA, LHA, Az, Alt, coAlt, LPA] in degrees) or None."""
    if not s:
        return None
    h = HMS_TOK.search(s)
    toks = [dms_deg(*t) for t in DMS_TOK.findall(s)]
    if not h or len(toks) < 8:
        return None
    ra = (int(h.group(1)) + int(h.group(2)) / 60.0 + float(h.group(3)) / 3600.0) * 15.0
    return ra, toks[:8]


def sep_deg_d(ra1, de1, ra2, de2):
    """Angular separation of two (ra, de) pairs given in DEGREES."""
    return F44.sep_deg((math.radians(ra1), math.radians(de1)),
                       (math.radians(ra2), math.radians(de2)))


# ------------------------------------------------------------ the two models
def reconstruct(cam, body, ref_axis_rot):
    """(pre, post) RA/DE in DEGREES, rebuilt offline from the dumped state.

    Both start from `observedToLocalPos(o) = viewRotation^T . o`, which is
    exactly what the UNAFFECTED alt/az channel reports -- so inverting
    `ModularObject::altAz` (`az_raw = pi/2 - az_report`, ModularObject.cpp:163)
    and taking the length from the body's own dumped `mat` translation
    (ModularBody.hpp:1493-1495) rebuilds the input of both expressions from the
    dump alone, with no view matrix (§11.158(f1)'s method, kept).

    That bridge is valid while `observedPosToAltAz` stays the honest one AND the
    B17 view offset is inert -- `getObservedPosition()` carries the offset
    rotation R' [observed: Camera.cpp:138] while `observedToLocalPos` divides by
    the offset-FREE `viewRotation()`.  The caller asserts viewOffsetEff == 0."""
    nb, aa = body.get("new"), body.get("altaz_new")
    if not nb or not aa:
        return None, None
    mat = nb.get("mat")
    if not mat or len(mat) < 15:
        return None, None
    o = (mat[12], mat[13], mat[14])
    r = math.sqrt(sum(x * x for x in o))
    if r == 0.0:
        return None, None
    az_raw = math.pi / 2 - aa[1]
    local = F44.sphe_to_rect(az_raw, aa[0], r)     # == viewRotation^T . o
    lon, lat, dist = cam["longitude"], cam["latitude"], cam["distance"]
    free, bound = cam["freeMode"], cam["boundToSurface"]
    pos = cam["position"]

    if free:                                        # Camera.hpp:266-267
        pre = tuple(local[i] - pos[i] for i in range(3))
        post = local                                # topocentric: no origin term
    else:                                           # Camera.hpp:268-271
        v = (local[0], local[1], local[2] - dist)
        pre = F44.rz(-lon, F44.ry(lat - math.pi / 2, v))
        post = F44.rz(lon, F44.rx(math.pi / 2 - lat, local))
    if bound:
        # the fold the shipped expression omits ENTIRELY (§5.86), restored in
        # FULL: computeSurfaceToBody() is Z(-getAxisRotation()) and
        # getAxisRotation() is `axisRotation + M_PI_2`
        # [observed: ModularBody.hpp:575-585].  The dump emits the RAW
        # axisRotation [ModularBody.cpp:926], so the +pi/2 is added here --
        # this is the term §11.158(f2) measured as a "-90.0003 deg zero point".
        post = F44.rz(ref_axis_rot + math.pi / 2, post)
    out = []
    for v in (pre, post):
        ra, de = F44.rect_to_sphe(v)
        out.append(((ra * DEG) % 360.0, de * DEG))
    return out[0], out[1]


# ------------------------------------------------------------------ the farm
def no_instance():
    hits = []
    for p in Path("/proc").glob("[0-9]*/comm"):
        try:
            if p.read_text().strip() == "spacecrafter":
                hits.append(str(p.parent))
        except OSError:
            pass
    return hits


def build_farm(farm, locale):
    farm = Path(farm)
    if farm.exists():
        shutil.rmtree(farm)
    r = subprocess.run([str(HERE / "b3_farm.sh"), str(farm)],
                       capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError("b3_farm.sh failed: %s%s" % (r.stdout, r.stderr))
    dst = farm / ".spacecrafter"
    cfg = dst / "config.ini"
    shape = {"config_is_regular": cfg.is_file() and not cfg.is_symlink(),
             "ssystem_is_regular": (dst / "ssystem.ini").is_file()
                                   and not (dst / "ssystem.ini").is_symlink(),
             "log_is_real_dir": (dst / "log").is_dir() and not (dst / "log").is_symlink()}
    if locale == "en":
        # §5.136: `app_locale` ALONE is inert -- the readout's labels follow
        # `sky_locale`.  Both keys move together or the control measures nothing.
        txt = cfg.read_text(encoding="latin-1")
        txt = re.sub(r'^(app_locale\s*=\s*)\S+', r'\1en', txt, flags=re.M)
        txt = re.sub(r'^(sky_locale\s*=\s*)\S+', r'\1en', txt, flags=re.M)
        cfg.write_text(txt, encoding="latin-1")
    got = dict(re.findall(r'^(app_locale|sky_locale)\s*=\s*(\S+)',
                          cfg.read_text(encoding="latin-1"), flags=re.M))
    shape["app_locale"], shape["sky_locale"] = got.get("app_locale"), got.get("sky_locale")
    for k, v in shape.items():
        if v is False:
            raise RuntimeError("farm shape assert failed: %s" % k)
    return shape


# ---------------------------------------------------------------- app driving
class App:
    def __init__(self, binary, farm, out):
        self.binary, self.farm, self.out = str(binary), Path(farm), Path(out)
        self.home = self.farm / ".spacecrafter"
        self.sock = self.proc = None
        self.n = 0

    def start(self):
        hits = no_instance()
        if hits:
            raise RuntimeError("another spacecrafter is running: %s" % hits)
        env = {**os.environ, "HOME": str(self.farm),
               "DISPLAY": os.environ.get("DISPLAY", ":2")}
        self.proc = subprocess.Popen([self.binary], cwd=str(self.home),
                                     stdout=open(self.out / "f91.applog", "w"),
                                     stderr=subprocess.STDOUT, env=env)
        t0 = time.time()
        while time.time() - t0 < 180:
            try:
                self.sock = socket.create_connection(("127.0.0.1", PORT), timeout=2)
                break
            except OSError:
                if self.proc.poll() is not None:
                    raise RuntimeError("the app exited before opening %d" % PORT)
                time.sleep(1)
        if self.sock is None:
            raise RuntimeError("port %d never opened" % PORT)
        return time.time() - t0

    def send(self, cmd, pause=0.6):
        self.sock.sendall((cmd + "\n").encode())
        time.sleep(pause)
        try:
            self.sock.settimeout(0.25); self.sock.recv(65536)
        except socket.timeout:
            pass
        finally:
            self.sock.settimeout(None)

    def dump(self, tag):
        self.n += 1
        p = self.out / ("%s_%03d.json" % (tag, self.n))
        if p.exists():
            p.unlink()
        self.send("body action dual_dump filename %s" % p, 0.9)
        for _ in range(60):
            if p.exists() and p.stat().st_size > 0 and Path(str(p) + ".navstr").exists():
                time.sleep(0.4)
                return p
            time.sleep(0.2)
        raise RuntimeError("dump %s never written" % tag)

    def settle_scale(self, tries=30):
        """§5.109: wait the Moon's display-scale ramp out BY MEASUREMENT."""
        prev = None
        for i in range(tries):
            b, _c, _j = F44.load_dump(self.dump("settle"))
            m = (b.get("Moon", {}).get("new") or {})
            sc, tg = m.get("scaling"), m.get("scalingTarget")
            if sc is None:
                return None
            if abs(sc - tg) < 1e-4 and prev is not None and abs(sc - prev) < 1e-6:
                ok("scale settled: Moon scaling %.8f == target %.8f (%d probes)"
                   % (sc, tg, i + 1))
                return sc
            prev = sc
        fail("scale never settled (last %s vs %s)" % (sc, tg))
        return None

    def stop(self):
        try:
            self.send("shutdown action now", 2.0)
        except Exception:
            pass
        for _ in range(40):
            if self.proc.poll() is not None:
                break
            time.sleep(0.5)
        if self.proc.poll() is None:
            self.proc.kill()
        return self.proc.poll()


# ------------------------------------------------------------------- analysis
FIELD = ["DE", "SA", "GHA", "LHA", "Az", "Alt", "coAlt", "LPA"]


def analyse(tag, dumpfile, rep):
    bodies, cam, jd = F44.load_dump(dumpfile)
    nav = parse_navstr_full(str(dumpfile) + ".navstr")
    ref = cam["reference"]
    ref_axis = (bodies.get(ref, {}).get("new") or {}).get("axisRot")
    leg = {"tag": tag, "dump": str(dumpfile), "jd": jd, "reference": ref,
           "refAxisRot": ref_axis, "freeMode": cam["freeMode"],
           "boundToSurface": cam["boundToSurface"],
           "viewOffsetEff": cam.get("viewOffsetEff"),
           "longitude_deg": cam["longitude"] * DEG,
           "latitude_deg": cam["latitude"] * DEG,
           "distance": cam["distance"], "bodies": {}}
    for name, r in sorted(nav.items()):
        o = nav_fields(r.get("OLD_nav"))
        n = nav_fields(r.get("NEW_nav"))
        if not o or not n:
            continue                      # composed body: no old twin (§11.158(h))
        row = {"ra_old": o[0], "ra_new": n[0],
               "de_old": o[1][0], "de_new": n[1][0],
               "sep_printed_deg": sep_deg_d(o[0], o[1][0], n[0], n[1][0]),
               "nav_old": dict(zip(FIELD, o[1])), "nav_new": dict(zip(FIELD, n[1])),
               "rade_str_equal": _rade_str(r.get("OLD_nav")) == _rade_str(r.get("NEW_nav"))}
        for f in ("SA", "GHA", "LHA", "LPA"):
            row["d" + f + "_arcsec"] = abs(row["nav_old"][f] - row["nav_new"][f]) * 3600.0
        b = bodies.get(name)
        if b:
            ao, an = b.get("altaz_old"), b.get("altaz_new")
            if ao and an and all(math.isfinite(x) for x in ao + an):
                row["dalt_deg"] = abs(ao[0] - an[0]) * DEG
                row["daz_deg"] = abs(F44.wrap180((ao[1] - an[1]) * DEG))
            pre, post = reconstruct(cam, b, ref_axis or 0.0)
            if pre:
                row["sep_model_pre_vs_printednew_deg"] = sep_deg_d(pre[0], pre[1], n[0], n[1][0])
                row["sep_model_post_vs_printednew_deg"] = sep_deg_d(post[0], post[1], n[0], n[1][0])
                # THE PARITY NUMBER, in §11.158(f)'s own form: the model at
                # DOUBLE precision against OLD's printed answer, so only ONE
                # print quantum enters (printed-vs-printed carries two).
                row["sep_post_vs_old_deg"] = sep_deg_d(post[0], post[1], o[0], o[1][0])
                row["sep_pre_vs_old_deg"] = sep_deg_d(pre[0], pre[1], o[0], o[1][0])
        leg["bodies"][name] = row
    rep["legs"][tag] = leg
    return leg


RADE_STR = re.compile(r'(\d+h\d+m[\d.]+s\s*/\s*[+-]\d+[^\d\'"]{1,4}\d+\'[\d.]+")')
def _rade_str(s):
    m = RADE_STR.search(s or "")
    return re.sub(r'\s+', '', m.group(1)) if m else None


def col(leg, key):
    return [r[key] for r in leg["bodies"].values()
            if key in r and isinstance(r[key], float) and math.isfinite(r[key])]


def top(leg, key, n=6):
    xs = [(r[key], nm) for nm, r in leg["bodies"].items()
          if key in r and isinstance(r[key], float) and math.isfinite(r[key])]
    return sorted(xs, reverse=True)[:n]


# --------------------------------------------------------------------- gates
def gates(rep, expect, out):
    A = rep["legs"].get("A")
    D = rep["legs"].get("D")
    if not A:
        fail("no leg A"); return 1

    n = len(A["bodies"])
    ok("LEG A jd=%s reference=%s axisRot=%.9f free=%s bound=%s viewOffsetEff=%s "
       "lon=%.6f deg lat=%.6f deg -- %d bodies carry BOTH paths' nav string"
       % (A["jd"], A["reference"], A["refAxisRot"] or 0.0, A["freeMode"],
          A["boundToSurface"], A["viewOffsetEff"], A["longitude_deg"],
          A["latitude_deg"], n))
    (ok if n >= 89 else fail)("G0 body count %d (>= 89 expected; F44 measured 90)" % n)
    (ok if A["viewOffsetEff"] in (0, 0.0) else fail)(
        "G1 viewOffsetEff == 0 (the reconstruction's own precondition)")
    (ok if not A["freeMode"] and A["boundToSurface"] else fail)(
        "G1 anchored and bound to the surface, as the shipped place is")

    # ---- the alt/az CONTROL: it is what makes the RA/DE gap the conversion's
    # own (§11.158(f), last paragraph).  Earth is at the nadir (azimuth
    # undefined) and Eris is a body-position divergence between the trees.
    daz = [(v, nm) for nm, r in A["bodies"].items() if "daz_deg" in r
           for v in [max(r["daz_deg"], r["dalt_deg"])]]
    bad = sorted([x for x in daz if x[0] > 3e-5], reverse=True)
    ok("G2 alt/az control: %d of %d agree to <= 3e-5 deg; exceptions %s"
       % (len(daz) - len(bad), len(daz), [(nm, round(v, 6)) for v, nm in bad]))
    (ok if all(nm in ("Earth", "Eris") for _v, nm in bad) else fail)(
        "G2 every alt/az exception is Earth (nadir) or Eris (§11.158(f3)(f4))")

    # ---- G3: DOES THE MODEL KNOW WHICH BINARY IT IS LOOKING AT? -------------
    mpre = F44.median(col(A, "sep_model_pre_vs_printednew_deg"))
    mpost = F44.median(col(A, "sep_model_post_vs_printednew_deg"))
    ok("G3 model vs the binary's own printed NEW RA/DE: H_pre median %.6f deg, "
       "H_post median %.6f deg" % (mpre, mpost))
    named, other = (mpre, mpost) if expect == "pre" else (mpost, mpre)
    (ok if named <= 0.005 else fail)(
        "G3a the %s model reproduces the printed NEW readout: median %.6f deg "
        "(<= 0.005, one RA print quantum)" % (expect, named))
    (ok if other > 1.0 else fail)(
        "G3b and the OTHER model does NOT (median %.6f deg > 1) -- the model can "
        "tell the two binaries apart" % other)

    # ---- THE LOCAL HOUR ANGLE'S OWN TERM, which is not a frame question -----
    # LHA - GHA is the observer's LONGITUDE by definition (old: `HA = sidereal +
    # Le - RA`, `GHA = sidereal - RA`, Le = observatory->getLongitude()
    # [body.cpp:411-415]).  The new path spells that term
    # `Camera::instance->getLatitude()` [ModularObject.cpp:82].  This gate reads
    # the difference-of-differences and so is INDEPENDENT of the RA fix.
    dl_old = [F44.wrap180(r["nav_old"]["LHA"] - r["nav_old"]["GHA"])
              for r in A["bodies"].values()]
    dl_new = [F44.wrap180(r["nav_new"]["LHA"] - r["nav_new"]["GHA"])
              for r in A["bodies"].values()]
    ok("LHA-GHA: OLD [%.6f, %.6f] deg (longitude %.6f) | NEW [%.6f, %.6f] deg "
       "(latitude %.6f)" % (min(dl_old), max(dl_old), A["longitude_deg"],
                            min(dl_new), max(dl_new), A["latitude_deg"]))
    (ok if max(abs(x - A["longitude_deg"]) for x in dl_old) < 0.02 else fail)(
        "the OLD path's LHA-GHA is the observer's LONGITUDE, every body")

    if expect == "pre":
        med = F44.median(col(A, "sep_printed_deg"))
        ok("P1 PRE-FIX headline: median sep(OLD, NEW) = %.4f deg "
           "(§11.158(e) measured 60.3636)" % med)
        (ok if med > 20.0 else fail)("P1 the pre-fix gap is the recorded one (> 20 deg)")
        (ok if max(abs(x - A["latitude_deg"]) for x in dl_new) < 0.02 else fail)(
            "P2 the NEW path's LHA-GHA is the observer's LATITUDE, every body "
            "-- the port slip at ModularObject.cpp:82, measured")
    else:
        # ---- THE PARITY TARGET (§11.158(f)) --------------------------------
        xs = [(r["sep_post_vs_old_deg"], nm) for nm, r in A["bodies"].items()
              if "sep_post_vs_old_deg" in r]
        xs.sort(reverse=True)
        worst = xs[0] if xs else (float('nan'), '-')
        # §11.158(f) publishes its target as "<= 0.002014 deg" and 0.002014 IS
        # that entry's measured max -- the same body (Deimos) at the same
        # value, which this file pre-registered from F44's own landed dump
        # before the fix existed.  The comparison is therefore made AT THE
        # PUBLISHED PRECISION (six decimals) rather than against a widened
        # band: a body whose residual rounds to more than 0.002014 still fails.
        # The number's own floor is the print quantum -- this compares a DOUBLE
        # model against OLD's answer PRINTED to 1 s of time in RA and 1" in DE
        # -- which is why Q2 below, comparing the two paths' printed strings to
        # each other, is the criterion with no band at all.
        over = [(v, nm) for v, nm in xs if round(v, 6) > PARITY_TARGET]
        ok("Q1 parity, model(double) vs OLD printed: max %.6f deg on %s; "
           "over the target: %s" % (worst[0], worst[1],
                                      [(nm, round(v, 6)) for v, nm in over]))
        (ok if [nm for _v, nm in over] in ([], ["Eris"]) else fail)(
            "Q1 <= 0.002014 deg for every body but Eris (§11.158(f)'s target and "
            "its single named exception)")
        # what the READER sees: two printed strings, two quantisations
        pxs = sorted([(r["sep_printed_deg"], nm) for nm, r in A["bodies"].items()],
                     reverse=True)
        same = sum(1 for r in A["bodies"].values() if r.get("rade_str_equal"))
        diff = sorted([(r["sep_printed_deg"], nm) for nm, r in A["bodies"].items()
                       if not r.get("rade_str_equal")], reverse=True)
        ok("Q2 printed OLD vs printed NEW, byte for byte: %d of %d bodies print the "
           "SAME RA/DE string; the rest: %s"
           % (same, len(A["bodies"]), [(nm, round(v, 9)) for v, nm in diff]))
        (ok if same >= len(A["bodies"]) - 2 else fail)(
            "Q2 at most two bodies print a different RA/DE string")
        (ok if all(v <= 0.005 for v, nm in pxs if nm != "Eris") else fail)(
            "Q2 every non-Eris body agrees within one RA print quantum (0.004167 deg)")
        # ---- the nav fields ------------------------------------------------
        for f in ("SA", "GHA", "LHA", "LPA"):
            k = "d%s_arcsec" % f
            ys = sorted([(r[k], nm) for nm, r in A["bodies"].items() if k in r],
                        reverse=True)
            bad = [(v, nm) for v, nm in ys if v > 2.0 and nm != "Eris"]
            ok("Q3 %-3s max |old-new| = %.2f arcsec on %s; over 2 arcsec (non-Eris): %s"
               % (f, ys[0][0], ys[0][1], [(nm, round(v, 2)) for v, nm in bad]))
            (ok if not bad else fail)("Q3 %s equal old vs new to <= 2 arcsec" % f)

    # ---- leg D: the hour angles at a PRE-J2000 date -------------------------
    if D:
        neg_old = {f: sum(1 for r in D["bodies"].values() if r["nav_old"][f] < 0)
                   for f in ("GHA", "LHA", "LPA")}
        neg_new = {f: sum(1 for r in D["bodies"].values() if r["nav_new"][f] < 0)
                   for f in ("GHA", "LHA", "LPA")}
        ok("LEG D jd=%s: negative hour angles printed -- OLD %s, NEW %s (of %d bodies)"
           % (D["jd"], neg_old, neg_new, len(D["bodies"])))
        if expect == "post":
            (ok if neg_new == neg_old else fail)(
                "Q4 the new path normalises its hour angles exactly as old does "
                "at a pre-J2000 date")

    rep["fails"], rep["notes"] = FAILS, NOTES
    (Path(out) / "f91_result.json").write_text(json.dumps(rep, indent=1))
    print("\n%d FAIL(s), %d NOTE(s); report -> %s"
          % (len(FAILS), len(NOTES), Path(out) / "f91_result.json"), flush=True)
    return 1 if FAILS else 0


# ---------------------------------------------------------------------- table
def write_table(rep, out, expect):
    A = rep["legs"].get("A")
    if not A:
        return
    lines = ["F91 leg A  jd=%s  reference=%s  expect=%s  locale=%s"
             % (A["jd"], A["reference"], expect, rep.get("locale")),
             "%-14s %11s %11s %11s %11s %10s %9s %9s %9s %9s"
             % ("body", "RA_old", "RA_new", "DE_old", "DE_new", "sep_prt",
                "dSA\"", "dGHA\"", "dLHA\"", "dLPA\"")]
    for nm, r in sorted(A["bodies"].items()):
        lines.append("%-14s %11.6f %11.6f %11.6f %11.6f %10.6f %9.2f %9.2f %9.2f %9.2f"
                     % (nm, r["ra_old"], r["ra_new"], r["de_old"], r["de_new"],
                        r["sep_printed_deg"], r["dSA_arcsec"], r["dGHA_arcsec"],
                        r["dLHA_arcsec"], r["dLPA_arcsec"]))
    (Path(out) / ("f91_table_%s.txt" % expect)).write_text("\n".join(lines) + "\n")


# ----------------------------------------------------------------------- main
def main():
    argv = sys.argv[1:]
    def opt(name, default=None):
        return argv[argv.index(name) + 1] if name in argv else default
    binp = Path(opt("--bin", str(HERE.parents[1] / "build-claude/src/spacecrafter")))
    expect = opt("--expect", "pre")
    locale = opt("--locale", "fr")
    out = Path([a for a in argv if not a.startswith("--")][0]).resolve()
    out.mkdir(parents=True, exist_ok=True)

    rep = {"bin": str(binp), "expect": expect, "locale": locale, "legs": {}}

    if "--offline" in argv:
        for tag, pat in (("A", "legA_*"), ("D", "legD_*")):
            f = [p for p in sorted(out.glob(pat))
                 if ".navstr" not in str(p) and "_result" not in str(p)]
            f = [Path(str(p)[:-3]) if str(p).endswith(".gz") else p for p in f]
            if f:
                analyse(tag, f[-1], rep)
        write_table(rep, out, expect)
        return gates(rep, expect, out)

    md5_in = subprocess.run(["md5sum", str(REAL_HOME / "config.ini"),
                             str(REAL_HOME / "ssystem.ini")],
                            capture_output=True, text=True).stdout
    print("md5 IN (real home):\n" + md5_in, flush=True)
    rep["md5_in"] = md5_in

    farm = FARM_ROOT / ("%s-%s" % (expect, locale))
    rep["farm"] = str(farm)
    rep["farm_shape"] = build_farm(farm, locale)
    print("farm %s : %s" % (farm, rep["farm_shape"]), flush=True)

    app = App(binp, farm, out)
    try:
        rep["tcp_seconds"] = app.start()
        time.sleep(8)
        app.send("flag experimental_path on")
        app.send("timerate rate 0")
        app.send("date jday %s" % JD, 1.2)
        app.settle_scale()
        pA = app.dump("legA")
        app.send("date jday %s" % JD_PRE2000, 1.5)
        pD = app.dump("legD")
        app.send("date jday %s" % JD, 1.2)
        analyse("A", pA, rep)
        analyse("D", pD, rep)
    finally:
        rep["exit_code"] = app.stop()

    md5_out = subprocess.run(["md5sum", str(REAL_HOME / "config.ini"),
                              str(REAL_HOME / "ssystem.ini")],
                             capture_output=True, text=True).stdout
    rep["md5_out"] = md5_out
    (ok if md5_out == md5_in else fail)("md5 in == out on the real ~/.spacecrafter")
    (ok if rep["exit_code"] == 0 else fail)("app exit code %s" % rep["exit_code"])
    write_table(rep, out, expect)
    return gates(rep, expect, out)


if __name__ == "__main__":
    sys.exit(main())
