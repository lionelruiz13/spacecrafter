#!/usr/bin/env python3
"""F87 -- do the two paths print the SAME translated labels for the same body?
(INTENT §5.111, §11.209; the channel is F44's `.navstr` sidecar, found not built.)

WHAT IS MEASURED, and why this shape.  `ssystem_factory.cpp:1206-1210` writes, per
body, at ONE frame, the four caller-visible strings:

    <name>
      OLD nav: Body::getShortInfoNavString
      NEW nav: ModularObject::getShortInfoNavString
      OLD inf: Body::getInfoString
      NEW inf: ModularObject::getInfoString

So the parity question has a direct observable and needs no new instrument.

THE CRITERION IS NOT STRING EQUALITY, and saying why is half the measurement.
The port also changed things that are not labels: the new nav string carries a
`std::endl` after RA/DE that old does not (`ModularObject.cpp:74` vs
`body.cpp:400`), and it separates the two angles with `" / "` where old uses
`"/"`.  A whole-string diff would go red on those forever and would tell us
nothing about translation.  What §5.111 asks is per-MSGID:

    for each msgid the function uses, EXPECTED = catalogue.get(msgid, msgid)
    G1 (positive)      EXPECTED occurs in OLD's text AND in NEW's text
    G2 (discriminating) when EXPECTED != msgid, the RAW msgid does NOT occur in
                        NEW's text -- this is the half that was red before the
                        wraps, and `--offline` on the pre-fix artifact PROVES it
                        goes red, which is the only way a green means anything

G3 is RECORDED, NOT GATED: the first line of the info string is the body NAME,
where old writes `_(englishName)` and the new path writes `body->getEnglishName()`
bare.  That is a runtime lookup and not a msgid, so it is outside this task's
wrap rule; the count and examples are reported for the owner (§11.209).

THE CATALOGUE IS NOT A .po.  `_()` is `Translator::translateUTF8`
(`src/tools/translator.cpp:45`), a std::map read from `<localeDir>/<lang>.txt`
(`"key";"value"`, empty values skipped) with IDENTITY FALLBACK.  It is installed
field data, D9-frozen.  Two facts this gate depends on, both measured:
`fr.txt` holds 599 entries and answers 5 of the 10 msgids; `en.txt` parses to
ZERO entries -- every value in it is empty -- so `app_locale = en` is pure
identity and the English control is exactly the pre-wrap behaviour.

THE PRE-FIX BASELINE, measured FIRST and offline, so that every number below is
a prediction against a known red rather than a hope (`--offline
../artifacts/f44/legA_003.json.navstr.gz --locale fr`, F44's own French session on
the unwrapped binary):

    bodies 90 | G1 bad 540 | G2 bad 540 | OLD missing 0
    G3 names: 68 same, 22 differ, 22/22 explained by fr.txt | U+00A0 count 0

540 = 90 bodies x 6 translated msgid occurrences (5 in `inf`, `RA/DE: ` in `nav`).
`OLD missing 0` is the load-bearing half: the BASELINE carries every expected
rendering, so a NEW-side miss is attributable to NEW.

PREDICTIONS, WRITTEN AND COMMITTED BEFORE THE FIRST LAUNCH (2026-09-05, code
`1d839b9d`, binary md5 407b3d1d).  A refuted one is kept with its number, not
tidied away.

  P1  fr, live: G1 bad = 0 over the same 90 bodies (5 `inf` + 5 `nav` msgids).
  P2  fr, live: G2 bad = 0 -- none of `Magnitude: `, `RA/DE: `, `Alt/Az: `,
      `Distance: `, `AU` survives in any NEW text.
  P3  the gate is able to fail: offline on the PRE-FIX artifact it is
      540/540 red.  MEASURED ABOVE, BEFORE THE LIVE RUN.
  P4  fr, live: G3 unchanged at 68 same / 22 differ / 22 explained -- this change
      does not touch the name channel, so the one divergence it does NOT close
      must still be there, at exactly its old size.
  P5  en, live: every NEW `inf`/`nav` string is BYTE-IDENTICAL to the pre-fix
      artifact's NEW string for the same body (same JD, same place, `en.txt`
      parses to ZERO entries so `_()` is the identity) -- the control that the
      wraps changed nothing outside French.  Machine-checked by `--compare-new`.
  P6  fr, live: U+00A0 count = 90 (was 0) -- the new path now prints a byte that
      no literal in `src/` contains and that only the catalogue can supply, which
      is the sharpest single proof that the lookup, not the literal, is answering.

Usage:
    f87_labels.py <outdir> --locale fr|en [--bin PATH] [--jd JD]
    f87_labels.py --offline <navstr[.gz]> [--locale fr|en]
    f87_labels.py --compare-new <navstrA> <navstrB>     # P5: NEW side, byte-exact
    f87_labels.py --self-test
"""
import gzip
import json
import os
import re
import socket
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from f87_census import load_catalogue                       # noqa: E402

LANGDIR = Path.home() / ".spacecrafter" / "language"
JD = 2461233.5              # dual-dump.sts's canonical epoch, F44's leg A
PORT = 7805

# The msgids each function uses, in source order, taken from `body.cpp` (old is
# the baseline) and now matched byte-for-byte by `ModularObject.cpp`.
INF_MSGIDS = ["Magnitude: ", "RA/DE: ", "Alt/Az: ", "Distance: ", "AU"]
NAV_MSGIDS = ["RA/DE: ", "SA ", " GHA ", " LHA ", " Az/Alt/coA: "]
# " Day length: " is printed only for a star (old: englishName == "Sun"), so it
# is checked only on the bodies whose text actually holds a day-length field.
SUN_MSGID = " Day length: "

FAILS, NOTES = [], []


def fail(m):
    FAILS.append(m)
    print("FAIL: " + m, flush=True)


def ok(m):
    print("ok:   " + m, flush=True)


def note(m):
    NOTES.append(m)
    print("NOTE: " + m, flush=True)


# --------------------------------------------------------------- the sidecar
MARKERS = ("\n  OLD nav: ", "\n  NEW nav: ", "\n  OLD inf: ", "\n  NEW inf: ")


def read_text(path):
    p = str(path)
    if p.endswith(".gz"):
        return gzip.open(p, "rt", encoding="utf-8").read()
    return open(p, encoding="utf-8").read()


def parse_navstr(path):
    """{name: {'OLD nav','NEW nav','OLD inf','NEW inf'}} -- WHOLE strings.

    The record shape is fixed by the writer: a name line, then the four marker
    lines, and the printed strings themselves contain newlines.  Splitting on the
    `OLD nav` marker therefore recovers records exactly, and the tail of each
    record's `NEW inf` value is the NEXT record's name line -- which is checked
    against that record's own name rather than assumed.
    """
    text = read_text(path)
    chunks = text.split(MARKERS[0])
    out, order = {}, []
    names = [chunks[0].strip()]
    for i, ch in enumerate(chunks[1:]):
        rec = {}
        rest = ch
        for a, b in ((MARKERS[1], 'OLD nav'), (MARKERS[2], 'NEW nav'),
                     (MARKERS[3], 'OLD inf')):
            if a not in rest:
                rec = None
                break
            v, rest = rest.split(a, 1)
            rec[b] = v
        if rec is None:
            continue
        rec['NEW inf'] = rest
        tail = rec['NEW inf']
        if i + 1 < len(chunks) - 1:
            # trailing "\n<next name>\n" belongs to the next record
            body, _, nxt = tail.rstrip('\n').rpartition('\n')
            rec['NEW inf'] = body
            names.append(nxt.strip())
        else:
            rec['NEW inf'] = tail.rstrip('\n')
        name = names[i]
        out[name] = rec
        order.append(name)
    return out, order


# ------------------------------------------------------------------- the gates
def gate(recs, cat, tag, strict=True):
    """G1/G2 over every body that carries BOTH paths' strings."""
    n = 0
    g1_bad, g2_bad, missing = [], [], []
    for name, r in recs.items():
        if not (r.get('OLD inf') and r.get('NEW inf')):
            continue
        n += 1
        for field, ids in (('inf', INF_MSGIDS), ('nav', NAV_MSGIDS)):
            old, new = r['OLD ' + field], r['NEW ' + field]
            for m in ids:
                exp = cat.get(m, m)
                if exp not in old:
                    missing.append((name, field, m, 'OLD'))
                if exp not in new:
                    g1_bad.append((name, field, m))
                if exp != m and m in new:
                    g2_bad.append((name, field, m))
        if SUN_MSGID in r['OLD nav'] or cat.get(SUN_MSGID, SUN_MSGID) in r['OLD nav']:
            exp = cat.get(SUN_MSGID, SUN_MSGID)
            if exp not in r['NEW nav']:
                g1_bad.append((name, 'nav', SUN_MSGID))
    say = fail if strict else note
    if g1_bad:
        say("%s G1: %d (body,field,msgid) triples where the EXPECTED rendering is "
            "absent from NEW -- e.g. %s" % (tag, len(g1_bad), g1_bad[:4]))
    else:
        ok("%s G1: every expected rendering present in NEW, %d bodies" % (tag, n))
    if g2_bad:
        say("%s G2: %d triples where the RAW msgid survives in NEW -- e.g. %s"
            % (tag, len(g2_bad), g2_bad[:4]))
    else:
        ok("%s G2: no raw msgid survives in NEW, %d bodies" % (tag, n))
    if missing:
        note("%s: %d expected renderings absent from OLD too (the baseline itself) "
             "-- e.g. %s" % (tag, len(missing), missing[:4]))
    return {"bodies": n, "g1_bad": len(g1_bad), "g2_bad": len(g2_bad),
            "old_missing": len(missing),
            "g1_examples": g1_bad[:8], "g2_examples": g2_bad[:8]}


def names_report(recs, cat, tag):
    """G3 -- RECORDED, NOT GATED: the name channel (`_(englishName)` in old)."""
    diff, same = [], 0
    for name, r in recs.items():
        if not (r.get('OLD inf') and r.get('NEW inf')):
            continue
        o = r['OLD inf'].split('\n', 1)[0].strip()
        w = r['NEW inf'].split('\n', 1)[0].strip()
        if o == w:
            same += 1
        else:
            diff.append((w, o))
    in_cat = sum(1 for w, o in diff if cat.get(w) == o)
    note("%s G3 name channel: %d bodies print the SAME name, %d differ; of the "
         "differing, %d are exactly `fr.txt[english] == old's spelling` -- "
         "e.g. %s" % (tag, same, len(diff), in_cat, diff[:4]))
    return {"same": same, "differ": len(diff), "explained_by_catalogue": in_cat,
            "examples": diff[:12]}


def nbsp_report(recs, tag):
    """P6 -- the French Alt/Az label's U+00A0, a byte no literal in src/ holds."""
    hits = sum(1 for r in recs.values()
               if r.get('NEW inf') and ' ' in r['NEW inf'])
    note("%s P6: %d NEW inf strings carry U+00A0 (the catalogue's `Alt/Az\\xa0: `)"
         % (tag, hits))
    return hits


# ------------------------------------------------------------------ the launch
def wait_port(timeout=120):
    t0 = time.time()
    while time.time() - t0 < timeout:
        try:
            return socket.create_connection(("127.0.0.1", PORT), timeout=1)
        except OSError:
            time.sleep(1)
    raise RuntimeError("port %d never opened" % PORT)


def send(sock, cmd, pause=0.6):
    sock.sendall((cmd + "\n").encode())
    time.sleep(pause)
    try:
        sock.settimeout(0.25)
        sock.recv(8192)
        sock.settimeout(None)
    except socket.timeout:
        sock.settimeout(None)


def no_instance():
    """§11.121(m) / F26 shape: /proc/<pid>/comm, every account, no self-match."""
    hit = []
    for p in Path('/proc').glob('[0-9]*/comm'):
        try:
            if 'spacecrafter' in p.read_text():
                hit.append(str(p))
        except OSError:
            pass
    return hit


def make_farm(farm, locale):
    subprocess.run(["bash", str(HERE / "b3_farm.sh"), str(farm)], check=True)
    cfg = Path(farm) / ".spacecrafter" / "config.ini"
    # b3_farm.sh COPIES config.ini (it symlinks everything else), so this edit
    # never reaches the field config.  Asserted below by md5 in == out on $HOME.
    txt = cfg.read_text(encoding='iso-8859-1')
    new = re.sub(r'^(app_locale\s*=\s*)\S+', r'\g<1>' + locale, txt, flags=re.M)
    if new == txt and locale != 'fr':
        raise RuntimeError("app_locale line not found in the farm config")
    cfg.write_text(new, encoding='iso-8859-1')
    got = re.search(r'^app_locale\s*=\s*(\S+)', new, re.M)
    ok("farm %s: app_locale = %s" % (farm, got.group(1) if got else "?"))
    return cfg


def run_live(out, locale, binp, jd):
    home = Path.home() / ".spacecrafter"
    md5_in = subprocess.run(["md5sum", str(home / "config.ini"), str(home / "ssystem.ini")],
                            capture_output=True, text=True).stdout
    print("md5 IN (real $HOME):\n" + md5_in, flush=True)
    inst = no_instance()
    if inst:
        fail("a spacecrafter process is already running: %s" % inst)
        return None
    ok("concurrent-instance probe: 0")

    farm = out / ("farm_" + locale)
    make_farm(farm, locale)
    env = {**os.environ, "HOME": str(farm),
           "DISPLAY": os.environ.get("DISPLAY", ":2")}
    proc = subprocess.Popen([str(binp)], cwd=str(farm / ".spacecrafter"),
                            stdout=open(out / ("f87_%s.applog" % locale), "w"),
                            stderr=subprocess.STDOUT, env=env)
    dump = out / ("dump_%s.json" % locale)
    try:
        s = wait_port()
        time.sleep(10)
        send(s, "flag experimental_path on")
        send(s, "timerate rate 0")
        send(s, "date jday %s" % jd, 1.2)
        if dump.exists():
            dump.unlink()
        send(s, "body action dual_dump filename %s" % dump, 1.0)
        for _ in range(60):
            side = Path(str(dump) + ".navstr")
            if dump.exists() and dump.stat().st_size > 0 and side.exists():
                time.sleep(0.5)
                break
            time.sleep(0.2)
        else:
            fail("dump never written (%s)" % dump)
            return None
    finally:
        try:
            send(s, "shutdown action now", 2.0)
        except Exception:
            pass
        for _ in range(30):
            if proc.poll() is not None:
                break
            time.sleep(0.5)
        if proc.poll() is None:
            proc.kill()
    rc = proc.poll()
    (ok if rc == 0 else note)("exit code %s" % rc)
    md5_out = subprocess.run(["md5sum", str(home / "config.ini"), str(home / "ssystem.ini")],
                             capture_output=True, text=True).stdout
    if md5_out == md5_in:
        ok("md5 in == out on the real $HOME (the farm took every write)")
    else:
        fail("the real $HOME config/ssystem MOVED:\n%s\n%s" % (md5_in, md5_out))
    return Path(str(dump) + ".navstr")


SELF_TEST = ("Adrastea\n  OLD nav: AD/DE : 08h19m42s/+20d\n  NEW nav: RA/DE: 04h\n"
             "SA +288\n  OLD inf: Adrastea\nMagnitude : 20.07\n"
             "  NEW inf: Adrastea\nMagnitude: 20.07\n"
             "Amalthea\n  OLD nav: a\n  NEW nav: b\n  OLD inf: Amalthée\n"
             "  NEW inf: Amalthea\nMagnitude: 15.92\n")


def self_test(tmp):
    p = tmp / "st.navstr"
    p.write_text(SELF_TEST, encoding='utf-8')
    recs, order = parse_navstr(p)
    prob = []
    if order != ['Adrastea', 'Amalthea']:
        prob.append("record order %r" % (order,))
    if recs.get('Adrastea', {}).get('NEW nav') != "RA/DE: 04h\nSA +288":
        prob.append("multi-line NEW nav not kept whole: %r"
                    % recs.get('Adrastea', {}).get('NEW nav'))
    if recs.get('Adrastea', {}).get('NEW inf') != "Adrastea\nMagnitude: 20.07":
        prob.append("NEW inf swallowed the next record's name: %r"
                    % recs.get('Adrastea', {}).get('NEW inf'))
    if recs.get('Amalthea', {}).get('OLD inf') != "Amalthée":
        prob.append("last record's OLD inf %r" % recs.get('Amalthea', {}).get('OLD inf'))
    cat = {"Magnitude: ": "Magnitude : ", "Amalthea": "Amalthée"}
    r = gate(recs, cat, "self-test", strict=False)
    # In the fixture the NEW side is DELIBERATELY pre-fix: the gate must be red.
    if r["g2_bad"] == 0:
        prob.append("G2 green on a pre-fix fixture -- the gate cannot fail")
    print("self-test: %d records, G1 bad %d, G2 bad %d (both must be > 0 here)"
          % (len(recs), r["g1_bad"], r["g2_bad"]))
    for x in prob:
        print("   PROBLEM: " + x)
    print("SELF-TEST", "PASS" if not prob else "FAIL")
    return 0 if not prob else 1


def main(argv):
    if "--self-test" in argv:
        tmp = Path("/tmp/f87_selftest")
        tmp.mkdir(exist_ok=True)
        return self_test(tmp)

    locale = argv[argv.index("--locale") + 1] if "--locale" in argv else "fr"
    cat = load_catalogue(str(LANGDIR / ("%s.txt" % locale)))
    ok("catalogue %s.txt: %d entries" % (locale, len(cat)))

    if "--compare-new" in argv:
        i = argv.index("--compare-new")
        a, b = Path(argv[i + 1]), Path(argv[i + 2])
        ra, _ = parse_navstr(a)
        rb, _ = parse_navstr(b)
        common = sorted(set(ra) & set(rb))
        bad = []
        for nm in common:
            for f in ('NEW inf', 'NEW nav'):
                if ra[nm].get(f) != rb[nm].get(f):
                    bad.append((nm, f))
        if bad:
            fail("P5: %d NEW strings differ between %s and %s -- e.g. %s"
                 % (len(bad), a.name, b.name, bad[:4]))
            for nm, f in bad[:2]:
                print("   %s %s\n     A: %r\n     B: %r"
                      % (nm, f, ra[nm].get(f), rb[nm].get(f)))
        else:
            ok("P5: all %d common bodies' NEW inf AND NEW nav strings are "
               "BYTE-IDENTICAL between %s and %s" % (len(common), a.name, b.name))
        return 1 if bad else 0

    if "--offline" in argv:
        side = Path(argv[argv.index("--offline") + 1])
        recs, _ = parse_navstr(side)
        tag = "OFFLINE(%s,%s)" % (side.name, locale)
        r = gate(recs, cat, tag, strict=False)
        r["names"] = names_report(recs, cat, tag)
        r["nbsp"] = nbsp_report(recs, tag)
        print(json.dumps(r, indent=1, ensure_ascii=False))
        return 0

    out = Path([a for a in argv if not a.startswith("--")][0]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    binp = Path(argv[argv.index("--bin") + 1]) if "--bin" in argv \
        else HERE.parents[1] / "build-claude/src/spacecrafter"
    jd = argv[argv.index("--jd") + 1] if "--jd" in argv else JD

    side = run_live(out, locale, binp, jd)
    if side is None:
        return 1
    recs, _ = parse_navstr(side)
    tag = "LIVE(%s)" % locale
    rep = gate(recs, cat, tag, strict=(locale == "fr"))
    rep["names"] = names_report(recs, cat, tag)
    rep["nbsp"] = nbsp_report(recs, tag)
    rep["locale"] = locale
    rep["navstr"] = str(side)
    (out / ("f87_%s_result.json" % locale)).write_text(
        json.dumps(rep, indent=1, ensure_ascii=False))
    print("\n%d FAIL, %d NOTE" % (len(FAILS), len(NOTES)))
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
