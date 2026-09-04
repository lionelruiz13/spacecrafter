#!/usr/bin/env python3
"""f80_corpus.py -- C3's stellar-system half: every finding dispositioned.

THE RULE OF EVIDENCE (F76's shape, f70_dispositions.py's before it): the census
below is MECHANICAL -- it runs the checker and takes what it prints -- and it is
joined to a table of HAND TRACES, one per distinct defect. A finding that
matches no trace is an ERROR, not a default: `--strict` exits 1 on it. That is
what makes "every finding is TRUE" a claim that could have failed.

WHAT C3 ASKS AND WHAT THIS ANSWERS. C3 (scedit/INTENT.md S2) requires zero FALSE
positives over the shipped corpus before a rule ships, with true findings
recorded upstream rather than silenced. Its script half was answered 2026-09-01
(F76). Its other clause -- the field `~/.spacecrafter/ssystem.ini` -- was
"still item 4's". This is item 4's answer, over THREE corpora:

    field     ~/.spacecrafter/ssystem.ini            untracked, read-only
    shipped   data/default_ssystem.ini               tracked here
    composed  ~/.spacecrafter/modularSystem/*.disabled  machine-owned twins

ROUTING follows C2's ownership split: an authored data slip goes to the
script-surface owner as an SS-n; engine behaviour goes to the parent ledger as a
S5 candidate. Nothing here is fixed -- the field data is READ-ONLY and the
engine is out of F80's scope by its own boundary.

Usage:
    python3 f80_corpus.py --scedit <path> [--strict] [--tsv OUT]
"""

import argparse
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
HOME = os.path.expanduser("~")

LINE_RE = re.compile(r"^(.*?):(\d+): (info|warning|error): (.*) \[-W([a-z-]+)\]$")

# The subject of a finding: the key or the construct the message is about. Taken
# from the message's own leading quote where there is one, so the join key is
# the checker's own word and not a second parse of the data.
SUBJ_RE = re.compile(r"^'([^']+)'")


def corpora():
    out = []
    f = os.path.join(HOME, ".spacecrafter", "ssystem.ini")
    if os.path.exists(f):
        out.append(("field", f))
    s = os.path.join(ROOT, "data", "default_ssystem.ini")
    if os.path.exists(s):
        out.append(("shipped", s))
    d = os.path.join(HOME, ".spacecrafter", "modularSystem")
    if os.path.isdir(d):
        for n in sorted(os.listdir(d)):
            if n.endswith(".ini") or n.endswith(".ini.disabled"):
                out.append(("composed", os.path.join(d, n)))
    return out


# --- the traces -------------------------------------------------------------
# (id, subject) -> (verdict, routed_to, ground, note)
#
# `ground` is the engine code the judgment was read at -- the column F76 added
# because "TRUE" without the site it was judged at is an opinion. `verdict` is
# TRUE-* or FALSE-POSITIVE; there are no other states, and an unmatched finding
# is neither.
TRACES = {
    # ---- dead keys: present in the data, read by NO loader -------------------
    ("ss-dead-key", "tex_halo"): (
        "TRUE-dead-key", "SS (data) + S5 candidate (engine)",
        "protosystem.cpp:102 + ssystem_factory.cpp:390 are WRITES in the "
        "synthesize-a-star-from-a-catalogue-object blocks; no read site anywhere in src/",
        "Every body in both corpora carries it. The halo it names is controlled by "
        "`halo` and `tex_big_halo`; this key does nothing and has no reader to do "
        "anything with."),
    ("ss-dead-key", "lighting"): (
        "TRUE-dead-key", "SS (data) + S5 candidate (engine)",
        "protosystem.cpp:105 + ssystem_factory.cpp:393, both WRITES; no read site",
        "Every body in both corpora carries it."),
    ("ss-dead-key", "model3D"): (
        "TRUE-dead-key", "SS (data)",
        "the only literal in src/ is REP_MODEL3D (spacecrafter.hpp:71), a DIRECTORY "
        "name used by app_settings.cpp:162 and call_system.cpp:130",
        "`model_name` is the key that selects a model; `model3D` names the directory "
        "those models live in and is not a section key."),
    ("ss-dead-key", "ring_shadow"): (
        "TRUE-dead-key", "SS (data)",
        "no occurrence of the literal in src/",
        "The composed path has `ring_shadow_color` (RingLoader.cpp:35-37), which is a "
        "different key; a boolean `ring_shadow` is read by nothing."),
    ("ss-dead-key", "sidereal_period"): (
        "TRUE-dead-key", "SS (data)",
        "no occurrence of the literal in src/",
        "`orbit_visualization_period` and `orbit_period` are the keys that exist."),
    ("ss-dead-key", "tex_cloud"): (
        "TRUE-dead-key", "SS (data)",
        "no occurrence of the literal in src/", "Shipped corpus only."),
    ("ss-dead-key", "tex_cloud_normal"): (
        "TRUE-dead-key", "SS (data)",
        "no occurrence of the literal in src/", "Shipped corpus only."),

    # ---- keys only ONE loader reads ----------------------------------------
    ("ss-experimental-only-key", "rot_frame"): (
        "TRUE-one-regime", "no route -- deliberate authoring, recorded",
        "ModularSystem.cpp:930 (composed); absent from protosystem.cpp",
        "20 lines, authored on purpose: B28's explicit rotation-frame declaration "
        "(INTENT S11.67/S11.79(a)). Inert on the old path, which ignores an unknown "
        "key. Reported at INFO because it is true and worth knowing, not because it "
        "is wrong."),
    ("ss-experimental-only-key", "rot_pole_w0"): (
        "TRUE-one-regime", "no route -- deliberate authoring, recorded",
        "ModularSystem.cpp:975-1022 (composed); absent from protosystem.cpp",
        "20 lines, the IAU prime meridian, same B28 wave as rot_frame."),
    ("ss-legacy-only-key", "big_halo_size"): (
        "TRUE-one-regime", "S5 candidate (engine)",
        "protosystem.cpp:700,726,754 only -- no composed reader",
        "A star's big-halo SIZE is lost when the composed format is adopted: the "
        "twin carries the key faithfully and the composed loader never reads it."),
    ("ss-legacy-only-key", "tex_skin"): (
        "TRUE-one-regime", "S5 candidate (engine)",
        "protosystem.cpp:667 only -- no composed reader",
        "Same class as big_halo_size: preserved in the twin, read by nobody there."),

    # ---- authored slips ------------------------------------------------------
    ("ss-unknown-key", "orbit_MeanLongitude"): (
        "TRUE-authored-slip", "SS (data)",
        "orbit_creator_cor.cpp:83 reads `orbit_meanlongitude`; no reader lowercases "
        "a key (checked across every loader)",
        "[Sedna]. The body silently loses its mean longitude and gets the default 0."),
    ("ss-unknown-key", "orbit_Period"): (
        "TRUE-authored-slip", "SS (data)",
        "orbit_creator_cor.cpp:76,181 read `orbit_period`",
        "[Sedna]. Under comet_orbit an absent period is only an error when the parent "
        "has a parent (orbit_creator_cor.cpp:182-186); Sedna's parent is the Sun, so "
        "the mean motion is derived from the Gaussian constant instead and nothing is "
        "reported."),
    ("ss-unknown-key", "rotation_periode"): (
        "TRUE-authored-slip", "SS (data)",
        "protosystem.cpp:960 and ModularSystem.cpp:1203 read `rot_periode`",
        "Shipped corpus, 2 lines. The body falls back to `orbit_period`, or to 24 "
        "hours -- and on the old path that fallback is not even logged."),
    ("ss-unknown-key", "absolute_magnitude"): (
        "TRUE-authored-slip", "SS (data)",
        "protosystem.cpp:838,840 read `apparent_magnitude`",
        "Shipped corpus, 1 line. Sharp, because `apparent_magnitude` AND `slope` "
        "together gate the whole comet-tail block: with the key misspelled, every "
        "tail key on that body is skipped too. The setter is named "
        "setAbsoluteMagnitudeAndSlope, which is probably where the spelling came "
        "from."),
    ("ss-malformed-line", None): (
        "TRUE-authored-slip", "SS (data)",
        "protosystem.cpp:132-137 -- `find('=')` returns npos into an int, and the "
        "substr arithmetic then stores a junk key",
        "[Sedna] line 2426, `orbit_LongOfPericenter 95.58754` with no '='. Already "
        "recorded engine-side at INTENT S11.109(i) as the one structural divergence "
        "between the two parsers of this file. THREE slips in one section (this plus "
        "the two case slips above), and all three are the CamelCase spelling the "
        "engine's own error messages use -- orbit_creator_cor.cpp:184 and :208 say "
        "`orbit_MeanMotion`, `orbit_Period`, `orbit_Epoch`, `orbit_MeanAnomaly`, none "
        "of which any reader accepts. That is a hypothesis about how the slips got "
        "there, not a measurement; the case mismatch itself is measured."),

    # ---- the two readers of one file disagree --------------------------------
    ("ss-mid-line-comment", None): (
        "TRUE-two-readers", "S5 candidate (engine)",
        "protosystem.cpp:134 (column 0 only) vs tools/ini_line.hpp:16 (anywhere)",
        "3 lines across the two legacy corpora, all trailing unit comments on a "
        "numeric value. INERT TODAY and measured so: the legacy reader keeps "
        "'1.39639 #degrees/j.century...' as the value string and strToDouble's "
        "std::stod parses the leading number and drops the tail. The two readers "
        "therefore hold different VALUE STRINGS that convert to the same number -- "
        "the INTENT S11.109(i) pair, on the comment axis. It stops being inert the "
        "moment such a value is read as a string rather than a number."),
}


def run(scedit, grammar, path):
    p = subprocess.run([scedit, "--grammar", grammar, "--check", path],
                       capture_output=True)
    if p.returncode > 1:
        raise SystemExit("scedit exited %d on %s: %s"
                         % (p.returncode, path, p.stderr.decode("latin-1", "replace")))
    return p.stdout.decode("latin-1").split("\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--scedit", required=True)
    ap.add_argument("--grammar",
                    default=os.path.join(ROOT, "util", "scedit", "grammar",
                                         "sc-grammar.json"))
    ap.add_argument("--tsv")
    ap.add_argument("--strict", action="store_true")
    a = ap.parse_args()

    rows, unadjudicated = [], 0
    by_verdict, by_id = {}, {}
    for corpus, path in corpora():
        for line in run(a.scedit, a.grammar, path):
            if not line.strip():
                continue
            m = LINE_RE.match(line)
            if not m:
                raise SystemExit("unparsed diagnostic: %r" % line)
            f, n, sev, msg, lid = m.groups()
            sm = SUBJ_RE.match(msg)
            subj = sm.group(1) if sm else None
            trace = TRACES.get((lid, subj)) or TRACES.get((lid, None))
            if trace is None:
                verdict, routed, ground, note = "UNADJUDICATED", "", "", ""
                unadjudicated += 1
            else:
                verdict, routed, ground, note = trace
            rows.append((corpus, os.path.basename(f), n, sev, lid, subj or "",
                         verdict, routed, ground, note))
            by_verdict[verdict] = by_verdict.get(verdict, 0) + 1
            by_id[lid] = by_id.get(lid, 0) + 1

    if a.tsv:
        with open(a.tsv, "w", encoding="ascii", errors="backslashreplace") as fh:
            fh.write("corpus\tfile\tline\tseverity\tid\tsubject\tverdict\trouted\t"
                     "ground\tnote\n")
            for r in rows:
                fh.write("\t".join(str(x).replace("\t", " ") for x in r) + "\n")

    print("findings           %d" % len(rows))
    print("distinct defects   %d" % len(TRACES))
    for k in sorted(by_id):
        print("  id %-28s %d" % (k, by_id[k]))
    for k in sorted(by_verdict):
        print("  verdict %-25s %d" % (k, by_verdict[k]))
    print("UNADJUDICATED      %d" % unadjudicated)
    print("FALSE POSITIVES    %d" % by_verdict.get("FALSE-POSITIVE", 0))
    if a.strict and (unadjudicated or by_verdict.get("FALSE-POSITIVE", 0)):
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
