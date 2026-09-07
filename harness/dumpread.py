"""ONE reader for the `body action dual_dump` channel (INTENT §5.103, F39).

This module now HOLDS the grammar that `b24_equivalence.sanitize_nonfinite`
used to hold (it re-exports it, so every one of its fifteen importers is
unaffected): the authority moved here rather than being copied, because F39
adds a second layer to the same problem and two half-authorities is exactly
what I2 forbids.

**Layer 1 - bare C spellings (unchanged, and its two corrections with it).**
The OLD path's body half and the control-surface block still print bare
`nan` / `-nan` / `inf`; Python's json accepts `NaN` and `Infinity` but not
those. Without the substitution the `except` in every loader swallowed the
WHOLE body, so a body carrying one non-finite float read as *absent* to every
gate - the silent-absence failure this exists to prevent.

**Layer 2 - quoted tokens (new, F39).** The new path's own emitters now write
`"nan"` / `"inf"` / `"-inf"` (`src/experimentalModule/JsonNum.hpp`, §5.103's
fix), which IS legal JSON and therefore parses - as a STRING. Every existing
consumer compares numbers, so the reader converts them back to floats: the app
became JSON-legal without any gate changing its meaning.

**A body missing from EITHER path.** `rec["new"] is None` was handled; a body
that exists only in the new path - every COMPOSED body, e.g. the B24 rover -
makes `rec["old"]` None, and that raised AttributeError mid-analysis in
`analyze.py`. The two are the same fact from two sides, reported as two lists.

**An EMPTY OLD HALF is now refused by default (F101, INTENT S5.143).** A dual
dump whose body records ALL carry `"old": null` gives `pairs == []`, and every
per-body comparison built on it is then vacuous: a gate that counts differences
counts zero and PASSES.  That state is reachable from the tester's own shipped
corpus - `fscripts/14.sts` leaves the loaded system and the old half follows
`currentSystem` (measured F101: the half is already empty at the show's line
23, before any `body action load`) - so it is not hypothetical and it is not
visible to any reader that only looks at `pairs`.  `load_dump` therefore raises
`EmptyOldHalf` unless the caller says `require_old=False`, which is what a
reader whose JOB is to REPORT the emptiness must say.  The threshold is ZERO
and deliberately not a small number: `pairs` is the set a comparison is defined
on, and "defined on nothing" is a property, where "defined on few" would be a
magic constant.  A one-body old half is a real state (F98's own post-`14.sts`
bisect dump reads 1) and passes.

**AND SINCE F105 (INTENT S11.226) THE MESSAGE NAMES THE SYSTEM.**  The dump's
header now carries `oldSystem` (`SolarSystem` / `galactic` / the `systems` map
key) and `inSystem`, written by the enumerator's own class
(`ssystem_factory.cpp`), so the reader answers "empty against what" out of the
FILE instead of sending its caller to the process's stdout.  A dump written
before that field exists carries no such key; the message says so rather than
printing `None`, because an absent field dates the file.
"""
import json
import re

# The lookbehind covers every position a VALUE can start at, not just `:`
# (2026-08-09, F33 §11.143): the first version matched only after a colon, so
# `"screen":[-nan,-nan]` - a body sitting exactly at the eye, reachable from
# `camera action transition_to target point` - still raised and the body was
# still dropped WHOLE. A string value cannot be hit: `:"Nanuq"` puts a quote
# between the anchor and the token.
_NONFINITE = re.compile(r'(?<=[:\[,])\s*(-?)(nan|inf)\b')

_QUOTED = {'nan': float('nan'), 'inf': float('inf'), '-inf': float('-inf')}


def sanitize_nonfinite(line):
    """C++ `nan`/`inf` spellings -> the JSON ones. ONE grammar (I2): every
    reader of this dump uses this function, never its own copy."""
    # The SIGN is dropped for nan: Python's json accepts `NaN` and `-Infinity`
    # but NOT `-NaN`, so carrying the sign through left the line unparseable and
    # the body still silently dropped. A signed nan is not a distinguishable
    # value anyway.
    return _NONFINITE.sub(
        lambda m: "NaN" if m.group(2) == "nan" else m.group(1) + "Infinity", line)


def unquote_nonfinite(o):
    """`"nan"` / `"inf"` / `"-inf"` -> float, recursively. The inverse of
    JsonNum.hpp's quoting, so a consumer never has to know which half of the
    dump line produced the value."""
    if isinstance(o, str):
        return _QUOTED.get(o, o)
    if isinstance(o, list):
        return [unquote_nonfinite(v) for v in o]
    if isinstance(o, dict):
        return {k: unquote_nonfinite(v) for k, v in o.items()}
    return o


def loads(line):
    return unquote_nonfinite(json.loads(sanitize_nonfinite(line.strip().rstrip(','))))


class EmptyOldHalf(Exception):
    """The dump holds body records and NOT ONE of them has an old-path half.

    Carries the path and both counts, because the first question anyone asks of
    this is "how empty, and empty against what" (INTENT S5.143 / S11.221)."""

    def __init__(self, path, bodies_old, bodies_new, records, header=None):
        self.path = str(path)
        self.bodies_old = bodies_old
        self.bodies_new = bodies_new
        self.records = records
        header = header or {}
        self.old_system = header.get("oldSystem")
        self.in_system = header.get("inSystem")
        # THE HEADER NOW SAYS WHICH SYSTEM (F105, INTENT S11.226): the reader
        # reports the file's OWN answer instead of sending its caller to the
        # process's stdout for it.  A dump written before that field existed -
        # every landed artifact, including the controls this module's self-test
        # reads - carries no such key, and saying so is part of the answer: an
        # absent field is not `None`, it dates the file.
        if self.old_system is None:
            where = ("This dump's header carries no `oldSystem` field, so it "
                     "predates F105 and cannot say which system its old column "
                     "was taken in; the process's own stdout transitions "
                     "(`->InSolarSystem`, `->InGalaxy`) are the only witness "
                     "for it. ")
        else:
            where = ("The header says the old column was taken in system "
                     "'%s' (inSystem = %s). " % (self.old_system,
                                                 self.in_system))
        super().__init__(
            "%s: the dual dump holds %d body records and NOT ONE of them has an "
            "old-path half (bodies_old = %d, bodies_new = %d). %sEvery per-body "
            "comparison this file supports is vacuous - `pairs` is empty, so a "
            "gate that counts differences counts 0 and PASSES. This is the "
            "INTENT S5.143 state: a scene that has left the loaded system takes "
            "the old half with it, because the dump enumerates `currentSystem` "
            "(ssystem_factory.cpp:1181) and `leaveSystem` points that at a "
            "system holding no bodies. To fix: take the dump from a scene that "
            "is INSIDE the loaded system - the executor prints `->InSolarSystem` "
            "on its own stdout when it enters one - or, if MEASURING the "
            "emptiness is the point, call load_dump(path, require_old=False)."
            % (self.path, records, bodies_old, bodies_new, where))


def load_dump(path, *, require_old=True):
    """-> (header, pairs, missing_new, missing_old). `pairs` are the records
    present on BOTH sides - the only ones a comparison is defined on.

    `require_old` is KEYWORD-ONLY so that every existing
    `header, pairs, mn, mo = load_dump(p)` call site keeps its exact meaning and
    its exact arity (I1: the widening is invisible to the contract's existing
    users).  With it True - the default - a dump whose body records ALL lack an
    old half raises `EmptyOldHalf` instead of returning an empty `pairs` that
    every downstream gate reads as agreement.  A file with NO body records at
    all does not raise: that is a different fault (an empty or truncated dump)
    and this reader has no business naming it."""
    header, pairs, missing_new, missing_old = None, [], [], []
    for line in open(path, encoding='utf-8', errors='replace'):
        if not line.strip():
            continue
        try:
            rec = loads(line)
        except json.JSONDecodeError:
            continue
        t = rec.get("type")
        if t == "header":
            header = rec
        elif t != "body":
            continue
        elif rec.get("new") is None:
            missing_new.append(rec["name"])
        elif rec.get("old") is None:
            missing_old.append(rec["name"])
        else:
            pairs.append(rec)
    bodies_old = len(pairs) + len(missing_new)
    records = bodies_old + len(missing_old)
    if require_old and records and bodies_old == 0:
        raise EmptyOldHalf(path, bodies_old, len(pairs) + len(missing_old),
                           records, header)
    return header, pairs, missing_new, missing_old


# ---------------------------------------------------------------- self-test
#
# The proof of THIS contract lives WITH the contract and not in a driver.  Every
# other harness here imports this module; if the guard's both-ways evidence sat
# in one campaign's driver, a reader of the module would have to know which
# campaign that was, and a future change to the guard could be made without ever
# running it.  `python3 dumpread.py selftest` needs nothing but this file and the
# committed control dumps beside it.

_SELFTEST_SYNTH = [
    ('{"type":"header","jd":2460000.5}', None),
    ('{"type":"body","name":"A","old":null,"new":{"ecl":[1,2,3]}}', None),
    ('{"type":"body","name":"B","old":null,"new":{"ecl":[4,5,6]}}', None),
]


def _selftest():
    import gzip
    import os
    import tempfile
    from pathlib import Path

    here = Path(__file__).resolve().parent
    ok, fail = [], []

    def check(name, cond, detail=""):
        (ok if cond else fail).append(name)
        print("%s  %-58s %s" % ("PASS" if cond else "FAIL", name, detail))

    def green(path):
        """A case that must NOT raise -> the four values, or a FAIL rather than
        a traceback.  A must-not-raise check that dies with a stack trace tells
        a mutation run nothing about the OTHER cases, and the mutation run is
        how this suite is shown able to fail."""
        try:
            return load_dump(path), None
        except EmptyOldHalf as e:                                 # noqa: BLE001
            return (None, [], [], []), e

    with tempfile.TemporaryDirectory() as td:
        td = Path(td)

        # (1) THE SYNTHETIC CASE, both ways.
        synth = td / "empty_old.json"
        synth.write_text("\n".join(l for l, _ in _SELFTEST_SYNTH) + "\n")
        try:
            load_dump(synth)
            check("synthetic empty old half RAISES by default", False,
                  "it returned instead")
        except EmptyOldHalf as e:
            check("synthetic empty old half RAISES by default",
                  e.bodies_old == 0 and e.bodies_new == 2 and e.records == 2,
                  "bodies_old=%d bodies_new=%d" % (e.bodies_old, e.bodies_new))
            check("the message names the fix (require_old=False)",
                  "require_old=False" in str(e))
            check("the message names the path", str(synth) in str(e))
        h, pairs, mn, mo = load_dump(synth, require_old=False)
        check("the OPT-OUT returns the same four values",
              h is not None and pairs == [] and mn == [] and len(mo) == 2,
              "pairs=%d missing_old=%d" % (len(pairs), len(mo)))

        # (2) THE CONTROL THAT MAKES (1) MEAN SOMETHING: a dump with a NON-empty
        #     old half must not raise, or the guard is a constant.
        good = td / "good.json"
        good.write_text(
            '{"type":"header","jd":2460000.5}\n'
            '{"type":"body","name":"A","old":{"ecl":[1,2,3]},'
            '"new":{"ecl":[1,2,3]}}\n'
            '{"type":"body","name":"B","old":null,"new":{"ecl":[4,5,6]}}\n')
        (h, pairs, mn, mo), err = green(good)
        check("a dump with ONE paired body does not raise",
              err is None and len(pairs) == 1 and len(mo) == 1,
              "raised: %s" % type(err).__name__ if err else "")

        # (3) A dump with NO body records is NOT this fault and must not be
        #     reported as it.
        empty = td / "header_only.json"
        empty.write_text('{"type":"header","jd":2460000.5}\n')
        (h, pairs, mn, mo), err = green(empty)
        check("a header-only dump does not raise",
              err is None and pairs == [] and mo == [],
              "raised: %s" % type(err).__name__ if err else "")

        # (4) OLD-ONLY records keep the old half non-empty (the S5.137 shape:
        #     a body on the old path and not on the new one).
        oldonly = td / "old_only.json"
        oldonly.write_text(
            '{"type":"header","jd":2460000.5}\n'
            '{"type":"body","name":"A","old":{"ecl":[1,2,3]},"new":null}\n')
        (h, pairs, mn, mo), err = green(oldonly)
        check("an OLD-ONLY body keeps the half non-empty",
              err is None and pairs == [] and mn == ["A"],
              "raised: %s" % type(err).__name__ if err else "")

        # (5) THE REAL-DATA CONTROLS, committed beside this file.  Two of them
        #     are F98's own dumps - the ones S5.143 was minted from - and one is
        #     F101's, taken on a fresh launch at the show's line 23.
        controls = [
            ("f98_bisect_pre14", "GREEN", 246, 276),
            ("f98_bisect_post14", "GREEN", 1, 277),
            ("f98_soak_c002_06old", "RED", 0, 286),
            ("f101_p23_post", "RED", 0, 120),
            ("f101_p25_post", "GREEN", 1, 121),
        ]
        for stem, want, n_old, n_new in controls:
            src = here / "artifacts" / "f101" / "controls" / (stem + ".json.gz")
            if not src.is_file():
                check("real-data control %s present" % stem, False, str(src))
                continue
            plain = td / (stem + ".json")
            with gzip.open(src, "rb") as fh:
                plain.write_bytes(fh.read())
            try:
                h, pairs, mn, mo = load_dump(plain)
                got, detail = "GREEN", "old=%d new=%d" % (
                    len(pairs) + len(mn), len(pairs) + len(mo))
            except EmptyOldHalf as e:
                got, detail = "RED", "old=%d new=%d" % (e.bodies_old,
                                                        e.bodies_new)
            check("real data %-22s is %s" % (stem, want), got == want, detail)
            h2, p2, mn2, mo2 = load_dump(plain, require_old=False)
            check("real data %-22s opt-out returns %d/%d"
                  % (stem, n_old, n_new),
                  len(p2) + len(mn2) == n_old and len(p2) + len(mo2) == n_new,
                  "old=%d new=%d" % (len(p2) + len(mn2), len(p2) + len(mo2)))
            os.unlink(plain)

    print("\n%d PASS, %d FAIL" % (len(ok), len(fail)))
    return 1 if fail else 0


if __name__ == "__main__":
    import sys
    if len(sys.argv) == 2 and sys.argv[1] == "selftest":
        sys.exit(_selftest())
    print(__doc__)
    print("usage: python3 dumpread.py selftest")
