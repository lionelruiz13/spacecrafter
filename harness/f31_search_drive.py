#!/usr/bin/env python3
"""F31 / INTENT §5.74 — which of the two it is: the star/constellation NAME
catalogues are not loaded, or the prefix match never fires.

    cd claude/harness && export XAUTHORITY=$(ls /run/user/$(id -u)/.mutter-Xwaylandauth.*) \
        && DISPLAY=:2 ./f31_search_drive.py <absOutdir> [--bin <binary>]

THE ROW'S OWED ITEM (§5.74, verbatim): *"Owed before it is judged: which of the
two it is, on one launch, with the catalogue's own count as the positive
control"*. F28 measured the symptom only: a 26-letter `search` sweep at
`maxobject 320` answers with `(P)` and `(N)` entries and never a `(S)` or a
`(C)` (§11.136(h)).

ONE LAUNCH, TWO SURFACES, BOTH WAYS.

  Phase 1 (default launch, nothing driven yet). The gdb probe reads, at the
  four `listMatchingObjectsI18n` entries the aggregation itself calls, EACH
  CATALOGUE'S OWN COUNT — the container the match loop walks. Planets and
  nebulae, the two that DO answer, are read on the same channel in the same
  call: they are the positive control. The 26-letter sweep runs against those
  counts, so "zero answers" is read beside "how many there were to answer".
  The probe also counts entries into the load sites themselves, which separates
  "loaded nothing" from "never ran".

  Phase 2 (same launch). A sky culture is loaded from a fixture directory OUTSIDE
  the frozen field, by the shipped command `sky_culture action load path <dir>`
  (no file under ~/.spacecrafter is written; the frozen md5 pair is asserted in
  == out by the Session). Its `star_names.fab` is a VERBATIM COPY of the
  installed `~/.spacecrafter/stars/name.fab` — real data, md5-asserted equal,
  nothing from recall. Its `constellationship.fab` is SYNTHETIC and labelled as
  such: no constellation data exists anywhere on this host to copy, so the
  constellation leg uses probe tokens (`Zzprobe*`) over HIP ids taken verbatim
  from that same real file. Then the same `search` command is driven again, with
  a prefix READ OUT OF THE LIVE INDEX by the probe (never typed from recall).

PREDICTIONS, STATED BEFORE THE RUN (asserted at the end, both ways):
  P1  phase 1: constellation catalogue count == 0 AND star name-index count == 0,
      while planet count > 0 AND nebula count > 0 — on the same channel.
  P2  phase 1: `ConstellationMgr::loadLinesAndArt` and `HipStarMgr::loadCommonNames`
      are entered ZERO times during startup, and `Core::setSkyCultureDir` reaches
      its reject line core.cpp:1430 — i.e. the load never ran, it was gated out.
  P3  phase 1: the 36-prefix sweep yields 0 `(S)` and 0 `(C)` (F28 reproduced),
      with `(P)` and `(N)` nonzero.
  P4  phase 2: after the culture load the star index count > 0 and the
      constellation count > 0, and THE SAME 36 COMMANDS then return `(S)` and
      `(C)` entries — in particular the prefix taken from the live index goes
      from 0 `(S)` to nonzero — while `(P)`/`(N)` are unchanged wherever the
      answer is not clamped. So the match fires, and the phase-1 zero was load,
      not match.

If P1/P2 hold and P4 holds, the disjunction is closed on both sides rather than
by elimination: NOT LOADED, and the match works when there is something to match.
"""

import argparse, hashlib, json, os, re, shutil, sys, time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f27_reply as f27

HERE = Path(__file__).resolve().parent
DEFAULT_BIN = str(HERE.parents[1] / "build-claude/src/spacecrafter")
GDB_SCRIPT = HERE / "f31_search.gdb"
SRC_HOME = Path.home() / ".spacecrafter"
REAL_STAR_NAMES = SRC_HOME / "stars" / "name.fab"
# 26 letters (F28's sweep) PLUS the ten digits: the installed star-name file is
# Bayer/Flamsteed, so a large part of the index is keyed on names beginning with
# a digit (`1 AND`). A sweep that cannot reach those prefixes cannot compare a
# prefix taken from the live index against its own pre-load answer.
PREFIXES = "abcdefghijklmnopqrstuvwxyz0123456789"
MAXOBJ = 320                      # F28's sweep setting: the quota does not bind
TAGRE = re.compile(r"\((P|C|N|S)\)")
KEYRE = re.compile(r'\["((?:[^"\\]|\\.)*)"\]')


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def ask(c, cmd, budget=6.0, quiet=0.8):
    """Send, then read until the wire has been quiet for `quiet` seconds or the
    budget runs out. Under gdb every search stops the inferior four times, so a
    fixed pause is not a reply."""
    c.sock.sendall((cmd + "\n").encode())
    got, t0, last = b"", time.time(), time.time()
    while time.time() - t0 < budget:
        d = c.read(0.2)
        if d:
            got += d
            last = time.time()
        elif got and time.time() - last > quiet:
            break
    return got


def tags(raw: bytes):
    """Per-catalogue entry counts in a `search` answer, by the tag the
    aggregation itself appends (core.cpp:2375/2381/2387/2393)."""
    out = {"P": 0, "C": 0, "N": 0, "S": 0}
    payload = "".join(f27.messages(raw))
    for t in TAGRE.findall(payload):
        out[t] += 1
    return out, payload


def probe_lines(path):
    try:
        return Path(path).read_text(errors="replace").splitlines()
    except OSError:
        return []


def last_of(lines, prefix):
    for l in reversed(lines):
        if l.startswith(prefix):
            return l
    return ""


def count_of(lines, prefix):
    return sum(1 for l in lines if l.startswith(prefix))


def last_with(lines, head, needle):
    hits = [l for l in lines if l.startswith(head) and needle in l]
    return hits[-1] if hits else ""


def parse_kv(line, key):
    m = re.search(re.escape(key) + r"=(\S+)", line)
    return m.group(1) if m else None


# ------------------------------------------------------------------ fixture
def build_fixture(root):
    """A sky culture OUTSIDE ~/.spacecrafter. Two provenances, kept apart:
    star_names.fab is REAL installed data copied byte for byte; the two
    constellation files are SYNTHETIC probe tokens (no astronomical claim) over
    HIP ids read verbatim out of that same real file."""
    root.mkdir(parents=True, exist_ok=True)
    dst = root / "star_names.fab"
    shutil.copy(REAL_STAR_NAMES, dst)
    assert md5(dst) == md5(REAL_STAR_NAMES), "star_names.fab copy is not verbatim"

    hips = []
    for line in REAL_STAR_NAMES.read_text(errors="replace").splitlines():
        m = re.match(r"\s*(\d+)\s*\|", line)
        if m and m.group(1) not in hips:
            hips.append(m.group(1))
        if len(hips) >= 12:
            break
    assert len(hips) >= 6, "not enough HIP ids in the real star-name file"

    # `ABR nbSegments HP HP ...` (constellation.cpp:54-97). Three independent
    # records over different real HIP ids: a record whose stars the loaded
    # catalogue cannot resolve is dropped alone, so three is three chances.
    ship = [f"F3A 1 {hips[0]} {hips[1]}",
            f"F3B 1 {hips[2]} {hips[3]}",
            f"F3C 1 {hips[4]} {hips[5]}"]
    (root / "constellationship.fab").write_text("\n".join(ship) + "\n")
    (root / "constellation_names.eng.fab").write_text(
        "F3A Zzprobealfa\nF3B Zzprobebeta\nF3C Zzprobegamma\n")
    return {"star_names_src": str(REAL_STAR_NAMES),
            "star_names_md5": md5(dst),
            "star_names_lines": len(REAL_STAR_NAMES.read_text(errors="replace").splitlines()),
            "synthetic_constellationship": ship,
            "synthetic_names": ["Zzprobealfa", "Zzprobebeta", "Zzprobegamma"]}


def run_sweep(c, label, out):
    """The same 36 `search` commands, driven identically in both phases: the
    pre/post pair is then a comparison of one command with itself, not of two
    questions."""
    sw, raws = {}, {}
    for p in PREFIXES:
        raw = ask(c, f"search name {p} maxobject {MAXOBJ}")
        t, payload = tags(raw)
        sw[p], raws[p] = t, payload
        print(f"  {label} {p}: {t} len={len(payload)}", flush=True)
    (out / f"f31_{label}_replies.txt").write_text(
        "\n".join(f"[{p}] {raws[p]}" for p in PREFIXES), errors="replace")
    return sw, raws


def pick_prefix(sample_line, phase1_len):
    """A prefix taken FROM the loaded catalogue's own content — the keys the
    probe read out of the LIVE `common_names_index_i18n`, never typed.

    It is ONE ASCII character, and that is a property of the data, not a
    shortcut: every ASCII-initial key in the installed star-name file is a
    single-letter Bayer designation followed by a space (`B AND`), so no
    space-free ASCII prefix longer than 1 exists to take
    `[measured: 0 lines matching ^\\d+\\|[A-Za-z]{2,} in stars/name.fab]`. One
    character also makes the post-load command BYTE-IDENTICAL to one of the 26
    phase-1 commands, which is what makes pre/post a both-ways comparison on the
    same drive rather than two different questions.

    Among the candidate letters, take the one whose phase-1 answer was SHORTEST:
    the answer is clamped at 1024 B and sorted, so a crowded letter could push
    the new `(S)` entries past the clamp and turn a fired match into a zero."""
    cands = []
    for k in KEYRE.findall(sample_line):
        if k and re.fullmatch(r"[A-Z0-9]", k[0]):
            cands.append((phase1_len.get(k[0].lower(), 10 ** 6), k[0].lower(), k))
    if not cands:
        return None, None
    cands.sort()
    return cands[0][1], cands[0][2]


def sweep_totals(sw):
    return {k: sum(v[k] for v in sw.values()) for k in "PCNS"}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("--bin", default=DEFAULT_BIN)
    a = ap.parse_args()
    out = Path(a.outdir)
    out.mkdir(parents=True, exist_ok=True)
    probe = out / "f31_probe.txt"
    probe.unlink(missing_ok=True)
    os.environ["F31_PROBE"] = str(probe)

    res = {"binary": a.bin, "binary_md5": md5(a.bin),
           "wall_start": time.strftime("%F %T %Z"),
           "predictions": ["P1 phase1 const==0 and star_index==0 while planets>0 and nebulae>0",
                           "P2 phase1 loadLinesAndArt==0 hits, loadCommonNames==0 hits, gate reject fired",
                           "P3 phase1 36-prefix sweep: 0 (S), 0 (C), nonzero (P), nonzero (N)",
                           "P4 phase2 after culture load: star_index>0, const>0; the SAME 36 "
                           "commands return (S) and (C); the live-index prefix goes 0 -> nonzero "
                           "(S); (P)/(N) unchanged on every unclamped prefix"]}
    print(json.dumps(res["predictions"], indent=2), flush=True)

    fixture = out / "culture_f31"
    res["fixture"] = build_fixture(fixture)

    sess = f27.Session(out, "f31", a.bin,
                       launch_prefix=("gdb", "-q", "-batch", "-x", str(GDB_SCRIPT), "--args"),
                       port_wait=180)
    c = sess.client("drv")
    try:
        # ---------------------------------------------------------- phase 1
        sweep, raws = run_sweep(c, "phase1", out)
        res["phase1_sweep"] = sweep
        res["phase1_totals"] = sweep_totals(sweep)

        lines = probe_lines(probe)
        res["phase1_probe"] = {
            "planet": last_of(lines, "PLANET"),
            "const": last_of(lines, "CONST"),
            "neb": last_of(lines, "NEB"),
            "star": last_with(lines, "STAR", "common_names_index_i18n="),
            "star_sample": last_with(lines, "STAR", "sample="),
            "setculture": [l for l in lines if l.startswith("SETCULTURE")],
            "gate_reject_hits": count_of(lines, "GATE-REJECT"),
            "loadlines_hits": count_of(lines, "LOADLINES #"),
            "loadcommon_hits": count_of(lines, "LOADCOMMON"),
            "core_hits": count_of(lines, "CORE #"),
        }

        # ---------------------------------------------------------- phase 2
        load_cmd = f"sky_culture action load path {fixture}"
        print(f"  phase2 {load_cmd}", flush=True)
        ask(c, load_cmd, budget=20.0, quiet=3.0)
        # a throwaway search purely to make the probe re-read the live indices
        ask(c, "search name Qqzz-nothing maxobject 4")
        lines = probe_lines(probe)
        star_sample = last_with(lines, "STAR", "sample=")
        prefix, full_key = pick_prefix(star_sample, {p: len(raws[p]) for p in PREFIXES})
        res["phase2_probe"] = {
            "loadsky": last_of(lines, "LOADSKY"),
            "loadlines": last_of(lines, "LOADLINES #"),
            "loadlines_done": last_of(lines, "LOADLINES-DONE"),
            "loadcommon": last_of(lines, "LOADCOMMON"),
            "updatei18n": last_of(lines, "UPDATEI18N"),
            "const": last_with(lines, "CONST", "catalogue_count="),
            "const_sample": last_with(lines, "CONST", "sample="),
            "star": last_with(lines, "STAR", "common_names_index_i18n="),
            "star_sample": star_sample,
            "prefix_from_live_index": prefix,
            "prefix_source_key": full_key,
        }
        print(f"  phase2 prefix from live index: {prefix!r} (key {full_key!r})", flush=True)

        phase2 = {}
        # the SAME 36 commands again — the both-ways record
        sweep2, raws2 = run_sweep(c, "phase2", out)
        res["phase2_sweep"] = sweep2
        res["phase2_totals"] = sweep_totals(sweep2)
        res["sweep_delta"] = {p: {k: sweep2[p][k] - sweep[p][k] for k in "PCNS"}
                              for p in PREFIXES if sweep2[p] != sweep[p]}
        if prefix:
            phase2["prefix_from_live_index"] = {
                "cmd": f"search name {prefix} maxobject {MAXOBJ}",
                "key_it_came_from": full_key,
                "pre": sweep[prefix], "post": sweep2[prefix],
                "pre_reply": raws[prefix], "post_reply": raws2[prefix]}
            print(f"  live-index prefix {prefix!r} (from {full_key!r}): "
                  f"pre={sweep[prefix]} post={sweep2[prefix]}", flush=True)
        raw = ask(c, f"search name Zzprobe maxobject {MAXOBJ}")
        t, payload = tags(raw)
        phase2["constellation_token"] = {"cmd": f"search name Zzprobe maxobject {MAXOBJ}",
                                         "tags": t, "reply": payload}
        print(f"  phase2 Zzprobe: {t}", flush=True)
        res["phase2"] = phase2
        res["reply_len"] = {"phase1": {p: len(raws[p]) for p in PREFIXES},
                            "phase2": {p: len(raws2[p]) for p in PREFIXES}}
        # A post-load answer that reaches the 1024 clamp can LOSE (P)/(N)
        # entries to the newly-inserted, alphabetically-interleaved (S) ones —
        # that is the clamp, not a change of catalogue. The controls are
        # therefore only compared where the post answer is unclamped.
        res["unclamped_prefixes"] = [p for p in PREFIXES if len(raws2[p]) < 1000]
    finally:
        try:
            rc = sess.stop(c)
        except Exception as e:              # noqa: BLE001
            rc = f"stop failed: {e}"
        res["app_exit"] = rc
        res["md5_in"] = sess.md5_in
        res["md5_out"] = {n: md5(SRC_HOME / n) for n in f27.FROZEN}
        res["probe_lines"] = len(probe_lines(probe))
        # A breakpoint that never resolved would report "0 hits" exactly like a
        # site that never ran — the silent-no-op-probe class (§11.47). gdb lists
        # every breakpoint before `run`; a pending one says <PENDING> there.
        applog = (out / "f31.applog").read_text(errors="replace")
        res["pending_breakpoints"] = [l.strip() for l in applog.splitlines()
                                      if "PENDING" in l]
        res["breakpoints_listed"] = len(re.findall(r"^\d+\s+breakpoint\s+keep",
                                                   applog, re.M))
        res["wall_end"] = time.strftime("%F %T %Z")

    # ------------------------------------------------------------- verdict
    p1p = res["phase1_probe"]
    def num(line, key):
        v = parse_kv(line or "", key)
        try:
            return int(v)
        except (TypeError, ValueError):
            return None
    checks = {}
    checks["P1_const_zero"] = num(p1p["const"], "catalogue_count") == 0
    checks["P1_star_zero"] = num(p1p["star"], "common_names_index_i18n") == 0
    checks["P1_planets_nonzero"] = (num(p1p["planet"], "catalogue_count") or 0) > 0
    checks["P1_nebulae_nonzero"] = (num(p1p["neb"], "catalogue_count") or 0) > 0
    checks["P2_load_never_ran"] = (p1p["loadlines_hits"] == 0 and p1p["loadcommon_hits"] == 0)
    checks["P2_gate_rejected"] = p1p["gate_reject_hits"] >= 1
    checks["P3_sweep_zero_S"] = res["phase1_totals"]["S"] == 0
    checks["P3_sweep_zero_C"] = res["phase1_totals"]["C"] == 0
    checks["P3_sweep_nonzero_P"] = res["phase1_totals"]["P"] > 0
    checks["P3_sweep_nonzero_N"] = res["phase1_totals"]["N"] > 0
    p2p = res.get("phase2_probe", {})
    checks["P4_star_index_nonzero"] = (num(p2p.get("star"), "common_names_index_i18n") or 0) > 0
    checks["P4_const_nonzero"] = (num(p2p.get("const"), "catalogue_count") or 0) > 0
    checks["P4_sweep_returns_S"] = res.get("phase2_totals", {}).get("S", 0) > 0
    checks["P4_sweep_returns_C"] = res.get("phase2_totals", {}).get("C", 0) > 0
    sp = res.get("phase2", {}).get("prefix_from_live_index")
    checks["P4_live_index_prefix_both_ways"] = (
        bool(sp) and sp["pre"]["S"] == 0 and sp["post"]["S"] > 0)
    ct = res.get("phase2", {}).get("constellation_token")
    checks["P4_search_returns_C_by_name"] = bool(ct) and ct["tags"]["C"] > 0
    checks["P4_controls_unmoved"] = bool(res.get("unclamped_prefixes")) and all(
        res["phase2_sweep"][p]["P"] == res["phase1_sweep"][p]["P"]
        and res["phase2_sweep"][p]["N"] == res["phase1_sweep"][p]["N"]
        for p in res["unclamped_prefixes"])
    # instrument chain, both ways: the two load-site breakpoints that must read
    # ZERO in phase 1 are the SAME breakpoints that must read nonzero in phase 2
    # (loadSkyCulture calls both) — so their phase-1 zero is a measurement, not
    # an unresolved symbol.
    checks["probe_loadsites_positively_mapped"] = (
        bool(res.get("phase2_probe", {}).get("loadlines"))
        and bool(res.get("phase2_probe", {}).get("loadcommon")))
    checks["no_pending_breakpoints"] = not res.get("pending_breakpoints")
    checks["frozen_md5_in_eq_out"] = res["md5_in"] == res["md5_out"]
    res["checks"] = checks
    res["verdict"] = ("NOT LOADED (and the match fires when the catalogue is not empty)"
                      if all(checks.values()) else "INCOMPLETE — see checks")

    (out / "f31_result.json").write_text(json.dumps(res, indent=2, ensure_ascii=False))
    print("\n=== F31 checks ===", flush=True)
    for k, v in checks.items():
        print(f"  {'PASS' if v else 'FAIL'}  {k}", flush=True)
    print(f"verdict: {res['verdict']}", flush=True)
    return 0 if all(checks.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
