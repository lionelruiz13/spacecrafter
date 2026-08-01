#!/usr/bin/env python3
"""F21 gate, part 1: the READ HALF of the flag surface (INTENT §11.129).

`setFlag`'s FV_TOGGLE branch used to be the only code in the tree that knew a
flag's current value, and it knew it only while mutating it. This task extracted
that read into `AppCommandInterface::readFlag` and made the toggle branch its
CONSUMER, which is a ~97-site rewrite of the old command surface. That rewrite
has exactly one behavioural contract worth measuring:

  TOGGLE TWICE == IDENTITY, per flag, measured on the flag's OWN value.

and it is measured through the session file, because the session file's [flags]
section is now the readback: `session action save` writes every flag's current
value, so a save before and a save after two toggles is a direct read of the
authority the toggle branch consumes. One instrument, both halves.

  L1  baseline save -> the value of every flag this build can read.
      For each flag: toggle, toggle, save, compare THAT flag's value.
      Side effects on OTHER flags are reported, not failed: `flag atmosphere`
      deliberately drives fog and star twinkle too, and that is old behaviour.
  L2  COUNTERFACTUAL - the check must be able to fail. Three flags are toggled
      an ODD number of times and the same comparison must report them MOVED.
  L3  DISCRIMINATION on the write half: `flag X off` then `flag X on` must be
      visible in the file as false then true.

    cd claude/harness && DISPLAY=:2 ./f21_flags.py [outdir]
"""

import os
import re
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f21_s563 as H

_args = [a for a in sys.argv[1:] if not a.startswith("--")]
OUT = (Path(_args[0]) if _args else Path(__file__).resolve().parent / "artifacts/f21flags").resolve()
OUT.mkdir(parents=True, exist_ok=True)
H.OUT = OUT
SESSIONS = H.SESSIONS
FAILS = []


def fail(m):
    FAILS.append(m)
    print("FAIL: " + m, flush=True)


def ok(m):
    print("ok:   " + m, flush=True)


def read_section(path, header):
    """{key: value} of one section, from the session file itself."""
    out, cur = {}, None
    for ln in Path(path).read_text().splitlines():
        s = ln.strip()
        if s.startswith("["):
            cur = s[1:s.index("]")] if "]" in s else None
            continue
        if cur != header or not s or s.startswith("#") or "=" not in s:
            continue
        k, v = s.split("=", 1)
        out[k.strip()] = v.strip()
    return out


def main():
    if not H.assert_no_other_instance():
        return 1
    SESSIONS.mkdir(exist_ok=True)
    for f in SESSIONS.glob("f21f*.ini"):
        f.unlink()
    frozen_in = {n: H.md5(H.USERDIR / n) for n in H.FROZEN if (H.USERDIR / n).exists()}

    app = H.App("flags")
    app.cmd("timerate rate 0", 1.0)

    def save(tag):
        p = SESSIONS / "f21f.ini"
        p.unlink(missing_ok=True)
        app.cmd("session action save filename f21f", 0.9)
        for _ in range(20):
            if p.exists():
                break
            time.sleep(0.2)
        return p

    base_path = save("base")
    base = read_section(base_path, "flags")
    values0 = read_section(base_path, "values")
    colors0 = read_section(base_path, "colors")
    print(f"   the file reports {len(base)} flags, {len(values0)} `set` values, "
          f"{len(colors0)} colours", flush=True)
    if len(base) < 90 or len(values0) < 20 or len(colors0) < 40:
        fail(f"the bulk sections are too small to be the real inventory: "
             f"{len(base)}/{len(values0)}/{len(colors0)}")
    for owned in ("track_object", "lock_sky_position", "experimental_path",
                  "experimental_shadows"):
        if owned in base:
            fail(f"[flags] carries '{owned}', which another §2 row owns")
    for owned in ("heading", "home_planet", "landscape_name", "zoom_offset"):
        if owned in values0:
            fail(f"[values] carries '{owned}', which another §2 row owns "
                 f"(heading is D28's carve-out)")

    names = sorted(base)
    moved_self, side = [], {}
    for i, n in enumerate(names):
        app.cmd(f"flag {n} toggle", 0.25)
        app.cmd(f"flag {n} toggle", 0.25)
        cur = read_section(save(n), "flags")
        if cur.get(n) != base.get(n):
            moved_self.append((n, base.get(n), cur.get(n)))
        for k, v in cur.items():
            if v != base.get(k) and k != n:
                side.setdefault(n, []).append(k)
        base = cur          # side effects are cumulative; compare against NOW
        if (i + 1) % 25 == 0:
            print(f"   ... {i+1}/{len(names)}", flush=True)

    if moved_self:
        fail(f"toggle x2 changed the flag's own value for {len(moved_self)}: {moved_self[:8]}")
    else:
        ok(f"toggle x2 == identity for all {len(names)} flags the file carries, "
           f"measured on each flag's own value")
    if side:
        print(f"   (flags whose toggle also moved OTHER flags, old behaviour, "
              f"reported not failed: {side})", flush=True)

    # L2 - the counterfactual: an ODD number of toggles MUST be visible.
    probe = [n for n in ("stars", "planets", "nebulae") if n in base]
    before = read_section(save("cf0"), "flags")
    for n in probe:
        app.cmd(f"flag {n} toggle", 0.3)
    after = read_section(save("cf1"), "flags")
    got = [n for n in probe if after.get(n) != before.get(n)]
    if sorted(got) != sorted(probe):
        fail(f"COUNTERFACTUAL: a single toggle of {probe} did not move {set(probe)-set(got)} "
             f"— the identity check above cannot fail and means nothing")
    else:
        ok(f"counterfactual: one toggle moves exactly {probe} in the file "
           f"(so the identity leg can fail)")
    for n in probe:
        app.cmd(f"flag {n} toggle", 0.3)

    # L3 - the write half, both ways.
    app.cmd("flag stars off", 0.4)
    off = read_section(save("off"), "flags")
    app.cmd("flag stars on", 0.4)
    on = read_section(save("on"), "flags")
    if off.get("stars") != "false" or on.get("stars") != "true":
        fail(f"write half: stars off/on read back as {off.get('stars')}/{on.get('stars')}")
    else:
        ok("write half: `flag stars off` reads false, `flag stars on` reads true")

    app.quit()
    frozen_out = {n: H.md5(H.USERDIR / n) for n in H.FROZEN if (H.USERDIR / n).exists()}
    if frozen_in != frozen_out:
        fail("frozen md5 in != out")
    else:
        ok("frozen config/ssystem/galactic/anchor md5 in == out")

    print(f"\n{'FAILURES: ' + str(len(FAILS)) if FAILS else 'ALL GREEN'}", flush=True)
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
