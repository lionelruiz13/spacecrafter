#!/usr/bin/env python3
"""B33 — the heading READOUT must report the roll of the path that DRAWS
(INTENT §11.108(f) + the D28 rider §11.113(g), delivered §11.118).

WHY IT IS A DEFECT AND NOT A DETAIL. `CoreLink::setHeading` has always written
BOTH authorities; `CoreLink::getHeading` read only the old `Navigator`. D28
answers that the new path HOLDS the whole eye orientation across a reference
switch, so the two authorities diverge permanently by design - and every reader
of the getter was then reporting, and computing from, a number that is not the
roll on screen. `heading delta_azimuth d` builds its ABSOLUTE target from it
(app_command_interface.cpp), and `set heading 0` - the standing remedy for the
accumulating tilt (§5.26) - is the command the operator reaches for after
reading it.

    cd claude/harness && DISPLAY=:2 ./f12_b33_heading.py <out> [--bin B] [--prebin B]

THE INSTRUMENT. The readout is sampled through the app's OWN script log:
`heading delta_azimuth 0` writes "heading from : X to: Y" with X =
getHeading() before it acts. That channel WRITES after it reads (it sets both
authorities), so every sample is the last act of its leg and the two
`experimental_path` pins need two launches. The drawn roll is `Camera::heading`
from the dual dump. The scene is `s526_ref.py`'s: Earth reference, then the
Earth->Moon reference switch that makes the two authorities disagree.

Legs:
  R1  after the switch, the readout vs the DRAWN roll ............ must agree
  R2  the same scene with `flag experimental_path off`, SEPARATE launch: the
      readout must switch back to the old Navigator - the getter asks which
      path draws, it does not simply prefer the new one
  R3  the §5.26 remedy: `set heading 0` collapses the cross-path pixel
      divergence, and the readout still tracks the drawn roll after it
  R4  the seam crossed a SECOND time (Moon -> Earth), from the state R3 left
  RED the pre-fix binary, same scene: reports a number it is not drawing, and
      that number is exactly what the fixed binary reports under the old pin
"""

import json, math, os, re, socket, subprocess, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import b25_galactic as b25g
import b24_equivalence as b24   # the NaN-tolerant dump reader (I2)

HERE = Path(__file__).resolve().parent
JD = "2461233.5"
DEFAULT_BIN = str(HERE.parents[1] / "build-claude/src/spacecrafter")

FAILS = []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


def norm180(deg):
    return deg - math.floor((deg + 180.0) / 360.0) * 360.0


def px32(a, b):
    ia = np.asarray(Image.open(a).convert("RGB")).astype(np.int32)
    ib = np.asarray(Image.open(b).convert("RGB")).astype(np.int32)
    return int((np.abs(ia - ib).max(axis=2) > 32).sum())


class App:
    def __init__(self, farm, binary, applog):
        self.dst = b25g.build_farm(farm=farm, dotted=False, corpus=None)
        self.proc = subprocess.Popen(
            [binary], cwd=str(self.dst),
            stdout=open(applog, "w"), stderr=subprocess.STDOUT,
            env={**os.environ, "HOME": str(farm),
                 "DISPLAY": os.environ.get("DISPLAY", ":2")})
        self.sock, t0 = None, time.time()
        while time.time() - t0 < 90:
            if self.proc.poll() is not None:
                raise RuntimeError("app died before opening its port")
            try:
                self.sock = socket.create_connection(("127.0.0.1", 7805), timeout=1)
                break
            except OSError:
                time.sleep(1)
        if self.sock is None:
            self.proc.kill()
            raise RuntimeError("port 7805 never opened")
        time.sleep(10)

    def send(self, cmd, pause=0.7):
        self.sock.sendall((cmd + "\n").encode())
        time.sleep(pause)
        try:
            self.sock.settimeout(0.3)
            out = self.sock.recv(8192).decode(errors="replace")
        except socket.timeout:
            out = ""
        self.sock.settimeout(None)
        return out

    def reported_heading(self):
        """The value `CoreLink::getHeading()` returns, read off the app's OWN
        script log: `heading delta_azimuth d` writes "heading from : X to: Y"
        with X = getHeading() BEFORE it acts (app_command_interface.cpp).

        Why not `get status position`, which is side-effect free and reads the
        same getter: its reply never reaches the client on this build - the
        ServerSocket outputQueue produced nothing on the driving connection in
        a 6 s poll, twice, on a scene where every other command worked
        [measured 2026-07-30]. Recorded as an out-of-scope find; here it means
        the readout has exactly one observable channel, and that channel WRITES
        after it reads. So each sample is the last act of its leg."""
        before = self._script_log_len()
        self.send("heading delta_azimuth 0", 2.0)
        added = self._script_log_text()[before:]
        m = re.findall(r"heading from : (-?[\d.eE+]+) to: (-?[\d.eE+]+)", added)
        if not m:
            raise RuntimeError(f"no 'heading from' line appeared in the script log: {added[-400:]!r}")
        return float(m[-1][0])

    def _script_logs(self):
        return sorted((self.dst / "log").glob("script*.log"))

    def _script_log_text(self):
        return "".join(p.read_text(errors="replace") for p in self._script_logs())

    def _script_log_len(self):
        return len(self._script_log_text())

    def drawn_roll(self, out, tag):
        p = out / f"b33_{tag}.json"
        p.unlink(missing_ok=True)
        self.send(f"body action dual_dump filename {p}", 2.5)
        # NaN-tolerant, for the same reason b24_equivalence.load_dump is: C++
        # prints `nan`, Python's json rejects it, and the §11.18 cold-launch
        # ASmooth NaN fires intermittently on a scaled body. It never touches
        # the camera block, but it does sit in the same LINE.
        line = b24._NONFINITE.sub(
            lambda m: m.group(1) + ("NaN" if m.group(2) == "nan" else "Infinity"),
            open(p).readline())
        cam = json.loads(line)["camera"]
        return norm180(math.degrees(cam["heading"])), cam

    def shot(self, out, tag):
        p = out / f"b33_{tag}.png"
        self.send(f"body action screenshot filename {p}", 2.2)
        return p

    def cross_px(self, out, tag):
        """New-vs-old rendered divergence of the SAME state (the §5.26 metric)."""
        self.send("flag experimental_path on", 2)
        a = self.shot(out, f"{tag}_new")
        self.send("flag experimental_path off", 2)
        b = self.shot(out, f"{tag}_old")
        self.send("flag experimental_path on", 1.5)
        ia = np.asarray(Image.open(a).convert("RGB")).astype(np.int32)
        ib = np.asarray(Image.open(b).convert("RGB")).astype(np.int32)
        return int((np.abs(ia - ib).max(axis=2) > 32).sum())

    def stop(self):
        self.send("shutdown action now", 1)
        self.sock.close()
        try:
            self.proc.wait(timeout=40)
        except subprocess.TimeoutExpired:
            self.proc.kill()


def scene(app):
    app.send("timerate rate 0", 1)
    # Everything the OLD path draws in the observer frame is turned off: those
    # layers legitimately follow `Navigator::heading`, so leaving one on would
    # put 6 deg of sky rotation into a frame comparison that is about the BODY
    # path holding still. Measured cost of missing one: `nebula_circle` left on
    # contributed 1225 px>32 to the no-jump leg before this list was widened.
    for f in ("atmosphere", "fog", "landscape", "planet_names", "planet_orbits",
              "planet_trails", "cardinal_points", "equatorial_grid", "stars",
              "milky_way", "constellation_drawing", "constellation_lines",
              "constellation_names", "constellation_art",
              "constellation_boundaries", "nebulae", "nebula_names",
              "nebula_circle", "azimuthal_grid", "equator_grid", "ecliptic_grid",
              "galactic_grid", "equator_line", "galactic_line", "ecliptic_line",
              "meridian_line", "zenith_line", "polar_circle", "precession_circle",
              "zodiac", "star_lines", "object_coordinates", "analemma_line",
              "greenwich_line", "aries_line", "vertical_line"):
        app.send(f"flag {f} off")
    app.send("set home_planet Earth", 3)
    app.send(f"date jday {JD}", 1)
    app.send("select planet Moon pointer off", 1)
    app.send("flag track_object on", 3)
    app.send("zoom fov 10 duration 0", 2)
    app.send("timerate rate 0", 1)


def switch(app, to_body, look_at):
    app.send("deselect", 0.6)
    app.send("flag track_object off", 1)
    app.send(f"set home_planet {to_body}", 4)
    app.send(f"select planet {look_at} pointer off", 1)
    app.send("flag track_object on", 3)
    app.send("zoom fov 10 duration 0", 2)
    app.send("timerate rate 0", 1)


def run(out, tag, binary, pin_old=False):
    """One fresh launch. `pin_old` runs the same scene with the OLD path pinned
    as the drawn one - a SEPARATE launch because the only readout channel
    writes both authorities after reading, so two pins cannot be sampled in one
    run without the first sample setting up the second."""
    app = App(out / f"farm_{tag}", binary, out / f"b33_{tag}.applog")
    r = {"pin": "old" if pin_old else "new"}
    try:
        scene(app)
        switch(app, "Moon", "Earth")
        if pin_old:
            app.send("flag experimental_path off", 2.5)
        # the drawn roll of the NEW path, sampled before anything writes
        r["drawn_deg"], cam = app.drawn_roll(out, f"{tag}_switch")
        r["reference"] = cam.get("reference")
        if pin_old:
            r["reported"] = app.reported_heading()
            app.send("flag experimental_path on", 2)
            return r

        # The cross-path divergence AS THE SWITCH LEAVES IT - measured before
        # anything samples the readout, because the only sampling channel
        # writes both authorities and would collapse what is being measured.
        r["cross_switch"] = app.cross_px(out, f"{tag}_at_switch")
        # THE OPERATOR-VISIBLE CONSEQUENCE. `heading delta_azimuth 0` is a
        # semantic no-op ("turn by zero"), so the DRAWN view must not move. It
        # writes getHeading() to BOTH paths, so it holds still only if that
        # number is the roll actually being drawn - which is the whole of B33,
        # and D28's "no visual jump mid-show" made measurable.
        before = app.shot(out, f"{tag}_nojump_before")
        r["reported"] = app.reported_heading()
        after = app.shot(out, f"{tag}_nojump_after")
        r["nojump_px"] = px32(before, after)
        r["cross_after_delta0"] = app.cross_px(out, f"{tag}_after_delta0")

        # R3 - the §5.26 remedy: `set heading 0` re-syncs both paths.
        app.send("set heading 0", 3)
        r["cross_after"] = app.cross_px(out, f"{tag}_post_remedy")
        r["drawn_after"], _ = app.drawn_roll(out, f"{tag}_after")
        r["reported_after"] = app.reported_heading()

        # R4 - second entry of the reversible pair, from the state R3 produced.
        switch(app, "Earth", "Moon")
        r["drawn_back"], _ = app.drawn_roll(out, f"{tag}_back")
        r["reported_back"] = app.reported_heading()
    finally:
        app.stop()
    return r


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    out = Path(args[0]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    binary, prebin = DEFAULT_BIN, None
    for a in sys.argv[1:]:
        if a.startswith("--bin="):
            binary = a.split("=", 1)[1]
        if a.startswith("--prebin="):
            prebin = a.split("=", 1)[1]

    src_md5 = b25g.real_tree_md5()
    rep = {"post": run(out, "post", binary),
           "post_oldpin": run(out, "post_oldpin", binary, pin_old=True)}
    if prebin:
        rep["pre"] = run(out, "pre", prebin)
    for k, v in rep.items():
        print(f"[{k}] {json.dumps(v)}", flush=True)

    p = rep["post"]
    TOL = 0.05  # degrees

    if abs(norm180(p["drawn_deg"])) > 0.1:
        ok(f"the scene DOES diverge: after the Earth->Moon switch the drawn roll is "
           f"{p['drawn_deg']:.4f} deg, not 0 - the readout has something to be wrong about")
    else:
        fail(f"the drawn roll after the switch is {p['drawn_deg']:.4f} deg (~0): this scene "
             f"does not separate the two authorities, the legs below prove nothing")

    if abs(p["reported"] - p["drawn_deg"]) < TOL:
        ok(f"R1: with the NEW path drawing, the readout reports {p['reported']:.6f} deg "
           f"== the drawn roll {p['drawn_deg']:.6f} deg")
    else:
        fail(f"R1: readout {p['reported']:.6f} != drawn roll {p['drawn_deg']:.6f}")

    o = rep["post_oldpin"]
    if abs(o["reported"] - o["drawn_deg"]) > TOL:
        ok(f"R2: same binary, same scene, OLD path pinned as the drawn one - the readout "
           f"switches back to it ({o['reported']:.6f} deg, against the new path's roll of "
           f"{o['drawn_deg']:.6f}): the getter asks WHICH PATH DRAWS, it does not simply "
           f"prefer the new one")
    else:
        fail(f"R2: the old-pinned readout ({o['reported']:.6f}) did not differ from the new "
             f"path's roll ({o['drawn_deg']:.6f}) - either the branch is dead or the scene "
             f"lost its divergence")

    if p["nojump_px"] < 500:
        ok(f"R1b NO VISUAL JUMP: `heading delta_azimuth 0` - a semantic no-op that writes the "
           f"readout to both paths - moves the drawn view by {p['nojump_px']} px>32 "
           f"(pre-fix: {rep.get('pre', {}).get('nojump_px', 'n/a')})")
    else:
        fail(f"R1b: the no-op heading command moved the drawn view by {p['nojump_px']} px>32")

    if p["cross_after"] < max(0.02 * p["cross_switch"], 200):
        ok(f"R3: the cross-path divergence the switch leaves ({p['cross_switch']} px>32) is "
           f"gone after `set heading 0` ({p['cross_after']} px>32) - §5.26's standing remedy, "
           f"intact (and `heading delta_azimuth 0` alone already brought it to "
           f"{p['cross_after_delta0']})")
    else:
        fail(f"R3: `set heading 0` left {p['cross_after']} px>32 (from {p['cross_switch']})")
    if abs(p["reported_after"] - p["drawn_after"]) < TOL:
        ok(f"R3b: after the remedy the readout still tracks the drawn roll "
           f"({p['reported_after']:.6f} == {p['drawn_after']:.6f})")
    else:
        fail(f"R3b: after the remedy readout {p['reported_after']:.6f} != drawn "
             f"{p['drawn_after']:.6f}")
    if abs(p["reported_back"] - p["drawn_back"]) < TOL:
        ok(f"R4: second entry of the reference pair (Moon->Earth), readout "
           f"{p['reported_back']:.6f} == drawn {p['drawn_back']:.6f}")
    else:
        fail(f"R4: second entry readout {p['reported_back']:.6f} != drawn {p['drawn_back']:.6f}")

    if prebin:
        q = rep["pre"]
        if abs(q["reported"] - q["drawn_deg"]) > TOL:
            ok(f"RED CONTROL: the pre-fix binary reports {q['reported']:.6f} deg while "
               f"DRAWING {q['drawn_deg']:.6f} deg - a gap of "
               f"{abs(q['reported'] - q['drawn_deg']):.4f} deg, which is the defect")
        else:
            fail(f"RED CONTROL: the pre-fix binary already agreed "
                 f"({q['reported']:.6f} vs {q['drawn_deg']:.6f}) - this leg cannot fail, "
                 f"so it proves nothing")
        if q["nojump_px"] > 10 * max(p["nojump_px"], 1):
            ok(f"RED CONTROL: and on the pre-fix binary that same no-op command JUMPS the drawn "
               f"view - {q['nojump_px']} px>32 against the fixed binary's {p['nojump_px']} - "
               f"because it writes a roll the new path was not at")
        else:
            fail(f"RED CONTROL: the no-op command moved the pre-fix view by only "
                 f"{q['nojump_px']} px>32 vs {p['nojump_px']} - the jump leg is not "
                 f"discriminating")
        if abs(q["reported"] - o["reported"]) < TOL:
            ok(f"RED CONTROL: and the number it reported ({q['reported']:.6f}) is the one the "
               f"fixed binary reports when the OLD path is the drawn one ({o['reported']:.6f}) "
               f"- i.e. the old value was never wrong, it was reported for the wrong path")
        else:
            fail(f"RED CONTROL: the pre-fix readout {q['reported']:.6f} does not match the "
                 f"old-pinned readout {o['reported']:.6f} - the two are supposed to be the "
                 f"same authority, so the scene is not reproducing")

    if b25g.real_tree_md5() != src_md5:
        fail("the real ~/.spacecrafter tree was written by this run")
    else:
        ok("real tree md5 in == out")

    (out / "f12_b33.json").write_text(json.dumps(rep, indent=1))
    print(f"\n{'OK' if not FAILS else str(len(FAILS)) + ' FAILS'} -> {out}/f12_b33.json",
          flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
