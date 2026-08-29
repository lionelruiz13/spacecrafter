#!/usr/bin/env python3
"""F34 / INTENT §5.80 — WHICH PARAMETRIZATION THE FREE-FLIGHT MOVERS WERE BUILT
AGAINST.  RECORD-ONLY: this script changes nothing and asks the shipped binary.

    cd claude/harness && DISPLAY=:2 ./f34_convention.py <outdir> [--bin B]

*** THIS INSTRUMENT IS PRE-`18b6f13f` AND ITS GATES ARE NOT RE-BASELINED. ***
Its C3 gates assert convention A — `position == spheToRect(-lon,lat)*distance`,
the DRAWN eye after `free_mode on` being `-Z(theta).pA` — which is exactly the
defect §5.80 named and F40 removed (§11.153).  Run against a binary at or after
`18b6f13f` those gates FAIL BY CONSTRUCTION; that is not a regression, it is
the fix.  Its recorded 27/27 stands as a measurement of the PRE binary and was
replayed as such during F40 (`artifacts/f40/f34_replay`, on `/tmp/sc_f40_pre`
= code `7ef11aca`).

It is deliberately NOT re-pointed at the new convention: `f40_inverse.py` scores
BOTH conventions from one run under `--expect pre|post` (§11.153(o)(2)), so
re-baselining this file would put the same claim in two homes (I2).  Use
`f40_inverse.py` for any binary from `18b6f13f` on; keep this one for reading
what the pre binary did.  [F43, 2026-08-29, §11.157]

WHAT §5.80 RECORDS AND WHAT IT OWES.  `camera action free_mode state on` swings
the observer ~125 deg around its reference at constant distance, because
`Camera::setFreeMode`'s pose conversion is not the inverse of what `viewMat()`'s
anchored branch composes.  The row names two conventions and OWES the datum
*which of the two the free-flight movers were BUILT against* — B10/B21 measured
their behaviour, not their frame.  The two, verbatim from the row:

    (A)  spheToRect(-longitude, latitude) * distance      [setFreeMode]
    (B)  Z(lon) . X(pi/2 - lat) . (0,0,distance)          [viewMat, anchored]

THE THIRD TERM THE ROW DOES NOT NAME, AND IT IS HALF THE DEFECT.  (B) is the
EYE's position in the pose frame; (A) is written into the member `position`,
and `viewMat`'s FREE branch is `mat = R . T(position)`, so under the composer
`position` is MINUS the eye (a view matrix is R.T(-eye); `Camera::placeAt`
states it — "p = -position" — and F33 discriminated that composition against
the old path at 2.63e-08 AU).  So the composer's own answer for the member is
`position = -pB`, and (A) writes `+pA`.  The two therefore differ by BOTH a
sign and an azimuth handedness, and that is a checkable claim, not a reading:

    pB(lam,phi,d) = d * ( cos(phi) sin(lam), -cos(phi) cos(lam), sin(phi) )
    pA(lam,phi,d) = d * ( cos(phi) cos(lam), -cos(phi) sin(lam), sin(phi) )
    azimuth(pB) = lam - pi/2      azimuth(pA) = -lam       (opposite handedness)
    eye_free = -pA = Z(pi/2) . X(pi) . pB  =  a 180 deg rotation about the
    horizontal axis at azimuth 45 deg in the reference's pose frame.

    ==> swing across `free_mode on`:  cos(theta) = cos^2(phi) (1 - sin 2 lam) - 1
    An azimuth-ONLY defect would instead give cos(theta) = cos^2(phi) sin(2 lam)
    + sin^2(phi), i.e. the SUPPLEMENT.  At the shipped place (43.3 N, 5.3667 E)
    the two read 124.68 deg and 55.32 deg — F33 measured 124.8 deg (§11.143(k)),
    so the sign term is already the one the field says.  This run re-measures it
    against a prediction committed here, on places chosen to separate the pair
    much further than the shipped one does.

HOW EACH MOVER IS PLACED (the deliverable).  Every dump gives the drawn matrix
`camera.mat` (== `lastDispatchedMat`, the map the renderer consumed), so the
eye is read the convention-proof way, E = -Rot^T . t, exactly as
`Camera::getReferenceRelativePosition` does.  E lives in the reference's
ACCUMULATED EQUATORIAL frame, i.e. with the surface fold S = Z(-axisRot-pi/2)
undone; the per-body dump carries `axisRot`, so every prediction below is
closed-form with NO free parameter.  theta is ALSO fitted from the anchored leg
and the two are compared — an instrument check, not a tuning knob.

    C1  anchored composer      E == Z(theta) . pB(lon, lat, distance)
    C2  free composer          E == -Z(theta) . position
    C3  setFreeMode(true)      position == pA(lon,lat,distance) exactly, hence
                               E_after == -Z(theta) . pA  and the swing above
    C4  moveTo free branch     same, at the COMMANDED triple; the anchored form
                               of the SAME command is the control
    C5  moveEyeRel / descend   dE == -Rot^T . v for the eye-frame v the mover
                               passes; for `descend coef>1` v = (0,0,-step) so
                               dE == +step * (r[2],r[6],r[10]).  H_composer and
                               H_flipped differ by the SIGN of dE, which needs
                               no knowledge of the ground radius.
    C6  setFreeMode(false)     lam' = -atan2(P.y,P.x), phi' = asin(P.z/|P|),
                               d' = |P|, hence E' == Z(theta) . pB(lam',phi',d')

Each of C3/C4/C5/C6 is scored against BOTH hypotheses, and the losing one is
reported with its own residual so the discrimination is two-sided.

CONTROLS.  (i) An A/A control: two dumps and two screenshots with no command in
between — 0 AU and 0 px, the §11.143(k) pattern, so a non-zero elsewhere is an
effect and not the instrument.  (ii) The reversible pair is entered TWICE, the
second entry starting from the state the first exit produced, and with a FLIGHT
(a descend) in between so the pair is not a trivial A/A^-1 round trip.
(iii) The composed SCREEN across the toggle, landscape off (a landscape masks
the ground half of the frame).  (iv) `get status position` (F27, §11.135) at
every step, because what the shipped readout says while the observer teleports
is part of the answer.

Fresh launch, `/proc/<pid>/comm` concurrent-instance assert, frozen config /
ssystem md5 in == out — all three from `f27_reply.Session` (I2).

TOLERANCES, each with its mechanism:
  POSE_FLOOR 3.0e-07 AU  — the pose members are float32 of magnitude ~1e-4 AU
                           here but the matrix they compose is float32 at ~1 AU
                           scale in `mat`, so a reconstruction of E from the
                           pose triple carries the float32 floor of the 1 AU
                           subtraction inside -Rot^T.t: 1 AU * 2^-24 = 6e-08 AU,
                           x4 for the three-factor product.
  MEMBER_FLOOR 1.0e-09 AU — a member-to-member check (position vs pA) does no
                           1 AU subtraction; float32 at 5e-05 AU is 3e-12.
  ANG_FLOOR  1.0e-04 rad  — the swing is reconstructed from two float32
                           matrices at ~1 AU; 6e-08 AU over a 4e-05 AU arm.
"""

import argparse, json, math, sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from f27_reply import Session
import b24_equivalence as b24          # the ONE non-finite dump grammar (I2)

HERE = Path(__file__).resolve().parent
JD0 = 2461233.5
AU_KM = 149597870.691                  # sc_const.hpp:45, the value the engine uses
POSE_FLOOR = 3.0e-07
MEMBER_FLOOR = 1.0e-09
ANG_FLOOR = 1.0e-04

CHECKS = []
DATA = {}


def chk(ok, label, detail=""):
    CHECKS.append({"ok": bool(ok), "label": label, "detail": detail})
    print(("OK   " if ok else "FAIL ") + label + (("  -- " + detail) if detail else ""),
          flush=True)
    return bool(ok)


# --------------------------------------------------------------- predictions
# COMMITTED BEFORE THE RUN.  Transcribed from the source, term for term:
#   Utility::spheToRect(lng,lat,v) : v = (cos lng cos lat, sin lng cos lat, sin lat)
#                                    [tools/utility.cpp:95]
#   Mat4f::zrotation(a)            : column-major (c,s,0 / -s,c,0 / 0,0,1), i.e.
#                                    the CONVENTIONAL counter-clockwise rotation
#                                    when applied to a column vector by
#                                    multiplyWithoutTranslation (vecmath.hpp:1783,
#                                    :1879 — the file's own "unconventionally
#                                    clockwise" comment describes the OTHER
#                                    overload, zrotation(cos,sin), which is its
#                                    transpose; both are in the tree)
#   Mat4f::xrotation(a)            : conventional CCW about x (vecmath.hpp:1686)
#   Mat4f::multiplyTranslation(a)  : this := this . T(a)  (vecmath.hpp:1967)
#   ModularBody::computeSurfaceToBody() = Z(-getAxisRotation())
#                                       = Z(-(axisRot + pi/2))   [ModularBody.hpp:558,:550]

def zrot(a):
    c, s = math.cos(a), math.sin(a)
    return np.array([[c, -s, 0.0], [s, c, 0.0], [0.0, 0.0, 1.0]])


def pB(lam, phi, d):
    """viewMat()'s anchored branch, solved for the eye: the input-frame eye
    position of mat = R . T(0,0,-d) . X(lat-pi/2) . Z(-lon) is
    Z(lon) . X(pi/2-lat) . (0,0,d).  Camera.cpp:504-507 states the same."""
    return np.array([d * math.cos(phi) * math.sin(lam),
                     -d * math.cos(phi) * math.cos(lam),
                     d * math.sin(phi)])


def pA(lam, phi, d):
    """setFreeMode(true)'s conversion: spheToRect(-lon, lat) * distance
    [Camera.cpp:740-741], and moveTo's free branch verbatim [Camera.cpp:710-711]."""
    return np.array([d * math.cos(phi) * math.cos(lam),
                     -d * math.cos(phi) * math.sin(lam),
                     d * math.sin(phi)])


def swing_signed(lam, phi):
    """Angle between the anchored eye pB and the free eye -pA — the SIGN-AND-
    AZIMUTH hypothesis (H_A, what the source says)."""
    return math.acos(max(-1.0, min(1.0, math.cos(phi) ** 2 * (1 - math.sin(2 * lam)) - 1)))


def swing_azimuth_only(lam, phi):
    """The rival reading of §5.80's own text ("the two differ by a rotation",
    with `position` taken to BE the eye): the angle between pB and +pA."""
    return math.acos(max(-1.0, min(1.0, math.cos(phi) ** 2 * math.sin(2 * lam)
                                   + math.sin(phi) ** 2)))


def leave_free(P):
    """setFreeMode(false) [Camera.cpp:743-745]: rectToSphe then negate lng."""
    lng = math.atan2(P[1], P[0])
    lat = math.asin(P[2] / np.linalg.norm(P))
    return -lng, lat, float(np.linalg.norm(P))


# ------------------------------------------------------------------ readers
def mat_of(m16):
    """The dump writes Mat4f::r[0..15]; r is COLUMN-major (multiplyWithoutTranslation
    reads row i as r[i], r[i+4], r[i+8]).  Returns (Rot3x3, translation)."""
    m = np.array(m16, dtype=float).reshape(4, 4).T
    return m[:3, :3], m[:3, 3]


def eye_of(m16):
    """-Rot^T . t — the eye is the origin of the eye frame.  The same expression
    Camera::getReferenceRelativePosition evaluates (Camera.cpp:455-462)."""
    R, t = mat_of(m16)
    return -R.T @ t


def ang(u, v):
    n = np.linalg.norm(u) * np.linalg.norm(v)
    if n == 0:
        return float("nan")
    return math.acos(max(-1.0, min(1.0, float(np.dot(u, v)) / n)))


class App:
    def __init__(self, sess, drv, outdir):
        self.sess, self.drv, self.outdir = sess, drv, outdir
        self.n = 0
        self.log = []

    def cmd(self, c, pause=0.8):
        self.drv.send(c, pause)
        self.log.append(c)

    def dump(self, tag):
        self.n += 1
        p = f"/tmp/f34_{self.n:03d}_{tag}.json"
        self.drv.send(f"body action dual_dump filename {p}", 1.2)
        with open(p) as f:
            head = json.loads(b24.sanitize_nonfinite(f.readline()))
            bodies = {}
            for line in f:
                line = line.strip()
                if line:
                    o = json.loads(b24.sanitize_nonfinite(line))
                    if o.get("type") == "body":
                        bodies[o["name"]] = o
        cam = head["camera"]
        st = {
            "tag": tag,
            "ref": cam["reference"],
            "free": cam["freeMode"],
            "bound": cam["boundToSurface"],
            "lon": cam["longitude"], "lat": cam["latitude"], "dist": cam["distance"],
            "position": list(cam["position"]),
            "E": eye_of(cam["mat"]).tolist(),
            "mat": list(cam["mat"]),
            "rootPos": list(cam["rootPos"]),
            "control": {k: head["control"][k]["new"]
                        for k in ("latitude", "longitude", "altitude")},
        }
        nb = bodies.get(st["ref"], {}).get("new") or {}
        st["axisRot"] = nb.get("axisRot")
        st["theta"] = (nb["axisRot"] + math.pi / 2) if "axisRot" in nb else None
        return st

    def place(self):
        """`get status position` — the shipped readout (F27, §11.135)."""
        self.drv.sock.sendall(b"get status position\n")
        r, _, _ = self.drv.poll_for_reply(4.0)
        return r

    def shot(self, tag):
        p = self.outdir / f"f34_{tag}.png"
        self.drv.send(f"body action screenshot filename {p}", 2.5)
        return p


def px(a, b, thr=8):
    A = np.asarray(Image.open(a).convert("L"), dtype=int)
    B = np.asarray(Image.open(b).convert("L"), dtype=int)
    return int((np.abs(A - B) > thr).sum())


# ------------------------------------------------------------- per-state law
def score_state(st, label):
    """Score one dumped state against the composer (C1 anchored / C2 free).
    Returns the residual of the winning claim and of its rival."""
    E = np.array(st["E"])
    th = st["theta"]
    P = np.array(st["position"])
    if st["free"]:
        r_composer = float(np.linalg.norm(E + zrot(th) @ P))       # E == -Z(th).P
        r_rival = float(np.linalg.norm(E - zrot(th) @ P))          # E == +Z(th).P
        DATA.setdefault("C2", []).append(
            {"state": label, "res_minus": r_composer, "res_plus": r_rival})
        return r_composer, r_rival
    r_composer = float(np.linalg.norm(E - zrot(th) @ pB(st["lon"], st["lat"], st["dist"])))
    r_rival = float(np.linalg.norm(E - zrot(th) @ pA(st["lon"], st["lat"], st["dist"])))
    DATA.setdefault("C1", []).append(
        {"state": label, "res_B": r_composer, "res_A": r_rival})
    return r_composer, r_rival


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=str(HERE.parents[1] / "build-claude/src/spacecrafter"))
    a = ap.parse_args()
    # RESOLVED: the farm becomes the app's `HOME`, and `main.cpp:194` chdirs to
    # `$HOME/.spacecrafter/` — from a cwd that is already the farm.  A relative
    # outdir therefore kills the app before it opens its port, with
    # "cannot set current path" and nothing else (measured).
    out = Path(a.outdir).resolve()
    out.mkdir(parents=True, exist_ok=True)

    sess = Session(out, "f34", a.bin)
    drv = sess.client("drv")
    app = App(sess, drv, out)
    for c in ("flag atmosphere off", "flag landscape off", "flag fog off",
              "timerate rate 0"):
        app.cmd(c, 0.5)
    app.cmd(f"date jday {JD0:.9f}", 1.2)
    # A place chosen so the two hypotheses' swings are far apart AND the azimuth
    # term is large: lam = 70 deg, phi = 30 deg gives 137.062 deg (H_A) against
    # 42.938 deg (azimuth-only) — the two are supplementary by construction, so
    # a place with a big azimuth term is what separates them.  Verified against
    # the project's own Mat4f/spheToRect by `f34_probe.cpp` before this run.
    # Altitude 1000 km clears the free-mode ground clamp so nothing but the
    # movers moves the observer.
    LAM0, PHI0, ALT0 = 70.0, 30.0, 1.0e6
    app.cmd(f"moveto lat {PHI0} lon {LAM0} alt {ALT0} duration 0", 1.2)

    # ---------------- leg 0: A/A control (§11.143(k) pattern) ---------------
    s_aa1 = app.dump("aa1")
    sh_aa1 = app.shot("aa1")
    s_aa2 = app.dump("aa2")
    sh_aa2 = app.shot("aa2")
    d_aa = float(np.linalg.norm(np.array(s_aa2["E"]) - np.array(s_aa1["E"])))
    chk(d_aa == 0.0, "A/A control: no command, the eye does not move",
        f"{d_aa:.3e} AU")
    px_aa = px(sh_aa1, sh_aa2)
    chk(px_aa == 0, "A/A control: no command, the composed screen is identical",
        f"{px_aa} px>8")
    DATA["AA"] = {"dE": d_aa, "px": px_aa}

    # ---------------- leg 1: the composer, calibrated -----------------------
    st = s_aa2
    rb, ra = score_state(st, "anchored_start")
    chk(rb < POSE_FLOOR and rb < ra / 100,
        "C1 anchored: the drawn eye IS Z(theta).pB(lon,lat,distance)",
        f"res_B = {rb:.3e} AU vs res_A = {ra:.3e} AU (theta = {st['theta']:.6f} rad)")
    # theta fitted from the same state, as an instrument check on axisRot
    E = np.array(st["E"])
    p = pB(st["lon"], st["lat"], st["dist"])
    th_fit = math.atan2(E[1], E[0]) - math.atan2(p[1], p[0])
    th_fit = (th_fit + math.pi) % (2 * math.pi) - math.pi
    th_dump = (st["theta"] + math.pi) % (2 * math.pi) - math.pi
    chk(abs(th_fit - th_dump) < 1e-4,
        "C1 the surface fold read from the body's axisRot IS the one in the matrix",
        f"fitted {th_fit:.6f} vs axisRot+pi/2 {th_dump:.6f} rad")
    DATA["theta"] = {"fit": th_fit, "dump": th_dump, "axisRot": st["axisRot"]}
    DATA["ground"] = {"distance_AU": st["dist"],
                      "altitude_m": st["control"]["altitude"],
                      "altitudeReference_AU": st["dist"] - st["control"]["altitude"] / (1000 * AU_KM)}
    place0 = app.place()

    # ---------------- leg 2: the converter, setFreeMode(true) ---------------
    before = st
    app.cmd("camera action free_mode state on", 1.5)
    after = app.dump("free_on")
    sh_on = app.shot("free_on")
    place1 = app.place()
    lam, phi, d = before["lon"], before["lat"], before["dist"]
    th = before["theta"]
    predA_pos = pA(lam, phi, d)
    predA_E = -zrot(th) @ predA_pos
    predB_E = np.array(before["E"])                       # the composer's inverse: no move
    Eaf = np.array(after["E"])
    resA = float(np.linalg.norm(Eaf - predA_E))
    resB = float(np.linalg.norm(Eaf - predB_E))
    chk(float(np.linalg.norm(np.array(after["position"]) - predA_pos)) < MEMBER_FLOOR,
        "C3 member: setFreeMode(true) writes position == spheToRect(-lon,lat)*distance",
        f"|position - pA| = {np.linalg.norm(np.array(after['position']) - predA_pos):.3e} AU")
    chk(resA < POSE_FLOOR and resA < resB / 100,
        "C3 composed: the DRAWN eye after `free_mode on` is -Z(theta).pA  (convention A)",
        f"res_A = {resA:.3e} AU vs res_B(no-move) = {resB:.3e} AU")
    sw = ang(np.array(before["E"]), Eaf)
    sw_A, sw_az = swing_signed(lam, phi), swing_azimuth_only(lam, phi)
    chk(abs(sw - sw_A) < ANG_FLOOR and abs(sw - sw_az) > 0.1,
        "C3 swing: the sign-AND-azimuth law, not the azimuth-only one",
        f"measured {math.degrees(sw):.4f} deg ; H_A {math.degrees(sw_A):.4f} ; "
        f"azimuth-only {math.degrees(sw_az):.4f}")
    chk(abs(np.linalg.norm(Eaf) - np.linalg.norm(before["E"])) < POSE_FLOOR,
        "C3 the distance to the reference is preserved (why selDist is blind)",
        f"{np.linalg.norm(before['E']):.9e} -> {np.linalg.norm(Eaf):.9e} AU")
    px_on = px(sh_aa2, sh_on)
    # The gate is the A/A control, not a round number: at 1000 km with the
    # atmosphere and the landscape off and the view pointing AWAY from the
    # reference, almost every lit pixel is a star, and a star is at infinity —
    # a 13 700 km translation moves it by nothing.  The strong screen witness
    # is the dedicated leg at the end, with the body in frame.
    chk(px_on > px_aa, "C3 screen: the composed frame changes across the toggle",
        f"{px_on} px>8 against the A/A control's {px_aa}")
    DATA["C3"] = {"before": before, "after": after, "resA": resA, "resB": resB,
                  "swing_meas": sw, "swing_H_A": sw_A, "swing_azimuth_only": sw_az,
                  "chord_km": float(np.linalg.norm(Eaf - np.array(before["E"])) * AU_KM),
                  "px": px_on, "place_before": place0, "place_after": place1}
    chk(place0 and place1 and abs(place0["lat"] - place1["lat"]) < 1e-6
        and abs(place0["lon"] - place1["lon"]) < 1e-6,
        "C3 readout: `get status position` reports the SAME place across the teleport",
        f"{place0} -> {place1}")

    # ---------------- leg 3: moveTo's free branch ---------------------------
    LAM1, PHI1, ALT1 = -130.0, -20.0, 2.0e6
    app.cmd(f"moveto lat {PHI1} lon {LAM1} alt {ALT1} duration 0", 1.5)
    m_free = app.dump("moveto_free")
    th1 = m_free["theta"]
    # THE MOVER'S OWN DISTANCE, not the `distance` member: moveTo's free branch
    # scales by `reference->getAltitudeReference() + pos[2]` (Camera.cpp:711) and
    # writes ONLY `position` — `distance` keeps a stale value in free flight by
    # design (B10 §11.71, lateral-velocity parity).  Predicting with the member
    # instead costs exactly the stale-vs-commanded gap, which is measured below
    # as the attributed control rather than hidden in a tolerance.
    altRef = DATA["ground"]["altitudeReference_AU"]
    d1 = altRef + ALT1 / (1000 * AU_KM)
    d1_stale = m_free["dist"]
    lam1, phi1 = math.radians(LAM1), math.radians(PHI1)
    predA_E1 = -zrot(th1) @ pA(lam1, phi1, d1)
    predB_E1 = zrot(th1) @ pB(lam1, phi1, d1)
    E1 = np.array(m_free["E"])
    rA1, rB1 = float(np.linalg.norm(E1 - predA_E1)), float(np.linalg.norm(E1 - predB_E1))
    rA1_stale = float(np.linalg.norm(E1 + zrot(th1) @ pA(lam1, phi1, d1_stale)))
    chk(rA1 < POSE_FLOOR and rA1 < rB1 / 100,
        "C4 moveTo free branch: lands at -Z(theta).pA of the COMMANDED triple (A)",
        f"res_A = {rA1:.3e} AU vs res_B = {rB1:.3e} AU")
    chk(abs(rA1_stale - abs(d1 - d1_stale)) < POSE_FLOOR,
        "C4 control: predicting with the STALE `distance` member misses by exactly "
        "the stale-vs-commanded gap",
        f"res = {rA1_stale:.3e} AU, gap = {abs(d1-d1_stale):.3e} AU "
        f"({d1_stale*AU_KM:.3f} -> {d1*AU_KM:.3f} km)")
    r2, r2r = score_state(m_free, "free_after_moveto")
    chk(r2 < POSE_FLOOR and r2 < r2r / 100,
        "C2 free composer: the drawn eye IS -Z(theta).position",
        f"res(-) = {r2:.3e} AU vs res(+) = {r2r:.3e} AU")
    # the control: the SAME command, anchored
    app.cmd("camera action free_mode state off", 1.5)
    app.cmd(f"moveto lat {PHI1} lon {LAM1} alt {ALT1} duration 0", 1.5)
    m_anch = app.dump("moveto_anchored")
    E1a = np.array(m_anch["E"])
    rB1a = float(np.linalg.norm(E1a - zrot(m_anch["theta"]) @ pB(lam1, phi1, m_anch["dist"])))
    chk(rB1a < POSE_FLOOR,
        "C4 control: the SAME command anchored lands at Z(theta).pB (B)",
        f"res_B = {rB1a:.3e} AU")
    sep = float(np.linalg.norm(E1a - E1) * AU_KM)
    DATA["C4"] = {"free": m_free, "anchored": m_anch, "resA": rA1, "resB": rB1,
                  "control_resB": rB1a, "separation_km": sep,
                  "swing_deg": math.degrees(ang(E1a, E1))}
    chk(sep > 1000, "C4 one command, two modes, two PLACES",
        f"{sep:.1f} km apart, {math.degrees(ang(E1a, E1)):.3f} deg")

    # ---------------- leg 4: descend / moveEyeRel ---------------------------
    # AIM FIRST.  `descend`'s near-field geometry is the VIEW RAY (B21/§11.72),
    # and after the transitions above the view points away from the reference —
    # measured on the first run: the backward ray sat 133.4 deg off radial-out
    # and the ray missed the ground entirely, so BOTH branches ran in their
    # fallback/ray-independent form.  Tracking aims the view at the body, then
    # is switched OFF so the view is static across the descend (a moving view
    # would change the very matrix the prediction reads).  The staticity is
    # CHECKED, not assumed.
    app.cmd("camera action free_mode state on", 1.5)
    app.cmd("select planet Earth pointer off", 1.0)
    app.cmd("flag track_object on", 6.0)
    app.cmd("flag track_object off", 1.5)
    aim1 = app.dump("aim1")
    aim2 = app.dump("aim2")
    chk(np.array_equal(np.array(aim1["mat"])[:12], np.array(aim2["mat"])[:12]),
        "C5 precondition: with tracking off the view rotation is static",
        "mat rotation identical across two dumps")
    d0 = aim1
    R0, t0 = mat_of(d0["mat"])
    # dE = -Rot^T . v  (derivation in the header).  Rot maps the reference's
    # accumulated-equatorial frame to the eye frame, so Rot^T . z_eye is the
    # eye's BACKWARD ray already expressed in the frame E lives in — no further
    # fold, and in particular no second Z(theta).
    up = np.array(R0[2, :])                               # == Rot^T . (0,0,1)
    g = DATA["ground"]["altitudeReference_AU"]
    COEF_UP = 1.25
    # ASCEND has no fallback: it always backs off along the ray (Camera.cpp:1087-1099),
    #   step = max((coef-1)*(|C|-g), ANTISTUCK_ESCAPE_FLOOR*radius)   -> dE = +step*up
    step_pred = max((COEF_UP - 1) * (float(np.linalg.norm(d0["E"])) - g), 1e-6 * g)
    predU = step_pred * up
    app.cmd(f"camera action descend coef {COEF_UP}", 1.2)
    d1 = app.dump("desc_up")
    dE = np.array(d1["E"]) - np.array(d0["E"])
    rU = float(np.linalg.norm(dE - predU))
    rUflip = float(np.linalg.norm(dE + predU))
    chk(rU < POSE_FLOOR and rU < rUflip / 100,
        "C5 descend(ascend): dE == +step*(Rot^T.z), the composer's own free frame",
        f"res = {rU:.3e} AU vs H_flipped {rUflip:.3e} AU ; "
        f"|dE| = {np.linalg.norm(dE)*AU_KM:.3f} km predicted {step_pred*AU_KM:.3f} km")
    dr_up = (np.linalg.norm(d1["E"]) - np.linalg.norm(d0["E"])) * AU_KM
    cos_rad = float(np.dot(up, d0["E"]) / np.linalg.norm(d0["E"]))
    r3, r3r = score_state(d1, "free_after_ascend")
    chk(r3 < POSE_FLOOR and r3 < r3r / 100,
        "C2 after a flight the free composer still holds (E == -Z(theta).position)",
        f"res(-) = {r3:.3e} AU vs res(+) = {r3r:.3e} AU")
    # DESCEND picks its branch from the ray/sphere intersection (Camera.cpp:1101-1115)
    COEF_DOWN = 0.8
    R1, C1v = mat_of(d1["mat"])
    fwd1 = np.array(-R1[2, :])                            # Rot^T.(0,0,-1)
    Cz, disc = C1v[2], C1v[2] ** 2 - (float(C1v @ C1v) - g * g)
    s = (-Cz - math.sqrt(disc)) if disc >= 0 else -1.0
    if s > 0:
        branch, predD = "ray", (1 - COEF_DOWN) * s * fwd1
    else:
        alt_l = max(float(np.linalg.norm(d1["E"])) - g, 0.0)
        branch = "radial"
        predD = np.array(d1["E"]) * ((COEF_DOWN - 1) * alt_l / float(np.linalg.norm(d1["E"])))
    app.cmd(f"camera action descend coef {COEF_DOWN}", 1.2)
    d2 = app.dump("desc_down")
    dE2 = np.array(d2["E"]) - np.array(d1["E"])
    rD = float(np.linalg.norm(dE2 - predD))
    rDflip = float(np.linalg.norm(dE2 + predD))
    chk(rD < POSE_FLOOR and rD < rDflip / 100,
        f"C5 descend(descend), {branch} branch: dE == the composer's own prediction",
        f"res = {rD:.3e} AU vs H_flipped {rDflip:.3e} AU ; "
        f"|dE| = {np.linalg.norm(dE2)*AU_KM:.3f} km predicted {np.linalg.norm(predD)*AU_KM:.3f} km")
    chk(np.linalg.norm(d2["E"]) < np.linalg.norm(d1["E"]),
        "C5 descend coef<1 lowers the observer", f"{np.linalg.norm(d1['E'])*AU_KM:.3f} "
        f"-> {np.linalg.norm(d2['E'])*AU_KM:.3f} km")
    DATA["C5"] = {"before": d0, "up": d1, "down": d2,
                  "res_up": rU, "res_up_flipped": rUflip,
                  "res_down": rD, "res_down_flipped": rDflip, "down_branch": branch,
                  "step_up_km": step_pred * AU_KM, "d_radius_up_km": dr_up,
                  "cos_backray_radial": cos_rad, "ray_s_km": s * AU_KM,
                  "ground_used_km": g * AU_KM}

    # ---------------- leg 5: setFreeMode(false), twice, after a flight ------
    trav = []
    cur = d2
    for i in (1, 2):
        P = np.array(cur["position"])
        lam2, phi2, d2v = leave_free(P)
        pred_off = zrot(cur["theta"]) @ pB(lam2, phi2, d2v)
        app.cmd("camera action free_mode state off", 1.5)
        off = app.dump(f"free_off_{i}")
        Eo = np.array(off["E"])
        r_off = float(np.linalg.norm(Eo - pred_off))
        r_hold = float(np.linalg.norm(Eo - np.array(cur["E"])))
        chk(r_off < POSE_FLOOR and r_off < r_hold / 100,
            f"C6 exit #{i}: setFreeMode(false) lands at Z(theta).pB of A^-1(position)",
            f"res_A^-1 = {r_off:.3e} AU vs res(no-move) = {r_hold:.3e} AU ; "
            f"exit teleport {np.linalg.norm(Eo - np.array(cur['E']))*AU_KM:.1f} km")
        # re-enter from the state the exit produced
        pred_on = -zrot(off["theta"]) @ pA(off["lon"], off["lat"], off["dist"])
        app.cmd("camera action free_mode state on", 1.5)
        on = app.dump(f"free_on_{i}")
        r_on = float(np.linalg.norm(np.array(on["E"]) - pred_on))
        chk(r_on < POSE_FLOOR,
            f"C3 entry #{i+1}: predicted from the state the previous exit produced",
            f"res = {r_on:.3e} AU ; teleport "
            f"{np.linalg.norm(np.array(on['E'])-Eo)*AU_KM:.1f} km")
        trav.append({"exit": off, "entry": on, "res_exit": r_off,
                     "res_exit_rival": r_hold, "res_entry": r_on,
                     "exit_km": float(np.linalg.norm(Eo - np.array(cur["E"])) * AU_KM),
                     "entry_km": float(np.linalg.norm(np.array(on["E"]) - Eo) * AU_KM)})
        # fly between the pairs so the second traverse is not an A/A^-1 identity
        app.cmd("camera action descend coef 1.1", 1.2)
        cur = app.dump(f"fly_{i}")
    DATA["C6"] = trav

    # ---------------- leg 6: the terminal observable, with the body in frame -
    # The verification height is the composed screen (F33's own pattern): three
    # shots around ONE toggle pair.  free->anchored is the A/B; free->free is
    # the A/A control, and it is also the "the pair is inert" witness on the
    # screen instead of in a number.  The reference is in frame here (the view
    # was aimed at it in leg 4), so the teleport has something to be seen on.
    # RE-AIM.  Leg 4 aimed the view, but the C6 traverses then teleported the
    # observer ~17 000 km around the reference while HOLDING the absolute
    # orientation (A38) — so by here the reference is out of frame again, and a
    # frame of stars witnesses a translation with 231 px (measured).  Aiming
    # again puts the content on the surface the comparison is made on.
    app.cmd("flag track_object on", 6.0)
    app.cmd("flag track_object off", 1.5)
    sh0 = app.shot("scr_free_1")
    s_free1 = app.dump("scr_free_1")
    app.cmd("camera action free_mode state off", 1.5)
    sh1 = app.shot("scr_anchored")
    s_anch = app.dump("scr_anchored")
    app.cmd("camera action free_mode state on", 1.5)
    sh2 = app.shot("scr_free_2")
    s_free2 = app.dump("scr_free_2")
    px_ab, px_ctl = px(sh0, sh1), px(sh0, sh2)
    d_pair = float(np.linalg.norm(np.array(s_free2["E"]) - np.array(s_free1["E"])))
    # The MAGNITUDE here is small and the dump says exactly why: the reference's
    # own `screen` is (1.8, 0.6), outside the [0,1] viewport, so the frame is
    # stars and distant planets — and a star at infinity is invariant under a
    # 16 700 km translation.  The claim this leg carries is therefore the
    # DIRECTION of the two comparisons, not a size: the toggle changes the
    # frame, the pair restores it exactly.  The magnitude witness is leg 7.
    chk(px_ab > px_ctl and px_ctl == 0 and d_pair < 1e-11,
        "SCREEN: the toggle moves the composed frame; the toggle PAIR does not",
        f"free->anchored {px_ab} px>8 ; free->free (A/A) {px_ctl} px>8 ; "
        f"pair returns to {d_pair:.3e} AU")
    DATA["SCREEN"] = {"px_ab": px_ab, "px_pair": px_ctl, "pair_dE_AU": d_pair,
                      "teleport_km": float(np.linalg.norm(
                          np.array(s_anch["E"]) - np.array(s_free1["E"])) * AU_KM)}

    # ---------------- leg 7: the SHIPPED place, on the surface --------------
    # The leg above is weak on purpose-free grounds and the dump says why: the
    # reference's own `screen` is (1.8, 0.6), i.e. off the [0,1] viewport, so
    # the frame is stars and distant planets and a 16 700 km translation moves
    # a star at infinity by nothing.  Standing ON the surface the body fills the
    # lower dome, so the teleport has to show.  This also re-measures §5.80's
    # OWN headline case (the shipped Marseille place, 43d18' N / 5d22' E, 75 m)
    # against a prediction: swing 124.68 deg, chord 2*d*sin(swing/2).
    # AND AT THE SHIPPED DEFAULTS.  With the atmosphere and the landscape off,
    # the reference is not on the composed screen even from 75 m — MEASURED:
    # Earth's own `screen` is (1.887, 0.184) anchored and (-0.666, 0.240) free,
    # both outside the [0,1] viewport, because the body's CENTRE is behind an
    # observer standing on it, and with the atmosphere off nothing else in the
    # lower dome is drawn (a separate observation, recorded not chased).  The
    # atmosphere is exactly the content the teleport acts on — 11 300 km of
    # ground track moves the Sun's position in the observer's local sky, i.e.
    # day into night — so this leg restores the shipped flags for the shot and
    # puts them back afterwards.
    app.cmd("camera action free_mode state off", 1.5)
    LAM2, PHI2, ALT2 = 5.0 + 22.0 / 60.0, 43.0 + 18.0 / 60.0, 75.0
    app.cmd(f"moveto lat {PHI2} lon {LAM2} alt {ALT2} duration 0", 1.5)
    app.cmd("flag atmosphere on", 1.0)
    app.cmd("flag landscape on", 6.0)      # the module faders are per-FRAME and
    # keep ramping under a frozen simulation clock, so the control below is
    # taken adjacent in time, at these flags, with no command between the shots
    # (a first attempt read 10 273 px of residual fade as if it were an effect).
    sh_ctl1 = app.shot("ship_ctl1")
    sh_ctl2 = app.shot("ship_ctl2")
    px_settled = px(sh_ctl1, sh_ctl2)
    sh_s0 = app.shot("ship_anchored")
    s_s0 = app.dump("ship_anchored")
    app.cmd("camera action free_mode state on", 1.5)
    sh_s1 = app.shot("ship_free")
    s_s1 = app.dump("ship_free")
    app.cmd("camera action free_mode state off", 1.5)
    sh_s2 = app.shot("ship_anchored_2")
    s_s2 = app.dump("ship_anchored_2")
    lam2, phi2 = math.radians(LAM2), math.radians(PHI2)
    sw2_pred = swing_signed(lam2, phi2)
    E_s0, E_s1 = np.array(s_s0["E"]), np.array(s_s1["E"])
    sw2 = ang(E_s0, E_s1)
    chord = float(np.linalg.norm(E_s1 - E_s0) * AU_KM)
    chord_pred = 2 * float(np.linalg.norm(E_s0)) * math.sin(sw2_pred / 2) * AU_KM
    px_s, px_s_ctl = px(sh_s0, sh_s1), px(sh_s0, sh_s2)
    chk(abs(sw2 - sw2_pred) < ANG_FLOOR and abs(chord - chord_pred) < 1.0,
        "SHIPPED place: the swing and the chord are the predicted ones",
        f"swing {math.degrees(sw2):.4f} deg (pred {math.degrees(sw2_pred):.4f}); "
        f"chord {chord:.1f} km (pred {chord_pred:.1f})")
    app.cmd("flag atmosphere off", 0.8)
    app.cmd("flag landscape off", 0.8)
    # The A/A floor at these flags is MEASURED, not assumed to be zero: with the
    # atmosphere and the landscape on, per-frame faders keep moving under a
    # frozen simulation clock (`ModularBody::deltaTime` is wall-clock), so two
    # shots with no command between them already differ.  The leg is legal
    # because that floor is tiny against the effect, and the round trip lands
    # ON the floor rather than above it — both stated in the check.
    # NOT "the round trip is <= the floor": the round-trip pair spans two more
    # commands and one more shot than the adjacent A/A pair, so it accumulates
    # a little more of the same drift (measured 782 against a 753 floor, 3.9 %
    # apart — the same floor, not an effect).  The claim is that BOTH sit far
    # below the effect, which is what licenses reading the effect off the frame.
    chk(px_s > 1000 and px_settled < px_s / 100 and px_s_ctl < px_s / 100,
        "SHIPPED place SCREEN (shipped flags): the toggle moves the frame, "
        "the PAIR returns it",
        f"anchored->free {px_s} px>8 ; the round trip back {px_s_ctl} px>8, "
        f"the adjacent A/A floor {px_settled} px>8 — both {px_s/max(px_s_ctl,1):.0f}x "
        f"and {px_s/max(px_settled,1):.0f}x below the effect")
    DATA["SHIPPED"] = {"lon_deg": LAM2, "lat_deg": PHI2, "alt_m": ALT2,
                       "swing_meas": sw2, "swing_pred": sw2_pred,
                       "chord_km": chord, "chord_pred_km": chord_pred,
                       "px_ab": px_s, "px_pair": px_s_ctl,
                       "px_settled_control": px_settled,
                       "pair_dE_AU": float(np.linalg.norm(np.array(s_s2["E"]) - E_s0))}

    res = {"checks": CHECKS, "data": DATA,
           "failures": [c for c in CHECKS if not c["ok"]],
           "commands": app.log}
    (out / "f34_result.json").write_text(json.dumps(res, indent=1, default=str))
    n_ok = sum(1 for c in CHECKS if c["ok"])
    print(f"\nf34_convention: {n_ok}/{len(CHECKS)} checks OK", flush=True)
    sess.stop(drv)
    return 0 if n_ok == len(CHECKS) else 1


if __name__ == "__main__":
    sys.exit(main())
