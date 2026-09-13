#!/usr/bin/env python3
"""f117_census.py - the DSO layer's FIELD census, by command (F117, B5 / T1.3).

Why this exists: the F117 mint's premise line counted the field corpus with
`/usr/bin/grep -rl <token> ~/.spacecrafter/scripts | wc -l`, which counts BINARY
assets (png/mp4/avi) whose bytes happen to contain the token - `ojm` matched 96
files, nearly all of them images. A design note that says "the field uses X"
needs the count of SCRIPT LINES that issue a COMMAND, partitioned by which
engine object the command reaches. That partition is the grammar's, so it is
computed here rather than eyeballed.

The partition (from the command layer at code master-beta @ 87d429bd):
  dso3d  <action> ...                    -> DsoNavigator  (volumetric DSO)
         [app_command_interface.cpp:1676 commandDso3D -> coreLink->dsoNav*]
  dso2D  <action> ...                    -> Dso3d         (the point cloud)
         [app_command_interface.cpp:1695 commandDso2D -> Core::loadDso2d
          -> dso3d->loadCommand, core.cpp:2635]
  dso    <action|hidden> ...             -> NebulaMgr     (the 2-D nebula layer)
  body   ... mode in_galaxy|in_universe|in_sandbox ...  -> OjmMgr
         [app_command_interface.cpp:4118-4135 -> coreLink->BodyOJM*]
  body   ... <anything with .ojm and no mode=>            -> mesh module / other
  flag   oort|tully|tully_color_mode|nebula|nebula_names|star_names ...
                                         -> the show/label channels

Usage:
  python3 f117_census.py [--root DIR] [--files] [--lines]
Default root: ~/.spacecrafter/scripts
Exit code 0 always (a census, not a gate).
"""

import argparse
import os
import sys

OJM_MODES = ("in_galaxy", "in_universe", "in_sandbox")
FLAGS = ("oort", "tully", "tully_color_mode", "nebula", "nebula_names",
         "star_names", "milky_way")


def is_binary(path):
    """A file holding a NUL byte in its first 8 KiB is not a script.

    This is the discriminator the mint's grep lacked: `grep -rl` reports a
    binary file as a match and `wc -l` counts it like any other.
    """
    try:
        with open(path, "rb") as f:
            head = f.read(8192)
    except OSError:
        return True
    return b"\x00" in head


def decode(path):
    """Field scripts are ASCII or ISO-8859-1 (accented French in comments).

    Never UTF-8-strict: a strict decode raises on the shipped corpus and a
    whole-file latin-1 decode of a UTF-8 file mojibakes it - but for TOKEN
    matching on ASCII command words both decodings agree byte for byte, so
    latin-1 (total, never raises) is the safe reader here.
    """
    with open(path, "rb") as f:
        return f.read().decode("iso-8859-1")


def classify(line):
    """Return a list of (category, detail) for one script line."""
    out = []
    s = line.strip()
    if not s or s.startswith("#"):
        return out
    words = s.split()
    cmd = words[0].lower()
    args = {}
    # command grammar: `name key value key value ...`
    i = 1
    while i + 1 < len(words) + 1 and i < len(words):
        if i + 1 < len(words):
            args[words[i].lower()] = words[i + 1]
        i += 2
    if cmd == "dso3d":
        out.append(("dsoNavigator", "dso3d action " + args.get("action", "?")))
    elif cmd == "dso2d":
        out.append(("Dso3d", "dso2D action " + args.get("action", "?")))
    elif cmd == "dso":
        out.append(("NebulaMgr", "dso " + (words[1].lower() if len(words) > 1 else "?")))
    elif cmd == "body":
        mode = args.get("mode", "")
        if mode in OJM_MODES:
            out.append(("OjmMgr", "body mode " + mode + " action "
                        + args.get("action", "?")))
        elif ".ojm" in s.lower():
            out.append(("mesh/other .ojm", s[:60]))
    elif cmd == "flag":
        if len(words) > 1 and words[1].lower() in FLAGS:
            out.append(("flag:" + words[1].lower(), s[:60]))
    # the raw token appearance, independent of the grammar (the mint's measure)
    low = s.lower()
    for tok in ("tully", "oort", "dso3d", "ojm"):
        if tok in low:
            out.append(("token:" + tok, s[:60]))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=os.path.expanduser("~/.spacecrafter/scripts"))
    ap.add_argument("--files", action="store_true", help="list files per category")
    ap.add_argument("--lines", action="store_true", help="list matching lines")
    a = ap.parse_args()

    n_files = n_text = n_binary = 0
    cat_files = {}
    cat_lines = {}
    detail_count = {}
    for dirpath, _dirs, files in os.walk(a.root):
        for name in sorted(files):
            p = os.path.join(dirpath, name)
            if not os.path.isfile(p) or os.path.islink(p):
                continue
            n_files += 1
            if is_binary(p):
                n_binary += 1
                continue
            n_text += 1
            rel = os.path.relpath(p, a.root)
            try:
                text = decode(p)
            except OSError:
                continue
            for lineno, line in enumerate(text.split("\n"), 1):
                for cat, detail in classify(line):
                    cat_files.setdefault(cat, set()).add(rel)
                    cat_lines.setdefault(cat, []).append((rel, lineno, line.strip()))
                    key = (cat, detail if not cat.startswith("token:") else "")
                    detail_count[key] = detail_count.get(key, 0) + 1

    print("== F117 field census: root %s ==" % a.root)
    print("files walked %d | text %d | binary (skipped) %d" % (n_files, n_text, n_binary))
    print("")
    print("%-24s %6s %6s" % ("CATEGORY", "FILES", "LINES"))
    for cat in sorted(cat_files):
        print("%-24s %6d %6d" % (cat, len(cat_files[cat]), len(cat_lines[cat])))
    print("")
    print("== command detail (non-token categories) ==")
    for (cat, detail), n in sorted(detail_count.items()):
        if detail:
            print("%-24s %-44s %4d" % (cat, detail, n))
    if a.files:
        print("")
        print("== files per category ==")
        for cat in sorted(cat_files):
            print("-- %s" % cat)
            for f in sorted(cat_files[cat]):
                print("   %s" % f)
    if a.lines:
        print("")
        print("== lines per category ==")
        for cat in sorted(cat_lines):
            print("-- %s" % cat)
            for rel, lineno, line in cat_lines[cat]:
                print("   %s:%d: %s" % (rel, lineno, line[:110]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
