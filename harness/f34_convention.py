#!/usr/bin/env python3
"""F34 / INTENT §5.80 — WHICH PARAMETRIZATION THE FREE-FLIGHT MOVERS WERE BUILT
AGAINST.  RECORD-ONLY: this script changes nothing and asks the shipped binary.

    cd claude/harness && DISPLAY=:2 ./f34_convention.py <outdir> [--bin B]

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
    the two read 124.68 deg and 55.32 deg — F33 measured 124.8 deg (§11.141(k)),
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
between — 0 AU and 0 px, the §11.141(k) pattern, so a non-zero elsewhere is an
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
    out = Path(a.outdir)
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

    # ---------------- leg 0: A/A control (§11.141(k) pattern) ---------------
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
    chk(px_on > 1000, "C3 screen: the composed frame changes across the toggle",
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
    d1 = m_free["dist"]
    lam1, phi1 = math.radians(LAM1), math.radians(PHI1)
    predA_E1 = -zrot(th1) @ pA(lam1, phi1, d1)
    predB_E1 = zrot(th1) @ pB(lam1, phi1, d1)
    E1 = np.array(m_free["E"])
    rA1, rB1 = float(np.linalg.norm(E1 - predA_E1)), float(np.linalg.norm(E1 - predB_E1))
    chk(rA1 < POSE_FLOOR and rA1 < rB1 / 100,
        "C4 moveTo free branch: lands at -Z(theta).pA of the COMMANDED triple (A)",
        f"res_A = {rA1:.3e} AU vs res_B = {rB1:.3e} AU")
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
    app.cmd("camera action free_mode state on", 1.5)
    d0 = app.dump("desc_before")
    R0, t0 = mat_of(d0["mat"])
    # dE = -Rot^T . v  (derivation in the header).  Rot maps the reference's
    # accumulated-equatorial frame to the eye frame, so Rot^T . z_eye is the
    # eye's BACKWARD ray already expressed in the frame E lives in — no further
    # fold, and in particular no second Z(theta).
    up = np.array(R0[2, :])                               # == Rot^T . (0,0,1)
    COEF_UP = 1.25
    app.cmd(f"camera action descend coef {COEF_UP}", 1.2)
    d1 = app.dump("desc_up")
    dE = np.array(d1["E"]) - np.array(d0["E"])
    # H_composer: dE == +step * up ; H_flipped (position taken as +eye): -that
    upB = up
    cos_c = float(np.dot(dE, upB) / (np.linalg.norm(dE) * np.linalg.norm(upB)))
    step = float(np.dot(dE, upB) / np.linalg.norm(upB))
    chk(cos_c > 0.9999,
        "C5 descend(ascend): dE is +step along the eye's BACKWARD ray (composer)",
        f"cos = {cos_c:+.8f} (H_flipped would be {-cos_c:+.8f}), step = {step*AU_KM:.3f} km")
    grew = np.linalg.norm(d1["E"]) - np.linalg.norm(d0["E"])
    chk(grew > 0, "C5 descend coef>1 ASCENDS (the distance to the reference grows)",
        f"{np.linalg.norm(d0['E'])*AU_KM:.3f} -> {np.linalg.norm(d1['E'])*AU_KM:.3f} km")
    g_implied = float(np.linalg.norm(d0["E"]) - step / (COEF_UP - 1))
    chk(abs(g_implied - DATA["ground"]["altitudeReference_AU"]) / max(g_implied, 1e-12) < 1e-3,
        "C5 the step's own law: step == (coef-1)*(|E| - ground_radius)",
        f"implied ground {g_implied*AU_KM:.3f} km vs altitudeReference "
        f"{DATA['ground']['altitudeReference_AU']*AU_KM:.3f} km")
    r3, r3r = score_state(d1, "free_after_ascend")
    chk(r3 < POSE_FLOOR and r3 < r3r / 100,
        "C2 after a flight the free composer still holds (E == -Z(theta).position)",
        f"res(-) = {r3:.3e} AU vs res(+) = {r3r:.3e} AU")
    COEF_DOWN = 0.8
    app.cmd(f"camera action descend coef {COEF_DOWN}", 1.2)
    d2 = app.dump("desc_down")
    dE2 = np.array(d2["E"]) - np.array(d1["E"])
    fwdB = -upB
    cos_d = float(np.dot(dE2, fwdB) / (np.linalg.norm(dE2) * np.linalg.norm(fwdB)))
    chk(cos_d > 0.9999,
        "C5 descend(descend): dE is along the eye's FORWARD ray (composer)",
        f"cos = {cos_d:+.8f}, |dE| = {np.linalg.norm(dE2)*AU_KM:.3f} km")
    DATA["C5"] = {"before": d0, "up": d1, "down": d2, "cos_up": cos_c,
                  "cos_down": cos_d, "step_km": step * AU_KM,
                  "ground_implied_km": g_implied * AU_KM}

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
