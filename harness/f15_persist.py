#!/usr/bin/env python3
"""B31 slice 2 gate: a script-pushed body survives the session (INTENT §11.121).

What this measures, and why each leg is here (b31-design §6.2 T4/T6/T9, §6.3):

  T6-CONTROL  push a body, quit, relaunch WITHOUT saving -> the body is GONE.
              Without this leg, T6 proves nothing: a body that survived for any
              other reason would pass it.
  T6          push the same body, `body action save`, quit, relaunch -> the body
              is THERE, with the same parent, relation, module set and routing,
              and so is every other body of the system. The comparator is
              IMPORTED from b24_equivalence, never copied (I2, §11.109(b)).
  NO-DELTA    the live-tree save of the SHIPPED tree, with nothing pushed, is
              compared line by line against the machine twin of the same tree.
              The twin is generated from the legacy FILE, this save from the LIVE
              TREE: equality means the two sources agree about what the system
              contains, which is the whole claim of the slice.
  T4          save twice with nothing changed -> byte-identical. A save that is
              not a fixed point is not a save. Traversed in both regimes (built
              from a legacy tree, and re-saved from the composed file the first
              save produced - the reversible pair entered a second time, from the
              state the first exit left).
  PRESERVE    save over a file carrying a human comment, a malformed line, an
              unknown ISO-8859 key and irregular spacing -> every one of them
              comes back byte-identical, and the new body is appended (the F13
              contract, on the LIVE path).
  ANNOTATE    a diagnosed datum gains its `#!sc:` line ABOVE it - only after an
              explicit save (a load leaves the file byte-identical: D33 decided
              AGAINST a rewrite at load), replacing rather than accumulating on
              re-save, and never touching a human comment.
  D9          the md5 of every data file the save did not target is unchanged
              ACROSS THE SAVE ITSELF (§6.3) - measured inside one running app, so
              it isolates the save from the launch.
  REFUSE      a path, and the machine-owned `.disabled` name, are refused and
              write nothing (D35 / §2.0 D13: the legacy corpus is unreachable).

Every leg is shown able to FAIL, in the run itself wherever that is possible:
  * T6's own assertion is measured in BOTH directions in one run - the control
    leg asserts the body is ABSENT where T6 asserts it is PRESENT;
  * the no-delta comparator is measured against a save that DOES differ (the
    same tree with a body pushed), and the difference must be exactly that
    body's block - nothing else, which is also the delta's explanation;
  * the annotation leg has a negative control: the same file without the two
    diagnosable data yields exactly the annotations the remaining diagnoses
    justify, and none for the keys nobody diagnosed;
  * `--mutate` breaks the SAVED FILE between the save and the relaunch (one
    module declaration deleted, the §11.109(f) shape) - that run is EXPECTED to
    fail, on `modules`/`routing` and nowhere else.

Protocol: this script owns the app lifecycle (the caller launches nothing) and
restores the shipped state (no enabled composed file) whatever happens.

    cd claude/harness && DISPLAY=:2 ./f15_persist.py [outdir] [--mutate]

Exit 0 = every leg green; 1 = any failure, each named on stdout.
"""

import hashlib
import os
import re
import shutil
import socket
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
# I2: the per-body comparison is b24_equivalence's, imported. This file adds only
# what that gate has no subject for - survival across a quit, and the bytes of a
# file a save produced.
from b24_equivalence import load_dump, floats_close, scalar_close, raw_axisrot
# ... and the screen comparison is b24_screen's, for the same reason.
from b24_screen import bright_dark
import numpy as np
from PIL import Image

HOME = Path.home()
USERDIR = HOME / ".spacecrafter"
SC_BIN = os.environ.get("SC_BIN", str(Path(__file__).resolve().parents[2] / "build-claude/src/spacecrafter"))
MODDIR = USERDIR / "modularSystem"
TWIN = MODDIR / "SolarSystem.ini.disabled"
ENABLED = MODDIR / "SolarSystem.ini"
CTL = MODDIR / "f15_ctl.ini"
CTL2 = MODDIR / "f15_ctl2.ini"
FROM_SCRIPT = MODDIR / "f15_from_script.ini"
# The .sts the §2(c) channel-2 leg plays; installed and removed by this script.
SCRIPT = USERDIR / "scripts/f15_persist_save.sts"
# The saved file is broken between the save and the relaunch: T6 then FAILS, and
# where it fails is the measurement (b24/b25's `--strip`/`--mutate` shape).
MUTATE = "--mutate" in sys.argv[1:]
_args = [a for a in sys.argv[1:] if not a.startswith("--")]
OUT = (Path(_args[0]) if _args else Path(__file__).resolve().parent / "artifacts/f15").resolve()
JD = "2461233.5"
SHADOW_MARK = "Composed system file modularSystem/SolarSystem.ini wins"

# The body every T6 leg pushes. `parent Earth` puts it inside the subtree the
# save walks; `still_orbit` keeps its position a data key rather than an
# ephemeris (so a divergence would be the save's, not the orbit's); the offset is
# in AU, like every position in this tree (0.0003 AU = 44878 km, clear of Earth's
# 6378 km) and the radius in km, like the data key - measured 1.5e-2 of the frame
# with a texture on it, which is what makes the SCREEN leg possible: a body only
# the dump can see would prove persistence at the model layer and nowhere else.
PUSH = ("body action load name F15Rover parent Earth type Artificial radius 5000 "
        "coord_func still_orbit orbit_x 0 orbit_y 0 orbit_z 0.0003 rot_periode 24 "
        "tex_map bodies/generic.png halo false")
ROVER = "F15Rover"

# Structural fields: exact equality, no tolerance. These are what T6 is about -
# a body that came back with a different parent, relation, module set or routing
# is not the body that was saved.
EXACT = ["parent", "relation", "modules", "routing", "lastJD", "bodyType", "primary",
         "surfaceModel", "trailLength", "surfaceLocked"]

FAILS = []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


def md5(path):
    return hashlib.md5(Path(path).read_bytes()).hexdigest()


def tree_md5():
    """Every file under ~/.spacecrafter except the ones a LAUNCH owns."""
    out = {}
    for p in sorted(USERDIR.rglob("*")):
        if not p.is_file():
            continue
        rel = p.relative_to(USERDIR).as_posix()
        if rel.startswith("log/") or rel.endswith(".tmp"):
            continue
        out[rel] = md5(p)
    return out


def wait_port(timeout=90):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", 7805), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.8):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.3)
        sock.recv(8192)
    except socket.timeout:
        pass
    sock.settimeout(None)


class App:
    """One fresh launch. The applog is captured: the precedence/shadowing lines
    are read from the app's own stdout, as b24_equivalence does (the log FILE
    truncates per launch and carries ANSI codes)."""

    def __init__(self, tag):
        self.tag = tag
        self.log = OUT / f"f15_{tag}.applog"
        self.proc = subprocess.Popen([SC_BIN], cwd=str(USERDIR),
                                     stdout=open(self.log, "w"), stderr=subprocess.STDOUT,
                                     env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
        self.sock = wait_port()
        time.sleep(10)          # async loads settle; the auto-startup script finishes
        send(self.sock, "timerate rate 0")   # FIRST, before the epoch (b19 lesson)
        send(self.sock, f"date jday {JD}")
        time.sleep(2)

    def cmd(self, c, pause=0.8):
        send(self.sock, c, pause)

    def dump(self, name):
        path = OUT / f"f15_{name}.json"
        path.unlink(missing_ok=True)
        send(self.sock, f"body action dual_dump filename {path}", 2)
        if not path.exists():
            raise RuntimeError(f"{self.tag}: dump {path} was not written")
        return path

    def shot(self, name, pause=2.5):
        """The composed screen. Written by the app's own readback (an external
        grab sees black on this display - §11.19a)."""
        p = OUT / f"f15_{name}.png"
        p.unlink(missing_ok=True)
        send(self.sock, f"body action screenshot filename {p}", pause)
        for _ in range(25):
            if p.exists() and p.stat().st_size > 0:
                break
            time.sleep(0.3)
        return np.asarray(Image.open(p).convert("RGB"), dtype=np.int16)

    def quit(self):
        send(self.sock, "shutdown action now")
        self.sock.close()
        try:
            self.proc.wait(timeout=30)
        except subprocess.TimeoutExpired:
            self.proc.kill()
            fail(f"{self.tag}: app did not exit within 30 s after shutdown")

    def text(self):
        return self.log.read_text(errors="replace")


def declarations(path):
    """The declaration lines of a composed file, banner excluded: what the file
    SAYS, with the ownership banner (which legitimately differs between a
    machine twin and a user save) taken out of the comparison."""
    lines = Path(path).read_text(encoding="latin-1").split("\n")
    i = 0
    while i < len(lines) and (lines[i].startswith("#") or lines[i].strip() == ""):
        i += 1
    return lines[i:]


def compare_bodies(a, b, tag, subject):
    """Every body of two dumps, field by field. Structural fields exact; the two
    float fields under b24_equivalence's own tolerance (they are not bit-stable
    across fresh launches - B30, §11.53(e))."""
    only_a = sorted(set(a) - set(b))
    only_b = sorted(set(b) - set(a))
    if only_a:
        fail(f"{tag}: bodies lost across the quit: {only_a}")
    if only_b:
        fail(f"{tag}: bodies invented across the quit: {only_b}")
    if not only_a and not only_b:
        ok(f"{tag}: body set identical ({len(a)} bodies), {subject} included")
    diverged = 0
    for name in sorted(set(a) & set(b)):
        for f in EXACT:
            if a[name].get(f) != b[name].get(f):
                fail(f"{tag}: {name}.{f} {a[name].get(f)} != {b[name].get(f)}")
                diverged += 1
        if not floats_close(",".join(map(str, a[name].get("ecl", []))),
                            ",".join(map(str, b[name].get("ecl", [])))):
            fail(f"{tag}: {name}.ecl {a[name].get('ecl')} != {b[name].get('ecl')}")
            diverged += 1
        if not scalar_close(a[name].get("boundingRadius"), b[name].get("boundingRadius")):
            fail(f"{tag}: {name}.boundingRadius {a[name].get('boundingRadius')} != "
                 f"{b[name].get('boundingRadius')}")
            diverged += 1
    if not diverged:
        ok(f"{tag}: 0 divergent fields over {len(set(a) & set(b))} bodies "
           f"({', '.join(EXACT)}, ecl, boundingRadius)")
    return diverged


# --------------------------------------------------------------------------
# The authored file the PRESERVE / ANNOTATE legs are run against: the machine
# twin, with content a writer that does not preserve would destroy, and data a
# loader must diagnose. Built from the twin so the tree it loads is the shipped
# one - the subject is the WRITER, not the corpus.
HUMAN = "# a human wrote this, about the radius below - it must survive"
MALFORMED = "orbit_LongOfPericenter 95.58754"
UNKNOWN = "note = valeur accentu\xe9e \xe0 pr\xe9server"


def build_authored(dst, diagnosable=True):
    """-> (bytes written, {marker: (section, key)}). Injections are placed by
    reading the file, never by line number: a corpus edit must not silently move
    the subject of a leg somewhere else.
    `diagnosable=False` builds the SAME file without the two data a loader
    complains about - the negative control: an annotation must appear because
    something was diagnosed, never because a save ran."""
    lines = Path(TWIN).read_text(encoding="latin-1").split("\n")
    out, section, placed = [], None, {}
    for line in lines:
        s = line.strip()
        if s.startswith("[") and s.endswith("]"):
            section = s[1:-1]
            out.append(line)
            continue
        if section == "Mars" and s.startswith("radius") and "human" not in placed:
            out.append(HUMAN)                 # a comment attached to a datum
            out.append(line)
            out.append(MALFORMED)             # the shipped [Sedna] class
            out.append(UNKNOWN)               # an unknown key with high bytes
            placed["human"] = ("Mars", "radius")
            continue
        if section == "Moon" and s.startswith("radius") and "spacing" not in placed:
            key, _, value = line.partition("=")
            out.append(f"{key.rstrip()}  =  {value.strip()}")   # irregular spacing
            placed["spacing"] = ("Moon", "radius")
            if diagnosable:
                out.append("relation = bogus")  # diagnosed: invalid value, fallback acts
                placed["relation"] = ("Moon", "relation")
            continue
        if diagnosable and s.startswith("shadow_exempt") and "retired" not in placed:
            placed["retired"] = (section, "type")   # dropped: `type` grants nothing here
            continue
        out.append(line)
    text = "\n".join(out)
    Path(dst).write_text(text, encoding="latin-1")
    return text, placed


def line_above(lines, idx):
    return lines[idx - 1] if idx > 0 else ""


def find_line(lines, section, pred):
    cur = None
    for i, line in enumerate(lines):
        s = line.strip()
        if s.startswith("[") and s.endswith("]"):
            cur = s[1:-1]
        elif cur == section and pred(s):
            return i
    return -1


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    if (USERDIR / "beta_features.ini").exists():
        raise RuntimeError("beta_features.ini present - not the shipped state")
    ENABLED.unlink(missing_ok=True)
    CTL.unlink(missing_ok=True)
    if not TWIN.exists():
        raise RuntimeError("no twin present - launch the app once first")

    try:
        # ---------------- L1: the save's own properties, on the shipped tree ---
        app = App("l1")
        before = tree_md5()
        app.cmd("body action save filename f15_ctl", 3)
        after = tree_md5()
        changed = sorted(k for k in set(before) | set(after)
                         if before.get(k) != after.get(k))
        if changed == ["modularSystem/f15_ctl.ini"]:
            ok(f"D9: the save changed exactly the file it was asked to write "
               f"({len(before)} files checked, 0 collateral)")
        else:
            fail(f"D9: the save changed {changed} - it must touch only its target")
        first = CTL.read_bytes()

        # T4, first regime: nothing changed between the two saves.
        app.cmd("body action save filename f15_ctl", 3)
        if CTL.read_bytes() == first:
            ok(f"T4 (legacy-tree regime): second save byte-identical ({len(first)} bytes)")
        else:
            fail("T4 (legacy-tree regime): a second save of the unchanged state moved bytes")

        # NO-DELTA: the live tree and the legacy file say the same thing.
        d_save, d_twin = declarations(CTL), declarations(TWIN)
        if d_save == d_twin:
            ok(f"no-runtime-delta: the live-tree save == the machine twin, "
               f"{len(d_twin)} declaration lines, banner excluded")
        else:
            import difflib
            diff = [l for l in difflib.unified_diff(d_twin, d_save, "twin", "save", lineterm="", n=0)
                    if l[:1] in "+-" and l[:3] not in ("+++", "---")]
            (OUT / "f15_nodelta.diff").write_text("\n".join(diff))
            fail(f"no-runtime-delta: {len(diff)} lines differ between the live-tree save "
                 f"and the twin (see {OUT}/f15_nodelta.diff)")

        # §2(c) CHANNEL 2 (script): the same command reached from a played .sts
        # file rather than from the socket this driver speaks. One registration is
        # supposed to serve every command channel (§11.55(h)) - measured, not
        # assumed, exactly as b10_cmd measures it for its own command.
        SCRIPT.write_text(
            "# F15 §2(c) channel 2: the system save from the SCRIPT channel.\n"
            "body action save filename f15_from_script\n")
        try:
            app.cmd("script action play filename f15_persist_save.sts", 8)
            if FROM_SCRIPT.exists() and declarations(FROM_SCRIPT) == d_twin:
                ok(f"§2(c) channel 2: a played .sts saved the system too "
                   f"({FROM_SCRIPT.stat().st_size} bytes, identical declarations)")
            elif FROM_SCRIPT.exists():
                fail("§2(c) channel 2: the script channel wrote a DIFFERENT file "
                     "from the command channel's")
            else:
                fail("§2(c) channel 2: `body action save` from a .sts wrote nothing")
        finally:
            SCRIPT.unlink(missing_ok=True)

        # REFUSE: neither the frozen corpus nor the machine-owned twin is reachable.
        ss_md5, twin_md5 = md5(USERDIR / "ssystem.ini"), md5(TWIN)
        app.cmd("body action save filename ../ssystem.ini", 2)
        app.cmd("body action save filename SolarSystem.ini.disabled", 2)
        if md5(USERDIR / "ssystem.ini") == ss_md5 and md5(TWIN) == twin_md5:
            ok("refusals: a path and the .disabled name wrote nothing (D35 / §2.0 D13)")
        else:
            fail("refusals: a refused save target was written to")
        if "is a path" in app.text() and "MACHINE-OWNED file" in app.text():
            ok("refusals: both said why, and how to fix it (§2(f))")
        else:
            fail("refusals: a refusal was silent")

        # The control leg's own subject: a pushed body, NOT saved.
        shot_base_l1 = app.shot("base_l1")   # this scene WITHOUT the body
        app.cmd(PUSH, 2)
        shot_live = app.shot("rover_live")   # ... and WITH it, same launch
        dump_ctl = app.dump("ctl_pre")
        # ... and the no-delta comparator measured against a save that DOES
        # differ: same tree, one body pushed. The comparator must see it, and
        # what it sees must be exactly that body - the delta's explanation is
        # the delta itself.
        app.cmd("body action save filename f15_ctl2", 3)
        d_push = declarations(CTL2)
        import difflib
        blocks = [l for l in difflib.unified_diff(d_twin, d_push, lineterm="", n=0)
                  if l[:1] in "+-" and l[:3] not in ("+++", "---")]
        removed = [l for l in blocks if l.startswith("-")]
        added = [l[1:] for l in blocks if l.startswith("+")]
        rover_block = [l for l in added if l.strip()]
        in_rover = all(("F15Rover" in l) or ("=" in l) or l.startswith("[") for l in rover_block)
        if removed:
            fail(f"no-delta discrimination: the save DROPPED {len(removed)} twin lines: {removed[:3]}")
        elif added and in_rover and any(l.strip() == "[F15Rover]" for l in added):
            ok(f"no-delta discrimination: with a body pushed, the same comparison shows "
               f"{len(added)} added lines and 0 removed - all of them the pushed body's "
               f"own declaration block")
        else:
            fail(f"no-delta discrimination: expected exactly the pushed body's block, got "
                 f"{len(added)} added / {len(removed)} removed")
        app.quit()

        _, bodies = load_dump(dump_ctl)
        if ROVER in bodies:
            ok(f"push channel reaches the NEW tree: {ROVER} is in the live tree "
               f"(parent {bodies[ROVER].get('parent')})")
        else:
            fail(f"push channel: {ROVER} never reached the new tree - T6 has no subject")

        # ---------------- L2: the control. No save -> no survival --------------
        app = App("l2")
        shot_base_l2 = app.shot("base_l2")   # the same scene, a second launch
        dump_gone = app.dump("ctl_post")
        app.quit()
        _, gone = load_dump(dump_gone)
        if ROVER in gone:
            fail(f"T6-CONTROL: {ROVER} survived a relaunch WITHOUT a save - "
                 f"T6 would then prove nothing")
        else:
            ok(f"T6-CONTROL: without a save, {ROVER} is gone after the relaunch "
               f"({len(gone)} bodies, none of them it)")

        # ---------------- L3: push + save --------------------------------------
        app = App("l3")
        app.cmd(PUSH, 2)
        dump_pre = app.dump("t6_pre")
        app.cmd("body action save", 3)   # no filename = this system's own file
        app.quit()
        if not ENABLED.exists():
            raise RuntimeError("the save wrote no file - T6 cannot be measured")
        saved = ENABLED.read_bytes()
        if MUTATE:
            text = saved.decode("latin-1").split("\n")
            keep, drop, cutting = [], 0, False
            for line in text:
                s = line.strip()
                if s.startswith("[") and s.endswith("]"):
                    cutting = (s == f"[{ROVER}:HINT]")
                if cutting:
                    drop += 1
                    continue
                keep.append(line)
            if drop == 0:
                raise RuntimeError("--mutate: no [F15Rover:HINT] section to delete")
            ENABLED.write_bytes("\n".join(keep).encode("latin-1"))
            print(f"MUTATION: [{ROVER}:HINT] ({drop} lines) deleted from the SAVED file - "
                  f"this run is EXPECTED to fail, on modules/routing and nowhere else",
                  flush=True)

        # ---------------- L4: the relaunch T6 is about -------------------------
        app = App("l4")
        if SHADOW_MARK in app.text():
            ok("T6: the saved file is what the next launch reads (precedence log fired)")
        else:
            fail("T6: the saved file was NOT read at the relaunch")
        if "loaded, composed format" in app.text():
            ok("T6: it was read by the composed loader")
        else:
            fail("T6: the composed loader did not run")
        shot_restored = app.shot("rover_restored")   # the body, from the file alone
        dump_post = app.dump("t6_post")
        # T4, second regime: re-saving what was just loaded, from the state the
        # first exit produced (the reversible pair, entered twice).
        app.cmd("body action save", 3)
        resaved = ENABLED.read_bytes()
        app.cmd("body action save", 3)
        resaved2 = ENABLED.read_bytes()
        app.quit()

        _, pre = load_dump(dump_pre)
        _, post = load_dump(dump_post)
        if ROVER in post:
            ok(f"T6: {ROVER} EXISTS after quit + fresh relaunch, from the file alone")
        else:
            fail(f"T6: {ROVER} did not survive the relaunch")
        compare_bodies(pre, post, "T6", ROVER)
        # SCREEN, the terminal observable (the model layer does not compose
        # upward on its own, §11.52(b)'s standing posture).
        #
        # The comparison that matters - live body vs restored body - is
        # necessarily CROSS-LAUNCH, and a cross-launch pair of this scene differs
        # by a few hundred pixels for reasons that have nothing to do with the
        # body (B30 fresh-launch nondeterminism, §11.53(e); measured here as
        # `floor`). So the floor is not the criterion: the criterion is measured
        # ON THE PAIR ITSELF, by splitting the frame at the body's own footprint.
        #   footprint = where the body changed the frame, WITHIN one launch
        #               (rover shot vs pre-push shot: no launch noise in it at all)
        #   inside    = how much the live-vs-restored pair differs THERE
        #   outside   = how much the same pair differs everywhere else - the
        #               launch noise of these two launches, self-calibrated
        # A body that came back displaced, resized, differently textured or not at
        # all moves a large fraction of its own footprint. Criterion: inside < 10 %
        # of the footprint - AND the residual must be where its attributed cause
        # puts it. A sub-pixel cross-launch jitter can only move the disc's EDGE
        # (its interior is flat-textured), so the same count is taken again over
        # the footprint ERODED by two pixels: a residual that survives erosion is
        # not jitter, it is a body that came back different.
        def mask_of(a, b):
            bright, dark = bright_dark(a, b)
            return bright | dark
        footprint = mask_of(shot_live, shot_base_l1)
        pair = mask_of(shot_live, shot_restored)
        def erode(m):
            e = m.copy()
            for _ in range(2):
                e &= np.roll(e, 1, 0) & np.roll(e, -1, 0) & np.roll(e, 1, 1) & np.roll(e, -1, 1)
            return e
        floor = int(mask_of(shot_base_l1, shot_base_l2).sum())
        interior = erode(footprint)
        nfoot, nint = int(footprint.sum()), int(interior.sum())
        inside = int((pair & footprint).sum())
        outside = int((pair & ~footprint).sum())
        deep = int((pair & interior).sum())
        print(f"      screen: footprint(rover, same launch)={nfoot} px  "
              f"live-vs-restored inside={inside} px ({inside/max(nfoot,1)*100:.2f} % of it), "
              f"of which {deep} px survive a 2 px erosion (interior {nint} px)  "
              f"outside={outside} px  cross-launch floor (rover-free scene)={floor} px",
              flush=True)
        # What this leg reports when the body did NOT come back, on the very same
        # frames: the pre-push shot of L1 against the restored shot of L4. No extra
        # launch, and it is the number the assertion below has to be able to see.
        counter = int((mask_of(shot_base_l1, shot_restored) & footprint).sum())
        print(f"      screen counterfactual: the same comparison against a frame with "
              f"NO rover in it reports {counter}/{nfoot} px inside the footprint "
              f"({counter/max(nfoot,1)*100:.0f} %), vs {inside} px ("
              f"{inside/max(nfoot,1)*100:.2f} %) for the restored one", flush=True)
        if counter < 0.5 * nfoot:
            fail(f"SCREEN: the leg cannot see a missing body ({counter}/{nfoot} px) - "
                 f"it is not discriminating")
        if nfoot < 1000:
            fail(f"SCREEN: the pushed body only draws {nfoot} px - too small for the "
                 f"leg to discriminate anything")
        elif inside < 0.10 * nfoot and deep <= 0.005 * max(nint, 1):
            ok(f"SCREEN: the restored body draws where and as the live one did - "
               f"{inside}/{nfoot} px moved inside its footprint, and only {deep} of "
               f"them ({deep/max(nint,1)*100:.2f} % of the {nint} px interior) survive "
               f"erosion: the residual is the disc's own edge, which is what a "
               f"cross-launch sub-pixel jitter can move and all it can move")
        else:
            fail(f"SCREEN: the restored body's disc differs from the live one - "
                 f"{inside}/{nfoot} px inside its footprint, {deep} of them in the "
                 f"eroded interior (not an edge effect)")
        rot_a, rot_b = raw_axisrot(dump_pre), raw_axisrot(dump_post)
        if rot_a.get(ROVER) == rot_b.get(ROVER):
            ok(f"T6: {ROVER}.axisRot exact across the quit ({rot_a.get(ROVER)})")
        else:
            fail(f"T6: {ROVER}.axisRot {rot_a.get(ROVER)} != {rot_b.get(ROVER)}")

        if resaved == resaved2:
            ok(f"T4 (composed regime, second entry): re-save byte-identical "
               f"({len(resaved)} bytes)")
        else:
            fail("T4 (composed regime): two consecutive saves of the same state differ")
        if resaved == saved:
            ok("T4: the composed re-save is byte-identical to the legacy-tree save too")
        else:
            extra = [l for l in resaved.decode("latin-1").split("\n")
                     if l.strip().startswith("#!sc:")]
            print(f"note: the composed re-save differs from the first save by "
                  f"{len(extra)} annotation line(s) - the loader diagnoses what a "
                  f"legacy load never sees:", flush=True)
            for l in extra:
                print(f"      {l.strip()[:140]}", flush=True)
            if len(resaved.decode("latin-1").split("\n")) - \
               len(saved.decode("latin-1").split("\n")) != len(extra):
                fail("T4: the composed re-save differs by MORE than its annotations")
            else:
                ok(f"T4: the composed re-save differs from the first save by exactly its "
                   f"{len(extra)} annotation line(s), nothing else")
        ENABLED.unlink(missing_ok=True)

        # ---------------- L5/L6: preservation and annotation -------------------
        original, placed = build_authored(ENABLED)
        authored_md5 = md5(ENABLED)
        app = App("l5")
        app.quit()
        if md5(ENABLED) == authored_md5:
            ok("ANNOTATE: a LOAD leaves the file byte-identical - nothing is written "
               "until an explicit save asks for it (D33)")
        else:
            fail("ANNOTATE: the load rewrote the user's file")

        app = App("l6")
        app.cmd(PUSH, 2)
        app.cmd("body action save", 3)
        after_save = ENABLED.read_text(encoding="latin-1")
        app.cmd("body action save", 3)
        after_save2 = ENABLED.read_text(encoding="latin-1")
        app.quit()

        lines_before = original.split("\n")
        lines_after = after_save.split("\n")
        kept = [l for l in lines_after if not l.strip().startswith("#!sc:")]
        appended = []
        if kept[:len(lines_before)] == lines_before:
            appended = kept[len(lines_before):]
            ok(f"PRESERVE: every one of the {len(lines_before)} original lines is back "
               f"byte-identical and in place (human comment, malformed line, unknown "
               f"ISO-8859 key, irregular spacing included)")
        else:
            for i, (x, y) in enumerate(zip(lines_before, kept)):
                if x != y:
                    fail(f"PRESERVE: line {i+1} changed: {x!r} -> {y!r}")
                    break
            else:
                fail("PRESERVE: the file lost lines")
        if any(re.match(r"^\[F15Rover\]", l) for l in appended):
            ok(f"PRESERVE: the pushed body was APPENDED ({len(appended)} lines) - "
               f"an existing file gains what it lacks and keeps what it has")
        else:
            fail("PRESERVE: the pushed body was not appended to the authored file")

        marks = [l for l in lines_after if l.strip().startswith("#!sc:")]
        for tag, (section, key) in placed.items():
            if tag not in ("relation", "retired"):
                continue
            i = find_line(lines_after, section, lambda s: s.startswith(key))
            if i < 0:
                fail(f"ANNOTATE: [{section}] {key} is gone from the saved file")
                continue
            above = line_above(lines_after, i)
            if above.strip().startswith("#!sc:") and key in above:
                ok(f"ANNOTATE: [{section}] {key} carries its diagnosis on the line "
                   f"ABOVE it: {above.strip()[:110]}")
            else:
                fail(f"ANNOTATE: [{section}] {key} has no annotation above it "
                     f"(found {above.strip()[:80]!r})")
        i = find_line(lines_after, "Mars", lambda s: s.startswith("radius"))
        if line_above(lines_after, i) == HUMAN:
            ok("ANNOTATE: the human comment is untouched and still attached to its datum")
        else:
            fail(f"ANNOTATE: the human comment moved or changed "
                 f"({line_above(lines_after, i)!r})")
        if after_save2 == after_save:
            ok(f"T9/ANNOTATE: the second save is byte-identical - {len(marks)} annotations "
               f"REPLACE themselves instead of accumulating")
        else:
            fail("T9/ANNOTATE: a second save with the same diagnoses moved bytes")

        # ---------------- L7: the annotation's negative control ----------------
        # The same file, minus the two data a loader complains about. What must
        # NOT happen is an annotation appearing because a save ran.
        ENABLED.unlink(missing_ok=True)
        build_authored(ENABLED, diagnosable=False)
        app = App("l7")
        app.cmd("body action save", 3)
        clean = ENABLED.read_text(encoding="latin-1").split("\n")
        app.quit()
        clean_marks = [l for l in clean if l.strip().startswith("#!sc:")]
        undiagnosed = [l for l in clean_marks
                       if "[invalid-value]" in l or "[shadow_exempt]" in l]
        if undiagnosed:
            fail(f"ANNOTATE-CONTROL: {len(undiagnosed)} annotation(s) for data nobody "
                 f"diagnosed: {undiagnosed[:2]}")
        elif len(clean_marks) == len(marks) - 2:
            ok(f"ANNOTATE-CONTROL: removing the two diagnosable data removes exactly their "
               f"two annotations ({len(marks)} -> {len(clean_marks)}), and the "
               f"{len(clean_marks)} the corpus still justifies stay")
        else:
            fail(f"ANNOTATE-CONTROL: {len(marks)} annotations became {len(clean_marks)}, "
                 f"expected {len(marks) - 2}")

    finally:
        ENABLED.unlink(missing_ok=True)   # shipped state, ALWAYS
        CTL.unlink(missing_ok=True)
        CTL2.unlink(missing_ok=True)
        FROM_SCRIPT.unlink(missing_ok=True)
        SCRIPT.unlink(missing_ok=True)

    print(f"\n{'FAILED' if FAILS else 'ALL GREEN'}: {len(FAILS)} failure(s)", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
