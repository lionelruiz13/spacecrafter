#!/usr/bin/env python3
"""f80_census.py -- the ssystem.ini key census, taken FROM THE LOADER.

WHAT-FOR (F80 mandate (1), scedit INTENT S5 item 4, base-D residual at
capability-surface.md S2): enumerate every key the engine READS out of a
stellar-system-file section, per reading site, with a file:line anchor, so the
second contract file (`util/scedit/grammar/ss-grammar.json`) is derived from the
code's own enumeration and never from the data file or from recall (C2, I2).

METHOD, and why it is a census and not a grep. The mechanical pass below finds
CANDIDATE sites: every string literal handed to one of the parameter maps that
carry a section's key/value pairs. That is an upper bound -- a literal may be a
value, a log word or a key of a different map. Each candidate is then
DISPOSITIONED by hand against the site, and the disposition table is what the
contract is built from. A candidate with no disposition is an ERROR, never a
default (the f70/f76 pattern).

THE TWO REGIMES the census must separate (F80 mandate (1)):
  legacy   -- ProtoSystem::load -> addBody (protosystem.cpp), the frozen
              comparison baseline (S11.52(b)), plus ssystem_factory.cpp and
              solarsystem.cpp;
  composed -- ModularSystem::loadBody and its module loaders
              (experimentalModule/), the B24 path.
A key read by only ONE regime is a finding, and that is the column that pays
the base-D residual ("old-path-only keys (bodyModule/) not swept").

Usage:
    python3 f80_census.py sites          # candidate sites, TSV on stdout
    python3 f80_census.py keys           # distinct keys x regime
    python3 f80_census.py datakeys       # keys PRESENT in a corpus file
"""
import re
import sys
import os
import glob
from collections import OrderedDict

SRC = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "src")
SRC = os.path.normpath(SRC)

# The parameter maps that carry one section's key/value pairs. Named
# explicitly rather than matched by shape: `params` in an unrelated function is
# not this census's subject, and the disposition pass is what catches one.
MAPS = ("param", "params", "bodyParams", "nodeParams", "orbitParams")

# file -> regime. solarsystem.cpp is the old path's second reader (its
# `load(stringHash_t)` is the same addBody shape); ModuleLoaderMgr.cpp is the
# family vocabulary, not a key reader, and is carried for completeness.
FILES = OrderedDict([
    ("bodyModule/protosystem.cpp", "legacy"),
    ("bodyModule/solarsystem.cpp", "legacy"),
    ("bodyModule/ssystem_factory.cpp", "legacy"),
    ("bodyModule/orbit_creator_cor.cpp", "legacy"),
    ("bodyModule/orbit.cpp", "legacy"),
    ("experimentalModule/ModularSystem.cpp", "composed"),
    ("experimentalModule/ModuleLoaderMgr.cpp", "composed"),
    ("experimentalModule/ModularBody.cpp", "composed"),
    ("experimentalModule/modules.cpp", "composed"),
])

# The composed regime does not read a body section in one place: loadBody reads
# the node's own keys and then every MODULE loader reads the same map for the
# keys its module needs. base D of the capability audit named exactly these
# directories (`moduleLoader/ orbitModules/ ModularBody.cpp ModularSystem.cpp`,
# capability-surface.md S2), and leaving them out is what made 811 keys look
# legacy-only in the first composed run -- the corpus caught the instrument.
for _d in ("moduleLoader", "orbitModules", "meshModules", "bodyModules",
           "environmentModules"):
    _dir = os.path.join(SRC, "experimentalModule", _d)
    if not os.path.isdir(_dir):
        continue
    for _f in sorted(os.listdir(_dir)):
        if _f.endswith((".cpp", ".hpp")):
            FILES["experimentalModule/%s/%s" % (_d, _f)] = "composed"

# Every access shape observed in the tree, each anchored to a real site:
#   param["k"]              the operator[] read (THE trap, S11.103(b))
#   param.count("k")        presence test
#   param.find("k")         presence test with an iterator
#   authored(param, "k")    ModularSystem's own "did the author write it" helper
ACCESS = re.compile(
    r'\b(?P<map>' + "|".join(MAPS) + r')\s*'
    r'(?:\[\s*"(?P<k1>[A-Za-z_0-9]+)"\s*\]'
    r'|\.\s*(?P<meth>count|find|at)\s*\(\s*"(?P<k2>[A-Za-z_0-9]+)"\s*\))'
)
AUTHORED = re.compile(
    r'\bauthored\s*\(\s*(?:' + "|".join(MAPS) + r')\s*,\s*"(?P<k>[A-Za-z_0-9]+)"')


# READ vs WRITE is the distinction the whole census turns on. `m["k"] = v` is
# the CODE authoring a value (a synthesized body, a default forced in); only a
# READ is evidence that an authored key in a data file reaches anything. Getting
# this wrong in the permissive direction is what makes a key look alive: the two
# blocks that synthesize a body from a star-catalogue Object write `tex_halo` and
# `lighting`, and those are the ONLY occurrences of either string in src/ --
# counted as reads, they hide 180 dead lines of the field file.
# The test is deliberately syntactic and conservative: a single '=' immediately
# after the closing bracket, not '==' and not '!='.
WRITE_TAIL = re.compile(r'\]\s*=(?!=)')


def scan_file(rel):
    """Yield (line_no, key, form, kind) for every candidate access in one file."""
    path = os.path.join(SRC, rel)
    with open(path, "r", encoding="utf-8", errors="surrogateescape") as fh:
        for n, line in enumerate(fh, 1):
            for m in ACCESS.finditer(line):
                key = m.group("k1") or m.group("k2")
                form = "[]" if m.group("k1") else m.group("meth")
                kind = "read"
                if form == "[]" and WRITE_TAIL.match(line, m.end() - 1):
                    kind = "write"
                yield n, key, form, kind
            for m in AUTHORED.finditer(line):
                yield n, m.group("k"), "authored", "read"


def sites():
    rows = []
    for rel, regime in FILES.items():
        if not os.path.exists(os.path.join(SRC, rel)):
            continue
        for n, key, form, kind in scan_file(rel):
            rows.append((rel, n, key, form, kind, regime))
    return rows


def read_corpus_keys(path):
    """Keys PRESENT in a legacy corpus file, with their section and line.

    ISO-8859 at the boundary (F80 mandate (4)): the file is read as BYTES and
    decoded latin-1, which is total -- every byte sequence decodes -- so a
    high-byte value can never abort the census. Positions stay BYTE positions.
    """
    out = []
    section = None
    with open(path, "rb") as fh:
        for n, raw in enumerate(fh.read().split(b"\n"), 1):
            line = raw.decode("latin-1")
            s = line.strip()
            if not s or s.startswith("#"):
                continue
            if s.startswith("["):
                section = s.strip("[]")
                continue
            # The legacy split is at the FIRST '=' (tools/ini_line.hpp).
            pos = line.find("=")
            if pos < 0:
                out.append((n, section, None, line))
                continue
            out.append((n, section, line[:pos].strip(), line[pos + 1:].strip()))
    return out


def main():
    what = sys.argv[1] if len(sys.argv) > 1 else "sites"
    if what == "sites":
        print("file\tline\tkey\tform\tkind\tregime")
        for r in sites():
            print("%s\t%d\t%s\t%s\t%s\t%s" % r)
    elif what == "keys":
        bykey = {}
        for rel, n, key, form, kind, regime in sites():
            if kind != "read":
                continue
            bykey.setdefault(key, set()).add(regime)
        print("key\tregimes")
        for k in sorted(bykey):
            print("%s\t%s" % (k, ",".join(sorted(bykey[k]))))
    elif what == "datakeys":
        path = sys.argv[2]
        seen = {}
        for n, sec, key, val in read_corpus_keys(path):
            if key is None:
                print("MALFORMED\t%d\t%s\t%s" % (n, sec, val))
                continue
            seen.setdefault(key, 0)
            seen[key] += 1
        for k in sorted(seen):
            print("%s\t%d" % (k, seen[k]))
    else:
        print(__doc__)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
