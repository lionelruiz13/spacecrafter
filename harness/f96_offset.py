#!/usr/bin/env python3
"""F96 -- what the B17 view offset does to the NEW path's alt/az readout, the
atmosphere's sun direction, the tracking aim and the view-directed descent
(INTENT §11.216; row §5.138, family §5.128 / B17 / §11.201).

THE MODEL, derived at source and committed to `artifacts/f96/prediction.txt`
BEFORE the first launch (`f96_frame.cpp` is the same derivation in the project's
own Mat4f):

    getObservedPosition()  carries  R' . Rv          [Camera.cpp:139-155, :184]
    observedToLocalPos     divides by  Rv only       [Camera.hpp:260-262]
    =>  the readout direction is  C . d  with  C = Rv^T . R' . Rv,
        a rotation by offset*halfFov about  Rv^T * x_eye  --  27.000 deg at the
        shipped fov 180 with `set zoom_offset 0.3`.

THE CHANNEL is the dual dump's per-body `altaz_old` / `altaz_new`
[observed: ssystem_factory.cpp:1196-1207]: old's alt/az (which carries no
eye-frame content at all, §11.201) is the TRUE local direction, the new path's
is the readout under test, both at ONE frame of ONE launch.  Both are reported
in old's convention (N=0, E=90), so both map into the new path's raw local frame
by `az_raw = pi/2 - az_report` [ModularObject.cpp:196-201, the §11.60 bridge
f91_parity.reconstruct already uses].

stages (each is ONE fresh launch on a private farm):
  cmd      the command channel: `set zoom_offset 0.3` at runtime; the alt/az
           table at offset 0 and at 0.3, the tracking datum at both, the
           Sun-low frame pair
  cfg      the config channel: `[navigation] view_offset = 0.3` in the farm's
           config.ini -- does it arm the new path's offset at startup?
  descend  `camera action descend` (the ONE shipped verb that reaches
           moveEyeRel, coreLink.hpp:959-961) at offset 0.3 in free flight

usage: f96_offset.py <absOutdir> --tag pre|post [--bin PATH]
                                 [--stages cmd,cfg,descend] [--jd-sun JD]
       f96_offset.py <absOutdir> --score            # re-score on disk, no launch
"""
import gzip
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
DEG = 180.0 / math.pi
PORT = 7805
JD = 2461233.5                      # F91's own frame, so the tables are comparable
OFFSET = 0.3                        # the shipped `set zoom_offset 0.3`
TARGET = "Jupiter"                  # the tracked body (F81's teleport leg used it)
FARM_ROOT = Path(os.environ.get("F96_FARM", "/home/claude/sc-f96/farm"))
REAL_HOME = Path.home() / ".spacecrafter"

FAILS, NOTES = [], []
def fail(m): FAILS.append(m); print("FAIL: " + m, flush=True)
def ok(m):   print("ok:   " + m, flush=True)
def note(m): NOTES.append(m); print("NOTE: " + m, flush=True)


# ------------------------------------------------------------------ 3x3 algebra
# Mat4f is COLUMN-MAJOR and `multiplyWithoutTranslation` computes the plain
# product v' = M.v [observed: vecmath.hpp:1879-1884]; xrotation/zrotation are
# the conventional counter-clockwise rotations [vecmath.hpp:1686-1696,
# :1783-1793].  These four helpers are therefore a SECOND, independently
# written implementation of what f96_frame.cpp computes with the engine's own
# type -- the two are scored against each other (control C0).
def mat_mul(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(3)) for j in range(3)]
            for i in range(3)]


def mat_t(a):
    return [[a[j][i] for j in range(3)] for i in range(3)]


def mat_vec(a, v):
    return [sum(a[i][k] * v[k] for k in range(3)) for i in range(3)]


def rx(t):
    c, s = math.cos(t), math.sin(t)
    return [[1, 0, 0], [0, c, -s], [0, s, c]]


def rz(t):
    c, s = math.cos(t), math.sin(t)
    return [[c, -s, 0], [s, c, 0], [0, 0, 1]]


def view_rotation(cam):
    """Camera::viewRotation() [Camera.cpp:93-104] with fold() [:86-91]."""
    m = mat_mul(rz(cam["heading"] + math.pi), rx(math.pi / 2 - cam["alt"]))
    m = mat_mul(m, rz(cam["az"] - math.pi / 2))
    if cam["mount"] == "equatorial" and not cam["freeMode"]:
        # fold() reads `foldLat`, which update() sets to `latitude` at the end
        # of every frame [Camera.cpp:639]; the dump is taken between frames.
        m = mat_mul(m, rx(math.pi / 2 - cam["latitude"]))
    return m


def conj(cam, offset_eff, sign=1, swap=False):
    """C = Rv^T . R'(sign) . Rv   (swap=True gives the MUTATED conjugation)."""
    rv = view_rotation(cam)
    rp = rx(sign * offset_eff * cam["halfFov"])
    return (mat_mul(mat_mul(rv, rp), mat_t(rv)) if swap
            else mat_mul(mat_mul(mat_t(rv), rp), rv))


def rot_angle_deg(m):
    tr = m[0][0] + m[1][1] + m[2][2]
    return math.degrees(math.acos(max(-1.0, min(1.0, (tr - 1.0) / 2.0))))


def rot_axis(m):
    a = [m[2][1] - m[1][2], m[0][2] - m[2][0], m[1][0] - m[0][1]]
    n = math.sqrt(sum(x * x for x in a))
    return [x / n for x in a] if n > 0 else a


def angle_between(a, b):
    na = math.sqrt(sum(x * x for x in a))
    nb = math.sqrt(sum(x * x for x in b))
    if na == 0 or nb == 0:
        return float("nan")
    c = sum(x * y for x, y in zip(a, b)) / (na * nb)
    return math.degrees(math.acos(max(-1.0, min(1.0, c))))


def dir_from_altaz(alt, az_report):
    """The reported (alt, az) pair -> a unit vector in the NEW path's raw local
    frame.  `az_raw = pi/2 - az_report` is §11.60's measured bridge
    [ModularObject.cpp:196-201]; BOTH paths report in that same convention
    [ssystem_factory.cpp:1175]."""
    az_raw = math.pi / 2 - az_report
    return [math.cos(alt) * math.cos(az_raw), math.cos(alt) * math.sin(az_raw),
            math.sin(alt)]


def altaz_from_dir(v):
    n = math.sqrt(sum(x * x for x in v))
    alt = math.asin(max(-1.0, min(1.0, v[2] / n)))
    az_raw = math.atan2(v[1], v[0])
    az = math.fmod(math.pi / 2 - az_raw, 2 * math.pi)
    if az < 0:
        az += 2 * math.pi
    return alt, az


# ------------------------------------------------------------------- dump I/O
def parse_dump(path):
    hdr, bodies = {}, {}
    op = gzip.open if str(path).endswith(".gz") else open
    with op(path, "rt", errors="replace") as fh:
        for line in fh:
            line = line.strip().rstrip(",")
            if not line:
                continue
            try:
                o = json.loads(line)
            except json.JSONDecodeError:
                continue
            if o.get("type") == "header":
                hdr = o
            elif o.get("type") == "body":
                bodies[o["name"]] = o
    return hdr, bodies


def cam_of(h):
    return h["camera"]


def nav_of(h):
    return h["oldView"]["nav"]


CAM_KEYS = ("reference", "tracked", "freeMode", "boundToSurface", "mount",
            "skyLocked", "viewOffset", "viewOffsetTransition", "viewOffsetEff",
            "longitude", "latitude", "distance", "alt", "az", "heading",
            "halfFov")


def cam_fields(h):
    c = cam_of(h)
    return {k: c[k] for k in CAM_KEYS if k in c}


def no_instance():
    hits = []
    for p in Path("/proc").glob("[0-9]*/comm"):
        try:
            if p.read_text().strip() == "spacecrafter":
                hits.append(str(p.parent))
        except OSError:
            pass
    return hits


def ini_set(path, section, key, value):
    """f81_offset.ini_set verbatim: rewrite ONE key in ONE section, byte-safe,
    and FAIL if the key is absent (a silent no-op makes the channel a lie)."""
    text = Path(path).read_bytes().decode("latin-1")
    lines = text.split("\n")
    cur = None
    for i, ln in enumerate(lines):
        s = ln.strip()
        if s.startswith("[") and s.endswith("]"):
            cur = s[1:-1]
            continue
        if cur == section and re.match(r"^\s*" + re.escape(key) + r"\s*=", ln):
            old = ln.split("=", 1)[1].strip()
            lines[i] = "%-31s= %s" % (key, value)
            Path(path).write_bytes("\n".join(lines).encode("latin-1"))
            return old
    raise KeyError("%s/%s not found in %s" % (section, key, path))


def build_farm(farm):
    farm = Path(farm)
    if farm.exists():
        shutil.rmtree(farm)
    r = subprocess.run([str(HERE / "b3_farm.sh"), str(farm)],
                       capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError("b3_farm.sh failed: %s%s" % (r.stdout, r.stderr))
    dst = farm / ".spacecrafter"
    cfg = dst / "config.ini"
    if not (cfg.is_file() and not cfg.is_symlink()):
        raise RuntimeError("farm shape: config.ini is not a real file")
    if not ((dst / "ssystem.ini").is_file() and not (dst / "ssystem.ini").is_symlink()):
        raise RuntimeError("farm shape: ssystem.ini is not a real file")
    return dst


# ---------------------------------------------------------------- app driving
class App:
    def __init__(self, binary, farm, out, name):
        self.binary, self.farm, self.out = str(binary), Path(farm), Path(out)
        self.home = self.farm / ".spacecrafter"
        self.name, self.n = name, 0
        self.sock = self.proc = None

    def start(self):
        hits = no_instance()
        if hits:
            raise RuntimeError("another spacecrafter is running: %s" % hits)
        env = {**os.environ, "HOME": str(self.farm),
               "DISPLAY": os.environ.get("DISPLAY", ":2")}
        self.proc = subprocess.Popen(
            [self.binary], cwd=str(self.home),
            stdout=open(self.out / ("%s.applog" % self.name), "w"),
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
        time.sleep(8)
        return round(time.time() - t0, 2)

    def send(self, cmd, pause=0.7):
        self.sock.sendall((cmd + "\n").encode())
        time.sleep(pause)
        try:
            self.sock.settimeout(0.25)
            self.sock.recv(65536)
        except socket.timeout:
            pass
        finally:
            self.sock.settimeout(None)

    def query(self, cmd, pause=1.0):
        """Send and READ THE ANSWER.  Since §11.135 the reply goes to the
        connection that asked (`io.cpp`), and `ServerSocket::send` puts the NUL
        terminator on the wire (F90's gotcha) -- so the blob is NUL-separated
        and is decoded UTF-8 (F94: the engine writes UTF-8 on the socket)."""
        self.sock.sendall((cmd + "\n").encode())
        time.sleep(pause)
        buf = b""
        try:
            self.sock.settimeout(1.5)
            while True:
                chunk = self.sock.recv(65536)
                if not chunk:
                    break
                buf += chunk
        except socket.timeout:
            pass
        finally:
            self.sock.settimeout(None)
        return [p for p in buf.decode("utf-8", "replace").split("\x00") if p.strip()]

    def dump(self, tag, pause=1.0, keep=True):
        self.n += 1
        p = self.out / "dumps" / ("%s_%s.json" % (self.name, tag))
        p.parent.mkdir(parents=True, exist_ok=True)
        if p.exists():
            p.unlink()
        self.send("body action dual_dump filename %s" % p, pause)
        for _ in range(60):
            if p.exists() and p.stat().st_size > 0:
                break
            time.sleep(0.2)
        if not p.exists():
            raise RuntimeError("dump %s never written" % tag)
        time.sleep(0.3)
        h, b = parse_dump(p)
        if not keep:
            p.unlink()
            sc = Path(str(p) + ".navstr")
            if sc.exists():
                sc.unlink()
        return h, b, p

    def shot(self, name, pause=1.5, timeout=30.0):
        p = self.out / "frames" / ("%s.png" % name)
        p.parent.mkdir(parents=True, exist_ok=True)
        if p.exists():
            p.unlink()
        t0 = time.time()
        self.send("body action screenshot filename %s" % p, pause)
        last = -1
        while time.time() - t0 < timeout:
            if p.exists():
                sz = p.stat().st_size
                if sz > 0 and sz == last:
                    return p
                last = sz
            time.sleep(0.2)
        fail("screenshot %s never completed" % name)
        return None

    def wait_until(self, tag, pred, describe, tries=40, pause=0.7):
        """Settle by MEASUREMENT (F81's helper): re-dump until pred holds; the
        poll count IS the evidence that this was not a sleep."""
        t0 = time.time()
        h = b = None
        for i in range(tries):
            h, b, _ = self.dump("%s_poll%02d" % (tag, i), pause=0.6, keep=False)
            if h and pred(h, b):
                ok("settled %s after %d poll(s), %.1fs" % (describe, i + 1,
                                                           time.time() - t0))
                return True, i + 1, h, b
            time.sleep(pause)
        fail("never settled: %s" % describe)
        return False, tries, h, b

    def settle_scale(self, tries=30):
        """§5.109: wait the Moon's display-scale ramp out BY MEASUREMENT."""
        prev = None
        for i in range(tries):
            _h, b, _p = self.dump("settle%02d" % i, pause=0.6, keep=False)
            m = (b.get("Moon", {}).get("new") or {})
            sc, tg = m.get("scaling"), m.get("scalingTarget")
            if sc is None:
                return None
            if abs(sc - tg) < 1e-4 and prev is not None and abs(sc - prev) < 1e-6:
                ok("scale settled: Moon scaling %.8f == target %.8f (%d probes)"
                   % (sc, tg, i + 1))
                return sc
            prev = sc
        fail("scale never settled")
        return None

    def stop(self):
        try:
            self.send("shutdown action now", 2.0)
        except Exception:                                        # noqa: BLE001
            pass
        for _ in range(40):
            if self.proc.poll() is not None:
                break
            time.sleep(0.5)
        if self.proc.poll() is None:
            self.proc.kill()
        return self.proc.poll()


def readout_setup(app, jd=JD):
    """The READOUT legs draw nothing they need: everything optional off, the
    clock stopped, the experimental path on, F91's own frame."""
    app.send("flag experimental_path on")
    app.send("timerate rate 0")
    app.send("meteors zhr 0")
    app.send("date jday %.9f" % jd, 1.5)


# ------------------------------------------------------------------- the model
def score_table(h, bodies, offset_eff_expected=None):
    """Score the model and its two mutations against the binary's own alt/az."""
    cam = {k: cam_of(h)[k] for k in CAM_KEYS if k in cam_of(h)}
    eff = cam["viewOffsetEff"]
    C = {"model": conj(cam, eff),
         "M1_sign": conj(cam, eff, sign=-1),
         "M2_swap": conj(cam, eff, swap=True)}
    rv = view_rotation(cam)
    out = {"camera": cam, "n": 0, "rows": {},
           "model_angle_deg": rot_angle_deg(C["model"]),
           "model_axis": rot_axis(C["model"]) if eff else None,
           "axis_Rv_T_x": mat_vec(mat_t(rv), [1, 0, 0])}
    for name, rec in sorted(bodies.items()):
        ao, an = rec.get("altaz_old"), rec.get("altaz_new")
        if not ao or not an:
            continue
        if not all(math.isfinite(x) for x in list(ao) + list(an)):
            continue
        d_old = dir_from_altaz(ao[0], ao[1])
        d_new = dir_from_altaz(an[0], an[1])
        row = {"alt_old_deg": ao[0] * DEG, "az_old_deg": ao[1] * DEG,
               "alt_new_deg": an[0] * DEG, "az_new_deg": an[1] * DEG,
               "dalt_deg": abs(ao[0] - an[0]) * DEG,
               "daz_deg": abs(((ao[1] - an[1]) * DEG + 180) % 360 - 180),
               "sep_old_new_deg": angle_between(d_old, d_new)}
        for key, m in C.items():
            row["sep_%s_deg" % key] = angle_between(mat_vec(m, d_old), d_new)
        out["rows"][name] = row
        out["n"] += 1
    return out


def kabsch(pairs):
    """The least-squares rotation taking every a onto its b (SVD-free: the
    3x3 correlation matrix's polar factor by Jacobi on M^T M is overkill for
    three points -- use numpy if it is there, else report None)."""
    try:
        import numpy as np
    except ImportError:                                          # pragma: no cover
        return None
    A = np.array([p[0] for p in pairs], dtype=float)
    B = np.array([p[1] for p in pairs], dtype=float)
    H = A.T @ B
    U, _S, Vt = np.linalg.svd(H)
    d = np.sign(np.linalg.det(Vt.T @ U.T))
    R = Vt.T @ np.diag([1, 1, d]) @ U.T
    return [[float(R[i][j]) for j in range(3)] for i in range(3)]


# ------------------------------------------------------------------ the stages
def body_pos(rec):
    """The NEW path's eye-frame position of a body: `mat`'s translation
    [observed: ModularBody.cpp:897-910, the dumped `mat` is what
    getObservedPosition() returns]."""
    m = (rec.get("new") or {}).get("mat")
    return None if not m or len(m) < 15 else [m[12], m[13], m[14]]


def freshness(b_a, b_b, C_pos=None, rel=1e-5):
    """Partition the bodies by whether their eye-frame position was RE-EVALUATED
    between two dumps.  Measured, never assumed: a body whose `mat` translation
    is BYTE-IDENTICAL across a change of camera state was not re-evaluated (its
    readout answers in the state it was last evaluated in); one whose position
    moved was.  When `C_pos` is given (the offset's own rotation R'), a
    re-evaluated body must land on `C_pos . p_a` exactly -- which is what says
    the two dumps differ by the OFFSET and by nothing else."""
    out = {"frozen": [], "moved": [], "matches_R": [], "mismatch_R": [], "n": 0}
    for name in sorted(set(b_a) & set(b_b)):
        pa, pb = body_pos(b_a[name]), body_pos(b_b[name])
        if pa is None or pb is None:
            continue
        out["n"] += 1
        if pa == pb:
            out["frozen"].append(name)
            continue
        out["moved"].append(name)
        if C_pos is not None:
            want = mat_vec(C_pos, pa)
            err = math.sqrt(sum((x - y) ** 2 for x, y in zip(want, pb)))
            scale = math.sqrt(sum(x * x for x in pa)) or 1.0
            (out["matches_R"] if err / scale < rel else
             out["mismatch_R"]).append([name, err / scale])
    return out


def ab_model(h_a, b_a, h_b, b_b):
    """THE A/B THAT NEEDS NO OLD PATH: one camera state, the offset the only
    thing that changes.  For a re-evaluated body the readout must turn by
    C = Rv^T . R' . Rv exactly; for a frozen one it must not turn at all."""
    cam = {k: cam_of(h_b)[k] for k in CAM_KEYS if k in cam_of(h_b)}
    eff = cam["viewOffsetEff"]
    C = conj(cam, eff)
    C1 = conj(cam, eff, sign=-1)
    C2 = conj(cam, eff, swap=True)
    rp = rx(eff * cam["halfFov"])
    fr = freshness(b_a, b_b, C_pos=rp)
    rows = {}
    for name in fr["moved"] + fr["frozen"]:
        aa, ab = b_a[name].get("altaz_new"), b_b[name].get("altaz_new")
        ao = b_b[name].get("altaz_old")
        if not aa or not ab:
            continue
        da, db = dir_from_altaz(*aa), dir_from_altaz(*ab)
        row = {"class": "frozen" if name in fr["frozen"] else "moved",
               "sep_ab_deg": angle_between(da, db),
               "sep_model_deg": angle_between(mat_vec(C, da), db),
               "sep_M1_sign_deg": angle_between(mat_vec(C1, da), db),
               "sep_M2_swap_deg": angle_between(mat_vec(C2, da), db)}
        if ao:
            row["sep_old_new_b_deg"] = angle_between(dir_from_altaz(*ao), db)
        rows[name] = row
    return {"camera": cam, "freshness": fr, "rows": rows,
            "model_angle_deg": rot_angle_deg(C),
            "model_axis": rot_axis(C) if eff else None,
            "axis_Rv_T_x": mat_vec(mat_t(view_rotation(cam)), [1, 0, 0])}


def stage_cmd(binp, out, rep, jd_sun=None):
    """The COMMAND channel: `set zoom_offset 0.3` at runtime, with the camera
    HELD -- the offset is the only thing that changes between A0 and A3."""
    farm = FARM_ROOT / "cmd"
    home = build_farm(farm)
    rep["cmd"] = leg = {"farm": str(farm), "channel": "command"}
    app = App(binp, farm, out, "cmd")
    try:
        leg["tcp_seconds"] = app.start()
        readout_setup(app)
        app.settle_scale()

        # --- P0: the launch state, before ANY camera move.  Its only role is
        # the freshness control below: it is the last frame in which every
        # body's eye-frame position was evaluated in the camera state the
        # readout is then taken in.
        hP, bP, pP = app.dump("p0_launch")
        leg["camera_p0"] = cam_fields(hP)
        leg["table_p0"] = score_table(hP, bP)
        leg["dump_p0"] = str(pP)
        ok("launch state: viewOffsetEff = %s, %d bodies carry both alt/az"
           % (leg["camera_p0"]["viewOffsetEff"], leg["table_p0"]["n"]))

        # --- arm the offset the way F81 measured it (README F81): `select
        # planet` + `flag track_object on` is Core::setFlagTracking(true), the
        # ONE arm site wired to BOTH paths.  `zoom center on` arms old only.
        # This is also a CAMERA MOVE, which is what the freshness control needs.
        app.send("select planet %s" % TARGET, 1.0)
        app.send("flag track_object on", 2.0)
        app.wait_until("arm", lambda H, B: (
            nav_of(H)["plans"]["flagAutoMove"] == 0
            and abs(nav_of(H)["viewOffsetTransition"] - 1.0) < 1e-6
            and abs(cam_of(H)["viewOffsetTransition"] - 1.0) < 1e-6),
            "view_offset_transition == 1 on BOTH paths, auto-move complete")
        h, b, _p = app.dump("track_o0", keep=False)
        leg["track_o0"] = track_datum(h, b)
        app.send("flag track_object off", 1.5)
        app.wait_until("hold0", lambda H, B: nav_of(H)["flagTraking"] == 0
                       and nav_of(H)["plans"]["flagAutoMove"] == 0,
                       "tracking released, view held (offset still 0)")

        # --- A0: the SAME camera state the A/B is taken in, offset still 0
        hA, bA, pA = app.dump("a0_off0")
        leg["camera_a0"] = cam_fields(hA)
        leg["table_a0"] = score_table(hA, bA)
        leg["dump_a0"] = str(pA)
        # THE FRESHNESS CONTROL, at offset 0 on BOTH sides: what a camera move
        # alone does to a body's eye-frame position.
        leg["freshness_move"] = freshness(bP, bA)
        ok("freshness across the camera move (offset 0 both sides): %d frozen, "
           "%d re-evaluated, of %d"
           % (len(leg["freshness_move"]["frozen"]),
              len(leg["freshness_move"]["moved"]), leg["freshness_move"]["n"]))

        # --- A3: the offset, and nothing else
        app.send("set zoom_offset %g" % OFFSET, 1.5)
        got, _n, h, b = app.wait_until("o3", lambda H, B: (
            abs(cam_of(H)["viewOffsetEff"] - OFFSET) < 1e-6
            and nav_of(H)["plans"]["flagAutoMove"] == 0),
            "viewOffsetEff == %g and the aim settled" % OFFSET)
        leg["armed"] = got
        h1, b1, p1 = app.dump("a3_off03_a")
        h2, b2, p2 = app.dump("a3_off03_b")
        leg["camera_a3"] = cam_fields(h1)
        leg["camera_stable"] = (cam_fields(h1) == cam_fields(h2))
        (ok if leg["camera_stable"] else fail)(
            "the camera state is IDENTICAL across two consecutive dumps "
            "(the model reads the state the readout was taken in)")
        held = {k: v for k, v in leg["camera_a0"].items()
                if k not in ("viewOffset", "viewOffsetTransition", "viewOffsetEff")}
        held3 = {k: v for k, v in leg["camera_a3"].items()
                 if k not in ("viewOffset", "viewOffsetTransition", "viewOffsetEff")}
        leg["camera_held_across_ab"] = (held == held3)
        (ok if leg["camera_held_across_ab"] else fail)(
            "the camera state is UNCHANGED between A0 and A3 except the three "
            "offset fields -- the offset is the only variable in the A/B")
        leg["table_a3"] = score_table(h1, b1)
        leg["dump_a3"] = str(p1)
        leg["dump_a3_b"] = str(p2)
        leg["ab"] = ab_model(hA, bA, h1, b1)

        # --- THE RETURN CONTROL: the offset is a render-only pitch, so putting
        # it back to 0 must return every readout to A0 exactly.  Without this,
        # "the offset moved the table" is consistent with "something else moved
        # it at the same time".
        app.send("set zoom_offset 0", 1.5)
        _s, _n, h, b = app.wait_until(
            "back0", lambda H, B: cam_of(H)["viewOffsetEff"] == 0,
            "viewOffsetEff back to 0")
        hR, bR, pR = app.dump("a0b_return")
        leg["camera_a0b"] = cam_fields(hR)
        leg["table_a0b"] = score_table(hR, bR)
        leg["dump_a0b"] = str(pR)
        same = [n for n in sorted(set(bA) & set(bR))
                if bA[n].get("altaz_new") == bR[n].get("altaz_new")]
        leg["return_identical_altaz"] = len(same)
        leg["return_n"] = len(set(bA) & set(bR))
        leg["return_camera_identical"] = (leg["camera_a0"] == leg["camera_a0b"])
        (ok if leg["return_camera_identical"] else fail)(
            "RETURN CONTROL premise: the camera state at A0b is IDENTICAL to A0")
        (ok if len(same) == leg["return_n"] else fail)(
            "RETURN CONTROL: %d of %d bodies have a BYTE-IDENTICAL new-path "
            "alt/az at A0 and after the offset went back to 0"
            % (len(same), leg["return_n"]))

        # --- the RA/DE half: it already rides the full inverse (§11.213(f)),
        # so it must NOT move with the offset.  A0 vs A3, one camera state.
        leg["rade_a0_vs_a3"] = rade_compare(str(pA), str(p1))
        # and, for the record, what the camera MOVE alone does to it
        leg["rade_p0_vs_a0"] = rade_compare(str(pP), str(pA))

        # --- THE TRACKING DATUM at offset 0.3 (F81's shape).  Taken AFTER the
        # return control, because re-arming the tracking re-aims the camera and
        # would make "the readout came back" untestable (measured: it did, and
        # the first pass of this driver reported the return control red for
        # exactly that reason).
        app.send("set zoom_offset %g" % OFFSET, 1.5)
        app.send("flag track_object on", 2.0)
        _s, _n, h, b = app.wait_until("track3", lambda H, B: (
            nav_of(H)["plans"]["flagAutoMove"] == 0
            and abs(cam_of(H)["viewOffsetEff"] - OFFSET) < 1e-6),
            "tracking re-armed at offset %g" % OFFSET)
        h, b, _p = app.dump("track_o3", keep=False)
        leg["track_o3"] = track_datum(h, b)
        app.send("flag track_object off", 1.5)
        app.wait_until("hold3", lambda H, B: nav_of(H)["flagTraking"] == 0
                       and nav_of(H)["plans"]["flagAutoMove"] == 0,
                       "tracking released again")

        # --- the frame pair for the owner's eye (no gate, no claim): the
        # default scene with the Sun low, at offset 0 and at 0.3.  LAST,
        # because it moves the clock.
        leg["frames"] = stage_frames(app, leg, jd_sun)
    finally:
        leg["exit_code"] = app.stop()
    return leg


def track_datum(h, b):
    """F81's shape: where the TRACKED body sits, on both paths, at one frame."""
    rec = b.get(TARGET, {})
    old, new = rec.get("old") or {}, rec.get("new") or {}
    return {"body": TARGET,
            "old_screen": old.get("screen"),          # render px [body.cpp:1365]
            "new_screen": new.get("screen"),          # normalized [ModularBody.cpp:906]
            "new_screenSize": new.get("screenSize"),
            "camera_tracked": cam_of(h).get("tracked"),
            "viewOffsetEff": cam_of(h)["viewOffsetEff"],
            "old_localVision": nav_of(h)["localVision"]}


RADE_RE = re.compile(r'(\d+h\d+m[\d.]+s\s*/\s*[+-]\d+[^\d\'"]{1,4}\d+\'[\d.]+")')


def rade_strings(navstr_path):
    """{body: (OLD rade string, NEW rade string)} from a .navstr sidecar."""
    out, name, cur = {}, None, {}
    op = gzip.open if str(navstr_path).endswith(".gz") else open
    with op(navstr_path, "rt", errors="replace") as fh:
        for line in fh:
            m = re.match(r'^  (OLD|NEW) (nav|inf): (.*)$', line.rstrip("\n"))
            if m:
                if m.group(2) == "nav":
                    r = RADE_RE.search(m.group(3))
                    cur[m.group(1)] = re.sub(r"\s+", "", r.group(1)) if r else None
            elif not line.startswith("  ") and line.strip():
                if name and cur:
                    out[name] = (cur.get("OLD"), cur.get("NEW"))
                name, cur = line.strip(), {}
        if name and cur:
            out[name] = (cur.get("OLD"), cur.get("NEW"))
    return out


def rade_compare(dump_a, dump_b):
    a = rade_strings(dump_a + ".navstr")
    b = rade_strings(dump_b + ".navstr")
    common = sorted(set(a) & set(b))
    moved = [n for n in common if a[n][1] != b[n][1]]
    same_as_old_a = sum(1 for n in common if a[n][0] and a[n][0] == a[n][1])
    same_as_old_b = sum(1 for n in common if b[n][0] and b[n][0] == b[n][1])
    return {"n": len(common), "new_rade_moved": moved,
            "old_eq_new_in_a": same_as_old_a, "old_eq_new_in_b": same_as_old_b}


def stage_frames(app, leg, jd_sun):
    """ONE frame pair at offset 0.3 vs 0, the shipped scene with the Sun low.
    RECORDED for the owner's eye -- no gate and no photometric claim (§0.5:
    the canary is the only photometric instrument here)."""
    res = {}
    if jd_sun is None:
        # find a Sun-low frame BY MEASUREMENT rather than by convention: a
        # coarse hourly scan, then a secant on the Sun's own altitude toward
        # +2 deg (a setting Sun, the atmosphere's interesting regime).
        def sun_alt(jd):
            app.send("date jday %.9f" % jd, 0.8)
            _h, b, _p = app.dump("sunscan", pause=0.6, keep=False)
            aa = (b.get("Sun") or {}).get("altaz_old")
            return None if not aa else aa[0] * DEG

        scan = []
        for k in range(24):
            jd = JD + k / 24.0
            a = sun_alt(jd)
            if a is not None:
                scan.append((jd, a))
        res["scan"] = scan
        if not scan:
            fail("no Sun altitude found in the scan")
            return res
        # a DESCENDING crossing of +2 deg = sunset
        pair = None
        for i in range(len(scan) - 1):
            if scan[i][1] > 2.0 >= scan[i + 1][1]:
                pair = (scan[i], scan[i + 1])
                break
        if pair is None:
            jd_sun = min(scan, key=lambda s: abs(s[1] - 2.0))[0]
        else:
            (j0, a0), (j1, a1) = pair
            for _ in range(4):
                j = j0 + (2.0 - a0) * (j1 - j0) / (a1 - a0)
                a = sun_alt(j)
                if a is None:
                    break
                if abs(a - 2.0) < 0.05:
                    j0, a0 = j, a
                    break
                if a > 2.0:
                    j0, a0 = j, a
                else:
                    j1, a1 = j, a
            jd_sun = j0
        res["scan_best"] = {"jd": jd_sun}
    res["jd_sun"] = jd_sun
    app.send("date jday %.9f" % jd_sun, 1.5)
    app.send("set zoom_offset 0", 1.2)
    app.wait_until("frame0", lambda H, B: cam_of(H)["viewOffsetEff"] == 0,
                   "offset back to 0 for the reference frame")
    h, b, _p = app.dump("frame_o0", keep=False)
    res["sun_alt_old_deg"] = (b.get("Sun") or {}).get("altaz_old", [float("nan")])[0] * DEG
    res["sun_altaz_new_o0"] = (b.get("Sun") or {}).get("altaz_new")
    p0 = app.shot("sun_low_offset0")
    app.send("set zoom_offset %g" % OFFSET, 1.5)
    app.wait_until("frame3", lambda H, B: abs(cam_of(H)["viewOffsetEff"] - OFFSET) < 1e-6,
                   "offset 0.3 for the second frame")
    h, b, _p = app.dump("frame_o3", keep=False)
    res["sun_altaz_new_o3"] = (b.get("Sun") or {}).get("altaz_new")
    res["sun_altaz_old_o3"] = (b.get("Sun") or {}).get("altaz_old")
    p3 = app.shot("sun_low_offset03")
    res["frames"] = [str(p0) if p0 else None, str(p3) if p3 else None]
    # the atmosphere's own input: EnvironmentManager.cpp:114-120 reads
    # sunLocal[2] of the SAME expression, so the readout's z IS the light
    # direction's z -- reported, never gated here.
    for key in ("sun_altaz_new_o0", "sun_altaz_new_o3", "sun_altaz_old_o3"):
        aa = res.get(key)
        if aa:
            res[key + "_sinalt"] = math.sin(aa[0])
            res[key + "_skyBrightness"] = sky_brightness(math.sin(aa[0]))
    app.send("date jday %.9f" % JD, 1.2)
    return res


def sky_brightness(sin_alt):
    """EnvironmentManager.cpp:116-117 verbatim, atmosphere-fade term omitted."""
    return 0.01 if sin_alt < -0.1 / 1.5 else 0.01 + 1.5 * (sin_alt + 0.1 / 1.5)


def stage_cfg(binp, out, rep):
    """The CONFIG channel: `[navigation] view_offset = 0.3` at startup.  §5.138's
    open sub-question; F81 §11.201(e)(3) found the OLD path's compensation
    multiplied by a zero transition -- this measures the NEW path's."""
    farm = FARM_ROOT / "cfg"
    home = build_farm(farm)
    rep["cfg"] = leg = {"farm": str(farm), "channel": "config"}
    leg["config_edit"] = {"view_offset_was":
                          ini_set(home / "config.ini", "navigation",
                                  "view_offset", "%g" % OFFSET)}
    app = App(binp, farm, out, "cfg")
    try:
        leg["tcp_seconds"] = app.start()
        readout_setup(app)
        h, b, _p = app.dump("startup", keep=False)
        leg["startup_camera"] = cam_fields(h)
        leg["startup_nav"] = {k: nav_of(h)[k] for k in
                              ("localVision", "viewOffset", "viewOffsetTransition")}
        ok("CONFIG CHANNEL at startup: camera viewOffset=%s transition=%s eff=%s | "
           "old nav viewOffset=%s transition=%s"
           % (leg["startup_camera"]["viewOffset"],
              leg["startup_camera"]["viewOffsetTransition"],
              leg["startup_camera"]["viewOffsetEff"],
              leg["startup_nav"]["viewOffset"],
              leg["startup_nav"]["viewOffsetTransition"]))
        app.settle_scale()
        h, b, p = app.dump("startup_table")
        leg["table_startup"] = score_table(h, b)
        leg["dump_startup"] = str(p)
        # then ARM it with a commanded view move -- no `set zoom_offset` at all
        app.send("select planet %s" % TARGET, 1.0)
        app.send("flag track_object on", 2.0)
        got, _n, h, b = app.wait_until("cfgarm", lambda H, B: (
            nav_of(H)["plans"]["flagAutoMove"] == 0
            and abs(cam_of(H)["viewOffsetTransition"] - 1.0) < 1e-6),
            "the config-armed offset reaches transition 1 on the new path")
        leg["armed_camera"] = cam_fields(h)
        leg["armed_nav"] = {k: nav_of(h)[k] for k in
                            ("localVision", "viewOffset", "viewOffsetTransition")}
        app.send("flag track_object off", 1.5)
        app.wait_until("cfghold", lambda H, B: nav_of(H)["flagTraking"] == 0
                       and nav_of(H)["plans"]["flagAutoMove"] == 0,
                       "tracking released, view held")
        h, b, p = app.dump("armed_table")
        leg["camera_armed_table"] = cam_fields(h)
        leg["table_armed"] = score_table(h, b)
        leg["dump_armed"] = str(p)
    finally:
        leg["exit_code"] = app.stop()
    return leg


GUARD_NAMES = ["MilkyWay", "51PegSystem", "Universe", "Mars", "Jupiter"]


def stage_guard(binp, out, rep):
    """IS THE POLE GUARD REACHABLE FROM A SHIPPED COMMAND?  §11.213(i2) reasoned
    it was measure-zero ("exact float equality of two coordinates").  The dump
    says otherwise: 29 of the 120 records in the shipped default scene carry an
    eye-frame position of EXACTLY (0,0,0) -- systems and anchor bodies whose
    `dist` is 0 -- and (0,0,0) takes the same `x == 0 && y == 0` branch.  This
    stage asks the question on the OPERATOR's own channel: `select planet <name>`
    resolves a NEW-ONLY name through the ModularObject bridge
    [observed: core.cpp:1090-1097 -> ssystem_factory.cpp:909-915] and
    `get status object` prints that object's nav string.  Mars and Jupiter are
    the CONTROL: ordinary bodies, whose answer must not move."""
    farm = FARM_ROOT / "guard"
    build_farm(farm)
    rep["guard"] = leg = {"farm": str(farm), "answers": {}}
    app = App(binp, farm, out, "guard")
    try:
        leg["tcp_seconds"] = app.start()
        readout_setup(app)
        app.settle_scale()
        for name in GUARD_NAMES:
            app.send("select planet %s" % name, 1.0)
            ans = app.query("get status object", 1.0)
            leg["answers"][name] = ans
            ok("select planet %-14s -> %s" % (name, ans))
    finally:
        leg["exit_code"] = app.stop()
    return leg


def descend_score(leg):
    """WHICH EXPRESSION DID THE STEP GO THROUGH?  `moveEyeRel(v)` is
    `moveRel(observedToLocalPos(v))` and in free flight `moveRel` adds straight
    to `position`, so the measured step IS `observedToLocalPos(v)` for the v
    the descent chose.  The two candidate v are the screen-centre ray (0,0,+-1)
    and the reference centre C, both in the RENDER eye frame; the two candidate
    expressions are Rv^T (shipped) and (R'.Rv)^T (the fix).  Scored in the SAME
    run, so the pre and post runs do not have to start from the same state --
    each run says for itself which expression its own step went through."""
    cam = leg["before_camera"]
    rv = view_rotation(cam)
    rp = rx(cam["viewOffsetEff"] * cam["halfFov"])
    shipped = mat_t(rv)                                  # Rv^T
    fixed = mat_t(mat_mul(rp, rv))                       # (R'.Rv)^T
    step = leg["step"]
    out = {}
    cands = {"ray_z": [0.0, 0.0, 1.0]}
    if leg.get("before_ref_pos"):
        cands["ref_C"] = leg["before_ref_pos"]
    for vname, v in cands.items():
        for ename, m in (("shipped_RvT", shipped), ("fixed_RRvT", fixed)):
            a = angle_between(mat_vec(m, v), step)
            out["%s_%s_deg" % (vname, ename)] = min(a, 180.0 - a)
    return out


def stage_descend(binp, out, rep):
    """`camera action descend coef <c>` -- the ONE shipped verb that reaches
    `Camera::moveEyeRel` [coreLink.hpp:959-961, app_command_interface.cpp:4539-4550;
    multAlt / moveRelAlt are UI-key-only, §11.71].  In free flight its input
    vectors are all expressed in the RENDER eye frame (`viewMat().getTranslation()`
    and the (0,0,-1) screen-centre ray, Camera.cpp:1159-1192), so the offset is
    inside them and only the full inverse undoes it."""
    farm = FARM_ROOT / "descend"
    build_farm(farm)
    rep["descend"] = leg = {"farm": str(farm)}
    app = App(binp, farm, out, "descend")
    try:
        leg["tcp_seconds"] = app.start()
        readout_setup(app)
        app.settle_scale()
        app.send("select planet %s" % TARGET, 1.0)
        app.send("flag track_object on", 2.0)
        app.wait_until("darm", lambda H, B: (
            abs(cam_of(H)["viewOffsetTransition"] - 1.0) < 1e-6
            and nav_of(H)["plans"]["flagAutoMove"] == 0), "armed on the new path")
        app.send("flag track_object off", 1.5)
        app.send("camera action free_mode state on", 1.5)
        app.send("set zoom_offset %g" % OFFSET, 1.5)
        got, _n, h, b = app.wait_until("dfree", lambda H, B: (
            cam_of(H)["freeMode"] is True
            and abs(cam_of(H)["viewOffsetEff"] - OFFSET) < 1e-6),
            "free mode ON and viewOffsetEff == %g" % OFFSET)
        # GAIN ALTITUDE FIRST: the shipped place sits ON the ground, so a
        # descent step there is 10 % of ~0 and the step direction is unreadable
        # at the dump's precision (measured: |d| = 6.4e-11 AU on the first
        # pass).  Five ascents give the descent something to be a fraction of.
        for i in range(5):
            app.send("camera action descend coef 3", 1.2)
        _s, _n, h, b = app.wait_until("dup", lambda H, B: True, "ascent settled",
                                      tries=3)
        leg["before_camera"] = cam_fields(h)
        leg["before_position"] = cam_of(h)["position"]
        leg["before_rootPos"] = cam_of(h).get("rootPos")
        leg["before_distance"] = cam_of(h)["distance"]
        # the reference's own eye-frame position: `C` in Camera.cpp:1159, the
        # other vector the descent can hand to moveEyeRel
        leg["before_ref_pos"] = body_pos(b.get(cam_of(h)["reference"], {}))
        app.send("camera action descend coef 0.9", 1.5)
        _s, _n, h, b = app.wait_until("dstep", lambda H, B:
                                      cam_of(H)["position"] != leg["before_position"],
                                      "the descent step landed")
        leg["after_camera"] = cam_fields(h)
        leg["after_position"] = cam_of(h)["position"]
        leg["after_rootPos"] = cam_of(h).get("rootPos")
        d = [a - bb for a, bb in zip(leg["after_position"], leg["before_position"])]
        leg["step"] = d
        leg["step_len"] = math.sqrt(sum(x * x for x in d))
        if leg["before_rootPos"] and leg["after_rootPos"]:
            dr = [a - bb for a, bb in zip(leg["after_rootPos"], leg["before_rootPos"])]
            leg["step_root"] = dr              # 17 digits [Camera.cpp:1452]
            leg["step_root_len"] = math.sqrt(sum(x * x for x in dr))
        leg["step_score"] = descend_score(leg)
        ok("descend step %s (|d| = %.9g AU) at viewOffsetEff %s"
           % (d, leg["step_len"], leg["before_camera"]["viewOffsetEff"]))
        ok("descend direction vs the two expressions: %s"
           % json.dumps(leg["step_score"]))
    finally:
        leg["exit_code"] = app.stop()
    return leg


# --------------------------------------------------------------------- report
def report(rep, out):
    lines = []
    for legname in ("cmd", "cfg", "descend"):
        leg = rep.get(legname)
        if not leg:
            continue
        lines.append("=== leg %s (%s)" % (legname, rep.get("tag")))
        fm = leg.get("freshness_move")
        if fm:
            lines.append("  FRESHNESS across the camera move, offset 0 both "
                         "sides: %d frozen / %d re-evaluated of %d"
                         % (len(fm["frozen"]), len(fm["moved"]), fm["n"]))
            lines.append("    frozen: %s" % ", ".join(fm["frozen"]))
        ab = leg.get("ab")
        if ab:
            lines.append("  A/B at ONE camera state (the offset is the only "
                         "variable), viewOffsetEff %s:" % ab["camera"]["viewOffsetEff"])
            lines.append("    C: rotation %.6f deg, axis vs Rv^T*x %.6f deg"
                         % (ab["model_angle_deg"],
                            angle_between(ab["model_axis"], ab["axis_Rv_T_x"])))
            f = ab["freshness"]
            lines.append("    positions: %d moved (%d land on R'.p exactly, %d "
                         "do not), %d frozen, of %d"
                         % (len(f["moved"]), len(f["matches_R"]),
                            len(f["mismatch_R"]), len(f["frozen"]), f["n"]))
            for cls in ("moved", "frozen"):
                sel = {n: r for n, r in ab["rows"].items() if r["class"] == cls}
                if not sel:
                    continue
                for key in ("sep_ab_deg", "sep_model_deg", "sep_M1_sign_deg",
                            "sep_M2_swap_deg", "sep_old_new_b_deg"):
                    xs = sorted([(r[key], n) for n, r in sel.items()
                                 if key in r and math.isfinite(r[key])], reverse=True)
                    if not xs:
                        continue
                    lines.append("    [%s] %-19s max %.6f (%s) | median %.6f | n=%d"
                                 % (cls, key, xs[0][0], xs[0][1],
                                    sorted(v for v, _ in xs)[len(xs) // 2], len(xs)))
        for tabkey in ("table_off0", "table_o3", "table_startup", "table_armed"):
            t = leg.get(tabkey)
            if not t:
                continue
            eff = t["camera"]["viewOffsetEff"]
            lines.append("  %s: viewOffsetEff=%s  bodies=%d  model rotation "
                         "%.6f deg" % (tabkey, eff, t["n"], t["model_angle_deg"]))
            if eff:
                lines.append("    axis(C) vs Rv^T*x : %.6f deg"
                             % angle_between(t["model_axis"], t["axis_Rv_T_x"]))
            for key in ("sep_old_new_deg", "sep_model_deg", "sep_M1_sign_deg",
                        "sep_M2_swap_deg"):
                xs = sorted([(r[key], n) for n, r in t["rows"].items()
                             if key in r and math.isfinite(r[key])], reverse=True)
                if not xs:
                    continue
                nonx = [x for x in xs if x[1] not in ("Eris",)]
                lines.append("    %-18s max %.6f (%s) | max non-Eris %.6f (%s) | "
                             "median %.6f"
                             % (key, xs[0][0], xs[0][1], nonx[0][0], nonx[0][1],
                                sorted(v for v, _ in xs)[len(xs) // 2]))
    txt = "\n".join(lines) + "\n"
    (Path(out) / "f96_report.txt").write_text(txt)
    print(txt, flush=True)


def main():
    argv = sys.argv[1:]

    def opt(name, default=None):
        return argv[argv.index(name) + 1] if name in argv else default

    out = Path([a for a in argv if not a.startswith("--")][0]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    tag = opt("--tag", "pre")
    binp = Path(opt("--bin", str(HERE.parents[1] / "build-claude/src/spacecrafter")))
    stages = opt("--stages", "cmd,cfg,descend,guard").split(",")
    jd_sun = opt("--jd-sun")
    jd_sun = float(jd_sun) if jd_sun else None

    if "--score" in argv:
        rep = json.loads((out / "f96_result.json").read_text())
        report(rep, out)
        return 0

    rep = {"tag": tag, "bin": str(binp), "stages": stages,
           "bin_md5": subprocess.run(["md5sum", str(binp)], capture_output=True,
                                     text=True).stdout.split()[0],
           "jd": JD, "offset": OFFSET, "target": TARGET}
    md5_in = subprocess.run(["md5sum", str(REAL_HOME / "config.ini"),
                             str(REAL_HOME / "ssystem.ini")],
                            capture_output=True, text=True).stdout
    rep["md5_in"] = md5_in
    print("md5 IN (real home):\n" + md5_in, flush=True)

    try:
        if "cmd" in stages:
            stage_cmd(binp, out, rep, jd_sun)
        if "cfg" in stages:
            stage_cfg(binp, out, rep)
        if "descend" in stages:
            stage_descend(binp, out, rep)
        if "guard" in stages:
            stage_guard(binp, out, rep)
    finally:
        md5_out = subprocess.run(["md5sum", str(REAL_HOME / "config.ini"),
                                  str(REAL_HOME / "ssystem.ini")],
                                 capture_output=True, text=True).stdout
        rep["md5_out"] = md5_out
        (ok if md5_out == md5_in else fail)(
            "md5 in == out on the real ~/.spacecrafter")
        rep["fails"], rep["notes"] = FAILS, NOTES
        (out / "f96_result.json").write_text(json.dumps(rep, indent=1))
    report(rep, out)
    print("\n%d FAIL(s), %d NOTE(s); report -> %s"
          % (len(FAILS), len(NOTES), out / "f96_result.json"), flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
