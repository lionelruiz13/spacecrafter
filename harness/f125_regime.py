#!/usr/bin/env python3
"""F125 - WHERE do the regimes switch? A positive measurement against the owner's constants
(INTENT 11.263 / 11.251(a)(5): early / full / bigTexture at 2 / 16 / 256 px).

    bash f124_run.sh ../f125/<label> f125_adapt.py --gdb f125_regime.py [--body Mars]
                                     [--perturb early=<px>|full=<px>|big=<px>]

No pixel is read. For ONE body, at screen sizes bracketing each constant (x0.95 0.98 1.02
1.05, ascending), gdb counts over about 1.5 s of real frames what ModularBody::draw calls:
    Renderer::clearDepth(z, boundingRadius of the body)   -> FULL regime (own depth slice)
    <near module>::drawNoDepth(renderer, the body, ...)    -> MID band (depth-less surface)
    neither                                                -> below EARLY (halo only)
and the dump header's `bigTextures[].acquired` tells the third gate. The screen size is the
app's own (`screenSize` of the body record) times the render width (`gates.viewportRadius`);
the constants are read from ModularBody.hpp (f125_constants). A check passes when the switch
lies inside (0.98 C, 1.02 C).

--perturb moves the ENGINE's gate through gdb (the static the draw path compares with), the
script's expectation staying on the declared constant: the matching check must go red. Run it
before trusting a green. NOT measured: the two un-named literals 0.004 / 0.2 (veto-open).
"""
import argparse, ast, json, os, re, signal, socket, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from f125_constants import owner_constant

ap = argparse.ArgumentParser()
ap.add_argument("out"); ap.add_argument("gdbpid", type=int); ap.add_argument("fifo")
ap.add_argument("--body", default="Mars")
ap.add_argument("--perturb", default="none")
ARGS = ap.parse_args()
OUT, GDBPID, FIFO = ARGS.out, ARGS.gdbpid, ARGS.fifo
GDBLOG = OUT + "/gdb.log"
FAILS = []
BODY = ARGS.body

# Take F124's reuse() from its file, then b21's helpers through it: change them there.
_src = ast.parse(open(HERE + "/f124_legs.py").read())
exec(compile(ast.Module([n for n in _src.body
                         if isinstance(n, ast.FunctionDef) and n.name == "reuse"], []),
             "f124_legs.py", "exec"))
reuse(HERE + "/b21_keypath.py", ["check", "send", "_clean", "gdb_marker_count", "gdb_inject"])

GATES = (("early", "BODY_EARLY_VISIBILITY_BOUNDING_SIZE", "earlyVisibilityScreenSize"),
         ("full", "BODY_FULL_VISIBILITY_BOUNDING_SIZE", "fullVisibilityScreenSize"),
         ("big", "BODY_BIG_TEXTURE_BOUNDING_SIZE", "bigTextureScreenSize"))
FACTORS = (0.95, 0.98, 1.02, 1.05)
BIN = os.environ.get("SC_BIN") or os.path.join(HERE, "..", "..", "build-claude", "src", "spacecrafter")
RES = {"body": BODY, "perturb": ARGS.perturb, "points": []}
STEP = [0]


def dump(tag, pause=2.5):
    path = "%s/%s.json" % (OUT, tag)
    send(S, "body action dual_dump filename " + path, pause)
    head = rec = None
    with open(path) as f:
        for ln in f:
            d = json.loads(_clean(ln))
            if d.get("type") == "header":
                head = d
            elif d.get("type") == "body" and d.get("name") == BODY:
                rec = d.get("new")
    return head, rec


def inject(cmds):
    mark = "F125STEP%d" % STEP[0]
    STEP[0] += 1
    since = os.path.getsize(GDBLOG)
    ok = gdb_inject(cmds, mark)
    with open(GDBLOG, "rb") as f:
        f.seek(since)
        txt = f.read().decode("latin-1")
    return ok, txt


def entry_points():
    """Load addresses of clearDepth and of every drawNoDepth: gdb refuses `break *` on a
    virtual member by name, and at the first instruction the arguments are still in registers."""
    offs = {}
    for ln in subprocess.run(["nm", "-C", BIN], capture_output=True, text=True).stdout.splitlines():
        m = re.match(r"^([0-9a-f]+) [TtWw] ((?:\w+::)+(?:drawNoDepth|clearDepth))\(", ln)
        if m and "[clone" not in ln:
            offs[m.group(2)] = int(m.group(1), 16)
    ok, txt = inject(["info address Renderer::clearDepth"])
    m = re.search(r"at address (0x[0-9a-f]+)", txt)
    if not (ok and m and "Renderer::clearDepth" in offs):
        sys.exit("cannot locate Renderer::clearDepth in the live process: %r" % txt[-200:])
    base = int(m.group(1), 16) - offs["Renderer::clearDepth"]
    return {name: base + off for name, off in offs.items()}


def probe(bounding):
    """Hit counts, for this body only, over about 1.5 s of real frames."""
    lo, hi = bounding * 0.9999, bounding * 1.0001
    arm = []
    for name, addr in sorted(ENTRY.items()):
        if name.endswith("clearDepth"):
            cond = "$xmm1.v4_float[0] > %.9g && $xmm1.v4_float[0] < %.9g" % (lo, hi)
        else:
            cond = ("((ModularBody *) $rdx)->boundingRadius > %.9g"
                    " && ((ModularBody *) $rdx)->boundingRadius < %.9g" % (lo, hi))
        arm += ["break *0x%x if %s" % (addr, cond), "ignore $bpnum 100000000"]
    ok1, txt1 = inject(arm)
    time.sleep(1.5)
    ok2, txt2 = inject(["info breakpoints", "delete breakpoints"])
    clear = nodepth = 0
    where = None
    for ln in txt2.splitlines():
        m = re.match(r"^\d+\s+breakpoint\s.*\sin\s+(\S+?)\(", ln) or \
            re.match(r"^\d+\s+breakpoint\s.*<(\S+?)\(", ln)
        if m:
            where = m.group(1)
        h = re.search(r"breakpoint already hit (\d+) time", ln)
        if h and where:
            if where.endswith("clearDepth"):
                clear += int(h.group(1))
            elif where.endswith("drawNoDepth"):
                nodepth += int(h.group(1))
    bad = [l for l in (txt1 + txt2).splitlines()
           if any(k in l for k in ("No symbol", "Cannot ", "Couldn't", "There is no member",
                                   "Invalid ", "Error in testing", "SIGSEGV", "SIGABRT"))]
    armed = len(re.findall(r"^\d+\s+breakpoint\s", txt2, re.M))
    return {"clearDepth": clear, "drawNoDepth": nodepth, "armed": armed,
            "handshake": ok1 and ok2, "gdb_said": bad[:3]}


S = socket.create_connection(("127.0.0.1", 7805), timeout=15)
send(S, "flag experimental_path on", 1)
send(S, "date jday 2461233.5", 1)
send(S, "timerate rate 0", 1)
for f in ("landscape", "atmosphere", "star_twinkle", "stars", "show_fps", "planet_names",
          "planet_orbits", "object_trails", "constellation_drawing", "constellation_art",
          "cardinal_points", "milky_way", "nebulae"):
    send(S, "flag %s off" % f, 0.3)
send(S, "set home_planet Earth", 3)
send(S, "moveto lat 48.85 lon 2.35 alt 100 duration 0", 2)
send(S, "select planet %s pointer off" % BODY, 1)
send(S, "flag track_object on", 8)
send(S, "timerate rate 0", 1)

REF_FOV = 60.0
send(S, "zoom fov %g duration 0" % REF_FOV, 3)
h0, b0 = dump("ref")
WIDTH = 2.0 * h0["gates"]["viewportRadius"]
K = b0["screenSize"] * REF_FOV              # screenSize x fov is invariant for a frozen scene
RES.update(width=WIDTH, gates_reported=h0["gates"], ref_screenSize=b0["screenSize"],
           boundingRadius=b0["boundingRadius"])
CONST = {g: owner_constant(name) for g, name, _ in GATES}
ENTRY = entry_points()
RES["probed_functions"] = sorted(ENTRY)
for g, _, _ in GATES:
    check("declared_%s_is_what_the_app_reports" % g,
          h0["gates"]["px"]["bigTexture" if g == "big" else g] == CONST[g],
          "header %g px, dump gates.px %s" % (CONST[g], h0["gates"]["px"]))

if ARGS.perturb != "none":
    which, px = ARGS.perturb.split("=")
    static = dict((g, s) for g, _, s in GATES)[which]
    ok, txt = inject(["set var ModularBody::%s = %.9g" % (static, float(px) / WIDTH),
                      "print ModularBody::%s" % static])
    print("PERTURB: the engine's %s gate moved to %s px through gdb (handshake %s): %s"
          % (which, px, ok, [l for l in txt.splitlines() if l.startswith("$")]), flush=True)

for g, _, _ in GATES:
    for fac in FACTORS:
        want_px = CONST[g] * fac
        send(S, "zoom fov %.9f duration 0" % (K / (want_px / WIDTH)), 3)
        tag = "%s_x%.2f" % (g, fac)
        h, b = dump(tag)
        pt = {"gate": g, "factor": fac, "wanted_px": want_px,
              "measured_px": b["screenSize"] * WIDTH,
              "bigTextures": [t for t in h["bigTextures"]]}
        if g != "big":
            pt.update(probe(b["boundingRadius"]))
            pt["regime"] = ("full" if pt["clearDepth"] else "mid" if pt["drawNoDepth"] else "below-early")
        else:
            time.sleep(3)   # the acquisition is asynchronous: read it again once settled
            h, _ = dump(tag + "_settled")
            pt["bigTextures"] = [t for t in h["bigTextures"]]
            pt["acquired"] = sorted(t["name"] for t in h["bigTextures"] if t["acquired"])
        RES["points"].append(pt)
        print("### %-11s wanted %8.3f px measured %8.3f px -> %s" % (
            tag, want_px, pt["measured_px"],
            pt.get("regime") or ("acquired " + str(pt["acquired"]))) + (
            "  (clearDepth %d, drawNoDepth %d, %d probes armed%s)" % (
                pt["clearDepth"], pt["drawNoDepth"], pt["armed"],
                ", gdb said " + str(pt["gdb_said"]) if pt["gdb_said"] else "")
            if g != "big" else ""), flush=True)
send(S, "zoom fov 180 duration 0", 2)


def pts(g):
    return [p for p in RES["points"] if p["gate"] == g]


probed = [p for p in RES["points"] if p["gate"] != "big"]
check("probes_armed_and_clean", all(p["handshake"] and p["armed"] == len(ENTRY) and not p["gdb_said"]
                                    for p in probed),
      "every point armed its %d breakpoints and gdb reported no error" % len(ENTRY))
want = {"early": ("below-early", "mid"), "full": ("mid", "full")}
for g in ("early", "full"):
    got = [p["regime"] for p in pts(g)]
    lo, hi = want[g]
    check("%s_switches_at_the_declared_%g_px" % (g, CONST[g]), got == [lo, lo, hi, hi],
          "regimes at x0.95 0.98 1.02 1.05 = %s: the switch lies in (%.3f, %.3f) px"
          % (got, pts(g)[1]["measured_px"], pts(g)[2]["measured_px"]) if got == [lo, lo, hi, hi]
          else "regimes at x0.95 0.98 1.02 1.05 = %s, expected %s" % (got, [lo, lo, hi, hi]))
acq = [bool(p["acquired"]) for p in pts("big")]
check("big_texture_acquired_at_the_declared_%g_px" % CONST["big"], acq == [False, False, True, True],
      "acquired at x0.95 0.98 1.02 1.05 = %s (%s)" % (acq, pts("big")[-1]["acquired"]))

with open(OUT + "/f125_regime_results.json", "w") as f:
    json.dump(RES, f, indent=1)
print("\n%s" % ("ALL PASS" if not FAILS else "%d FAILS: %s" % (len(FAILS), ", ".join(FAILS))), flush=True)
if ARGS.perturb != "none":
    red = [n for n in FAILS if n.startswith(ARGS.perturb.split("=")[0])]
    print("ABLE TO FAIL: %s" % ("RED as it must - " + ", ".join(red) if red
                                else "STAYED GREEN - THE INSTRUMENT IS BLIND"), flush=True)
    sys.exit(0 if red else 1)
sys.exit(1 if FAILS else 0)
