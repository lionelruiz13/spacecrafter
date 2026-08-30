#!/usr/bin/env python3
"""F14 / §5.28 — where does the IAU prime meridian land ON THE TEXTURE?

    cd claude/harness && DISPLAY=:2 ./f14_meridian.py <outdir> <tag> expect=0.50

WHY THIS INSTRUMENT EXISTS. §11.113(a) (D22) states the requirement its own
answer forces: *the correcting gate must anchor on the TEXTURE, never on the
conversion's own axis*. The pre-existing gate (`b14_w0_analyze.py`) reconstructs
"the rendered meridian" as `zrotation(axisRotation + pi/2) . x_hat` — the very
expression `resolveRotationFrame` solves against — so it confirms the conversion
against itself and is blind to a whole-body 90 deg rotation by construction
(§5.28's recorded root). This script never writes that expression. It asks, at
the RENDER's own matrices and at the PIXELS:

  (1) TEXTURE COLUMN of the IAU prime meridian, `u_pm`.
      mesh -> ecliptic-root is `tilt . spin`, both DUMPED (dumpHops); the IAU
      prime-meridian direction at the dumped jd is built offline from the file's
      pole + W0 + the rate implied by `rot_periode` (§11.86/§11.88 provenance,
      no new value). Expressing one in the other and reading the texcoord back
      through the mesh's OWN mapping gives the column the IAU meridian is drawn
      on. The mesh mapping is `u = theta/360 - 0.25` [observed:
      ojmModule/SphereObjL.cpp:153 — `p.tex[0] = i/10.f - 0.25f` against
      `pos = (cos(i*2pi/10), sin(i*2pi/10))`], and the ray-march regime agrees
      exactly [observed: meshModules/LayeredMesh.cpp:252 removes the +pi/2 and
      shaders/src/bodyRayMarch.frag:83-84 adds +0.5 => same u].
      GATE: u_pm == 0.50 (image centre) once the conversion is right; it reads
      0.75 (mesh x_hat) under the defect. `expect=` selects which, so this file
      is RED on the pre-fix binary rather than merely silent.

  (2) THE PIXELS AGREE WITH (1)'s MODEL. A dump-level identity is not a render.
      The sub-observer texture column `u_sub` is computed from the SAME dumped
      matrices, then an orthographic sphere is synthesised from the body's OWN
      texture file and correlated against the live screenshot over a full 360
      deg scan of a rotation `delta` about the pole. The scan must peak at
      delta = 0 and must REJECT the +-90 deg alternative — that rejection is
      what makes the screen leg discriminating for exactly this defect class.

  (3) THE TEXTURE'S OWN REGISTRATION, restated as a measurement (§11.101(b)):
      the equatorial dark-centroid `u` of `iapetus.png` (circular statistic).
      0.25 is where an IAU-registered map of a synchronous moon puts the leading
      hemisphere (90W = 270E) IF the image centre is longitude 0 — i.e. the
      u = 0.5 convention D22(i) ratifies. This is the premise (1)'s target rests
      on; it is measured here, not recalled.

WHAT IT IS NOT. It does not check the fetched W0 VALUES (§11.86 does, from the
cited pck00011 constants) — only the axis they are converted onto. It reads no
value it does not either measure or take from the loaded data file.

SCENES. One fresh launch, three scenes — the only three landed moons carrying a
real map. Observer on the PRIMARY (never on the moon: a surface-bound observer
co-rotates with its reference, so the sub-observer texture column would be
invariant under the very spin phase this measures — the scene would be blind).
jd per moon chosen for a lit disc (phase scan, recorded in §11.120).
"""

import json, math, os, re, socket, subprocess, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
SC_BIN = os.environ.get("SC_BIN", str(HERE.parents[1] / "build-claude/src/spacecrafter"))
USERDIR = Path.home() / ".spacecrafter"
SSYSTEM = USERDIR / "ssystem.ini"
TEXDIR = USERDIR / "textures"
J2000 = 2451545.0
d2r = math.pi / 180.0

# parent, moon, fov(deg), jd  — jd from the lit-phase scan (§11.120(b))
SCENES = [
    ("Saturn",  "Iapetus",  0.10, 2451584.66500),
    ("Jupiter", "Amalthea", 0.15, 2451545.24900),
    ("Neptune", "Proteus",  1.00, 2451545.70125),
]

FAILS = []
# Gate 1's own measured value, kept so the run's artifact carries it instead of
# only the failure message (§11.167(i): this value existed NOWHERE in the repo,
# which is why its margin reads "unknown" in the photometric-baseline census).
# Observation only - nothing reads this dict, no gate consults it. Module level,
# like FAILS, because the gate lives in run_scenes and the artifact is written
# in main; it therefore stays EMPTY in `reuse` mode, where no capture is taken
# and the gate does not run.
GATE1_LIT_PX_GT40 = {}
def fail(m): FAILS.append(m); print(f"FAIL: {m}", flush=True)
def ok(m): print(f"ok:   {m}", flush=True)


# ----------------------------------------------------------------- data (I2)
def ini_bodies():
    """{name.lower(): {key: value}} from the LOADED ssystem.ini (ISO-8859).

    The loaded file is the authority on what the app rendered; nothing here is
    hardcoded, so a data correction propagates into the gate instead of
    desynchronising it (§11.88(f) baseline-shift class)."""
    out, cur = {}, None
    for raw in SSYSTEM.read_text(encoding="iso-8859-1").splitlines():
        s = raw.strip()
        if s.startswith("["):
            cur = {}
            out[s.strip("[]").strip().lower()] = cur
        elif "=" in s and cur is not None:
            k, v = s.split("=", 1)
            cur[k.strip()] = v.strip()
    return out


# ----------------------------------------------------------------- geometry
def Ax(a): c, s = math.cos(a), math.sin(a); return np.array([[1,0,0],[0,c,-s],[0,s,c]])
def Az(a): c, s = math.cos(a), math.sin(a); return np.array([[c,-s,0],[s,c,0],[0,0,1]])
# mat_j2000_to_vsop87 (tools/vecmath / ModularSystem's own constant)
M_J2VSOP = Ax(-23.4392803055555555556 * d2r) @ Az(0.0000275 * d2r)

def s2r(l, b):
    return np.array([math.cos(l)*math.cos(b), math.sin(l)*math.cos(b), math.sin(b)])

def R3(m):
    """3x3 rotation out of a dumped column-major Mat4f."""
    return np.array([[m[j*4 + i] for j in range(3)] for i in range(3)])

def mesh_u(v3):
    """Texture column of a direction expressed in MESH coordinates.
    u = theta/360 - 0.25 [observed: SphereObjL.cpp:153]."""
    return (math.atan2(v3[1], v3[0]) / (2*math.pi) - 0.25) % 1.0

def mesh_v(v3):
    """Texture row coordinate; v = 1 at the north pole [observed:
    SphereObjL.cpp:148 north pole tex (0.5,1); bodyRayMarch.frag:124
    acos(-z/depth)/PI, same value]."""
    n = v3 / np.linalg.norm(v3)
    return math.acos(max(-1.0, min(1.0, -n[2]))) / math.pi

def iau_prime_meridian(ra0, de0, W0, Wdot, jd):
    """IAU prime-meridian direction in the ecliptic root frame at jd.
    Node/perp construction identical to §11.86(a)'s closed form — but used here
    only as the REFERENCE the render is measured against, never as the render's
    own axis."""
    n = s2r(ra0*d2r, de0*d2r)
    node = np.array([-math.sin(ra0*d2r), math.cos(ra0*d2r), 0.0])
    perp = np.cross(n, node)
    W = (W0 + Wdot * (jd - J2000)) * d2r
    return M_J2VSOP @ (math.cos(W)*node + math.sin(W)*perp), M_J2VSOP @ n


# ----------------------------------------------------------------- texture
def dark_centroid_u(path, band=0.30):
    """Circular (wrap-aware) intensity-weighted centroid of DARKNESS over an
    equatorial band — §11.101(b)'s instrument, restated so it is reproducible.
    A linear centroid is meaningless on a cyclic coordinate; the circular one
    is what reproduces 0.22."""
    a = np.asarray(Image.open(path).convert("L")).astype(float)
    h, w = a.shape
    prof = a[int(h*(0.5-band/2)):int(h*(0.5+band/2))].mean(axis=0)
    weight = prof.max() - prof
    ang = 2*np.pi*(np.arange(w) + 0.5)/w
    return float((math.atan2((weight*np.sin(ang)).sum(),
                             (weight*np.cos(ang)).sum()) / (2*np.pi)) % 1.0)


def synth(tex, RE, s_mesh, cx, cy, R, shape, delta, vflip, ysign):
    """Orthographic render of the textured sphere as the model says it stands.

    RE = mesh -> eye rotation; the near hemisphere is +z in eye coordinates
    [observed: ModularBody.hpp:421 uses -mat.r[14] as the forward distance, so
    the body sits at negative eye z]. `delta` rotates the TEXTURE about the
    pole; the scan over it is the measurement."""
    H, W = shape
    j, i = np.mgrid[0:H, 0:W]
    x = (i - cx) / R
    y = ysign * (j - cy) / R
    rr = x*x + y*y
    mask = rr <= 1.0
    z = np.sqrt(np.clip(1.0 - rr, 0, None))
    p_eye = np.stack([x, y, z], axis=-1)
    p_mesh = p_eye @ RE                       # RE.T @ v, batched
    # `delta` turns the MAP on the sphere and nothing else: the illumination
    # geometry lives in space and does not move when the meridian convention
    # does, so the Lambert term keeps the unrotated normal. Rotating both would
    # make the scan's peak a fit of two coupled things at once - and the
    # post-fix PREDICTION is exactly synth(delta = -90 deg) only under this
    # separation [derived: offset += 90 deg => RE_post = RE_pre . Rz(pi/2) =>
    # p_mesh_post = Rz(-pi/2) p_mesh_pre, while s_mesh rotates with it, leaving
    # p_mesh . s_mesh invariant].
    q = p_mesh
    if delta:
        c, s = math.cos(delta), math.sin(delta)
        q = np.stack([c*p_mesh[..., 0] - s*p_mesh[..., 1],
                      s*p_mesh[..., 0] + c*p_mesh[..., 1],
                      p_mesh[..., 2]], axis=-1)
    u = (np.arctan2(q[..., 1], q[..., 0]) / (2*np.pi) - 0.25) % 1.0
    v = np.arccos(np.clip(-q[..., 2], -1, 1)) / np.pi
    th, tw = tex.shape
    col = np.clip((u*tw).astype(int), 0, tw-1)
    vv = (1.0 - v) if vflip else v
    row = np.clip((vv*th).astype(int), 0, th-1)
    lam = np.clip(p_mesh @ s_mesh, 0, None)
    return np.where(mask, tex[row, col], 0.0), lam, mask


def ncc(a, b, m):
    x, y = a[m], b[m]
    if x.size == 0:
        return 0.0
    x = x - x.mean(); y = y - y.mean()
    n = np.sqrt((x*x).sum() * (y*y).sum())
    return float((x*y).sum()/n) if n > 0 else 0.0


def _box(a, r):
    """Separable box blur, radius r, zero-padded (numpy only - the harness's
    dependency set is numpy + PIL and this file does not widen it)."""
    for ax in (0, 1):
        c = np.cumsum(np.concatenate(
            [np.zeros_like(a.take([0], axis=ax)), a], axis=ax), axis=ax)
        n = a.shape[ax]
        hi = np.clip(np.arange(n) + r + 1, 0, n)
        lo = np.clip(np.arange(n) - r, 0, n)
        a = (np.take(c, hi, axis=ax) - np.take(c, lo, axis=ax))
    return a


def highpass(a, m, r=7):
    """a minus its LOCAL mean over the mask (3 box passes ~ gaussian sigma 8 px
    at the 256-px working size, i.e. ~0.065 disc radii).

    WHY: correlating the albedo maps directly left the +-90 deg alternative at
    0.44 against 0.54 at the truth on Iapetus [measured 2026-07-30] - the broad
    hemispheric brightness pattern is nearly rotation-symmetric, so it carries
    almost no longitude information while dominating the sum. The crater-scale
    detail does carry it: after this filter the same scene reads 0.44 against
    0.00/0.03/-0.07. The filter is applied identically to observation and
    model, and is mask-normalised so the disc edge does not leak in."""
    w = m.astype(float)
    num, den = np.where(m, a, 0.0), w
    for _ in range(3):
        num, den = _box(num, r), _box(den, r)
    return np.where(m, a - num/np.where(den > 1e-9, den, 1.0), 0.0)


# ----------------------------------------------------------------- app driver
def wait_port(proc, timeout=90):
    t0 = time.time()
    while time.time() - t0 < timeout:
        if proc.poll() is not None:
            raise RuntimeError("app died before opening its port")
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2); sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)


def run_scenes(out, tag):
    """One fresh launch, three scenes. Returns [(moon, png, dump, jd)]."""
    applog = out / f"f14_{tag}.applog"
    proc = subprocess.Popen([SC_BIN], cwd=str(USERDIR),
                            stdout=open(applog, "w"), stderr=subprocess.STDOUT,
                            env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
    sock = wait_port(proc)
    time.sleep(10)                       # async loads settle, startup script ends
    send(sock, "timerate rate 0", 1)     # FIRST, before the epoch (b19 lesson)
    for f in ("atmosphere", "landscape", "fog", "star_lines", "constellation_art",
              "cardinal_points", "planet_names", "nebula_names"):
        send(sock, f"flag {f} off", 0.4)
    made = []
    for parent, moon, fov, jd in SCENES:
        send(sock, f"set home_planet {parent}", 4)
        send(sock, f"select planet {moon} pointer off", 2)
        send(sock, "flag track_object on", 4)
        send(sock, f"zoom fov {fov} duration 0", 3)
        send(sock, f"date jday {jd:.5f}", 3)
        # SETTLE. Measured 2026-07-30: a large jd jump leaves the frame BLACK
        # for several seconds (luminance adaptation / faders - the §11.118 "6 s"
        # lesson); a shot at +3 s returned max grey 10 on a disc that reads 209
        # at +11 s. A too-early shot is not noise, it is a null result that
        # looks like data, so the emptiness check below is a FAILURE too.
        time.sleep(8)
        png = out / f"f14_{tag}_{moon}.png"
        dump = out / f"f14_{tag}_{moon}.json"
        png.unlink(missing_ok=True); dump.unlink(missing_ok=True)
        send(sock, f"body action screenshot filename {png}", 2.5)
        send(sock, f"body action dual_dump filename {dump}", 2.5)
        made.append((moon, png, dump, jd))
    send(sock, "shutdown action now", 1)
    sock.close()
    try:
        rc = proc.wait(timeout=40)
    except subprocess.TimeoutExpired:
        proc.kill(); rc = -1
        fail(f"{tag}: the app did not exit within 40 s")
    # F12's guard (§11.118(c)2): a leg that measured nothing because the app
    # refused a command must be a FAILURE, not a silent zero.
    txt = Path(applog).read_text(errors="replace")
    unknown = sorted(set(re.findall(r"[\w:]+ is unknown\. Did you mean [\w:]+ \?", txt)))
    if unknown:
        fail(f"{tag}: the app REJECTED a command - {unknown}")
    for moon, png, dump, _ in made:
        if not png.exists():
            fail(f"{tag}/{moon}: no screenshot"); continue
        if not dump.exists():
            fail(f"{tag}/{moon}: no dump"); continue
        lit = int((np.asarray(Image.open(png).convert("L")) > 40).sum())
        GATE1_LIT_PX_GT40[moon] = lit        # RECORD (observation only, F54)
        if lit < 20000:
            fail(f"{tag}/{moon}: only {lit} px > 40 in the capture - the body did "
                 f"not reach the frame (settle/phase), so this leg measured nothing")
    return made, rc


def read_dump(path, moon):
    body = hops = jd = None
    for line in open(path):
        line = line.strip().rstrip(",")
        if not line:
            continue
        try:
            o = json.loads(re.sub(r'(?<=:)\s*(-?)(nan|inf)\b',
                                  lambda m: m.group(1) + ("NaN" if m.group(2) == "nan" else "Infinity"),
                                  line))
        except json.JSONDecodeError:
            continue
        if o.get("type") == "header": jd = o["jd"]
        elif o.get("type") == "hops" and o.get("name") == moon: hops = o["new"]
        elif o.get("type") == "body" and o.get("name") == moon: body = o["new"]
    if body is None or hops is None:
        raise RuntimeError(f"{path}: no {moon} body/hops line")
    return jd, body, hops


# ----------------------------------------------------------------- analysis
def analyse(moon, png, dump, ini, expect):
    jd, body, hops = read_dump(dump, moon)
    row = ini[moon.lower()]
    ra0 = float(row["rot_pole_ra"]); de0 = float(row["rot_pole_de"])
    W0 = float(row["rot_pole_w0"])
    # Wdot from the file's own rot_periode: rot_periode(h) = 8640/Wdot, the
    # relation §11.88(a) wrote the corrected values BY. Sign-preserving, so a
    # retrograde body keeps its negative rate. No value is introduced here.
    Wdot = 8640.0 / float(row["rot_periode"])
    h = hops[0]
    A = body["axisRot"]
    Rtilt, Rspin = R3(h["tilt"]), R3(h["spin"])
    Rmat = R3(body["mat"]); t = np.array(body["mat"][12:15])
    RE = Rmat @ Az(A + math.pi/2)               # mesh -> eye
    RM2E = Rtilt @ Rspin                         # mesh -> ecliptic root

    res = {"moon": moon, "jd": jd, "offset": h["offset"], "axisRot": A}
    # spin-matrix identity: the dumped spin IS zrotation(axisRot + pi/2)
    res["spin_identity_err"] = float(np.abs(Rspin - Az(A + math.pi/2)).max())

    pm_ecl, pole_ecl = iau_prime_meridian(ra0, de0, W0, Wdot, jd)
    pm_mesh = RM2E.T @ pm_ecl
    res["u_pm"] = mesh_u(pm_mesh)
    res["pole_err_deg"] = math.degrees(math.acos(
        max(-1.0, min(1.0, float((RM2E.T @ pole_ecl)[2])))))

    d_mesh = RE.T @ (-t/np.linalg.norm(t))
    res["u_sub"] = mesh_u(d_mesh)
    res["lat_sub_deg"] = math.degrees(math.asin(np.clip(d_mesh[2], -1, 1)))

    # ---- screen leg ---------------------------------------------------------
    helio = np.zeros(3)
    for hop in hops:
        helio = helio + np.array(hop["ecl"])
    s_mesh = RM2E.T @ (-helio/np.linalg.norm(helio))
    res["sun_lon_mesh"] = mesh_u(s_mesh)

    full = Image.open(png).convert("L")
    W, H = full.size
    tex = np.asarray(Image.open(TEXDIR / ini[moon.lower()]["tex_map"]).convert("L")).astype(float)
    # Model-predicted disc: `screen` is in halfFov units and the fisheye
    # transfer is radial (r = theta/halfFov), so 1.0 == the screen half-height
    # [observed: ModularBody.hpp:412-442, the same f the render uses].
    cxf = W/2 + body["screen"][0]*(H/2)
    cyf = H/2 - body["screen"][1]*(H/2)
    Rf = body["screenSize"]*(H/2)
    # Crop a 4R box around it and work at N px so the delta scan is affordable
    # (the full 2048^2 frame is ~1000x the pixels the measurement needs).
    N, halfbox = 256, max(8.0, 2.0*Rf)
    x0, y0 = cxf - halfbox, cyf - halfbox
    img = np.asarray(full.resize((N, N), Image.BILINEAR,
                                 box=(x0, y0, x0 + 2*halfbox, y0 + 2*halfbox))).astype(float)
    sc = N/(2*halfbox)
    cx0, cy0, R0 = (cxf - x0)*sc, (cyf - y0)*sc, Rf*sc

    def score(cx, cy, R, delta_deg, vflip, ysign):
        """Correlate ALBEDO against ALBEDO, never brightness against brightness.

        The rendered brightness is albedo x shading, and the shading (limb
        darkening + terminator) is by far the larger signal AND is invariant
        under the rotation being measured - correlating raw brightness scored
        the +-90 deg alternatives at 0.91-0.97 against 0.99 at the truth, i.e.
        it nearly could not discriminate the defect [measured 2026-07-30].
        Dividing the observation by the model's Lambert term leaves the map,
        which is the only thing the meridian convention moves."""
        a, lam, m = synth(tex, RE, s_mesh, cx, cy, R, img.shape,
                          delta_deg*d2r, vflip, ysign)
        use = m & (lam > 0.35) & (img > 8)
        obs = img/np.where(lam > 0.05, lam, 1.0)
        return ncc(highpass(obs, use), highpass(a, use), use)

    # The v (north-up) convention of the loaded image and the screen y sign are
    # PROPERTIES OF THE CHAIN, not assumptions: all four combinations are scored
    # and the winner is reported. A disagreement between bodies or between
    # binaries would itself be the finding.
    best = None
    for vflip in (True, False):
        for ysign in (-1.0, 1.0):
            m = max((score(cx0, cy0, R0, dl, vflip, ysign), dl)
                    for dl in range(-180, 180, 4))
            if best is None or m[0] > best[0]:
                best = (m[0], m[1], vflip, ysign)
    _, dl0, vflip, ysign = best
    # refine the disc (centre, radius) at the coarse best, then rescan finely
    cx, cy, R, bestv = cx0, cy0, R0, best[0]
    for dx in (-4, -2, 0, 2, 4):
        for dy in (-4, -2, 0, 2, 4):
            for fr in (0.92, 0.96, 1.0, 1.04, 1.08):
                v = score(cx0+dx, cy0+dy, R0*fr, dl0, vflip, ysign)
                if v > bestv:
                    bestv, cx, cy, R = v, cx0+dx, cy0+dy, R0*fr
    peak = max((score(cx, cy, R, dl0+dl, vflip, ysign), dl0+dl)
               for dl in np.arange(-8, 8.01, 0.5))
    res.update(delta_deg=float(peak[1]), ncc_peak=float(peak[0]),
               vflip=bool(vflip), ysign=float(ysign),
               disc_px=[float(cx), float(cy), float(R)],
               disc_model_full=[float(cxf), float(cyf), float(Rf)],
               crop_scale=float(sc))
    # the +-90 alternative must be REJECTED - that rejection is the whole point
    for alt in (-90.0, 90.0, 180.0, 0.0):
        res[f"ncc_at_{int(alt)}"] = score(cx, cy, R, alt, vflip, ysign)
    _, _, m = synth(tex, RE, s_mesh, cx, cy, R, img.shape, 0.0, vflip, ysign)
    res["lit_px"] = int(((img > 8) & m).sum())

    # ---- gates --------------------------------------------------------------
    du = abs(((res["u_pm"] - expect + 0.5) % 1.0) - 0.5)
    if du <= 5e-4:
        ok(f"{moon}: IAU prime meridian on texture column u = {res['u_pm']:.5f} "
           f"(expected {expect:.2f}, |d| = {du*360:.4f} deg)")
    else:
        fail(f"{moon}: IAU prime meridian on texture column u = {res['u_pm']:.5f}, "
             f"expected {expect:.2f} - off by {du*360:.3f} deg")
    # The screen criterion is a MARGIN, not an absolute correlation: the
    # absolute value depends on how much crater-scale detail the visible face
    # happens to carry (0.44 on Iapetus's bland trailing hemisphere, 0.93 on
    # Amalthea), while the margin over the wrong hypotheses is what says the
    # pixels chose this orientation and not another.
    alt = max(res["ncc_at_-90"], res["ncc_at_90"], res["ncc_at_180"])
    res["margin"] = res["ncc_at_0"] - alt
    if abs(res["delta_deg"]) <= 4.0 and res["ncc_at_0"] >= 0.25 and res["margin"] >= 0.25:
        ok(f"{moon}: pixels agree with the dumped model - screen residual "
           f"{res['delta_deg']:+.2f} deg, NCC {res['ncc_at_0']:.3f} vs "
           f"{res['ncc_at_-90']:+.3f}/{res['ncc_at_90']:+.3f}/{res['ncc_at_180']:+.3f} "
           f"at -90/+90/180 (margin {res['margin']:.3f})")
    else:
        fail(f"{moon}: the pixels do not select the dumped model - residual "
             f"{res['delta_deg']:+.2f} deg, NCC(0) {res['ncc_at_0']:.3f}, "
             f"margin {res['margin']:.3f} over "
             f"{res['ncc_at_-90']:+.3f}/{res['ncc_at_90']:+.3f}/{res['ncc_at_180']:+.3f}")
    if res["pole_err_deg"] > 0.02:
        fail(f"{moon}: rendered pole is {res['pole_err_deg']:.4f} deg off the "
             f"declared IAU pole (§11.69 observable)")
    return res


def main(argv):
    if len(argv) < 3:
        print(__doc__); return 2
    out = Path(argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
    tag = argv[2]
    expect, reuse = 0.50, False
    for a in argv[3:]:
        if a.startswith("expect="):
            expect = float(a.split("=", 1)[1])
        elif a == "reuse":
            reuse = True          # re-analyse an existing capture (no launch)
    ini = ini_bodies()
    md5_in = subprocess.run(["md5sum", str(SSYSTEM), str(USERDIR/"config.ini")],
                            capture_output=True, text=True).stdout
    if reuse:
        made = [(m, out/f"f14_{tag}_{m}.png", out/f"f14_{tag}_{m}.json", jd)
                for _, m, _, jd in SCENES]
        rc = "reused"
    else:
        made, rc = run_scenes(out, tag)
    results = {"tag": tag, "expect": expect, "bin": SC_BIN, "rc": rc,
               "texture_dark_centroid_u": {}, "bodies": []}
    for moon, png, dump, jd in made:
        results["texture_dark_centroid_u"][moon] = dark_centroid_u(
            TEXDIR / ini[moon.lower()]["tex_map"])
        results["bodies"].append(analyse(moon, png, dump, ini, expect))
    md5_out = subprocess.run(["md5sum", str(SSYSTEM), str(USERDIR/"config.ini")],
                             capture_output=True, text=True).stdout
    if md5_in != md5_out:
        fail(f"frozen data changed across the run:\n{md5_in}{md5_out}")
    results["md5"] = md5_in
    results["gate1_lit_px_gt40"] = GATE1_LIT_PX_GT40   # {} in `reuse` mode
    results["fails"] = FAILS
    (out / f"f14_{tag}_results.json").write_text(json.dumps(results, indent=1))
    print(f"\ntexture dark-centroid u (image measurement): "
          f"{ {k: round(v,4) for k,v in results['texture_dark_centroid_u'].items()} }")
    print("RESULT:", "ALL OK" if not FAILS else f"{len(FAILS)} FAILURE(S)")
    return 0 if not FAILS else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
