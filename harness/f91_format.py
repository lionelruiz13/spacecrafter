#!/usr/bin/env python3
"""F91's FORMAT control (INTENT §11.213): did the frame fix move the readout's
SHAPE, as opposed to its numbers?

The fix changes every RA/DE the new path prints, so a byte comparison of two
navstr sidecars is guaranteed to differ and says nothing.  What must NOT change
is the format: the labels, their order, the separators, the number of angle
tokens, the unit.  So each nav/inf string is reduced to its SHAPE -- every
angle and every number replaced by a placeholder -- and the shapes are compared
per body, pre-fix against post-fix.

The leg it runs on is the ENGLISH one, and BOTH locale keys are moved to `en` in
the farm's config.ini: §5.136 measured that `app_locale` alone is INERT -- the
readout's labels follow `sky_locale`.  Running the control in English also makes
it independent of the catalogue F87 has just landed (§11.209), so a format change
cannot hide behind a translation.

usage: f91_format.py <preLegDir> <postLegDir> [--also <frPre> <frPost>]
"""
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import f91_parity as F91                            # noqa: E402

HMS = re.compile(r'\d+h\d+m[\d.]+s')
DMS = re.compile(r'[+-]\d+[^\d\'"]{1,4}\d{2}\'\d{2}"')
NUM = re.compile(r'-?\d+\.?\d*')


def shape(s):
    if s is None:
        return None
    s = DMS.sub('<DMS>', s)
    s = HMS.sub('<HMS>', s)
    return NUM.sub('<N>', s)


def shapes(legdir):
    f = [p for p in sorted(Path(legdir).glob("legA_*.json.navstr"))]
    if not f:
        raise SystemExit("no legA navstr under %s" % legdir)
    nav = F91.parse_navstr_full(str(f[-1]))
    return {nm: {k: shape(v) for k, v in r.items()} for nm, r in nav.items()}


def compare(a, b, label):
    common = sorted(set(a) & set(b))
    bad = []
    for nm in common:
        for k in ("OLD_nav", "NEW_nav", "OLD_inf", "NEW_inf"):
            if a[nm].get(k) != b[nm].get(k):
                bad.append((nm, k))
    print("%-28s %3d bodies compared, %d shape difference(s)%s"
          % (label, len(common), len(bad), ("" if not bad else ": %s" % bad[:8])))
    return bad


def main():
    argv = sys.argv[1:]
    pre, post = argv[0], argv[1]
    a, b = shapes(pre), shapes(post)
    print("F91 FORMAT CONTROL -- the readout's SHAPE, numbers replaced by placeholders")
    print("  pre  = %s" % pre)
    print("  post = %s" % post)
    bad = compare(a, b, "pre vs post (english):")
    ref = next(nm for nm in sorted(a) if a[nm].get("OLD_nav"))
    print("  the shape itself, on %s:" % ref)
    for k in ("OLD_nav", "NEW_nav"):
        print("  %-8s %s" % (k, a[ref].get(k)))
    extra = []
    if "--also" in argv:
        i = argv.index("--also")
        c, d = shapes(argv[i + 1]), shapes(argv[i + 2])
        extra = compare(c, d, "pre vs post (french):")
        # and what the LOCALE itself moves, which is the control's control
        compare(a, c, "english vs french (pre):")
    return 1 if (bad or extra) else 0


if __name__ == "__main__":
    sys.exit(main())
