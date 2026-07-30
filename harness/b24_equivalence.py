#!/usr/bin/env python3
"""B24/B25 legacy-vs-composed equivalence gate (INTENT 11.78; ledger B24/B25).

The generated `.ini.disabled` twin is the corpus-wide coverage test of the
composition grammar [vixy, 11.50(b)]: loading it (enabled) must reproduce the
legacy load exactly. This driver runs BOTH loads in fresh launches over the
whole shipped corpus (every body of ~/.spacecrafter/ssystem.ini) and compares
the new-path tree numerically.

Protocol (the caller launches NOTHING - this script owns the app lifecycle):
    cd claude/harness && ./b24_equivalence.py [outdir]     # default artifacts/b24

Phase A (legacy): shipped state asserted (no enabled composed file), fresh
launch, frozen scene, dual_dump, clean shutdown. The launch itself regenerates
the twin (generation-at-load is the product behavior under test).
Phase B (composed): the twin is copied to modularSystem/SolarSystem.ini
(the documented adoption workflow), fresh launch, SAME frozen scene,
dual_dump, clean shutdown, enabled file REMOVED (shipped state restored -
a leftover enabled file silently re-specifies every next run, B26 hygiene).

Comparison (new-path fields of the dual_dump, matched by body name):
  - body set equality (a body lost or invented by the composed load = FAIL),
  - per body EXACT match on the STRUCTURAL fields: parent, relation,
    modules (slot inventory), routing (per-list counts), lastJD - these carry
    no float jitter and are where a broken respell shows (a dropped/added/
    re-routed module, a lost/re-parented body); the [Moon:MESH]-deletion
    discrimination lands here,
  - ecl (parent-relative orbit output) and boundingRadius compared under a
    FLOAT32-EPSILON tolerance, NOT exact strings: both are new-path float
    fields and are not bit-stable across fresh launches (B30, INTENT 11.53(e)),
    proven pre-existing (the MilkyWay/Universe boundingRadius denormal flake +
    a one-off Mimas.ecl last-digit jitter reproduce on the pre-respell binary
    and composed-vs-composed) - the tolerance admits that jitter and nothing
    larger (see the comparison loop),
  - the shadowing log line fired in B and NOT in A (the self-naming
    precedence evidence, 11.51(a)).
`mat`/`screen`/`dist` are observer-composed (camera state) and carry the B30
fresh-launch variance - deliberately NOT part of this gate. axisRot IS now
compared (B32, D20 §11.79(n)): the recompute-at-use fix made the spin phase a
deterministic function of the exact-compared lastJD, so the exclusion is LIFTED
and the field is exact-string legacy-vs-composed (was excluded as the B30/B32
spin-phase twin - the launch-wall-clock staleness that scattered it is gone).

Exit 0 = equivalence holds; 1 = any divergence (each named on stdout).
"""

import json, os, re, socket, subprocess, sys, time
from pathlib import Path

HOME = Path.home()
SC_BIN = os.environ.get("SC_BIN", str(Path(__file__).resolve().parents[2] / "build-claude/src/spacecrafter"))
USERDIR = HOME / ".spacecrafter"
TWIN = USERDIR / "modularSystem/SolarSystem.ini.disabled"
ENABLED = USERDIR / "modularSystem/SolarSystem.ini"
LOG = USERDIR / "log/spacecrafter.log"
# ABSOLUTE (11.106): the app is launched with cwd = ~/.spacecrafter, so a
# RELATIVE outdir makes it write every dump/screenshot somewhere that does not
# exist - and the failure is SILENT, because the reader then finds the file
# left by a PREVIOUS run and verifies stale artifacts. Measured: a relative
# outdir here re-read 2.5-hour-old dumps and only the one leg asking for a
# file that had never existed crashed. Same rule as b3_ladder.py.
_args = [a for a in sys.argv[1:] if not a.startswith("--")]
OUT = (Path(_args[0]) if _args
       else Path(__file__).resolve().parent / "artifacts/b24").resolve()
JD = "2461233.5"
# `--strip SECTION:KEY` deletes ONE key from the enabled twin before phase B.
# This is the gate's own able-to-FAIL demonstration for the capability fields:
# a key that is consumed but not emitted (or emitted but not consumed) is
# exactly what §11.73(g)'s co-delivery rule forbids, and stripping it is the
# cheapest way to prove the field list would catch it. Added 2026-07-30 with
# `primary` (§11.118); it works for any key.
STRIP = next((a.split("=", 1)[1] if "=" in a else None
              for a in sys.argv[1:] if a.startswith("--strip")), None)
SHADOW_MARK = "Composed system file modularSystem/SolarSystem.ini wins"
TWIN_MARK = "Composed twin of ssystem.ini generated"

FAILS = []

# B30 float32-jitter tolerance (see the comparison loop): differ only when the
# gap clears BOTH an absolute denormal floor and a relative float32 bound.
TOL_ABS = 1e-18
TOL_REL = 1e-6


def _close(x, y):
    d = abs(x - y)
    return d <= TOL_ABS or d <= TOL_REL * max(abs(x), abs(y))


def scalar_close(sa, sb):
    if sa == sb:
        return True
    try:
        return _close(float(sa), float(sb))
    except (TypeError, ValueError):
        return False


def floats_close(sa, sb):
    """Compare two 'x,y,z' ecl strings component-wise under the B30 tolerance."""
    if sa == sb:
        return True
    if sa is None or sb is None:
        return False
    va, vb = sa.split(","), sb.split(",")
    if len(va) != len(vb):
        return False
    try:
        return all(_close(float(pa), float(pb)) for pa, pb in zip(va, vb))
    except ValueError:
        return False


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


def wait_port(timeout=90):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            s = socket.create_connection(("127.0.0.1", 7805), timeout=1)
            return s
        except OSError:
            time.sleep(1)
    raise RuntimeError("port 7805 never opened")


def send(sock, cmd, pause=0.7):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.2)
        sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)


def run_phase(tag):
    """Fresh launch -> frozen scene -> dual_dump -> clean shutdown."""
    dump = OUT / f"b24_{tag}.json"
    dump.unlink(missing_ok=True)
    proc = subprocess.Popen([SC_BIN], cwd=str(USERDIR),
                            stdout=open(OUT / f"b24_{tag}.applog", "w"),
                            stderr=subprocess.STDOUT,
                            env={**os.environ, "DISPLAY": os.environ.get("DISPLAY", ":2")})
    sock = wait_port()
    time.sleep(10)  # async loads settle; auto-startup script finishes
    send(sock, "timerate rate 0")           # FIRST, before the epoch (b19 lesson)
    send(sock, f"date jday {JD}")
    time.sleep(3)
    send(sock, f"body action dual_dump filename {dump}")
    time.sleep(2)
    send(sock, "shutdown action now")
    sock.close()
    try:
        rc = proc.wait(timeout=30)
    except subprocess.TimeoutExpired:
        proc.kill()
        fail(f"{tag}: app did not exit within 30 s after shutdown")
        rc = -1
    if not dump.exists():
        raise RuntimeError(f"{tag}: dump {dump} was not written")
    return dump, rc


# C++ ostream prints non-finite floats as `nan` / `inf` / `-inf`; Python's json
# accepts `NaN` / `Infinity` / `-Infinity` and REJECTS the lowercase spellings.
# Without this substitution a single non-finite field made json.loads raise and
# the `except` below dropped the WHOLE BODY silently - the body then reads as
# absent to every gate built on this loader. Measured 2026-07-30 (§11.118): an
# intermittent cold-launch ASmooth NaN on `scaling` (the §11.18 class, EntityCore
# ASmooth.hpp's own comment records the same shape) makes the Moon or the Sun
# vanish from a dump. A body carrying NaN must be VISIBLE and compared (it then
# fails a value check loudly), never silently missing.
_NONFINITE = re.compile(r'(?<=:)\s*(-?)(nan|inf)\b')


def load_dump(path):
    """-> (header, {name: new_path_body_dict}) - new-path halves only."""
    bodies = {}
    header = None
    with open(path) as f:
        for line in f:
            line = line.strip().rstrip(",")
            if not line or line in "[]{}":
                continue
            try:
                obj = json.loads(_NONFINITE.sub(
                    lambda m: m.group(1) + ("NaN" if m.group(2) == "nan" else "Infinity"),
                    line))
            except json.JSONDecodeError:
                continue
            if header is None and obj.get("type") == "header":
                header = obj
                continue
            # "hops" lines share the name key and would overwrite the body
            # entry (their "new" is a LIST) - body lines only.
            if obj.get("type") == "body" and obj.get("new") is not None:
                bodies[obj["name"]] = obj["new"]
    return header, bodies


def raw_new_fields(path):
    """Exact-string ecl per body: compare the printed floats byte-for-byte,
    immune to json float re-parsing."""
    import re
    out = {}
    with open(path) as f:
        for line in f:
            m = re.search(r'"name":"([^"]+)".*?"new":(\{.*\})[,\s]*$', line.strip().rstrip(","))
            if m:
                e = re.search(r'"ecl":\[([^\]]*)\]', m.group(2))
                if e:
                    out[m.group(1)] = e.group(1)
    return out


def raw_axisrot(path):
    """Exact-string axisRot per body (B32): the spin phase, byte-for-byte.

    Since B32 (D20 §11.79(n)) `axisRot` recomputes at the dump USE from the
    ROOT-fresh lastJD through computeAxisRotation (I2), so it is a deterministic
    function of the (already exact-compared) lastJD - the launch-wall-clock spin
    staleness (§5.24) that forced this field's exclusion is gone. Legacy and
    composed loads carry identical rotation elements + lastJD, so the printed
    doubles match exactly. On a PRE-B32 binary this comparison RED-flags the
    invisible/frozen bodies (Moon/Deimos/Phobos/Mars/Mercury scatter up to
    4.49 rad) - the discrimination that proves the field is now load-bearing."""
    import re
    out = {}
    with open(path) as f:
        for line in f:
            m = re.search(r'"name":"([^"]+)".*?"new":(\{.*\})[,\s]*$', line.strip().rstrip(","))
            if m:
                e = re.search(r'"axisRot":([^,\}]+)', m.group(2))
                if e:
                    out[m.group(1)] = e.group(1)
    return out


def main():
    OUT.mkdir(parents=True, exist_ok=True)

    # ---- shipped-state preconditions ----
    if (USERDIR / "beta_features.ini").exists():
        raise RuntimeError("beta_features.ini present - not the shipped state")
    ENABLED.unlink(missing_ok=True)

    # ---- phase A: legacy ----
    # Marks are read from the app's captured stdout (the applog): the log FILE
    # truncates per launch and carries ANSI codes - measured unreliable here.
    dump_a, rc_a = run_phase("legacy")
    log_a = (OUT / "b24_legacy.applog").read_text(errors="replace")
    if SHADOW_MARK in log_a:
        fail("phase A: shadowing log fired with no enabled file present")
    else:
        ok("phase A: no shadowing (no enabled file)")
    if TWIN_MARK in log_a:
        ok("phase A: twin regenerated at load (generation-at-load live)")
    else:
        fail("phase A: twin generation log line absent")
    if not TWIN.exists():
        raise RuntimeError("twin absent after legacy launch - nothing to enable")

    # ---- phase B: composed (the documented adoption workflow) ----
    twin_bytes = TWIN.read_bytes()
    if STRIP:
        section, _, key = STRIP.partition(":")
        text, out_lines, cur, dropped = twin_bytes.decode("latin-1"), [], None, 0
        for ln in text.split("\n"):
            s = ln.strip()
            if s.startswith("[") and s.endswith("]"):
                cur = s[1:-1]
            elif cur == section and re.match(rf"^\s*{re.escape(key)}\s*=", ln):
                dropped += 1
                continue
            out_lines.append(ln)
        if dropped != 1:
            raise RuntimeError(f"--strip {STRIP}: dropped {dropped} lines, expected 1")
        twin_bytes = "\n".join(out_lines).encode("latin-1")
        print(f"MUTATION: `{key}` removed from [{section}] of the enabled twin - "
              f"this run is EXPECTED to fail", flush=True)
    ENABLED.write_bytes(twin_bytes)
    try:
        dump_b, rc_b = run_phase("composed")
        log_b = (OUT / "b24_composed.applog").read_text(errors="replace")
    finally:
        ENABLED.unlink(missing_ok=True)   # shipped state restored, ALWAYS
    if SHADOW_MARK in log_b:
        ok("phase B: shadowing self-named (precedence log fired)")
    else:
        fail("phase B: precedence/shadowing log line absent")
    if "loaded, composed format" in log_b:
        ok("phase B: composed loader ran")
    else:
        fail("phase B: composed-format load log absent (legacy path ran instead?)")

    # ---- numeric comparison ----
    ha, a = load_dump(dump_a)
    hb, b = load_dump(dump_b)
    if ha is None or hb is None:
        raise RuntimeError("dump header missing")
    if ha.get("jd") != hb.get("jd"):
        fail(f"header jd differs: {ha.get('jd')} vs {hb.get('jd')} - scene control broken, comparison unsound")
    only_a = sorted(set(a) - set(b))
    only_b = sorted(set(b) - set(a))
    if only_a:
        fail(f"bodies lost by the composed load: {only_a}")
    if only_b:
        fail(f"bodies invented by the composed load: {only_b}")
    if not only_a and not only_b:
        ok(f"body set identical ({len(a)} bodies)")

    # INSTRUMENT STATE (B27 tail, INTENT §11.107): the capability comparison
    # below is only meaningful if the two legs really took DIFFERENT resolution
    # paths - legacy `type`-derived vs composed key-derived (D14 §11.79(h)).
    # `composedDecl` is that positive mapping, read from the process itself; a
    # composed leg that silently fell back to the legacy loader would otherwise
    # make every capability agree for the wrong reason (the b24 phase-B log check
    # above proves the FILE was chosen, this proves the RESOLUTION was).
    decl_a = {n for n, v in a.items() if v.get("composedDecl")}
    decl_b = {n for n, v in b.items() if v.get("composedDecl")}
    if decl_a:
        fail(f"phase A (legacy) reports composedDecl on {len(decl_a)} bodies, e.g. {sorted(decl_a)[:3]}")
    elif not decl_b:
        fail("phase B (composed) reports composedDecl on NO body - the composed capability "
             "resolution did not run, so the comparison below is vacuous")
    else:
        ok(f"D14 resolution path positively mapped: legacy 0 / composed {len(decl_b)} bodies "
           f"carry composedDecl")

    ecl_a, ecl_b = raw_new_fields(dump_a), raw_new_fields(dump_b)
    axr_a, axr_b = raw_axisrot(dump_a), raw_axisrot(dump_b)
    n_exact = 0
    checked = 0
    axr_checked = 0
    for name in sorted(set(a) & set(b)):
        na, nb = a[name], b[name]
        checked += 1
        # STRUCTURAL fields stay EXACT - a body lost/re-parented/re-related or a
        # module added/dropped/re-routed is the class the respell could break,
        # and the [Moon:MESH]-deletion discrimination lands here (modules +
        # routing). These are integers / small enum strings, not float sums, so
        # they carry no B30 jitter.
        # bodyType/surfaceModel/trailLength ADDED 2026-07-25 (B27 tail, INTENT
        # §11.107): these are the capabilities the legacy `type` string used to
        # carry and that the composed format now declares as keys (light_source,
        # shadow_exempt, surface_model, trail_length). Legacy-vs-composed
        # equality on them, body by body over the whole corpus, IS the
        # co-delivery proof (§11.73(g)): a key consumed but not emitted shows up
        # here as a per-body mismatch, on the exact bodies whose type granted
        # something. NOT compared: `composedDecl` - it differs by construction
        # (that is what makes the composed leg a composed leg).
        # `primary` ADDED 2026-07-30 (D27's split, INTENT §11.113(f)/§11.118):
        # `light_source` no longer carries the whole bundle, so `bodyType` alone
        # no longer witnesses the Tier-B resolution. A legacy star's twin must
        # emit BOTH keys value-for-value, and this field is where emitting only
        # one of them shows up - on the Sun, as `primary` true-vs-false.
        for field in ("parent", "relation", "modules", "routing", "lastJD",
                      "bodyType", "primary", "surfaceModel", "trailLength"):
            if na.get(field) != nb.get(field):
                fail(f"{name}.{field}: {na.get(field)!r} != {nb.get(field)!r}")
        # ecl and boundingRadius are FLOAT fields of the NEW path and are NOT
        # bit-stable across fresh launches (B30, INTENT 11.53(e)): exact-string
        # equality on them is unsound. PROVEN pre-existing this task: the SAME
        # MilkyWay/Universe boundingRadius denormal flake (0 <-> ~1e-40) and a
        # one-off Mimas.ecl float32 last-digit jitter (~1.2e-11 AU on 1e-3)
        # reproduce on the PRE-RESPELL binary and on a composed-vs-composed
        # B-vs-B control - launch non-determinism, not a format difference (the
        # respell only touches the composed LOAD, structurally). So these two
        # fields get a float32-epsilon tolerance: a value differs only when it
        # exceeds BOTH an absolute denormal floor (1e-18) AND a relative float32
        # bound (1e-6) - orders below any real orbit/size error, orders above
        # the jitter. axisRot is compared EXACT-string below (B32 lifted it).
        if not floats_close(ecl_a.get(name), ecl_b.get(name)):
            fail(f"{name}.ecl differs beyond float32 jitter: [{ecl_a.get(name)}] vs [{ecl_b.get(name)}]")
        elif not scalar_close(na.get("boundingRadius"), nb.get("boundingRadius")):
            fail(f"{name}.boundingRadius differs beyond float32 jitter: "
                 f"{na.get('boundingRadius')!r} vs {nb.get('boundingRadius')!r}")
        else:
            n_exact += 1
        # axisRot (B32, D20 §11.79(n)): the exclusion is LIFTED - the spin phase
        # now recomputes at the dump USE from the ROOT-fresh lastJD (I2), so it
        # is deterministic (a function of the exact-compared lastJD) and matches
        # EXACT-string legacy-vs-composed. This gate RED-flagged 5 invisible/
        # frozen bodies on the pre-B32 binary (Moon/Deimos/Phobos/Mars/Mercury,
        # up to 4.49 rad) - the discrimination proving the field is load-bearing.
        if name in axr_a and name in axr_b:
            axr_checked += 1
            if axr_a[name] != axr_b[name]:
                fail(f"{name}.axisRot differs (B32 spin-freshness): "
                     f"{axr_a[name]} vs {axr_b[name]}")
    ok(f"per-body fields checked on {checked} bodies; ecl+boundingRadius within float32 jitter on {n_exact}")
    ok(f"axisRot exact-string (B32 lifted exclusion) compared on {axr_checked} bodies")

    (OUT / "b24_result.json").write_text(json.dumps({
        "bodies": len(a), "float_within_jitter": n_exact,
        "axisRot_exact_checked": axr_checked, "fails": FAILS,
        "jd_a": ha.get("jd"), "jd_b": hb.get("jd"),
        "exit_a": rc_a, "exit_b": rc_b,
    }, indent=1))
    print(f"\n{'EQUIVALENCE HOLDS' if not FAILS else f'{len(FAILS)} DIVERGENCES'} "
          f"({len(a)} bodies) -> {OUT}/b24_result.json", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
