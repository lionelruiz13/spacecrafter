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


def load_dump(path):
    """-> (header, pairs, missing_new, missing_old). `pairs` are the records
    present on BOTH sides - the only ones a comparison is defined on."""
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
    return header, pairs, missing_new, missing_old
