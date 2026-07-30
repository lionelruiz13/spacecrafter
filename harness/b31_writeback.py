#!/usr/bin/env python3
"""B31 slice 1 gate: the §11.66(b) line-preserving write-back layer
(b31-design §5.2/§5.3, check T9; INTENT §11.119).

WHAT IT MEASURES. The layer promises that a file which goes through it comes
back whole: comments, blank lines, spacing, key order, lines it cannot read,
ISO-8859 bytes, a missing final newline. And that a CHANGE changes exactly what
was asked for: one value rewritten inside its own line, a new key at the end of
its section's declarations, a retired key commented out with its reason rather
than deleted, a loader's diagnosis placed ABOVE its datum and replaced - not
duplicated - when the same diagnosis is reached again.

WHAT IT DRIVES. `b31_format/format_gate.cpp` compiled against the PRODUCT
sources (`src/experimentalModule/ModularSystemFormat.cpp`, `src/tools/log.cpp`).
Nothing here reimplements the parser: a Python copy of it would only ever test
the copy.

DISCRIMINATION, three ways, all run:
  * the PRE-REWORK writer, compiled from the same tree at the commit before this
    slice, over the same corpus: it must LOSE the file (it drops every comment,
    drops the malformed line, reorders the keys, re-spaces every entry). A gate
    that both writers pass is testing nothing.
  * the NAIVE annotator (`--naive`, implemented here in Python as the obvious
    wrong way: insert the annotation above the datum and never look for an
    earlier one): the T9 idempotence leg must FAIL on it, and the failure is
    reported with the measured growth.
  * every leg asserts what must NOT move as well as what must: a value change
    that touches a second line is a failure even if the value is right.

    cd claude/harness && ./b31_writeback.py [outdir]

Exit 0 = every leg green. No display, no app launch: this is the layer's own
gate. The app-level legs (twin byte-identity, b24_equivalence, b25_galactic)
are separate runs - see INTENT §11.119.
"""

import os
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]                      # the code tree
SRC = ROOT / "src"
PRE_REWORK_REV = os.environ.get("B31_PRE_REV", "f1151c63")

FAILS = []
NOTES = []


def fail(msg):
    FAILS.append(msg)
    print(f"FAIL: {msg}")


def ok(msg):
    print(f"ok  : {msg}")


def note(msg):
    NOTES.append(msg)
    print(f"note: {msg}")


# ---------------------------------------------------------------- corpus ---
# Every line here exists to be destroyed by a writer that does not preserve.
# Values are SYNTHETIC and say so (§11.51(d): no physical constant is claimed).
# Bytes are explicit: high bytes are ISO-8859 (the shipped corpus' encoding),
# one section is CRLF-terminated, and the file does NOT end with a newline.

CORPUS = (
    b"# B31 write-back corpus - SYNTHETIC values, no physical claim.\n"
    b"# This banner belongs to no section and must survive every rewrite.\n"
    b"\n"
    b"[alpha]\n"
    b"name = Alpha\n"
    b"radius  =  1234.5\n"
    b"  indented_key=squeezed\n"
    b"empty_value =\n"
    b"note = caf\xe9 cr\xe8me\n"
    b"orbit_LongOfPericenter 95.58754\n"
    b"unknown_key = something this engine has never heard of\n"
    b"duplicate = first\n"
    b"duplicate = second\n"
    b"orbit_visualization_period = 0.9424218 #### \xe0 affiner source stellarium\n"
    b"# a human comment attached to the datum below\n"
    b"annotated_key = 42\n"
    b"\n"
    b"[beta]  # a header with a trailing comment\n"
    b"= value with no key\n"
    b"mass = 1e24\n"
    b"\n"
    b"[delta]\n"
    b"# this section declares nothing at all\n"
    b"\n"
    b"[gamma:MODULE]\r\n"
    b"type = MESH\r\n"
    b"body = Alpha"                          # no final newline, deliberately
)

# The same shape, plus a machine-owned annotation nobody re-diagnoses. Kept
# apart from CORPUS because the two properties are different: a human file
# round-trips byte-identically, a file carrying OUR marker has its marked lines
# regenerated from what the loader said this time (D12: an annotation is
# forbidden where nothing acted, so a diagnosis that is no longer reached must
# go).
CORPUS_ANNOTATED = (
    b"[alpha]\n"
    b"# a human comment, never ours to touch\n"
    b"#!sc: stale_key [stale-reason] nobody diagnosed this on this load\n"
    b"stale_key = 7\n"
    b"mass = 1e24\n"
)


def lines_of(data):
    return data.split(b"\n")


# ------------------------------------------------------------- the tools ---

def build(outdir):
    """Compile the gate against the product sources, and the PRE-REWORK writer
    against the same tree at the commit before this slice."""
    gate = outdir / "format_gate"
    cmd = ["g++", "-std=c++23", "-Wall", "-Wextra", "-Wno-unused-parameter",
           f"-I{SRC}", str(HERE / "b31_format" / "format_gate.cpp"),
           str(SRC / "experimentalModule" / "ModularSystemFormat.cpp"),
           str(SRC / "tools" / "log.cpp"), "-lSDL2", "-o", str(gate)]
    r = subprocess.run(cmd, capture_output=True)
    if r.returncode:
        print(r.stderr.decode(errors="replace"))
        sys.exit("could not build the gate")

    pre_dir = outdir / "pre"
    pre_dir.mkdir(exist_ok=True)
    for name in ("ModularSystemFormat.hpp", "ModularSystemFormat.cpp"):
        r = subprocess.run(["git", "-C", str(ROOT), "show",
                            f"{PRE_REWORK_REV}:src/experimentalModule/{name}"],
                           capture_output=True)
        if r.returncode:
            print(r.stderr.decode(errors="replace"))
            sys.exit(f"could not extract {name} at {PRE_REWORK_REV}")
        (pre_dir / name).write_bytes(r.stdout)
    (pre_dir / "pre_gate.cpp").write_text(
        '#include "ModularSystemFormat.hpp"\n'
        "#include <vector>\n"
        "int main(int argc, char *argv[]) {\n"
        "    std::vector<ModularSystemFormat::Section> sections;\n"
        "    if (argc < 3 || !ModularSystemFormat::parse(argv[1], sections)) return 1;\n"
        "    return ModularSystemFormat::write(argv[2], sections, {}) ? 0 : 1;\n"
        "}\n")
    pre = outdir / "pre_gate"
    cmd = ["g++", "-std=c++23", f"-I{SRC}", f"-I{pre_dir}",
           str(pre_dir / "pre_gate.cpp"), str(pre_dir / "ModularSystemFormat.cpp"),
           str(SRC / "tools" / "log.cpp"), "-lSDL2", "-o", str(pre)]
    r = subprocess.run(cmd, capture_output=True)
    if r.returncode:
        print(r.stderr.decode(errors="replace"))
        sys.exit(f"could not build the pre-rework writer at {PRE_REWORK_REV}")
    return gate, pre


def run_gate(gate, src, dst, *ops, expect=0):
    """Parse `src`, apply `ops`, write `dst`. Returns (stdout, stderr, rc)."""
    r = subprocess.run([str(gate), str(src), str(dst), *ops], capture_output=True)
    if r.returncode != expect:
        fail(f"gate {ops} exited {r.returncode} (expected {expect})\n"
             f"      {r.stderr.decode(errors='replace').strip()}")
    return r.stdout, r.stderr, r.returncode


# ----------------------------------------------------------------- legs ----

def leg_roundtrip(gate, pre, work):
    src = work / "corpus.ini"
    src.write_bytes(CORPUS)
    out = work / "rt.ini"
    run_gate(gate, src, out)
    got = out.read_bytes()
    if got != CORPUS:
        fail("round-trip: the rewritten corpus is not byte-identical")
        for i, (a, b) in enumerate(zip(lines_of(CORPUS), lines_of(got))):
            if a != b:
                print(f"      line {i}: {a!r} -> {b!r}")
        print(f"      {len(CORPUS)} bytes in, {len(got)} bytes out")
    else:
        ok(f"round-trip: {len(lines_of(CORPUS))} lines, {len(CORPUS)} bytes, "
           "byte-identical (comments, blanks, spacing, key order, a malformed "
           "line, ISO-8859 bytes, CRLF, no final newline)")

    # The discrimination: the SAME corpus through the writer this slice replaced.
    pre_out = work / "rt_pre.ini"
    subprocess.run([str(pre), str(src), str(pre_out)], capture_output=True)
    lost = pre_out.read_bytes()
    if lost == CORPUS:
        fail("discrimination: the PRE-REWORK writer round-trips the corpus too - "
             "the corpus does not exercise what changed")
    else:
        kept = set(lines_of(lost))
        dropped = [l for l in lines_of(CORPUS) if l not in kept]
        note(f"discrimination: the pre-rework writer ({PRE_REWORK_REV}) returns "
             f"{len(lost)} bytes for {len(CORPUS)} in, losing {len(dropped)} of "
             f"{len(lines_of(CORPUS))} lines")


def leg_malformed_is_not_a_key(gate, work):
    src = work / "corpus.ini"
    dump, _, _ = run_gate(gate, src, "-", "dump")
    text = dump.decode()
    # keys are hex in the dump: a malformed line must not have become one.
    garbage = b"orbit_LongOfPericenter 95.58754".hex()
    noKey = b"= value with no key".hex()
    if garbage in text or noKey in text:
        fail("malformed: a line without a usable '=' became a key")
    elif b"orbit_LongOfPericenter 95.58754" not in CORPUS:
        fail("corpus: the malformed line is missing")
    else:
        ok("malformed: neither malformed line became a key, and both survive "
           "the rewrite verbatim (§5.39's shipped [Sedna] class)")


def leg_value_change(gate, work):
    src = work / "corpus.ini"
    out = work / "changed.ini"
    run_gate(gate, src, out,
             "set|alpha|radius|9999",
             "set|alpha|orbit_visualization_period|1.5")
    before, after = lines_of(CORPUS), lines_of(out.read_bytes())
    if len(before) != len(after):
        fail(f"value change: line count moved {len(before)} -> {len(after)}")
        return
    moved = [i for i, (a, b) in enumerate(zip(before, after)) if a != b]
    if moved != [5, 13]:
        fail(f"value change: lines {moved} moved, expected exactly [5, 13]")
        for i in moved:
            print(f"      {i}: {before[i]!r} -> {after[i]!r}")
        return
    if after[5] != b"radius  =  9999":
        fail(f"value change: spacing not preserved: {after[5]!r}")
    elif after[13] != b"orbit_visualization_period = 1.5 #### \xe0 affiner source stellarium":
        fail(f"value change: trailing comment not preserved: {after[13]!r}")
    else:
        ok("value change: exactly 2 lines moved, each only in its value - "
           "irregular spacing kept (`radius  =  9999`) and the shipped [mimas] "
           "trailing comment kept with its high bytes (§11.109(e))")


def leg_new_key(gate, work):
    src = work / "corpus.ini"
    out = work / "newkey.ini"
    run_gate(gate, src, out, "set|alpha|brand_new|7")
    before, after = lines_of(CORPUS), lines_of(out.read_bytes())
    if len(after) != len(before) + 1:
        fail(f"new key: {len(before)} lines in, {len(after)} out (expected +1)")
        return
    at = next((i for i, l in enumerate(after) if l == b"brand_new = 7"), None)
    if at is None:
        fail("new key: the appended line is not there")
        return
    if after[:at] + after[at + 1:] != before:
        fail("new key: something else moved as well")
        return
    # after the section's last declaration, before its trailing blank line
    if after[at - 1] != b"annotated_key = 42" or after[at + 1] != b"":
        fail(f"new key: landed at line {at}, between {after[at-1]!r} and {after[at+1]!r}")
    else:
        ok(f"new key: appended at line {at}, after [alpha]'s last declaration and "
           "before the blank line that ends the section; nothing else moved")


def leg_removal(gate, work):
    src = work / "corpus.ini"
    out = work / "removed.ini"
    run_gate(gate, src, out,
             "remove|alpha|unknown_key|this engine no longer reads it; the value is kept here")
    after = lines_of(out.read_bytes())
    kept = b"#unknown_key = something this engine has never heard of"
    if kept not in after:
        fail("removal: the retired key was DELETED, not commented out")
        return
    at = after.index(kept)
    if b"unknown_key was retired by spacecrafter" not in after[at - 1]:
        fail(f"removal: no reason above the retired key: {after[at-1]!r}")
        return
    dump, _, _ = run_gate(gate, out, "-", "dump")
    if b"unknown_key".hex() in dump.decode():
        fail("removal: the retired key still parses as a key")
        return
    rest = [l for i, l in enumerate(after) if i not in (at - 1, at)]
    expect = [l for l in lines_of(CORPUS)
              if l != b"unknown_key = something this engine has never heard of"]
    if rest != expect:
        fail("removal: something other than the retired key moved")
    else:
        ok("removal: the key survives as a comment carrying its own text, with "
           "its reason above it; it no longer parses as a key; nothing else moved")


def annotation_ops(section="alpha", key="annotated_key", reason="out-of-range"):
    text = ("'42' is outside 0..10 for %s.\\nValid: any value in 0..10. "
            "Applied instead: the default 5.\\nTo fix: set %s to a value in range, "
            "or remove the key." % (key, key))
    return f"annotate|{section}|{key}|{reason}|{text}"


def leg_annotation(gate, work):
    src = work / "corpus.ini"
    out = work / "annotated.ini"
    run_gate(gate, src, out, annotation_ops())
    after = lines_of(out.read_bytes())
    marks = [i for i, l in enumerate(after) if l.startswith(b"#!sc:")]
    if len(marks) != 3:
        fail(f"annotation: {len(marks)} marked lines, expected 3 (one per text line)")
        return
    if after[marks[-1] + 1] != b"annotated_key = 42":
        fail(f"annotation: does not sit directly above its datum "
             f"({after[marks[-1]+1]!r} follows it)")
        return
    if after[marks[0] - 1] != b"# a human comment attached to the datum below":
        fail("annotation: the human comment above the datum was displaced")
        return
    rest = [l for i, l in enumerate(after) if i not in marks]
    if rest != lines_of(CORPUS):
        fail("annotation: something other than the annotation changed")
        return
    ok("annotation: 3 marked lines directly ABOVE the datum, the human comment "
       "above them untouched, nothing else changed")

    # T9: the same diagnosis again must produce the same bytes.
    twice = work / "annotated2.ini"
    run_gate(gate, out, twice, annotation_ops())
    if twice.read_bytes() != out.read_bytes():
        a, b = lines_of(out.read_bytes()), lines_of(twice.read_bytes())
        fail(f"T9 idempotence: the second rewrite differs ({len(a)} -> {len(b)} lines)")
    else:
        ok("T9 idempotence: a second rewrite with the same diagnosis is "
           "byte-identical (the marked lines are replaced, not appended)")

    # ... and the naive way must fail that same assertion.
    naive1 = naive_annotate(CORPUS)
    naive2 = naive_annotate(naive1)
    if naive2 == naive1:
        fail("T9 discrimination: the naive annotator is idempotent too - the "
             "assertion cannot tell the implementations apart")
    else:
        grew = len(lines_of(naive2)) - len(lines_of(naive1))
        note(f"T9 discrimination: the naive annotator (insert above the datum, "
             f"never look for an earlier one) grows the file by {grew} lines per "
             f"rewrite and FAILS this leg - which is what makes passing it mean "
             f"something")


def naive_annotate(data):
    """The obvious wrong implementation, for the discrimination leg only: put
    the annotation above the datum and never look for the one already there."""
    out = []
    for line in lines_of(data):
        if line == b"annotated_key = 42":
            out.append(b"#!sc: annotated_key [out-of-range] '42' is outside 0..10 "
                       b"for annotated_key.")
            out.append(b"#!sc: annotated_key [out-of-range] Valid: any value in "
                       b"0..10. Applied instead: the default 5.")
            out.append(b"#!sc: annotated_key [out-of-range] To fix: set "
                       b"annotated_key to a value in range, or remove the key.")
        out.append(line)
    return b"\n".join(out)


def leg_stale_annotation(gate, work):
    src = work / "annotated_in.ini"
    src.write_bytes(CORPUS_ANNOTATED)
    out = work / "annotated_out.ini"
    run_gate(gate, src, out)
    after = out.read_bytes()
    if b"#!sc:" in after:
        fail("stale annotation: a diagnosis nobody reached this load survived "
             "(D12: an annotation is forbidden where nothing acted)")
    elif b"# a human comment, never ours to touch" not in after:
        fail("stale annotation: the human comment was dropped with it")
    elif b"stale_key = 7" not in after:
        fail("stale annotation: the datum itself was dropped")
    else:
        ok("stale annotation: the marked line is gone, the human comment and the "
           "datum are not (D12 - the marker is a machine-owned namespace)")


def leg_annotation_replaces(gate, work):
    src = work / "corpus.ini"
    out = work / "replaced.ini"
    run_gate(gate, src, out,
             "annotate|alpha|mass_missing|absent-key|first text",
             "annotate|alpha|annotated_key|out-of-range|FIRST",
             "annotate|alpha|annotated_key|out-of-range|SECOND")
    after = lines_of(out.read_bytes())
    marked = [l for l in after if l.startswith(b"#!sc:")]
    if len(marked) != 2:
        fail(f"annotation replace: {len(marked)} marked lines, expected 2")
        return
    if any(b"FIRST" in l for l in marked):
        fail("annotation replace: the superseded text is still there")
        return
    tail = next((i for i, l in enumerate(after)
                 if l.startswith(b"#!sc: mass_missing")), None)
    if tail is None or after[tail - 1] != b"annotated_key = 42":
        fail(f"annotation replace: a diagnosis about a key the section does not "
             f"carry did not land at the section's end (line {tail})")
    else:
        ok("annotation replace: the same (key, reason) said twice is ONE line "
           "carrying the later text; a diagnosis about an absent key lands at "
           "the end of its section")


def leg_every_op_idempotent(gate, work):
    """T9 generalized, and the reversible-pair rule: apply the WHOLE operation
    set - a value change, a new key, a removal, an annotation - then apply it
    again to what came out, starting from the state the first pass produced."""
    ops = ("set|alpha|radius|9999",
           "set|alpha|brand_new|7",
           "remove|alpha|unknown_key|this engine no longer reads it",
           annotation_ops())
    src = work / "corpus.ini"
    first, second = work / "allops1.ini", work / "allops2.ini"
    run_gate(gate, src, first, *ops)
    run_gate(gate, first, second, *ops)
    if first.read_bytes() != second.read_bytes():
        a, b = lines_of(first.read_bytes()), lines_of(second.read_bytes())
        moved = [i for i, (x, y) in enumerate(zip(a, b)) if x != y]
        fail(f"second pass: {len(a)} -> {len(b)} lines, first difference at "
             f"{moved[:1]}")
        for i in moved[:3]:
            print(f"      {i}: {a[i]!r} -> {b[i]!r}")
    else:
        ok(f"second pass: the whole operation set (value change + new key + "
           f"removal + annotation) applied again to its own output is "
           f"byte-identical - every operation is a fixed point, not just the "
           f"annotation ({len(first.read_bytes())} bytes)")


def leg_representability(gate, work):
    src = work / "corpus.ini"
    ref = work / "rt.ini"
    for value, why in ((b"bad # value", "a comment character"),
                       (b"two\\nlines", "a newline"),
                       (b" padded ", "outer blanks")):
        out = work / f"refused_{len(value)}.ini"
        _, err, _ = run_gate(gate, src, out, f"set|alpha|name|{value.decode()}", expect=2)
        if not out.exists() or out.read_bytes() != ref.read_bytes():
            fail(f"representability: a value with {why} was refused but the file "
                 f"still changed")
            return
        # §2(f): the refusal has to SAY what it refused and what would work.
        if b"name" not in err or b"To fix" not in err:
            fail(f"representability: the refusal of a value with {why} is silent "
                 f"or nameless: {err!r}")
            return
    ok("representability: a value carrying a comment character, a newline or "
       "outer blanks is refused, each with a §2(f) diagnostic naming the key and "
       "what fixes it, and the file is left exactly as it was (3 cases)")


def leg_crlf(gate, work):
    src = work / "corpus.ini"
    out = work / "crlf.ini"
    run_gate(gate, src, out, "set|gamma:MODULE|type|OJM")
    data = out.read_bytes()
    if b"type = OJM\r\n" not in data:
        fail(f"CRLF: an in-place value change dropped the line's own terminator: "
             f"{data[-60:]!r}")
        return
    before, after = lines_of(CORPUS), lines_of(data)
    moved = [i for i, (a, b) in enumerate(zip(before, after)) if a != b]
    if moved != [25]:
        fail(f"CRLF: lines {moved} moved, expected exactly [25]")
    else:
        ok("CRLF: a value rewritten in a CRLF-terminated section keeps its own "
           "'\\r' and nothing else moves")


def leg_last_line(gate, work):
    src = work / "corpus.ini"
    out = work / "lastline.ini"
    run_gate(gate, src, out, "set|gamma:MODULE|added|yes")
    data = out.read_bytes()
    if b"body = Alpha\nadded = yes" not in data:
        fail(f"last line: appending after an unterminated final line went wrong: "
             f"{data[-40:]!r}")
        return
    dump, _, _ = run_gate(gate, out, "-", "dump")
    if b"body".hex() not in dump.decode() or b"added".hex() not in dump.decode():
        fail("last line: the file no longer parses both keys")
    else:
        ok("last line: a file that ended without a newline gets one back when "
           "something is appended after its last line, and both keys parse")


def main():
    outdir = Path(sys.argv[1]) if len(sys.argv) > 1 else HERE / "artifacts" / "b31_writeback"
    outdir.mkdir(parents=True, exist_ok=True)
    work = outdir / "work"
    if work.exists():
        shutil.rmtree(work)
    work.mkdir()
    gate, pre = build(outdir)
    print(f"gate: {gate}\npre : {pre} (from {PRE_REWORK_REV})\n")

    leg_roundtrip(gate, pre, work)
    leg_malformed_is_not_a_key(gate, work)
    leg_value_change(gate, work)
    leg_new_key(gate, work)
    leg_removal(gate, work)
    leg_annotation(gate, work)
    leg_stale_annotation(gate, work)
    leg_annotation_replaces(gate, work)
    leg_every_op_idempotent(gate, work)
    leg_representability(gate, work)
    leg_crlf(gate, work)
    leg_last_line(gate, work)

    print()
    for n in NOTES:
        print(f"note: {n}")
    if FAILS:
        print(f"\n{len(FAILS)} FAILURE(S)")
        return 1
    print("\nALL LEGS GREEN")
    return 0


if __name__ == "__main__":
    sys.exit(main())
