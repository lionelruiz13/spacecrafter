#!/usr/bin/env python3
"""B34 — the MECHANICAL SEAM MIRRORS: `body action clear`, `body action preload`,
`position save` / `position load` (INTENT §11.108(f)/(k); delivered §11.132).

    cd claude/harness && DISPLAY=:2 ./f24_b34_seams.py <out> [--bin B] [--prebin B]
                                                            [--only clear|preload|position]

Each of these is a shipped operator command whose engine call reached the OLD
tree alone, so under the new render path it acted on a universe nobody was
looking at. Every leg below is driven THROUGH THE COMMAND on a live app, and
every one of them is measured on BOTH binaries: the delivered one and the
pre-fix one (`--prebin`), where the SAME script must show the defect. A leg that
only runs green on the fixed binary proves nothing about what changed.

  CLEAR  — `body action clear` drops "all bodies that do not come from
    ssystem.ini" (old's own comment). Three bodies are pushed at runtime: a
    plain one, a HIDDEN one, and a CHILD of the plain one; hidden and nested
    are in because old's rule walks `systemBodies`, which hidden bodies are in
    (verified at source, not assumed), and because the new path owns children
    by unique_ptr so a subtree is a distinct removal shape.
    Observables: the dump's own new-only section (a body that survives on the
    new path alone comes out with `"old":null` — the §11.3 finding channel,
    which is exactly this defect's shape), `supplemental` per body (the
    provenance mark, without which a mistyped mark would pass vacuously), the
    composed screen, and the file a later `body action save` writes (F15: a
    cleared body's declaration must not be authored).
    The RED half is not an absence: on the pre-fix binary the three bodies MUST
    still be there, with `old` null, and the screen must still carry the disc.

  PRELOAD — `body action preload` reaches `ModularBody::preload` (which had no
    caller at all, B36) and through it BasicMesh/LayeredMesh/Photosphere.
    Observables: `preloadCount` per body (the seam ran on THIS body) and the
    engine's big-texture table (what it DID). The two are separate on purpose:
    the two paths share one texRecap per file name, so a texture old's own
    preload already acquired shows nothing new whichever path asked for it —
    "the seam did not run" and "the seam ran and had nothing left to pull" are
    different facts and need different readouts.

  POSITION — `position save` → move → `position load` must bring back the
    CAMERA. `camera action descend` is new-path-only by design, so two shipped
    commands split the two authorities; the bookmark then has to record the one
    that draws and restore into both. Pre-fix it records the old observer's
    altitude and restores it into the old observer alone, so the drawn view
    does not come back at all.
    §5.68 (record-only) is stated, not fixed: the two place setters are dual but
    NOT equivalent (old clamps latitude to ±90°, maps 0 to 1e-6, floors altitude
    at 0.1 m; the camera clamps nothing), so a restore lands the camera exactly
    and the old observer on its clamped image. A leg measures that rather than
    asserting it away.

Scene note (the §11.100(g)(ii) trap F23 paid for): below
`distance < 2·scaledRadius` a body draws NOTHING, so every screen leg here is
composed OUTSIDE that boundary at both ends and guarded by `lit()`.
"""

import json, os, socket, subprocess, sys, time
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import b25_galactic as b25g
import b24_equivalence as b24   # the NaN-tolerant dump reader (I2)

HERE = Path(__file__).resolve().parent
JD = 2461233.5
DEFAULT_BIN = str(HERE.parents[1] / "build-claude/src/spacecrafter")

FAILS = []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


def px8(a, b):
    ia = np.asarray(Image.open(a).convert("RGB")).astype(np.int32)
    ib = np.asarray(Image.open(b).convert("RGB")).astype(np.int32)
    return int((np.abs(ia - ib).max(axis=2) > 8).sum())


def lit(p):
    """Lit pixels — the guard that a px8 of 0 means AGREEMENT and not an empty
    frame (F23's first altitude scene had 28 lit px of 4.2 M)."""
    return int((np.asarray(Image.open(p).convert("RGB")).max(axis=2) > 8).sum())


class App:
    """One fresh launch in its own temp-HOME farm (b25_galactic.build_farm — the
    farm shape's single authority). The farm is what lets the `body action save`
    leg run at all: it writes into a modularSystem directory that belongs to
    this run, never into the installed field data."""

    def __init__(self, farm, binary, applog):
        self.farmroot = Path(farm)
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
        self.out = None

    def send(self, cmd, pause=0.7):
        self.sock.sendall((cmd + "\n").encode())
        time.sleep(pause)
        try:
            self.sock.settimeout(0.3)
            self.sock.recv(8192)
        except socket.timeout:
            pass
        self.sock.settimeout(None)

    def dumpfile(self, tag):
        p = self.out / f"{tag}.json"
        p.unlink(missing_ok=True)
        self.send(f"body action dual_dump filename {p}", 3.0)
        if not p.exists():
            raise RuntimeError(f"dump {p} was not written")
        return p

    def dump(self, tag):
        """(header, {name: body-record}) — the record keeps `old` so a body that
        exists on the NEW path alone is distinguishable from one both carry."""
        p = self.dumpfile(tag)
        header, bodies = None, {}
        for line in open(p, encoding="utf-8", errors="replace"):
            line = b24._NONFINITE.sub(
                lambda m: m.group(1) + ("NaN" if m.group(2) == "nan" else "Infinity"), line)
            d = json.loads(line)
            if d.get("type") == "header":
                header = d
            elif d.get("type") == "body":
                bodies[d["name"]] = d
        return header, bodies

    def shot(self, tag):
        p = self.out / f"{tag}.png"
        p.unlink(missing_ok=True)
        self.send(f"body action screenshot filename {p}", 2.5)
        for _ in range(25):
            if p.exists() and p.stat().st_size > 0:
                return p
            time.sleep(0.4)
        raise RuntimeError(f"screenshot {p} was not written")

    def stop(self):
        self.send("shutdown action now", 1)
        self.sock.close()
        try:
            self.proc.wait(timeout=40)
        except subprocess.TimeoutExpired:
            self.proc.kill()


SKY_OFF = ("atmosphere", "fog", "landscape", "milky_way", "nebulae",
           "nebula_names", "constellation_drawing", "constellation_lines",
           "constellation_names", "constellation_art", "cardinal_points",
           "azimuthal_grid", "equatorial_grid", "planet_names", "planet_orbits",
           "planet_trails", "meridian_line", "zenith_line", "ecliptic_line",
           "equator_line")


def base_scene(app):
    app.send("flag experimental_path on", 1)
    app.send("timerate rate 0", 1)
    app.send(f"date jday {JD}", 1)
    app.send("meteors zhr 0", 0.6)
    for f in SKY_OFF:
        app.send(f"flag {f} off")
    app.send("flag stars off")
    app.send("flag moon_scaled off", 1)
    app.send("set home_planet Earth", 3)


# ===================================================================== CLEAR ==
# The three pushed bodies. `still_orbit` keeps the position a data key rather
# than an ephemeris; the offset is in AU and the radius in km, like the data
# keys. PLAIN is f15_persist's rover recipe verbatim - the one body shape this
# harness has already measured as visible on the composed screen.
PLAIN = ("body action load name F24Push parent Earth type Artificial radius 5000 "
         "coord_func still_orbit orbit_x 0 orbit_y 0 orbit_z 0.0003 rot_periode 24 "
         "tex_map bodies/generic.png halo false")
HIDDEN = ("body action load name F24Hidden parent Earth type Artificial radius 5000 "
          "coord_func still_orbit orbit_x 0 orbit_y 0.0003 orbit_z 0 rot_periode 24 "
          "tex_map bodies/generic.png halo false hidden true")
CHILD = ("body action load name F24Child parent F24Push type Artificial radius 1500 "
         "coord_func still_orbit orbit_x 0 orbit_y 0 orbit_z 0.00004 rot_periode 24 "
         "tex_map bodies/generic.png halo false")
PUSHED = ["F24Push", "F24Hidden", "F24Child"]


def run_clear(out, tag, binary):
    app = App(out / f"farm_{tag}", binary, out / f"{tag}.applog")
    app.out = out
    r = {}
    try:
        base_scene(app)
        # Stars back ON: they are path-INDEPENDENT content, so the frame after
        # the clear still carries something. Without them the only lit thing in
        # this scene is the pushed body itself and the post-clear frame is
        # empty - a px diff equal to the whole content, and a `lit()` guard that
        # cannot then tell "the body went" from "the render broke".
        app.send("flag stars on", 1)
        # The observer looks at the pushed body from outside Earth's 2R, with
        # the body's declared offset (0.0003 AU = 44878 km from Earth's centre)
        # putting it clear of the parent's disc.
        app.send("moveto lat 0 lon 0 alt 40000000 duration 0", 3)
        app.send("camera action free_mode state on", 1.5)
        for cmd in (PLAIN, HIDDEN, CHILD):
            app.send(cmd, 2.5)
        app.send("select planet F24Push pointer off", 1.5)
        app.send("flag track_object on", 3)
        app.send("zoom fov 40 duration 0", 2.5)
        app.send("flag track_object off", 2)

        h0, b0 = app.dump(f"{tag}_pre")
        s0 = app.shot(f"{tag}_pre")
        r["pre_lit"] = lit(s0)
        r["pre_present"] = {n: (n in b0) for n in PUSHED}
        r["pre_supplemental"] = {n: (b0[n]["new"] or {}).get("supplemental")
                                 for n in b0 if b0[n].get("new")}
        r["pre_old_set"] = sorted(n for n, d in b0.items() if d.get("old"))
        r["pre_new_set"] = sorted(n for n, d in b0.items() if d.get("new"))

        # ------------------------------------------------------------------
        app.send("body action clear", 3.5)
        # ------------------------------------------------------------------

        h1, b1 = app.dump(f"{tag}_post")
        s1 = app.shot(f"{tag}_post")
        r["post_lit"] = lit(s1)
        r["post_present"] = {n: (n in b1) for n in PUSHED}
        r["post_new_only"] = {n: (n in b1 and b1[n].get("old") is None) for n in PUSHED}
        r["post_old_set"] = sorted(n for n, d in b1.items() if d.get("old"))
        r["post_new_set"] = sorted(n for n, d in b1.items() if d.get("new"))
        r["px_clear"] = px8(s0, s1)
        r["shots"] = [str(s0), str(s1)]

        # F15 interaction: a cleared body's declaration must not be authored by
        # a later save. The farm's modularSystem is this run's own directory.
        app.send("body action save filename f24_after_clear", 3.5)
        saved = app.dst / "modularSystem/f24_after_clear.ini"
        r["saved_exists"] = saved.exists()
        if saved.exists():
            text = saved.read_text("latin-1")
            r["saved_sections"] = sorted(
                ln.strip()[1:-1] for ln in text.splitlines()
                if ln.strip().startswith("[") and ln.strip().endswith("]"))
            r["saved_has_pushed"] = sorted(
                s for s in r["saved_sections"] if s.split(":")[0] in PUSHED)
    finally:
        app.stop()
    return r


# =================================================================== PRELOAD ==
def texmap(table):
    return {e["name"]: e for e in table}


# THE NEW-PATH-ONLY SUBJECT, and why it has to exist for this leg to mean
# anything. `parent SolarSystem` names a SYSTEM NODE, which lives in the new
# tree only, so old's addBody refuses the push outright ("can't find parent")
# while the new loader accepts it. The body therefore has a mesh with a real
# big-levelled texture that ONLY the new path carries - which is the only way
# to separate "the new path's preload did something" from "old's preload had
# already done it": the two paths share ONE texRecap per file name (texCache),
# so on a shipped body they acquire the very same records and the table cannot
# tell which path asked. Measured, not assumed - see the Earth leg below.
ONLY = ("body action load name F24Only parent SolarSystem type Planet radius 5000 "
        "coord_func still_orbit orbit_x 0 orbit_y 0 orbit_z 3 rot_periode 24 "
        "tex_map bodies/mars.jpg")
ONLY_TEX = "textures/bodies/mars.jpg"


def acquired(table):
    return sorted(e["name"] for e in (table or []) if e["acquired"])


def run_preload(out, tag, binary):
    app = App(out / f"farm_{tag}", binary, out / f"{tag}.applog")
    app.out = out
    r = {}
    try:
        base_scene(app)
        app.send("moveto lat 0 lon 0 alt 40000000 duration 0", 3)
        app.send(ONLY, 3.0)
        h0, b0 = app.dump(f"{tag}_pre")
        r["pre_count"] = {n: (b0[n]["new"] or {}).get("preloadCount")
                          for n in ("Earth", "F24Only") if n in b0 and b0[n].get("new")}
        r["only_newonly"] = ("F24Only" in b0 and b0["F24Only"].get("old") is None)
        r["only_modules"] = (b0.get("F24Only", {}).get("new") or {}).get("modules")
        r["pre_table"] = h0.get("bigTextures")
        r["pre_acquired"] = acquired(r["pre_table"])

        # ---- the NEW-PATH-ONLY body: old's preloadBody cannot even find it ---
        # Short pauses on purpose: a big-texture record's `lifetime` is an
        # unsigned char counted DOWN once per frame, so keep_time*fps saturates
        # around 255 frames (~1.8 s at this cadence) and a dump taken seconds
        # later reads an expired, released record. The dump is written inside
        # the command handler, so what it captures is the state at command time.
        app.send(f"body action preload name F24Only keep_time 3", 0.4)
        h1, b1 = app.dump(f"{tag}_only")
        r["only_count"] = (b1.get("F24Only", {}).get("new") or {}).get("preloadCount")
        r["only_table"] = h1.get("bigTextures")
        r["only_acquired"] = acquired(r["only_table"])

        # ---- a SHIPPED body: the seam counter, and the shared-cache fact -----
        app.send("body action preload name Earth keep_time 3", 0.4)
        h2, b2 = app.dump(f"{tag}_earth")
        r["earth_count"] = (b2.get("Earth", {}).get("new") or {}).get("preloadCount")
        r["earth_table"] = h2.get("bigTextures")
        r["earth_acquired"] = acquired(r["earth_table"])
    finally:
        app.stop()
    return r


# ================================================================== POSITION ==
def run_position(out, tag, binary):
    app = App(out / f"farm_{tag}", binary, out / f"{tag}.applog")
    app.out = out
    r = {}
    try:
        base_scene(app)
        # b3_ladder's earth site: 40 000 km -> 10 000 km, outside 2R (12 756 km)
        # at BOTH ends, so the disc is drawn throughout and its angular radius
        # IS the altitude.
        app.send("camera action free_mode state on", 1.5)
        app.send("select planet Earth pointer off", 1)
        app.send("moveto lat 0 lon 270 alt 40000000 duration 0", 4)
        app.send("flag track_object on", 3)
        app.send("zoom fov 90 duration 0", 2)
        app.send("flag track_object off", 2)

        # Split the two authorities with the one shipped command that does it.
        app.send("camera action descend coef 0.5", 1.5)
        app.send("camera action descend coef 0.5", 2.5)
        hA, _ = app.dump(f"{tag}_A")
        sA = app.shot(f"{tag}_A")
        r["A"] = hA["control"]
        r["A_lit"] = lit(sA)

        app.send("position save", 2.0)

        # Move away, on both paths (`moveto` is dual).
        app.send("moveto lat 0 lon 90 alt 40000000 duration 0", 4)
        hB, _ = app.dump(f"{tag}_B")
        sB = app.shot(f"{tag}_B")
        r["B"] = hB["control"]
        r["B_lit"] = lit(sB)
        r["px_AB"] = px8(sA, sB)

        app.send("position load", 4.0)
        hC, _ = app.dump(f"{tag}_C")
        sC = app.shot(f"{tag}_C")
        r["C"] = hC["control"]
        r["C_lit"] = lit(sC)
        r["px_AC"] = px8(sA, sC)
        r["px_BC"] = px8(sB, sC)

        # SECOND ENTRY of the reversible pair, starting from the state the first
        # exit produced (no re-setup): save/move/load again.
        app.send("position save", 2.0)
        app.send("moveto lat 0 lon 90 alt 40000000 duration 0", 4)
        hB2, _ = app.dump(f"{tag}_B2")
        sB2 = app.shot(f"{tag}_B2")
        r["B2"] = hB2["control"]
        app.send("position load", 4.0)
        hC2, _ = app.dump(f"{tag}_C2")
        sC2 = app.shot(f"{tag}_C2")
        r["C2"] = hC2["control"]
        r["px_AC2"] = px8(sA, sC2)
        r["px_BC2"] = px8(sB2, sC2)

        # §5.68: the two authorities after a restore, side by side, in the
        # dump's own units. Stated, not asserted away.
        r["asym"] = {"old": r["C"]["latitude"]["old"], "new": r["C"]["latitude"]["new"],
                     "altOld": r["C"]["altitude"]["old"], "altNew": r["C"]["altitude"]["new"]}
        r["shots"] = [str(sA), str(sB), str(sC), str(sB2), str(sC2)]
    finally:
        app.stop()
    return r


# ====================================================================== main ==
def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    out = Path(args[0]).resolve() if args else HERE / "artifacts/f24"
    out.mkdir(parents=True, exist_ok=True)

    def opt(name, default=None):
        for a in sys.argv[1:]:
            if a.startswith(f"--{name}="):
                return a.split("=", 1)[1]
        if f"--{name}" in sys.argv[1:]:
            i = sys.argv[1:].index(f"--{name}")
            if i + 1 < len(sys.argv[1:]):
                return sys.argv[1:][i + 1]
        return default

    binary = opt("bin", DEFAULT_BIN)
    prebin = opt("prebin")
    only = opt("only")
    res = {"bin": binary, "prebin": prebin}

    # ------------------------------------------------------------------ clear
    if only in (None, "clear"):
        post = run_clear(out, "clear_post", binary)
        res["clear_post"] = post
        for n in PUSHED:
            if post["pre_present"].get(n):
                ok(f"CLEAR setup: {n} is in the tree before the clear")
            else:
                fail(f"CLEAR setup: {n} never reached the tree - the leg is vacuous")
        marks = post["pre_supplemental"]
        bad = [n for n in PUSHED if marks.get(n) is not True]
        if bad:
            fail(f"CLEAR: pushed bodies not marked supplemental: {bad}")
        else:
            ok("CLEAR: the three pushed bodies carry supplemental=true")
        shipped = [n for n in ("Earth", "Mars", "Sun", "Moon")
                   if marks.get(n) is not False and n in marks]
        if shipped:
            fail(f"CLEAR: file-declared bodies marked supplemental: {shipped}")
        else:
            ok("CLEAR: file-declared bodies carry supplemental=false")
        left = [n for n in PUSHED if post["post_present"].get(n)]
        if left:
            fail(f"CLEAR: still present after `body action clear`: {left}")
        else:
            ok("CLEAR: all three pushed bodies (plain, HIDDEN, nested child) are gone "
               "from BOTH trees")
        kept = set(post["post_new_set"])
        expect = set(post["pre_new_set"]) - set(PUSHED)
        if kept != expect:
            fail(f"CLEAR: the new tree lost/kept the wrong bodies: "
                 f"missing {sorted(expect - kept)}, extra {sorted(kept - expect)}")
        else:
            ok(f"CLEAR: every one of the {len(kept)} declared bodies survived")
        if post["pre_lit"] < 1000 or post["post_lit"] < 1000:
            fail(f"CLEAR: an empty frame in the scene ({post['pre_lit']} -> "
                 f"{post['post_lit']} lit px) - a px diff here would not be "
                 f"attributable to the body")
        elif post["px_clear"] <= 0:
            fail("CLEAR: the composed screen did not change across the clear")
        else:
            ok(f"CLEAR: the composed screen moved {post['px_clear']} px>8 "
               f"({post['pre_lit']} -> {post['post_lit']} lit px)")
        if post.get("saved_exists"):
            if post.get("saved_has_pushed"):
                fail(f"CLEAR/F15: a later `body action save` authored the cleared "
                     f"bodies: {post['saved_has_pushed']}")
            else:
                ok(f"CLEAR/F15: the save after the clear declares "
                   f"{len(post['saved_sections'])} sections and none of them is a "
                   f"cleared body")
        else:
            fail("CLEAR/F15: `body action save` wrote nothing - the F15 leg is vacuous")

        if prebin:
            pre = run_clear(out, "clear_pre", prebin)
            res["clear_pre"] = pre
            surv = [n for n in PUSHED if pre["post_new_only"].get(n)]
            if len(surv) == len(PUSHED):
                ok(f"CLEAR RED: on the pre-fix binary all three SURVIVE the clear on "
                   f"the new path alone (dump `old`:null) - the defect reproduces")
            else:
                fail(f"CLEAR RED: the pre-fix binary did not show the defect "
                     f"(survivors on the new path: {surv})")
            if pre["pre_old_set"] == post["pre_old_set"] and \
               pre["post_old_set"] == post["post_old_set"]:
                ok(f"CLEAR: the OLD path's removal set is IDENTICAL on the two "
                   f"binaries ({len(pre['pre_old_set'])} -> "
                   f"{len(pre['post_old_set'])} bodies)")
            else:
                fail("CLEAR: the old path's removal set changed between the binaries")
            if pre["px_clear"] < post["px_clear"]:
                ok(f"CLEAR RED: the pre-fix screen barely moves across the clear "
                   f"({pre['px_clear']} px>8) against {post['px_clear']} delivered")
            else:
                fail(f"CLEAR RED: pre-fix screen moved {pre['px_clear']} px>8, "
                     f"delivered {post['px_clear']} - no discrimination")

    # ---------------------------------------------------------------- preload
    if only in (None, "preload"):
        post = run_preload(out, "preload_post", binary)
        res["preload_post"] = post
        if post["only_newonly"]:
            ok(f"PRELOAD setup: F24Only exists on the NEW path alone "
               f"(modules {post['only_modules']}) - old's preloadBody cannot "
               f"reach it, so anything that happens is the mirror's")
        else:
            fail("PRELOAD setup: F24Only is not new-path-only - the leg cannot "
                 "attribute its effect")
        if post["pre_count"].get("Earth") == 0 and post.get("earth_count", 0) >= 1:
            ok(f"PRELOAD: the seam reaches ModularBody::preload on the body the "
               f"command names (Earth preloadCount 0 -> {post['earth_count']})")
        else:
            fail(f"PRELOAD: preloadCount did not rise on the named body "
                 f"({post['pre_count']} -> {post.get('earth_count')})")
        if post.get("only_count", 0) >= 1:
            ok(f"PRELOAD: and on the new-path-only body too "
               f"(F24Only preloadCount -> {post['only_count']})")
        else:
            fail(f"PRELOAD: the new-path-only body was not reached "
                 f"({post.get('only_count')})")
        if ONLY_TEX not in post["pre_acquired"] and ONLY_TEX in post["only_acquired"]:
            e = next(x for x in post["only_table"] if x["name"] == ONLY_TEX)
            ok(f"PRELOAD: IT DOES SOMETHING - the big-texture record for "
               f"{ONLY_TEX} is ACQUIRED after the command ({e['w']}x{e['h']}, "
               f"lifetime {e['lifetime']} frames) and was not before")
        else:
            fail(f"PRELOAD: {ONLY_TEX} did not become acquired "
                 f"(before {post['pre_acquired']}, after {post['only_acquired']})")
        earth_new = sorted(set(post["earth_acquired"]) - set(post["only_acquired"]))
        res["preload_earth_acquired"] = post["earth_acquired"]
        print(f"      shipped-body leg: preloading Earth acquires {earth_new} "
              f"(the shared-texRecap set - see the RED half)", flush=True)

        if prebin:
            pre = run_preload(out, "preload_pre", prebin)
            res["preload_pre"] = pre
            if (pre.get("only_count") or 0) == 0 and (pre.get("earth_count") or 0) == 0:
                ok("PRELOAD RED: on the pre-fix binary preloadCount stays 0 for both "
                   "subjects - ModularBody::preload is never called")
            else:
                fail(f"PRELOAD RED: the pre-fix binary called preload "
                     f"(F24Only {pre.get('only_count')}, Earth "
                     f"{pre.get('earth_count')})")
            if ONLY_TEX not in pre["only_acquired"]:
                ok(f"PRELOAD RED: and nothing is acquired for the new-path-only "
                   f"body - {ONLY_TEX} is absent from the table "
                   f"({len(pre['only_acquired'])} acquired records, none of them it)")
            else:
                fail(f"PRELOAD RED: the pre-fix binary acquired {ONLY_TEX}")
            shared = sorted(set(pre["earth_acquired"]) & set(post["earth_acquired"]))
            res["preload_shared_earth"] = shared
            print(f"      RED half of the SHIPPED body, and it is a finding rather "
                  f"than a pass: preloading Earth acquires {len(pre['earth_acquired'])} "
                  f"records on the PRE-fix binary too ({shared}) - the two paths "
                  f"share one texRecap per file name, so on a body both trees carry "
                  f"the mirror's effect is invisible in this table. The seam counter "
                  f"is what separates them there.", flush=True)

    # --------------------------------------------------------------- position
    if only in (None, "position"):
        post = run_position(out, "position_post", binary)
        res["position_post"] = post

        def alt(c):
            return c["altitude"]

        if post["A_lit"] < 1000 or post["B_lit"] < 1000 or post["C_lit"] < 1000:
            fail(f"POSITION: an empty frame in the scene "
                 f"({post['A_lit']}/{post['B_lit']}/{post['C_lit']} lit px)")
        else:
            ok(f"POSITION: the scene is lit at every leg "
               f"({post['A_lit']}/{post['B_lit']}/{post['C_lit']} px)")
        dA, dB = alt(post["A"]), alt(post["B"])
        if abs(dA["old"] - dA["new"]) > 1e6:
            ok(f"POSITION: the two authorities ARE split before the bookmark "
               f"(old {dA['old']:.3f} m, drawn {dA['new']:.3f} m)")
        else:
            fail(f"POSITION: the authorities did not split - the leg cannot "
                 f"discriminate ({dA})")
        if post["px_AB"] <= 0:
            fail("POSITION: the move away did not change the screen")
        else:
            ok(f"POSITION: the move away moved the screen {post['px_AB']} px>8")
        dC = alt(post["C"])
        if abs(dC["new"] - dA["new"]) < max(1.0, 1e-6 * abs(dA["new"])):
            ok(f"POSITION: `position load` brings the CAMERA back "
               f"({dB['new']:.3f} -> {dC['new']:.3f} m against the saved "
               f"{dA['new']:.3f} m)")
        else:
            fail(f"POSITION: the camera did not return "
                 f"(saved {dA['new']:.3f}, restored {dC['new']:.3f} m)")
        if post["px_AC"] < post["px_AB"] / 10:
            ok(f"POSITION: and the composed screen is back "
               f"({post['px_AC']} px>8 from the bookmark, against {post['px_AB']} "
               f"for the move away and {post['px_BC']} from where it was)")
        else:
            fail(f"POSITION: the screen did not return ({post['px_AC']} px>8 from "
                 f"the bookmark, {post['px_AB']} for the move)")
        dC2 = alt(post["C2"])
        if abs(dC2["new"] - dC["new"]) < max(1.0, 1e-6 * abs(dC["new"])):
            ok(f"POSITION: second entry of the pair, from the first exit's state, "
               f"lands on the same place ({dC2['new']:.3f} m, {post['px_AC2']} px>8)")
        else:
            fail(f"POSITION: the second entry diverged ({dC['new']:.3f} -> "
                 f"{dC2['new']:.3f} m)")
        a = post["asym"]
        print(f"      §5.68 after the restore: latitude old {a['old']:.9f}° / camera "
              f"{a['new']:.9f}°, altitude old {a['altOld']:.6f} m / camera "
              f"{a['altNew']:.6f} m", flush=True)

        if prebin:
            pre = run_position(out, "position_pre", prebin)
            res["position_pre"] = pre
            pA, pB, pC = alt(pre["A"]), alt(pre["B"]), alt(pre["C"])
            if abs(pC["new"] - pA["new"]) > 1e6:
                ok(f"POSITION RED: on the pre-fix binary the camera does NOT come "
                   f"back (saved-view {pA['new']:.3f} m, after load "
                   f"{pC['new']:.3f} m) - it restores the old observer alone "
                   f"({pB['old']:.3f} -> {pC['old']:.3f} m)")
            else:
                fail(f"POSITION RED: the pre-fix binary restored the camera "
                     f"({pA['new']:.3f} -> {pC['new']:.3f} m)")
            if pre["px_AC"] > pre["px_BC"]:
                ok(f"POSITION RED: and the pre-fix screen stays where the move left "
                   f"it ({pre['px_BC']} px>8 from it, {pre['px_AC']} px>8 from the "
                   f"bookmark)")
            else:
                fail(f"POSITION RED: the pre-fix screen did return "
                     f"({pre['px_AC']} px>8 from the bookmark)")

    (out / "f24_result.json").write_text(json.dumps(res, indent=1, default=str))
    print(f"\n--- {len(FAILS)} FAIL ---" if FAILS else "\n--- ALL GREEN ---", flush=True)
    for f in FAILS:
        print("   " + f, flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
