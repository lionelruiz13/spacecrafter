#!/usr/bin/env python3
"""The intent-modified line, recomposed from the contract - the gates' own reader.

INTENT 11.193(a) [vixy 2026-09-01]: one rendering of a script error exists - the
line as it reads where it came from, the author's own comment kept, with the
machine's ` #! <message>` tail appended and any tail already there REPLACED -
and every sink shows IT: the file when the annotation lands, the log always,
prefixed with the origin.

This module is what the gates EXPECT that line to be. Three properties make it
worth its own file:

  1. It is a RE-IMPLEMENTATION from 11.184's stated contract (the tail begins at
     the first `#!` at or after the first `#` outside a "..." run - the parser's
     own quote toggle), not a call into the engine and not a transcription of
     its C++. A gate that asked the engine what it should have printed would
     measure nothing; two independent renderings that agree measure something.
  2. It lives in ONE place for every instrument that needs it. f68_provenance
     and f69_feedback both assert against this line, and a second copy of the
     composition inside a second gate would be exactly the defect the shape it
     checks exists to remove (I2 - and this task's own subject, F73).
  3. It is deliberately small enough to read in full. If it ever disagrees with
     the engine the legs go red, and that red is the instrument working.

Not a runnable gate: imported by f68_provenance.py and f69_feedback.py.
"""

PREFIX = "Error executing "


def annotation_begin(line):
    """Byte offset where the machine tail of a raw line starts: the first `#!`
    at or after the first `#` OUTSIDE a "..." run. None when there is none -
    including the case where the first `#` outside quotes opens an author's
    own comment that no `#!` follows."""
    in_quote = False
    for i, ch in enumerate(line):
        if ch == '"':
            in_quote = not in_quote
        elif ch == "#" and not in_quote:
            j = line.find("#!", i)
            return None if j < 0 else j
    return None


def without_annotation(line):
    """The line with its machine tail (and the blanks before it) removed."""
    at = annotation_begin(line)
    return line if at is None else line[:at].rstrip(" \t")


def intent_modified(raw, message):
    """The raw line with `message` as its machine tail, replacing any present."""
    return without_annotation(raw) + " #! " + message


def rendered(origin, raw, message):
    """What the log must show for a refusal of `raw` from an origin that has a
    name: the intent-modified line behind the origin prefix."""
    return PREFIX + origin + ": " + intent_modified(raw, message)


def script_line(raw_bytes, n):
    """Line `n` (1-based) of a script, read from the file's OWN bytes - never
    from what the instrument believes it wrote there."""
    return raw_bytes.decode("latin-1").split("\n")[n - 1]


def error_lines(log):
    """Every line the funnel or its bypassing sibling RENDERED, in order. The
    prefix is the whole filter: `Error executing ` is written by those two
    emitters and by nothing else in the tree."""
    return [l for l in (x.split("(Debug): ", 1)[-1] for x in log.splitlines())
            if l.startswith(PREFIX)]
