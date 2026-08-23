#!/usr/bin/env python3
"""F28 / INTENT §5.73 — how long an answer really is, and what the send path
does with it.

    cd claude/harness && DISPLAY=:2 ./f28_send_buffer.py <absOutdir> --mode census \
        [--bin <binary>]
    cd claude/harness && DISPLAY=:2 ./f28_send_buffer.py <absOutdir> --mode overflow \
        --tag <name> [--bin <binary>] [--expect pre|post]

WHAT §5.73 RECORDS (derived from source, never reproduced). `setOutput` clamps
an answer to `MAX_BUFFER` = 1024 (io.cpp:63, 364-368). The send path then
copies `data + '\\n'` with `strcpy` — L + 2 bytes for an answer of length L —
into `buffer = new char[bufferSize]` (io.cpp:184), where `bufferSize` is
`tcp_buffer_in_size` = 1024 in the loaded config. So L >= 1023 writes past the
end, up to 2 bytes at the clamp. The row names two producers with no length
bound between them and the clamp — `get status object` and `search` — and owes
ONE thing before the fix shape can be judged: *what those two commands actually
answer on a shipped scene*, which nothing had measured.

THE TWO MODES.

`census` — one fresh launch on the shipped corpus. It asks `get status object`
for a selected shipped body (and, for the bound, for one of each other
selectable kind the catalogue itself offers) and `search` at a ladder of
`max_object` values, and records the EXACT byte length of every answer as it
arrives on the wire. It also records, per command, whether the app logged its
own clamp warning (`ServerSocket data setOutput too big`) — so "the producer
exceeded 1024" is the app's statement, not an inference from a saturated wire
length. The ladder is the instrument's own positive control: an answer length
that GROWS with `max_object` and then stops growing at exactly 1024 shows both
that the channel measures length and that the clamp is what stops it.

`overflow` — the minimal drive that makes the maximal answer, used as the SAME
drive on four binaries (native pre/post, ASan pre/post). It writes the reply
bytes verbatim to `<out>/<tag>_reply.bin`, so pre/post byte-identity is a file
comparison and not a claim, and counts `ERROR: AddressSanitizer` reports in the
app's own output.

FRAMING ON THE WIRE. `ServerSocket::send` writes `strlen(buffer) + 1` bytes,
i.e. the message AND its terminating NUL, and the answer path appends '\\n' to
the queued string. So one answer on the wire is `payload + '\\n' + '\\0'`, and
the payload itself may contain newlines (`get status object` is multi-line) —
messages are split on NUL, never on newline.

Every launch carries the §11.121(m) concurrent-instance assert (F26's
/proc/<pid>/comm probe) and the frozen config/ssystem md5 in == out, through
`f27_reply.Session` — the same launcher F27 measured with (I2).
"""

import argparse, json, os, re, sys, time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f27_reply as f27

HERE = Path(__file__).resolve().parent
DEFAULT_BIN = str(HERE.parents[1] / "build-claude/src/spacecrafter")
MAX_BUFFER = 1024          # io.cpp:63 — the clamp in setOutput
CLAMP_WARNING = "ServerSocket data setOutput too big"   # io.cpp:366
# The command whose answer crosses 1023 B on the shipped corpus, fixed by the
# census (`maxobject` is the argument's real spelling,
# base_command_interface.hpp:100). Overridable so a shorter answer can be
# driven through the same code path as a control.
PLANETS_CMD = "get status planets_position"
# Measured by the census on the SHIPPED corpus: prefix `m` answers 854 B (fits)
# and prefix `n` answers 1024 B, clamped — the row's own producer reaching the
# overflow with no body loaded and no argument a shipped show could not use.
SHORT_SEARCH = "search name m maxobject 320"
LONG_SEARCH = "search name n maxobject 320"
# A non-satellite body, so it appears in `getPlanetsPosition`. Same shape as
# `b10_cmd.py`'s command-channel body (I2 — the form is that script's record).
PAD_BODY = ("parent Sun type Planet oblateness 0.0 albedo 0.3 halo false "
            "color 0.6,0.6,0.9 tex_map bodies/moon.png coord_func still_orbit "
            "radius 100 orbit_x 900 orbit_y 0 orbit_z 0")

FAILS, NOTES = [], []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}", flush=True)


def ok(msg):
    print(f"ok:   {msg}", flush=True)


def note(msg):
    NOTES.append(msg)
    print(f"      {msg}", flush=True)


def answers(raw: bytes):
    """The complete NUL-terminated messages in `raw`, as raw bytes, with the
    trailing NUL removed. An incomplete tail is dropped (and reported by the
    caller through `len(raw)` vs the sum of the parts)."""
    parts = raw.split(b"\x00")
    return [p for p in parts[:-1]]


def ask(sess, c, cmd, wait=2.0):
    """Send one command and collect what comes back, with the app's own log
    lines produced while it was served. Returns a dict — never a bare length,
    because a length with no idea whether the clamp fired says nothing."""
    mark = sess.logmark()
    raw = c.send(cmd, wait)
    msgs = answers(raw)
    newlog = sess.lognew(mark)
    r = {"cmd": cmd,
         "raw_bytes": len(raw),
         "messages": len(msgs),
         "clamped": CLAMP_WARNING in newlog,
         "refused": [l for l in newlog.splitlines() if "Could not execute" in l]}
    if msgs:
        m = msgs[0]
        # payload = message minus the '\n' the answer path appends
        r["wire_message_bytes"] = len(m) + 1      # + the NUL that framed it
        r["payload_bytes"] = len(m) - 1 if m.endswith(b"\n") else len(m)
        r["ends_with_newline"] = m.endswith(b"\n")
        r["head"] = m[:120].decode("latin-1")
        r["tail"] = m[-60:].decode("latin-1")
        r["bytes"] = m
    else:
        r["wire_message_bytes"] = 0
        r["payload_bytes"] = None
    return r


def show(r):
    if r["payload_bytes"] is None:
        print(f"      {r['cmd']!r}: NO ANSWER ({r['raw_bytes']} bytes read)",
              flush=True)
    else:
        print(f"      {r['cmd']!r}: payload {r['payload_bytes']} B, "
              f"on the wire {r['wire_message_bytes']} B"
              f"{', CLAMPED' if r['clamped'] else ''}", flush=True)


# ------------------------------------------------------------------- census
ENTRY_RE = re.compile(r"([^;]+)\((P|C|N|S)\)")


def mode_census(out, binary):
    sess = f27.Session(out, "census", binary)
    res = {"mode": "census", "binary": str(binary), "md5": f27.md5(binary)}
    c = sess.client("driver")
    try:
        # A still, dated scene so the numbers are reproducible; the corpus is
        # the SHIPPED one (build_farm symlinks ~/.spacecrafter).
        c.send("timerate rate 0", 1)
        c.send(f"date jday {f27.JD}", 1)

        # --- what the catalogue itself offers, so no name comes from recall --
        probe = ask(sess, c, "search name a maxobject 60", 2.5)
        show(probe)
        entries = ENTRY_RE.findall(probe.get("head", "") or "")
        full = probe["bytes"].decode("latin-1") if probe.get("bytes") else ""
        entries = ENTRY_RE.findall(full)
        res["catalogue_probe"] = {k: v for k, v in probe.items() if k != "bytes"}
        res["catalogue_entries"] = entries[:60]
        by_kind = {}
        for name, kind in entries:
            by_kind.setdefault(kind, []).append(name.strip())
        note(f"catalogue kinds from `search name a maxobject 60`: "
             + ", ".join(f"{k}={len(v)}" for k, v in sorted(by_kind.items())))

        # --- `get status object`, the row's first producer ------------------
        obj = []
        targets = [("planet", "Mars"), ("planet", "Saturn"), ("planet", "Earth"),
                   ("planet", "Sun"), ("planet", "Moon")]
        for kind, names in (("star", by_kind.get("S", [])),
                            ("nebula", by_kind.get("N", [])),
                            ("constellation", by_kind.get("C", []))):
            if names:
                targets.append((kind, names[0]))
        for kind, name in targets:
            c.send(f"select {kind} {name} pointer off", 1.2)
            r = ask(sess, c, "get status object", 1.6)
            r["selected"] = f"{kind} {name}"
            show(r)
            obj.append(r)
        res["get_status_object"] = [{k: v for k, v in r.items() if k != "bytes"}
                                    for r in obj]

        # --- `search`, the row's second producer: the max_object ladder -----
        # `maxobject` is the argument's ACTUAL spelling
        # (base_command_interface.hpp:100) — the first run of this ladder used
        # `max_object` and every rung answered the same 56 B, i.e. the default
        # 5. The ladder's own growth control caught it (that is what it is for).
        lad = []
        for n in (None, 10, 20, 40, 80, 160, 320):
            cmd = "search name a" if n is None else f"search name a maxobject {n}"
            r = ask(sess, c, cmd, 2.2)
            r["maxobject"] = n
            show(r)
            lad.append(r)
        res["search_ladder"] = [{k: v for k, v in r.items() if k != "bytes"}
                                for r in lad]
        (out / "census_search_max.bin").write_bytes(lad[-1].get("bytes", b""))

        # --- the other answers on the same path, for the bound --------------
        others = []
        for cmd in ("get status position", "get status planets_position",
                    "get status constellation", "get status media",
                    "search name e maxobject 320", "search name s maxobject 320"):
            r = ask(sess, c, cmd, 1.8)
            show(r)
            others.append(r)
        res["other_producers"] = [{k: v for k, v in r.items() if k != "bytes"}
                                  for r in others]

        # --- the whole first-letter surface of `search`, so "the shipped
        #     corpus never reaches 1023" is an enumerated negative and not a
        #     failed guess at a prefix ------------------------------------
        sweep = []
        for ch in "abcdefghijklmnopqrstuvwxyz":
            r = ask(sess, c, f"search name {ch} maxobject 320", 1.1)
            r["prefix"] = ch
            sweep.append({k: v for k, v in r.items() if k != "bytes"})
        res["search_sweep"] = sweep
        top = sorted((s for s in sweep if s["payload_bytes"] is not None),
                     key=lambda s: -s["payload_bytes"])[:5]
        note("search first-letter sweep, longest 5: "
             + ", ".join(f"{s['prefix']}={s['payload_bytes']}B" for s in top))

        # --- the ONE producer that comes close, and what makes it grow -----
        # `get status planets_position` enumerates every NON-SATELLITE body of
        # the current system (protosystem.cpp:451). It is 854 B on the shipped
        # corpus; a body added with `body action load` adds one entry, so the
        # answer's length is a function of the loaded system, not of the code.
        lad = []
        base = ask(sess, c, "get status planets_position", 1.6)
        show(base)
        lad.append({k: v for k, v in base.items() if k != "bytes"})
        for i, nlen in enumerate((30, 30, 30)):
            name = f"F28Pad{i}" + "X" * (nlen - 7)
            c.send(f"body action load name {name} {PAD_BODY}", 2.0)
            r = ask(sess, c, "get status planets_position", 1.6)
            r["added_name_len"] = len(name)
            show(r)
            lad.append({k: v for k, v in r.items() if k != "bytes"})
        res["planets_position_ladder"] = lad
        res["refused"] = sess.refused()
    finally:
        res["exit"] = sess.stop(c)

    # --- asserts: the instrument must be shown to measure length -----------
    lad = res["search_ladder"]
    lens = [r["payload_bytes"] for r in lad]
    if any(l is None for l in lens):
        fail(f"census: `search` gave no answer at some rung: {lens}")
    else:
        if len(set(lens)) < 2:
            fail(f"census: the search ladder never changes length ({lens}) — "
                 f"the channel is not measuring what it claims to")
        else:
            ok(f"census: the search answer grows with max_object: {lens}")
        sat = [r for r in lad if r["clamped"]]
        if sat:
            over = [r["payload_bytes"] for r in sat]
            if set(over) != {MAX_BUFFER}:
                fail(f"census: a clamped answer is not {MAX_BUFFER} B on the "
                     f"wire: {over}")
            else:
                ok(f"census: every clamped answer arrives at exactly "
                   f"{MAX_BUFFER} B ({len(sat)} rungs) — the clamp is what "
                   f"stops the growth")
        else:
            note("census: the clamp never fired on this ladder")
    for r in res["get_status_object"]:
        if r["payload_bytes"] is None:
            fail(f"census: no answer to `get status object` for {r['selected']}")
    if res["refused"]:
        fail(f"census: the app refused command(s): {res['refused']}")
    else:
        ok("census: the app refused nothing (its own §2(f) channel)")
    return res


# ----------------------------------------------------------------- overflow
def mode_overflow(out, binary, tag, expect):
    """The same answer, at four lengths, on one connection.

    §5.73's arithmetic: the send path copies `data + '\\n'` — L + 2 bytes — into
    a 1024-byte buffer, so L <= 1022 fits exactly and L = 1023 is the FIRST
    length that writes past the end. This drive walks an answer up to that
    boundary and past it, shortest lengths first, so that a binary which stops
    at the first fire has still produced both negative controls:

      step 0  `search name m maxobject 320` ...  854 B   FITS - shipped
      step 1  `get status planets_position` ...  854 B   FITS - shipped
      step 2  + one padding body, to price one entry
      step 3  + one body, NAME COMPUTED ...... 1022 B   FITS - the LAST length
              that does; a silent step 3 with a firing step 4 is the whole
              discrimination, one byte apart on one code path
      step 4  + one body, NAME COMPUTED ...... 1023 B   1 byte PAST the end
      step 5  `search name n maxobject 320` .. 1024 B   2 bytes past, and
              CLAMPED by `setOutput` - the row's own producer, on the shipped
              corpus, with no body loaded at all

    The name lengths are computed FROM THE MEASURED answer inside the launch,
    never from a constant carried between launches: step 2 measures what one
    body costs (its entry is name + ':' + three `std::to_string` doubles +
    ';'), and steps 3-4 solve for the name length that lands on the target. If
    a step misses its target the drive says so and does not pretend.
    """
    sess = f27.Session(out, tag, binary)
    res = {"mode": "overflow", "tag": tag, "binary": str(binary),
           "md5": f27.md5(binary), "expect": expect, "steps": []}
    c = sess.client("driver")
    died = False
    try:
        c.send("timerate rate 0", 1)
        c.send(f"date jday {f27.JD}", 1)

        def measure(step, cmd, added=None):
            r = ask(sess, c, cmd, 2.2)
            show(r)
            d = {k: v for k, v in r.items() if k != "bytes"}
            d["step"] = step
            d["added_name_len"] = added
            res["steps"].append(d)
            (out / f"{tag}_step{step}_reply.bin").write_bytes(r.get("bytes", b""))
            return r["payload_bytes"]

        measure(0, SHORT_SEARCH)
        L1 = measure(1, PLANETS_CMD)
        # step 2: a body of known name length, to price one entry
        n2 = 30
        c.send(f"body action load name {'A' * n2} {PAD_BODY}", 2.2)
        L2 = measure(2, PLANETS_CMD, n2)
        if L1 is None or L2 is None:
            fail(f"{tag}: no answer at step 1/2 ({L1}/{L2}) — nothing to tune")
        else:
            overhead = L2 - L1 - n2        # ':' + three doubles + ';'
            note(f"{tag}: one entry costs name + {overhead} B "
                 f"({L1} -> {L2} for a {n2}-char name)")
            res["entry_overhead"] = overhead
            need = 1022 - L2 - overhead
            if need < 1:
                fail(f"{tag}: 1022 B cannot be reached from {L2} "
                     f"(one entry costs at least {overhead + 1} B)")
            else:
                name3 = "S3" + "B" * (need - 2)
                c.send(f"body action load name {name3} {PAD_BODY}", 2.2)
                got = measure(3, PLANETS_CMD, len(name3))
                if got != 1022:
                    fail(f"{tag}: step 3 landed on {got} B, not 1022 B")
                else:
                    ok(f"{tag}: step 3 answer is exactly 1022 B (the LAST "
                       f"length that fits)")
                # 1023 cannot be REACHED from 1022 by adding: one entry costs
                # at least 33 B. So the step-3 body is DROPPED (`body action
                # drop`, the shipped inverse of `body action load`) and put
                # back with a name one character longer — same rung, one byte
                # higher, which is the whole point of the pair.
                c.send(f"body action drop name {name3}", 2.0)
                c.send(f"body action load name {name3}C {PAD_BODY}", 2.2)
                got = measure(4, PLANETS_CMD, len(name3) + 1)
                if got != 1023:
                    fail(f"{tag}: step 4 landed on {got} B, not 1023 B")
                else:
                    ok(f"{tag}: step 4 answer is exactly 1023 B (one byte past "
                       f"the end of a {MAX_BUFFER} B buffer)")
        measure(5, LONG_SEARCH)
        # a short answer AFTER the long ones: a corrupted heap does not have to
        # fault at the strcpy that corrupted it
        r2 = ask(sess, c, "get status position", 2.0)
        show(r2)
        res["after"] = {k: v for k, v in r2.items() if k != "bytes"}
        res["refused"] = sess.refused()
        died = sess.proc.poll() is not None
    finally:
        res["alive_before_stop"] = not died
        try:
            res["exit"] = sess.stop(c)
        except Exception as e:                       # a dead app cannot be shut down
            res["exit"] = f"stop raised {e}"
    applog = (out / f"{tag}.applog").read_text(errors="replace")
    reports = [l for l in applog.splitlines() if "ERROR: AddressSanitizer" in l]
    res["asan_reports"] = reports
    res["asan_count"] = len(reports)
    res["applog_bytes"] = len(applog)
    print(f"      asan reports: {len(reports)}", flush=True)
    for l in reports:
        print(f"        {l.strip()[:150]}", flush=True)

    # --- what the drive must show, whichever binary ran it -----------------
    steps = {s["step"]: s for s in res["steps"]}
    for k in (0, 1, 2, 3):
        s = steps.get(k)
        if s and s["payload_bytes"] is not None and s["payload_bytes"] > 1022:
            fail(f"{tag}: step {k} is {s['payload_bytes']} B — it was meant to "
                 f"FIT, so it cannot serve as the negative control")
    s5 = steps.get(5)
    if s5 and s5["payload_bytes"] is not None:
        if not s5["clamped"]:
            fail(f"{tag}: step 5 answered {s5['payload_bytes']} B without the "
                 f"clamp warning — `setOutput` did not clamp")
        else:
            ok(f"{tag}: step 5 clamped by setOutput, {s5['payload_bytes']} B "
               f"on the wire")
    if expect == "pre":
        note(f"{tag}: pre-fix run — an ASan build is expected to name the "
             f"overflow at step 3; a native build is expected to deliver the "
             f"bytes anyway (the 2 B land in malloc's slack)")
    else:
        if reports:
            fail(f"{tag}: {len(reports)} AddressSanitizer report(s) on the "
                 f"post-fix binary")
    return res


# -------------------------------------------------------------------- logon
# The other thing the fix touched: the fixed answers `computeNormalString`
# used to `strcpy` into `buffer` before sending. Those are the `$NOTICE` /
# `$LOGON` / `$LOGOFF` replies - a REVERSIBLE PAIR, so it is entered twice,
# the second entry starting from the state the first exit produced, and every
# byte is compared pre/post rather than asserted by shape.
LOGON_DRIVE = ["$NOTICE",
               "$LOGON",              # entry 1
               "$LOGON",              # already subscribed -> REQUEST ERROR
               "$LOGOFF",             # exit 1
               "$LOGOFF",             # not subscribed -> REQUEST ERROR
               "$LOGON",              # entry 2, from what exit 1 left behind
               "get status position",  # subscriber AND issuer: exactly one copy
               "$LOGOFF",             # exit 2
               "get status position"]  # addressed copy only


def mode_logon(out, binary, tag, expect):
    sess = f27.Session(out, tag, binary)
    res = {"mode": "logon", "tag": tag, "binary": str(binary),
           "md5": f27.md5(binary), "expect": expect, "exchanges": []}
    c = sess.client("driver")
    blob = b""
    try:
        for cmd in LOGON_DRIVE:
            raw = c.send(cmd, 1.5)
            blob += raw
            msgs = answers(raw)
            res["exchanges"].append(
                {"cmd": cmd, "messages": [m.decode("latin-1") for m in msgs],
                 "bytes": len(raw)})
            print(f"      {cmd!r} -> {[m.decode('latin-1')[:48] for m in msgs]}",
                  flush=True)
    finally:
        res["exit"] = sess.stop(c)
    (out / f"{tag}_logon.bin").write_bytes(blob)
    counts = [len(e["messages"]) for e in res["exchanges"]]
    if counts[6] != 1 or counts[8] != 1:
        fail(f"{tag}: a `get` answered {counts[6]} / {counts[8]} times "
             f"(subscribed / not) — one copy each is the F27 contract")
    else:
        ok(f"{tag}: one copy subscribed, one copy not — both entries of the "
           f"pair")
    applog = (out / f"{tag}.applog").read_text(errors="replace")
    res["asan_count"] = len([l for l in applog.splitlines()
                             if "ERROR: AddressSanitizer" in l])
    print(f"      asan reports: {res['asan_count']}", flush=True)
    return res


def main():
    a = argparse.ArgumentParser()
    a.add_argument("outdir")
    a.add_argument("--bin", default=DEFAULT_BIN)
    a.add_argument("--mode", choices=("census", "overflow", "logon"),
                   default="census")
    a.add_argument("--tag", default="ovf")
    a.add_argument("--expect", choices=("pre", "post"), default="post")
    a = a.parse_args()
    out = Path(a.outdir).resolve()
    out.mkdir(parents=True, exist_ok=True)
    print("=== F28 §5.73 send-buffer measurement ===", flush=True)
    print(f"binary : {a.bin}", flush=True)
    print(f"md5    : {f27.md5(a.bin)}", flush=True)
    print(f"mode   : {a.mode}   tag: {a.tag}   expect: {a.expect}", flush=True)
    print(f"asan   : ASAN_OPTIONS={os.environ.get('ASAN_OPTIONS', '(unset)')}",
          flush=True)
    print(f"wall   : {time.strftime('%F %T %Z')}", flush=True)

    if a.mode == "census":
        res = mode_census(out, a.bin)
        name = "f28_census.json"
    elif a.mode == "logon":
        res = mode_logon(out, a.bin, a.tag, a.expect)
        name = f"f28_logon_{a.tag}.json"
    else:
        res = mode_overflow(out, a.bin, a.tag, a.expect)
        name = f"f28_overflow_{a.tag}.json"
    res["fails"], res["notes"] = FAILS, NOTES
    res["wall"] = time.strftime("%F %T %Z")
    (out / name).write_text(json.dumps(res, indent=1, default=str))
    print(f"\n{len(FAILS)} FAIL  ->  {out / name}", flush=True)
    # f27's own module-level FAILS (the Session asserts write there)
    if f27.FAILS:
        print(f"{len(f27.FAILS)} FAIL from the shared launcher: {f27.FAILS}",
              flush=True)
    return 1 if (FAILS or f27.FAILS) else 0


if __name__ == "__main__":
    sys.exit(main())
