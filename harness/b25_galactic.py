#!/usr/bin/env python3
"""B25 GALACTIC gate: legacy-vs-composed equivalence on addSystem star systems
(INTENT §11.109; ledger B25). The untested half of the migration vehicle.

WHY A SEPARATE DRIVER (the I2 call, stated). `b24_equivalence.py` is the
corpus-wide SOLAR gate: it owns the real `~/.spacecrafter`, has one system, one
twin, one adoption. This gate needs a temp-HOME farm (§11.103(a)), an AUTHORED
galactic corpus, N systems and N adoptions, and three assertion families the
solar gate has no subject for (twin MEMBERSHIP, section ORDER, value BYTES).
Duplicating the per-body comparison would be the I2 violation, so this file
IMPORTS it from `b24_equivalence` (`floats_close` / `scalar_close` /
`load_dump` / `raw_new_fields` / `raw_axisrot`) and adds only the delta.

WHY THE FARM CARRIES DOT-PREFIXED COPIES (measured, §11.109(a)). `core.cpp:320`
calls `loadGalacticSystem(".", "galactic.ini")` and the callee opens
`path + name` — literally `.galactic.ini`, and per entry
`.stellar_systems/<file>`. So on a shipped install the galactic corpus is never
opened at all and no galactic twin has ever been generated. This driver feeds
the code the names it actually opens, byte-identical to the field data, so the
production path (`loadGalacticSystem` -> `loadSystem` -> `addSystem` ->
`createModularSystem` -> `generateComposedTwin`) runs unchanged. Fixing the
concatenation is a user-visible change (17 systems + anchors appear on every
install) and is SUSPENDED, not taken here — see §5.37 / §13.

THE CORPUS is `b25_corpus/` (committed next to this file): a full-featured
foreign system (`system_proxima.ini`: light_source / surface_model /
trail_length / shadow_exempt emission, RING+MESH+AXIS+HINT+ORBIT+TRAIL+TAIL+
OJM+CUSTOM+GRID deduction, a 4-deep parent chain, `bound_to_surface`, an
ISO-8859 value, an unknown key, an unnamed section, an orphan section), a
SHARED file (`system_white_dwarf.ini` — the shipped `galactic.ini` points FIVE
entries at one file, which is what makes the membership assertion load-bearing)
and 11 zero-byte files reproducing the shipped field state. Values are
SYNTHETIC and labelled as such in the files (§11.51(d) red line: no physical
constant is claimed).

    cd claude/harness && DISPLAY=:2 ./b25_galactic.py [outdir] [--mutate]

`--mutate` deletes `[PxB:MESH]` from the adopted ProximaSystem.ini before phase
B: the discrimination run, which MUST fail. Exit 0 = equivalence holds.

The real `~/.spacecrafter` is never written: every entry's md5 is asserted
in == out (D9 — this corpus class IS the paid product).
"""

import hashlib, json, os, re, shutil, socket, subprocess, sys, time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import b24_equivalence as b24   # the per-body comparison authority (I2)

HERE = Path(__file__).resolve().parent
HOME = Path.home()
SRC = HOME / ".spacecrafter"
CORPUS = HERE / "b25_corpus"
SC_BIN = os.environ.get("SC_BIN", str(HERE.parents[1] / "build-claude/src/spacecrafter"))
JD = "2461233.5"
MUTATE_SECTION = "PxB:MESH"

args = [a for a in sys.argv[1:] if not a.startswith("--")]
MUTATE = "--mutate" in sys.argv
OUT = (Path(args[0]) if args else HERE / "artifacts/b25gal").resolve()
FARM = OUT / "farm"

FAILS = []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


def md5(p):
    h = hashlib.md5()
    with open(p, "rb") as f:
        for b in iter(lambda: f.read(1 << 20), b""):
            h.update(b)
    return h.hexdigest()


def real_tree_md5():
    """Every REAL file under ~/.spacecrafter that this gate could plausibly
    touch, per file (D9): the top level, the galactic corpus, and the twins."""
    out = {}
    for p in sorted(SRC.iterdir()):
        if p.is_file():
            out[p.name] = md5(p)
    for d in ("stellar_systems", "modularSystem"):
        if (SRC / d).is_dir():
            for p in sorted((SRC / d).iterdir()):
                if p.is_file():
                    out[f"{d}/{p.name}"] = md5(p)
    return out


# ---------------------------------------------------------------- galactic.ini
def read_galactic(path):
    """Reproduce SSystemFactory::loadGalacticSystem's own section walk
    (ssystem_factory.cpp:466-491) -> [(name, system_file_or_None)] in order."""
    entries, params = [], {}
    raw = Path(path).read_bytes().decode("latin-1")

    def flush():
        if params:
            entries.append((params.get("name"), params.get("system")))
            params.clear()

    for line in raw.split("\n"):
        if not line or line.startswith("#"):
            continue
        if line.startswith("["):
            flush()
            continue
        line = line.rstrip("\r")
        pos = line.find("=")
        if pos != -1:
            params[line[:pos - 1]] = line[pos + 2:]
    flush()
    return entries


# ------------------------------------------------------------ legacy corpus IO
def parse_legacy(path):
    """The LOADER's own parse (ModularSystem::loadSystem, ModularSystem.cpp:1349
    -1381) - byte-exact, including its substr arithmetic. Returns the ordered
    list of per-section param dicts, values as bytes."""
    sections, cur = [], {}
    for line in Path(path).read_bytes().split(b"\n"):
        if len(line) < 2:
            continue
        if line[:1] == b"#":
            continue
        if line[:1] == b"[":
            if cur:
                sections.append(cur)
                cur = {}
            continue
        if line.endswith(b"\r"):
            line = line[:-1]
        pos = line.find(b"=", 2)
        if pos == -1:                     # (int)npos == -1 in the C++ source
            key, value = line, line[1:]
        else:
            key, value = line[:pos - 1], line[pos + 2:]
        cur[key] = value
    if cur:
        sections.append(cur)
    return sections


def parse_composed(path):
    """ModularSystemFormat::parse (ModularSystemFormat.cpp:23-58) -> ordered
    [(header, {key: value})], bytes throughout."""
    def trim(s):
        return s.strip(b" \t").rstrip(b"\r").strip(b" \t")

    out = []
    for line in Path(path).read_bytes().split(b"\n"):
        line = trim(line)
        if not line or line[:1] == b"#":
            continue
        if line[:1] == b"[":
            close = line.find(b"]")
            out.append((line[1:] if close == -1 else line[1:close], {}))
            continue
        eq = line.find(b"=")
        if eq == -1:
            continue
        k, v = trim(line[:eq]), trim(line[eq + 1:])
        if k and out:
            out[-1][1][k] = v
    return out


# ------------------------------------------------------------------- app cycle
def build_farm(adopt=()):
    """Temp-HOME farm (§11.103(a)) + the two dot-prefixed names the galactic
    loader actually opens. `adopt` = twin stems to enable (the documented
    adoption workflow: drop the .disabled extension)."""
    dst = FARM / ".spacecrafter"
    if FARM.exists():
        shutil.rmtree(FARM)
    dst.mkdir(parents=True)
    for e in sorted(SRC.iterdir()):
        n = e.name
        if n in ("config.ini", "ssystem.ini"):
            shutil.copy(e, dst / n)
            os.chmod(dst / n, 0o644)
        elif n in ("log", "screenshot", "modularSystem"):
            continue
        else:
            (dst / n).symlink_to(e)
    for d in ("log", "screenshot", "modularSystem"):
        (dst / d).mkdir()
    # the names loadGalacticSystem opens: "." + "galactic.ini",
    # "." + "stellar_systems/<file>"  (ssystem_factory.cpp:470/503)
    shutil.copy(SRC / "galactic.ini", dst / ".galactic.ini")
    assert md5(dst / ".galactic.ini") == md5(SRC / "galactic.ini")
    shutil.copytree(CORPUS, dst / ".stellar_systems")
    return dst


def run_phase(dst, tag):
    dump = OUT / f"b25gal_{tag}.json"
    dump.unlink(missing_ok=True)
    applog = OUT / f"b25gal_{tag}.applog"
    proc = subprocess.Popen([SC_BIN], cwd=str(dst),
                            stdout=open(applog, "w"), stderr=subprocess.STDOUT,
                            env={**os.environ, "HOME": str(FARM),
                                 "DISPLAY": os.environ.get("DISPLAY", ":2")})
    sock, t0 = None, time.time()
    while time.time() - t0 < 90:
        try:
            sock = socket.create_connection(("127.0.0.1", 7805), timeout=1)
            break
        except OSError:
            time.sleep(1)
    if sock is None:
        proc.kill()
        raise RuntimeError(f"{tag}: port 7805 never opened")
    time.sleep(10)

    def send(cmd, pause=0.7):
        sock.sendall((cmd + "\n").encode())
        time.sleep(pause)
        try:
            sock.settimeout(0.2)
            sock.recv(8192)
        except socket.timeout:
            pass
        sock.settimeout(None)

    send("timerate rate 0")            # FIRST, before the epoch (b19 lesson)
    send(f"date jday {JD}")
    time.sleep(3)
    send(f"body action dual_dump filename {dump}")
    time.sleep(2)
    send("shutdown action now")
    sock.close()
    try:
        rc = proc.wait(timeout=40)
    except subprocess.TimeoutExpired:
        proc.kill()
        fail(f"{tag}: app did not exit within 40 s after shutdown")
        rc = -1
    if not dump.exists():
        raise RuntimeError(f"{tag}: dump {dump} was not written")
    return dump, applog.read_text(errors="replace"), rc


def system_of(bodies, sysnodes):
    """Attribute every body to its nearest SYSTEM-node ancestor (the membership
    authority: a twin may only declare bodies of ITS system)."""
    out = {}
    for name in bodies:
        cur, seen = name, set()
        while cur and cur not in seen:
            seen.add(cur)
            if cur in sysnodes and cur != name:
                out[name] = cur
                break
            cur = bodies.get(cur, {}).get("parent")
    return out


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    pre = real_tree_md5()

    entries = read_galactic(SRC / "galactic.ini")
    with_file = [(n, f) for n, f in entries if f]
    corpus_present = [(n, f) for n, f in with_file if (CORPUS / Path(f).name).exists()]
    sysnodes = {"SolarSystem"} | {n + "System" for n, _ in with_file}
    print(f"galactic.ini: {len(entries)} entries, {len(with_file)} carry a system file, "
          f"{len(corpus_present)} have a corpus file present", flush=True)

    # ---------------- phase A: legacy ----------------
    dst = build_farm()
    dump_a, log_a, rc_a = run_phase(dst, "legacy")
    ms = dst / "modularSystem"

    if "wins over" in log_a:
        fail("phase A: a shadowing log fired with no enabled file present")
    else:
        ok("phase A: no shadowing (no enabled composed file)")

    twins = {}
    for name, f in corpus_present:
        t = ms / f"{name}System.ini.disabled"
        if t.exists():
            twins[name] = t
        else:
            fail(f"phase A: no twin generated for galactic system {name}")
    if len(twins) == len(corpus_present):
        ok(f"phase A: twin regenerated for all {len(twins)} galactic systems "
           f"(+ SolarSystem: {(ms / 'SolarSystem.ini.disabled').exists()})")
    if not (ms / "SolarSystem.ini.disabled").exists():
        fail("phase A: the solar twin is missing - the control leg is broken")

    _, bodies_a = b24.load_dump(dump_a)
    owner = system_of(bodies_a, sysnodes)

    # ---- ASSERTION 1: MEMBERSHIP. The twin of system S declares exactly the
    # bodies S loaded. `generateComposedTwin` resolves each section through the
    # GLOBAL name registry (findBodyOnce), so a body loaded by ANOTHER system
    # satisfies its lookup - and galactic.ini points five entries at one file.
    for name in sorted(twins):
        declared = [h.decode("latin-1") for h, _ in parse_composed(twins[name])
                    if b":" not in h]
        actual = sorted(b for b, s in owner.items() if s == name + "System")
        if sorted(declared) != actual:
            fail(f"membership: twin of {name}System declares {sorted(declared)} "
                 f"but the system contains {actual}")
    if not any(m.startswith("membership:") for m in FAILS):
        ok(f"membership: every twin declares exactly its own system's bodies "
           f"({len(twins)} systems)")

    # ---- ASSERTION 2+3: ORDER and BYTES (the §11.78(f) writer contract) ----
    for name, f in corpus_present:
        if name not in twins:
            continue
        legacy = parse_legacy(CORPUS / Path(f).name)
        comp = parse_composed(twins[name])
        nodes = [(h, p) for h, p in comp if b":" not in h]
        # order: the twin's node sequence is the legacy section sequence,
        # restricted to the sections that produced a body IN THIS SYSTEM.
        # `in bodies_a` alone is the writer's own §11.109(c) defect written into
        # the instrument: a section skipped because another system already owns
        # the name has its body in the global dump and must NOT be expected here.
        loaded = [s for s in legacy if s.get(b"name") and
                  owner.get(s[b"name"].decode("latin-1")) == name + "System"]
        exp_order = [s[b"name"] for s in loaded]
        got_order = [p.get(b"name") for _, p in nodes]
        if exp_order != got_order:
            fail(f"order: {name}System twin node order {got_order} != legacy order {exp_order}")
        # bytes: every legacy key/value of a loaded section is present verbatim
        for sec in loaded:
            nm = sec[b"name"]
            match = [p for _, p in nodes if p.get(b"name") == nm]
            if not match:
                continue
            got = match[0]
            for k, v in sec.items():
                if k == b"bound_to_surface":
                    if k in got:
                        fail(f"bytes: {name}/{nm.decode()} kept bound_to_surface "
                             f"alongside relation (two relation authorities)")
                    if got.get(b"relation") != b"grounded":
                        fail(f"bytes: {name}/{nm.decode()} bound_to_surface not "
                             f"translated to relation = grounded")
                    continue
                if got.get(k) != v:
                    fail(f"bytes: {name}/{nm.decode()} key {k!r}: legacy {v!r} != twin {got.get(k)!r}")
    if not any(m.startswith(("order:", "bytes:")) for m in FAILS):
        ok("order + bytes: every twin node reproduces its legacy section in file "
           "order, values byte-identical (bound_to_surface -> relation = grounded)")

    # ---- ASSERTION 4: ISO-8859 values verbatim (the explicit high-byte leg) --
    for name, f in corpus_present:
        if name not in twins:
            continue
        src_hi = [l for l in (CORPUS / Path(f).name).read_bytes().split(b"\n")
                  if any(b > 127 for b in l)]
        twin_hi = [l for l in twins[name].read_bytes().split(b"\n")
                   if any(b > 127 for b in l)]
        if src_hi and sorted(src_hi) != sorted(twin_hi):
            fail(f"iso8859: {name}System twin high-byte lines {twin_hi} != source {src_hi}")
        elif src_hi:
            ok(f"iso8859: {len(src_hi)} high-byte line(s) of {Path(f).name} verbatim in the twin")

    # ---------------- phase B: composed (adoption) ----------------
    # SolarSystem is deliberately NOT adopted: it stays legacy in both phases and
    # is the in-run control (its bodies must be identical AND composedDecl-free).
    for name in twins:
        (ms / f"{name}System.ini").write_bytes(twins[name].read_bytes())
    if MUTATE:
        p = ms / "ProximaSystem.ini"
        txt = p.read_bytes().decode("latin-1")
        cut = re.sub(r"\n\[" + re.escape(MUTATE_SECTION) + r"\][^\[]*", "\n", txt, count=1)
        if cut == txt:
            raise RuntimeError(f"--mutate: [{MUTATE_SECTION}] not found in the twin")
        p.write_bytes(cut.encode("latin-1"))
        print(f"--mutate: [{MUTATE_SECTION}] deleted from the adopted ProximaSystem.ini", flush=True)

    dump_b, log_b, rc_b = run_phase(dst, "composed")

    # Matched on the line's TAIL, not its head: the app's stdout interleaves the
    # cLog stream with SSystemFactory::loadSystem's raw std::cout, and one
    # shadowing line was measured truncated mid-prefix ("m/NGC2392DwarfSystem.ini
    # wins over ...") - the statement was there, the line start was not. The
    # distinctive "<X>System.ini wins over" is the evidence; the log-derived
    # channel is corroborated by the independent composedDecl assertion below.
    shadowed = re.findall(r"([A-Za-z0-9_]+System)\.ini wins over", log_b)
    if set(shadowed) == {n + "System" for n in twins}:
        ok(f"phase B: precedence log self-named for all {len(shadowed)} adopted systems")
    else:
        fail(f"phase B: shadowing log fired for {sorted(shadowed)}, expected "
             f"{sorted(n + 'System' for n in twins)}")
    if "SolarSystem.ini wins" in log_b:
        fail("phase B: the SOLAR system was shadowed - the control leg is broken")
    n_composed = len(re.findall(r"loaded, composed format", log_b))
    if n_composed == len(twins):
        ok(f"phase B: composed loader ran for all {n_composed} adopted systems")
    else:
        fail(f"phase B: composed-format load log fired {n_composed}x, expected {len(twins)}")

    # ---------------- comparison ----------------
    ha, a = b24.load_dump(dump_a)
    hb, b = b24.load_dump(dump_b)
    if ha.get("jd") != hb.get("jd"):
        fail(f"header jd differs: {ha.get('jd')} vs {hb.get('jd')} - scene control broken")
    only_a, only_b = sorted(set(a) - set(b)), sorted(set(b) - set(a))
    if only_a:
        fail(f"bodies lost by the composed load: {only_a}")
    if only_b:
        fail(f"bodies invented by the composed load: {only_b}")
    if not only_a and not only_b:
        ok(f"body set identical ({len(a)} bodies, {len(sysnodes)} system nodes)")

    # instrument state (§11.107(h)): the two legs must really take DIFFERENT
    # resolution paths, and ONLY for the galactic systems.
    gal_bodies = {n for n, s in owner.items() if s != "SolarSystem"} - sysnodes
    sol_bodies = {n for n, s in owner.items() if s == "SolarSystem"}
    decl_a = {n for n, v in a.items() if v.get("composedDecl")}
    decl_b = {n for n, v in b.items() if v.get("composedDecl")}
    if decl_a:
        fail(f"phase A (legacy) reports composedDecl on {sorted(decl_a)[:5]}")
    elif not (gal_bodies and gal_bodies <= decl_b):
        fail(f"phase B: composedDecl missing on galactic bodies "
             f"{sorted(gal_bodies - decl_b)[:5]} - the composed resolution did not run")
    elif decl_b & sol_bodies:
        fail(f"phase B: composedDecl on SOLAR bodies {sorted(decl_b & sol_bodies)[:5]} "
             f"- the control system was composed too")
    else:
        ok(f"D14 resolution path positively mapped: legacy 0 / composed "
           f"{len(decl_b)} bodies, all galactic; {len(sol_bodies)} solar control bodies legacy in both")

    ecl_a, ecl_b = b24.raw_new_fields(dump_a), b24.raw_new_fields(dump_b)
    axr_a, axr_b = b24.raw_axisrot(dump_a), b24.raw_axisrot(dump_b)

    # THE ECL FLOOR IS MEASURED IN-RUN, NOT ASSUMED (the b19/§11.103(f) pattern).
    # This gate's SUBJECT is the galactic bodies: legacy in phase A, composed in
    # phase B. The 90 solar bodies are legacy in BOTH phases - identical file,
    # identical loader, identical orbit objects - so a difference on them is
    # cross-launch nondeterminism of the new path's position pipeline BY
    # CONSTRUCTION (§11.53(e)/§11.87(c)/B30), never a format effect. Measured
    # 2026-07-25: one clean run in six showed 8 solar moons (Dione, Enceladus,
    # Io, Mimas, Miranda, Phobos, Tethys, Umbriel) at rel <= 9.5e-6 / abs <=
    # 1.2e-8 AU while every other run had zero - i.e. the constant 1e-6 of the
    # SOLAR gate is too tight for THIS scene, which carries 17 more systems.
    # Widening the constant would be fitting; using the control's own spread is
    # not, and it keeps the information visible (the floor is printed and stored
    # every run). The control is still asserted EXACT on every structural field,
    # and its floor is itself capped - a control that drifts past CONTROL_CEIL
    # means the scene stopped being comparable and fails the run.
    CONTROL_CEIL = 1e-3

    def ecl_dev(name):
        """max (relative, absolute) component deviation of ecl between phases."""
        sa, sb = ecl_a.get(name), ecl_b.get(name)
        if sa is None or sb is None or sa == sb:
            return 0.0, 0.0
        try:
            va = [float(x) for x in sa.split(",")]
            vb = [float(x) for x in sb.split(",")]
        except ValueError:
            return float("inf"), float("inf")
        if len(va) != len(vb):
            return float("inf"), float("inf")
        rel = max(abs(x - y) / max(abs(x), abs(y), 1e-300) for x, y in zip(va, vb))
        av = max(abs(x - y) for x, y in zip(va, vb))
        return rel, av

    control = sorted((set(a) & set(b)) - gal_bodies - {n for n in sysnodes if n != "SolarSystem"})
    subject = sorted((set(a) & set(b)) & (gal_bodies | (set(sysnodes) - {"SolarSystem"})))
    floor_rel = floor_abs = 0.0
    worst_ctl = None
    for name in control:
        r, v = ecl_dev(name)
        if r > floor_rel:
            floor_rel, worst_ctl = r, name
        floor_abs = max(floor_abs, v)
    if floor_rel > CONTROL_CEIL:
        fail(f"CONTROL ecl floor {floor_rel:.2e} (worst {worst_ctl}) exceeds {CONTROL_CEIL:.0e} - "
             f"the two launches are not comparable, the subject comparison below is unsound")
    else:
        ok(f"in-run ecl floor from {len(control)} legacy-in-both control bodies: "
           f"rel {floor_rel:.2e} / abs {floor_abs:.2e} AU"
           + (f" (worst {worst_ctl})" if worst_ctl else " (bit-identical)"))
    tol_rel = max(b24.TOL_REL, floor_rel)
    tol_abs = max(b24.TOL_ABS, floor_abs)

    checked = n_float = n_axr = 0
    for name in sorted(set(a) & set(b)):
        na, nb = a[name], b[name]
        checked += 1
        # STRUCTURAL fields stay EXACT for EVERY body, control included: they
        # carry no float jitter, and the [PxB:MESH] discrimination lands here.
        for field in ("parent", "relation", "modules", "routing", "lastJD",
                      "bodyType", "surfaceModel", "trailLength"):
            if na.get(field) != nb.get(field):
                fail(f"{name}.{field}: {na.get(field)!r} != {nb.get(field)!r}")
        r, v = ecl_dev(name)
        if name in subject and r > tol_rel and v > tol_abs:
            fail(f"{name}.ecl differs beyond the in-run floor (rel {r:.2e} > {tol_rel:.2e}): "
                 f"[{ecl_a.get(name)}] vs [{ecl_b.get(name)}]")
        elif not b24.scalar_close(na.get("boundingRadius"), nb.get("boundingRadius")):
            fail(f"{name}.boundingRadius differs beyond float32 jitter: "
                 f"{na.get('boundingRadius')!r} vs {nb.get('boundingRadius')!r}")
        else:
            n_float += 1
        if name in axr_a and name in axr_b:
            n_axr += 1
            if axr_a[name] != axr_b[name]:
                fail(f"{name}.axisRot differs (B32 spin-freshness): "
                     f"{axr_a[name]} vs {axr_b[name]}")
    ok(f"per-body fields checked on {checked} bodies ({len(subject)} subject / "
       f"{len(control)} control); boundingRadius + subject ecl within the floor on {n_float}; "
       f"axisRot exact on {n_axr}")

    # ---------------- D9: the real tree is untouched ----------------
    post = real_tree_md5()
    if pre == post:
        ok(f"D9: real ~/.spacecrafter md5 in == out on {len(pre)} files")
    else:
        fail(f"D9 VIOLATION: real tree changed: "
             f"{ {k: (pre.get(k), post.get(k)) for k in set(pre) | set(post) if pre.get(k) != post.get(k)} }")

    (OUT / "b25gal_result.json").write_text(json.dumps({
        "mutate": MUTATE, "bodies": len(a), "galactic_bodies": len(gal_bodies),
        "subject": len(subject), "control": len(control),
        "ecl_floor_rel": floor_rel, "ecl_floor_abs": floor_abs, "ecl_floor_worst": worst_ctl,
        "systems": len(twins), "float_within_jitter": n_float,
        "axisRot_exact_checked": n_axr, "exit_a": rc_a, "exit_b": rc_b,
        "jd_a": ha.get("jd"), "jd_b": hb.get("jd"), "fails": FAILS,
    }, indent=1))
    print(f"\n{'GALACTIC EQUIVALENCE HOLDS' if not FAILS else f'{len(FAILS)} DIVERGENCES'} "
          f"({len(a)} bodies / {len(twins)} galactic systems) -> {OUT}/b25gal_result.json",
          flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
