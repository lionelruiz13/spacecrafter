#!/usr/bin/env python3
"""B14-W0 data edit (INTENT §11.79(a) / D4): write the fetched IAU prime-meridian
W0 for the 14 landed moons as the NEW absolute-frame key rot_pole_w0, replacing
their garbage rot_rotation_offset = 14.9 (copy-paste value; I2 - a silently-
ignored stale datum is a desync trap once rot_pole_w0 is present and converted).

W0 values are the pck00011.tpc BODY<id>_PM constants (fetched, md5 3c0bdc01;
cross-verified pck00010 + Archinal report text, §11.69(a); each == the value
recorded at §11.69(e)). NEVER from recall (§11.51(d) red line).

The loader (resolveRotationFrame, ModularSystem.cpp) converts rot_pole_w0 from
the IAU ICRF-equator-node referential to the file's ecliptic-node referential.

Byte-exact edit: reads/writes latin-1 to preserve ISO-8859 (loaded file); only
the one rot_rotation_offset line per target section changes. Idempotent-guarded.

Usage: b14_w0_apply.py <ssystem.ini path>   (edits in place)
"""
import sys, re

# pck00011.tpc BODY_PM W0 constants (deg). Keys are the lowercase section names.
W0 = {
    "iapetus":"355.2",  "amalthea":"231.67", "thebe":"8.56",   "juliet":"302.56",
    "portia":"25.03",   "rosalind":"314.90", "belinda":"297.46","puck":"91.24",
    "naiad":"254.06",   "thalassa":"102.06", "despina":"306.51","galatea":"258.09",
    "larissa":"179.41", "proteus":"93.38",
}

def apply(path):
    with open(path, "r", encoding="latin-1") as f:
        text = f.read()
    lines = text.split("\n")
    out = []
    cur = None            # current section (lowercase)
    done = set()
    changed = 0
    for ln in lines:
        m = re.match(r"\s*\[([^\]]+)\]\s*$", ln)
        if m:
            cur = m.group(1).strip().lower()
        if cur in W0 and cur not in done and re.match(r"\s*rot_rotation_offset\s*=", ln):
            indent = ln[:len(ln)-len(ln.lstrip())]
            out.append(f"{indent}rot_pole_w0 = {W0[cur]}")
            done.add(cur); changed += 1
            continue
        if cur in W0 and re.match(r"\s*rot_pole_w0\s*=", ln):
            print(f"ERROR: {cur} already has rot_pole_w0 - refusing (not idempotent-safe)")
            sys.exit(2)
        out.append(ln)
    missing = set(W0) - done
    if missing:
        print(f"ERROR: sections not found / no rot_rotation_offset line: {sorted(missing)}")
        sys.exit(2)
    with open(path, "w", encoding="latin-1", newline="") as f:
        f.write("\n".join(out))
    print(f"OK: {changed} moons written rot_pole_w0 in {path}")

if __name__ == "__main__":
    apply(sys.argv[1])
