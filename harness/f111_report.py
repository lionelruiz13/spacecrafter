#!/usr/bin/env python3
"""F111 -- the reader that turns one or two `f111_rate.py` legs into the table
the entry quotes.  INTENT Sec.5.150 / Sec.11.239.

    ./f111_report.py <leg_dir> [<leg_dir2>] [--out FILE]

Per sampling walked-iterating record and per stage it prints the arm triple
(flag OFF / flag ON / flag OFF again), the model's prediction for the ON arm,
the ON/OFF ratio and the ANGLE the ON residual subtends at the record's own
dumped `dist`.  With two leg directories it also prints the PRE vs POST column
pair, which is the fix's proof.

THE ARMS ARE NOT (perturbed vs zero) BUT (perturbed vs a FLOOR), and saying so
is this file's job: at a rate high enough to force an every-frame resample the
per-frame date jump ITSELF leaves the solver under-converged, because the walk
buys ONE call (two Newton steps) per frame.  That floor is the flag-OFF arm and
it is measured, not assumed.
"""
import argparse
import json
import math
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import f105_dump                       # noqa: E402
import f111_predict as P               # noqa: E402

THRESHOLD = 5.6e-7


def dists(legdir, tag):
    """The dumped `dist` per record, from the arm's first dump."""
    p = Path(legdir) / "dumps" / ("%s_%s_0.json" % (Path(legdir).name, tag))
    if not p.exists():
        p = Path(legdir) / "dumps" / ("%s_%s.json" % (Path(legdir).name, tag))
    if not p.exists():
        return {}
    _, b = f105_dump.parse(p)
    return {n: (r.get("new") or {}).get("dist") or 0.0 for n, r in b.items()}


def arcsec(au, dist):
    return math.degrees(math.atan2(au, dist)) * 3600.0 if dist else float("nan")


def stages_of(res):
    return sorted({k.rsplit("_", 1)[0] for k in res
                   if isinstance(res.get(k), dict) and "per_body" in res[k]
                   and k.startswith("R")},
                  key=lambda s: float(s[1:]))


def one(legdir):
    res = json.load(open(Path(legdir) / "f111_rate.json"))
    return res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("legs", nargs="+")
    ap.add_argument("--out")
    a = ap.parse_args()
    L = [one(d) for d in a.legs]
    lines = []
    for d, res in zip(a.legs, L):
        lines.append("=" * 104)
        lines.append("LEG %s   binary %s   exit %s   home md5 in==out %s"
                     % (res["tag"], res["bin_md5"], res["exit_code"],
                        res["md5_in"] == res["md5_out"]))
        lines.append("  header timeSpeed per stage: %s  (expected rate/86400)"
                     % res.get("timeSpeed"))
        lines.append("  two-date control: %s moved of %s sampling records"
                     % tuple(res["ctl_two_dates"]["moved_of_scored"]))
        for t in ("pinned_flag_off", "pinned_flag_on_settled"):
            if t in res:
                lines.append("  %-24s %d PERTURBED of %d  (F107's pinned arm)"
                             % (t, res[t]["n_perturbed"], res[t]["n_scored"]))
        dd = dists(d, "pinned_flag_off")
        for st in stages_of(res):
            off, on, of2 = (res[st + "_off"], res[st + "_on"], res[st + "_off2"])
            lines.append("")
            lines.append("  STAGE %s   PERTURBED (>= %.3g AU): off %d / ON %d /"
                         " off2 %d   of %d scored"
                         % (st, THRESHOLD, off["n_perturbed"], on["n_perturbed"],
                            of2["n_perturbed"], on["n_scored"]))
            lines.append("  %-12s %5s %12s %12s %12s %12s %10s %12s"
                         % ("body", "every", "off_AU", "ON_AU", "off2_AU",
                            "model_AU", "ON/off", "ON_arcsec"))
            rows = sorted(on["per_body"], key=lambda n: -on["per_body"][n]["resid"])
            for n in rows:
                o = off["per_body"][n]["resid"]
                x = on["per_body"][n]["resid"]
                o2 = of2["per_body"][n]["resid"]
                pr = on["per_body"][n].get("pred_max", on["per_body"][n]["pred"])
                lines.append("  %-12s %5s %12.5g %12.5g %12.5g %12.5g %10.4g %12.2f"
                             % (n, "y" if on["per_body"][n]["every"] else "n",
                                o, x, o2, pr, (x / o if o > 0 else float("inf")),
                                arcsec(x, dd.get(n, 0.0))))
    if len(L) == 2:
        pre, post = L
        lines.append("=" * 104)
        lines.append("PRE vs POST, flag ON, per stage -- the fix's proof")
        for st in stages_of(pre):
            if st + "_on" not in post:
                continue
            lines.append("")
            lines.append("  STAGE %s   PERTURBED with the flag ON: pre %d  ->  post %d"
                         % (st, pre[st + "_on"]["n_perturbed"],
                            post[st + "_on"]["n_perturbed"]))
            lines.append("  %-12s %14s %14s %14s %12s"
                         % ("body", "pre_ON_AU", "post_ON_AU", "post_off_AU",
                            "pre/post"))
            rows = sorted(pre[st + "_on"]["per_body"],
                          key=lambda n: -pre[st + "_on"]["per_body"][n]["resid"])
            for n in rows:
                pv = pre[st + "_on"]["per_body"][n]["resid"]
                qv = post[st + "_on"]["per_body"].get(n, {}).get("resid")
                qo = post[st + "_off"]["per_body"].get(n, {}).get("resid")
                if qv is None:
                    continue
                lines.append("  %-12s %14.5g %14.5g %14.5g %12.4g"
                             % (n, pv, qv, qo, (pv / qv if qv > 0 else float("inf"))))
    txt = "\n".join(lines) + "\n"
    if a.out:
        open(a.out, "w").write(txt)
    print(txt)
    return 0


if __name__ == "__main__":
    sys.exit(main())
