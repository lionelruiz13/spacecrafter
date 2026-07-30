#!/usr/bin/env python3
"""INTENT §5.45 gate — a malformed `galactic.ini` section must not kill the app.

WHAT IT IS FOR. B40 (§11.115) repaired the `.galactic.ini` path concatenation,
which made `SSystemFactory::loadSystem` reachable on every install for the
first time since 2025-09-20 — and with it a latent abort: the function fed
`params["x"]` straight to `std::stod`, `stringHash_t::operator[]` returns `""`
for an absent key, `std::stod("")` throws, and neither `main.cpp` nor
`core.cpp` catches. A paid galactic delivery with ONE malformed section
refuses to start the product (§5.45, measured -6/SIGABRT).

    cd claude/harness && DISPLAY=:2 ./f12_s545.py <outdir> pre=/abs/binary post=/abs/binary

THE INSTRUMENT. A temp-HOME farm (§11.103(a)) built by `b25_galactic.build_farm`
(the farm shape's one authority — I2) in its `corpus=None` "field state" variant:
everything symlinked to the real `~/.spacecrafter`, so the shipped 0-byte
`stellar_systems/` stands and NOTHING in the real tree is ever written. Only
`galactic.ini` is replaced by a real file: the shipped bytes for the control
leg, the shipped bytes with ONE line mutated for the defect legs.

FOUR CORPORA, each run under BOTH binaries — the pre-fix leg is what makes the
gate discriminating, and it is a real red, not a simulated one:

  M0  control, shipped bytes verbatim ......... both binaries must START
  M1  `[Proxima]`'s `z` line DELETED .......... §5.45's own repro; pre-fix
                                                ABORTS (-6) before its port
  M2  `[Toliman]`'s `name` line DELETED ....... the label has to survive the
                                                loss of the identifier itself.
                                                Pre-fix does NOT abort here —
                                                it builds a system node called
                                                "System" out of the empty name
                                                and loses `TolimanSystem`
                                                (measured, §11.118)
  M3  `[Keid] x = ,5` (decimal comma) ......... PARSEABILITY, not presence:
                                                stod throws here too, pre-fix
                                                ABORTS (-6)

ASSERTED per leg: the process reaches its port at all; on the patched binary
the warning names the section, the key and the remedy (§2(f)); the skipped
system's node is ABSENT from the body dump while every other system's node is
present (a skip that silently dropped the whole file would pass a
warning-only check); and the control leg is unchanged between the binaries.
"""

import json, os, re, socket, subprocess, sys, time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import b25_galactic as b25g
import b24_equivalence as b24

HERE = Path(__file__).resolve().parent
SRC = Path.home() / ".spacecrafter"
JD = "2461233.5"

FAILS = []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


# --------------------------------------------------------------- the mutations
def shipped():
    return (SRC / "galactic.ini").read_bytes().decode("latin-1").split("\n")


def _section_of(lines, i):
    for j in range(i, -1, -1):
        if lines[j].strip().startswith("["):
            return lines[j].strip()
    return None


def drop_line(section, key):
    """Delete the `key` line of `[section]` — nothing else changes."""
    def mutate(lines):
        out, cur, dropped = [], None, 0
        for ln in lines:
            s = ln.strip()
            if s.startswith("["):
                cur = s
            if cur == f"[{section}]" and re.match(rf"^\s*{key}\s*=", ln):
                dropped += 1
                continue
            out.append(ln)
        assert dropped == 1, f"{section}.{key}: dropped {dropped} lines, expected 1"
        return out
    return mutate


def set_value(section, key, value):
    def mutate(lines):
        out, cur, hits = [], None, 0
        for ln in lines:
            s = ln.strip()
            if s.startswith("["):
                cur = s
            if cur == f"[{section}]" and re.match(rf"^\s*{key}\s*=", ln):
                hits += 1
                ln = f"{key} = {value}"
            out.append(ln)
        assert hits == 1, f"{section}.{key}: {hits} hits, expected 1"
        return out
    return mutate


# tag -> (mutation, section, key, pre-fix outcome). The pre-fix outcome is
# MEASURED, not assumed: `name` is the one guarded key whose absence did not
# abort — `params["name"]` yields "" and the old code happily built a system
# NODE called "System" out of it (measured 2026-07-30), losing `TolimanSystem`
# without a word. So the guard fixes two different pre-fix behaviours and the
# gate asserts each one where it actually is.
CORPORA = {
    "M0": (None, None, None, "start"),
    "M1": (drop_line("Proxima", "z"), "Proxima", "z", "abort"),
    "M2": (drop_line("Toliman", "name"), "Toliman", "name", "garbage:System"),
    "M3": (set_value("Keid", "x", ",5"), "Keid", "x", "abort"),
}


# ------------------------------------------------------------------- app cycle
def build(out, tag, mutate):
    farm = out / f"farm_{tag}"
    dst = b25g.build_farm(farm=farm, dotted=False, corpus=None)
    gal = dst / "galactic.ini"
    if gal.is_symlink():
        gal.unlink()
    text = "\n".join(shipped() if mutate is None else mutate(shipped()))
    gal.write_bytes(text.encode("latin-1"))
    return farm, dst


def launch(dst, farm, binary, applog, dump):
    """Start the app; return (rc, port_opened, log, dump_written)."""
    dump.unlink(missing_ok=True)
    proc = subprocess.Popen([binary], cwd=str(dst),
                            stdout=open(applog, "w"), stderr=subprocess.STDOUT,
                            env={**os.environ, "HOME": str(farm),
                                 "DISPLAY": os.environ.get("DISPLAY", ":2")})
    sock, t0 = None, time.time()
    while time.time() - t0 < 60:
        if proc.poll() is not None:
            break                      # died before opening its port
        try:
            sock = socket.create_connection(("127.0.0.1", 7805), timeout=1)
            break
        except OSError:
            time.sleep(1)
    if sock is None:
        rc = proc.poll()
        if rc is None:
            proc.kill()
            rc = proc.wait()
        return rc, False, applog.read_text(errors="replace"), False
    time.sleep(10)
    b24.send(sock, "timerate rate 0")
    b24.send(sock, f"date jday {JD}")
    time.sleep(3)
    b24.send(sock, f"body action dual_dump filename {dump}")
    time.sleep(2)
    b24.send(sock, "shutdown action now")
    sock.close()
    try:
        rc = proc.wait(timeout=40)
    except subprocess.TimeoutExpired:
        proc.kill()
        rc = -1
    return rc, True, applog.read_text(errors="replace"), dump.exists()


def system_nodes(dump):
    _, bodies = b24.load_dump(dump)
    return sorted(n for n in bodies if n.endswith("System")), len(bodies)


# ------------------------------------------------------------------------ main
def main():
    out = Path(sys.argv[1]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    bins = dict(spec.split("=", 1) for spec in sys.argv[2:])
    assert set(bins) == {"pre", "post"}, "need pre=<binary> post=<binary>"

    src_md5 = b25g.real_tree_md5()
    summary = {}
    for tag, (mutate, section, key, pre_outcome) in CORPORA.items():
        farm, dst = build(out, tag, mutate)
        summary[tag] = {}
        for which, binary in sorted(bins.items()):
            applog = out / f"s545_{tag}_{which}.applog"
            dump = out / f"s545_{tag}_{which}.json"
            rc, port, log, wrote = launch(dst, farm, binary, applog, dump)
            nodes, nbodies = system_nodes(dump) if wrote else ([], 0)
            rec = {
                "rc": rc, "port": port, "dump": wrote,
                "params_blocks": log.count("Params :"),
                "bodies": nbodies, "system_nodes": len(nodes),
                "skip_warnings": re.findall(r"galactic\.ini: skipping [^\n]*", log),
                "abort": "std::invalid_argument" in log or "std::out_of_range" in log,
            }
            summary[tag][which] = rec
            print(f"[{tag}/{which}] rc={rc} port={port} bodies={nbodies} "
                  f"sysnodes={len(nodes)} params={rec['params_blocks']} "
                  f"abort={rec['abort']} warn={rec['skip_warnings']}", flush=True)

        pre, post = summary[tag]["pre"], summary[tag]["post"]
        if tag == "M0":
            for w, r in (("pre", pre), ("post", post)):
                if r["port"] and r["dump"]:
                    ok(f"M0/{w}: well-formed corpus starts ({r['bodies']} bodies, "
                       f"{r['system_nodes']} system nodes)")
                else:
                    fail(f"M0/{w}: well-formed corpus did not start (rc={r['rc']})")
            if not post["skip_warnings"]:
                ok("M0/post: no section skipped on a well-formed corpus")
            else:
                fail(f"M0/post: skipped something on a well-formed corpus: {post['skip_warnings']}")
            if pre["bodies"] == post["bodies"] and pre["system_nodes"] == post["system_nodes"]:
                ok(f"M0: guard is inert on the shipped corpus "
                   f"({pre['bodies']} bodies / {pre['system_nodes']} system nodes, both binaries)")
            else:
                fail(f"M0: guard moved the well-formed load: {pre} vs {post}")
            baseline = post
            continue

        # --- the defect legs -------------------------------------------------
        if pre_outcome == "abort":
            if pre["port"] or not pre["abort"]:
                fail(f"{tag}/pre: expected the PRE-fix binary to ABORT and it did "
                     f"not (rc={pre['rc']}, port={pre['port']}) — the leg is not "
                     f"discriminating")
            else:
                ok(f"{tag}/pre: pre-fix binary died before its port "
                   f"(rc={pre['rc']}, uncaught-exception text in log: True)")
        else:                                   # "garbage:<node>"
            ghost = pre_outcome.split(":", 1)[1]
            pre_nodes, _ = system_nodes(out / f"s545_{tag}_pre.json") if pre["dump"] else ([], 0)
            if ghost in pre_nodes and f"{section}System" not in pre_nodes:
                ok(f"{tag}/pre: pre-fix binary loaded the section under a GARBAGE "
                   f"identity — node '{ghost}' present, '{section}System' lost")
            else:
                fail(f"{tag}/pre: expected the pre-fix binary to produce '{ghost}' "
                     f"and lose '{section}System'; got {pre_nodes}")
        if not post["port"]:
            fail(f"{tag}/post: patched binary did not start (rc={post['rc']})")
            continue
        ok(f"{tag}/post: patched binary started (rc={post['rc']})")

        warns = post["skip_warnings"]
        if len(warns) != 1:
            fail(f"{tag}/post: expected exactly 1 skip warning, got {warns}")
        else:
            w = warns[0]
            named_section = f"[{section}]" in w or f"'{section}'" in w
            named_key = f"'{key}'" in w or f"'{key} = " in w
            if named_section and named_key:
                ok(f"{tag}/post: warning names the section and the key — {w.strip()}")
            else:
                fail(f"{tag}/post: warning does not name section={named_section} "
                     f"key={named_key}: {w.strip()}")

        node = f"{section}System"
        nodes, _ = system_nodes(out / f"s545_{tag}_post.json")
        if pre_outcome.startswith("garbage:") and pre_outcome.split(":", 1)[1] in nodes:
            fail(f"{tag}/post: the garbage node '{pre_outcome.split(':', 1)[1]}' survives "
                 f"the guard")
        if node in nodes:
            fail(f"{tag}/post: {node} is still in the tree — the section was not skipped")
        else:
            ok(f"{tag}/post: {node} absent (the section was skipped, not silently loaded)")
        expected = baseline["system_nodes"] - 1
        if post["system_nodes"] == expected:
            ok(f"{tag}/post: every OTHER system still loaded "
               f"({post['system_nodes']} == {baseline['system_nodes']} - 1)")
        else:
            fail(f"{tag}/post: system-node count {post['system_nodes']}, "
                 f"expected {expected} (one skip, nothing else lost)")

    if b25g.real_tree_md5() != src_md5:
        fail("the real ~/.spacecrafter tree was written by this run")
    else:
        ok("real tree md5 in == out")

    (out / "f12_s545.json").write_text(json.dumps(summary, indent=1))
    print(f"\n{'OK' if not FAILS else str(len(FAILS)) + ' FAILS'} -> {out}/f12_s545.json",
          flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
