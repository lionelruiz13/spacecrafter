#!/usr/bin/env python3
"""B14-periode apply (INTENT §11.86(d)/§11.87(e)). Section-scoped rewrite of the
garbage rot_periode for the 20 pole-landed moons.

rot_periode is the sidereal rotation period in HOURS; the engine loads it as
re.period = rot_periode/24 (days, float32) and spins axisRotation =
(jd-epoch)/re.period*2pi + offset (ModularBody.hpp:350; negative period =>
retrograde, the Venus -5832 precedent). The IAU value is derived from the
pck00011 BODY_PM second coefficient Wdot (deg/day, 3-source verified pck00011 ==
pck00010 == Archinal report text):

    rot_periode_hours = 360/Wdot * 24 = 8640/Wdot   (sign of Wdot preserved)

Iapetus is left BIT-IDENTICAL (current 1903.940390 == 8640/4.5379572 to 2.6e-7 h,
already correct - §11.86(d)).

Edits are section-scoped (each moon's single rot_periode line inside its own
[section]); encoding preserved (shipped ASCII / loaded ISO-8859-1); byte-exact
outside the edited value token.

Usage: b14_periode_apply.py <file> <ascii|iso-8859-1>   (prints per-moon before->after)
"""
import re, sys

# moon -> derived rot_periode string (repr of 8640/Wdot; Wdot from pck00011 BODY_PM[1])
NEW = {
 "amalthea":  "11.956302107059122",   # Wdot=+722.6314560
 "thebe":     "16.188857715136475",   # Wdot=+533.7004100
 "telesto":   "45.307255590119844",   # Wdot=+190.6979332
 "pandora":   "15.084085922724439",   # Wdot=+572.7891000
 "janus":     "16.671941367894302",   # Wdot=+518.2359876
 "helene":    "65.64481316595713",    # Wdot=+131.6174056
 "epimetheus":"16.66375038498543",    # Wdot=+518.4907239
 "prometheus":"14.711666658152971",   # Wdot=+587.2890000
 "juliet":    "-11.833584206688144",  # Wdot=-730.1253660  RETROGRADE
 "portia":    "-12.316700219387089",  # Wdot=-701.4865870  RETROGRADE
 "rosalind":  "-13.403013989740234",  # Wdot=-644.6311260  RETROGRADE
 "belinda":   "-14.964593745218616",  # Wdot=-577.3628170  RETROGRADE
 "puck":      "-18.28397028517083",   # Wdot=-472.5450690  RETROGRADE
 "naiad":     "7.065495799776225",    # Wdot=+1222.8441209
 "thalassa":  "7.475629181510703",    # Wdot=+1155.7555612
 "despina":   "8.031724148762322",    # Wdot=+1075.7341562
 "galatea":   "10.28988207259928",    # Wdot=+839.6597686
 "larissa":   "13.311692650174",      # Wdot=+649.0534470
 "proteus":   "26.935571560613983",   # Wdot=+320.7654228
 # iapetus: NOT written (already correct, bit-identical)
}

def main(path, enc):
    text = open(path, encoding=enc).read()
    lines = text.split("\n")
    # section map
    secname = None
    span = {}   # name -> (start_idx, end_idx exclusive)
    starts = []
    for i, l in enumerate(lines):
        m = re.match(r'^\[([^\]]+)\]', l)
        if m:
            starts.append((i, m.group(1).strip().lower()))
    for k, (i, nm) in enumerate(starts):
        end = starts[k+1][0] if k+1 < len(starts) else len(lines)
        span.setdefault(nm, (i, end))
    changed = 0
    for mn, newval in NEW.items():
        if mn not in span:
            print(f"  {mn}: SECTION NOT FOUND", file=sys.stderr); sys.exit(2)
        s, e = span[mn]
        hits = [j for j in range(s, e) if re.search(r'rot_periode\s*=', lines[j])]
        if len(hits) != 1:
            print(f"  {mn}: expected 1 rot_periode, found {len(hits)}", file=sys.stderr); sys.exit(2)
        j = hits[0]
        old = lines[j]
        new = re.sub(r'(rot_periode\s*=\s*)(\S+)', lambda m: m.group(1)+newval, old, count=1)
        if new != old:
            lines[j] = new; changed += 1
            print(f"  {mn:<11} {old.strip()!r:40} -> {new.strip()!r}")
    out = "\n".join(lines)
    open(path, "w", encoding=enc).write(out)
    print(f"  [{changed} lines changed]")

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
