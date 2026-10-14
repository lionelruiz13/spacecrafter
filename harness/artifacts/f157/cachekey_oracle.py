#!/usr/bin/env python3
# F157 (INTENT 11.330) - the texture cache key, replicated without the engine.
#
#   python3 -I cachekey_oracle.py [<outdir>]      (default: this file's directory)
#
# TODAY = s_texture::getCacheEntryName / getCacheName as read at code 6a331fd5 (s_texture.cpp:1248-1275), on BYTES,
# with std::string's semantics: size_t wrap, substr(pos > size) THROWS std::out_of_range, front() on an empty string
# and memcpy before the buffer are UB (flagged, never given a value).
# STATED = the rule of fable-dispatch F157 / INTENT 11.329 (c)1: a name containing ".spacecrafter/" keeps today's key;
# any other name -> its whole path, leading '/' dropped, unabbreviated, the four-character cut; cache name = entry with
# '/' -> '-' plus ".dat".
# PROPOSED = the CORRECTED rule of the re-minted F157 (second run), replicating cacheSlice + the two cuts of
# s_texture.cpp as built: a name containing ".spacecrafter/" -> the part after it, a RELATIVE name -> itself, in both the
# first directory component abbreviated to its first letter; an ABSOLUTE name without ".spacecrafter/" -> itself whole,
# its leading '/' KEPT (CallSystem::isAbsolute = std::filesystem::path::is_absolute, on POSIX "starts with '/'"); a
# name with no '/' -> itself; the entry = the slice cut by four. Cache name in two forms: G2 (built) = the slice cut by
# three + "dat", G1 (not built, P8's control) = entry + ".dat"; in both a leading '/' -> '_', every other '/' -> '-'.
# (The first run's PROPOSED dropped the leading '/': its record is stop/.)
# Inputs: every file under ~/.spacecrafter/{textures,model3D,media}, in two forms - REL as the engine composes them
# (AppSettings' directories are relative to the curdir ~/.spacecrafter: "textures/<x>") and ABS
# ("<home>/.spacecrafter/<dir>/<x>", the mandate's form).
import os, sys

NPOS = (1 << 64) - 1
SPC = b".spacecrafter/"
HOME = os.path.expanduser(b"~")
DATA = HOME + b"/.spacecrafter/"
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.abspath(__file__))


class Throw(Exception):
    pass


class UB(Exception):
    pass


def u(x):
    return x % (1 << 64)


def find(s, sub, pos=0):
    r = s.find(sub, pos)
    return NPOS if r < 0 else r


def find_first_of(s, ch, pos):
    if pos >= len(s):
        return NPOS
    r = s.find(ch, pos)
    return NPOS if r < 0 else r


def substr(s, pos, count=NPOS):
    if pos > len(s):
        raise Throw("basic_string::substr: __pos (which is %d) > this->size() (which is %d)" % (pos, len(s)))
    return s[pos:pos + min(count, len(s) - pos)]


def at(s, i):
    if i < len(s):
        return s[i:i + 1]
    if i == len(s):
        return b"\0"
    raise UB("operator[] past the end")


def today_slice(name):
    p1 = u(find(name, SPC) + 14)
    if p1 == 13:
        p1 = 0
    p2 = u(find_first_of(name, b"/", p1) - 1)
    return p1, p2


def today_entry(name):
    p1, p2 = today_slice(name)
    f = substr(name, p2, u(len(name) - p2 - 4))
    if not f:
        raise UB("front() on an empty string")
    return at(name, p1) + f[1:]


def today_cache(name):
    p1, p2 = today_slice(name)
    f = substr(name, p2, u(len(name) - p2))
    if not f:
        raise UB("front() on an empty string")
    f = (at(name, p1) + f[1:]).replace(b"/", b"-")
    if len(f) < 3:
        raise UB("memcpy before the buffer")
    return f[:-3] + b"dat"


def cut4(s):
    return substr(s, 0, u(len(s) - 4))


def abbreviate(rel):
    i = rel.find(b"/")
    return rel if i < 0 else rel[:1] + rel[i:]


def stated_entry(name):
    p = name.find(SPC)
    if p >= 0:
        return cut4(abbreviate(name[p + 14:]))
    return cut4(name[1:] if name.startswith(b"/") else name)


def proposed_slice(name):
    pos = find(name, SPC)
    if pos == NPOS and name.startswith(b"/"):
        return name
    rel = name if pos == NPOS else substr(name, u(pos + 14))
    slash = find(rel, b"/")
    return rel if slash == NPOS else rel[:1] + substr(rel, slash)


def proposed_entry(name):
    return cut4(proposed_slice(name))


def file_form(f):
    return ((b"_" + f[1:]) if f[:1] == b"/" else f).replace(b"/", b"-")


def dash_dat(entry):
    return file_form(entry + b".dat")


def g2(name):
    s = proposed_slice(name)
    return file_form(substr(s, 0, u(len(s) - 3)) + b"dat")


RULES = {
    "today": (today_entry, today_cache),
    "stated": (stated_entry, lambda n: dash_dat(stated_entry(n))),
    "proposed_G1": (proposed_entry, lambda n: dash_dat(proposed_entry(n))),
    "proposed_G2": (proposed_entry, g2),
}


def run(fn, name):
    try:
        return "OK", fn(name)
    except Throw as e:
        return "THROW", str(e).encode()
    except UB as e:
        return "UB", str(e).encode()


def inputs():
    rows = []
    for d in (b"textures", b"model3D", b"media"):
        for root, _, files in os.walk(DATA + d):
            for f in files:
                rel = os.path.join(root, f)[len(DATA):]
                rows.append((d, rel))
    return sorted(rows)


def show(b):
    return b.decode("utf-8", "backslashreplace")


def main():
    out = []
    p = lambda *a: out.append(" ".join(str(x) for x in a))
    files = inputs()
    per = {}
    for d, _ in files:
        per[d] = per.get(d, 0) + 1
    p("inputs", len(files), {show(k): v for k, v in per.items()})
    fivex = [r for _, r in files if len(r) >= 5 and r[-5:-4] == b"."]
    p("five-character extensions", len(fivex), sorted({show(r[-5:]) for r in fivex}))
    cache = sorted(f.encode() if isinstance(f, str) else f for f in os.listdir(DATA + b"cache") if f.startswith(b"t-"))
    p("cache t-* files", len(cache))

    # self-check 1: F151's measured abort, s45_f151_pre/gdb.log (__pos 18446744073709551615, size 65)
    o_png = b"/home/claude/spacecrafter/claude/harness/artifacts/f142/run/o.png"
    st, msg = run(today_entry, o_png)
    want = b"basic_string::substr: __pos (which is 18446744073709551615) > this->size() (which is 65)"
    p("SELF-CHECK abort replica", "PASS" if (st, msg) == ("THROW", want) else "FAIL", st, show(msg))
    ok_all = (st, msg) == ("THROW", want)

    tables = {}
    for form in ("REL", "ABS"):
        names = [r if form == "REL" else DATA + r for _, r in files]
        for rule, (fe, fc) in RULES.items():
            t = []
            for n in names:
                t.append((n, run(fe, n), run(fc, n)))
            tables[(form, rule)] = t
            with open(os.path.join(OUT, "%s_%s.tsv" % (rule, form)), "wb") as f:
                for n, (se, e), (sc, c) in t:
                    f.write(b"\t".join([n, se.encode(), e, sc.encode(), c]) + b"\n")
        before = tables[(form, "today")]
        nok = sum(1 for _, (se, _), (sc, _) in before if se == "OK" and sc == "OK")
        p(form, "today: OK both", nok, "of", len(before),
          "| non-OK", [(show(n), se, sc) for n, (se, _), (sc, _) in before if se != "OK" or sc != "OK"][:5])
        produced = {c for _, _, (sc, c) in before if sc == "OK"}
        missing = [show(c) for c in cache if c not in produced]
        p(form, "SELF-CHECK the 27 cache names among today's getCacheName outputs:",
          "PASS" if not missing and len(cache) == 27 else "FAIL", "found", len(cache) - len(missing), "of", len(cache),
          "missing", missing)
        ok_all &= not missing and len(cache) == 27
        for rule in ("stated", "proposed_G1", "proposed_G2"):
            after = tables[(form, rule)]
            de = [(show(b[0]), show(b[1][1]), show(a[1][1])) for b, a in zip(before, after) if b[1] != a[1]]
            dc = [(show(b[0]), show(b[2][1]), show(a[2][1])) for b, a in zip(before, after) if b[2] != a[2]]
            dcache = [x for x in dc if x[1].encode() in cache]
            p(form, rule, "| entry diffs", len(de), "| cache-name diffs", len(dc), "| of which a field cache file", len(dcache))
            for x in de[:3]:
                p("   entry", x)
            for x in dc[:3]:
                p("   cache", x)
            for x in dcache[:3]:
                p("   FIELD", x)
            with open(os.path.join(OUT, "diff_%s_%s.txt" % (rule, form)), "w") as f:
                for x in de:
                    f.write("entry\t%s\t%s\t%s\n" % x)
                for x in dc:
                    f.write("cache\t%s\t%s\t%s\n" % x)

    probes = [b"/media/stick/img.png", b"/mnt/stick/img.png", b"/home/x/.spacecrafter", b"/home/x/.spacecrafter/foo.png",
              b"foo.png", b"a/b.png", b"textures/bodies/earth.jpg", DATA + b"textures/bodies/earth.jpg",
              b"/t/bodies/earth.jpg", o_png, DATA + b"textures/n_genrc.png",
              b"model3D/Maven/tex/foil_n_2.png.jpeg"]
    p("PROBES  name | rule: entry-status entry | cache-status cache")
    for n in probes:
        for rule, (fe, fc) in RULES.items():
            (se, e), (sc, c) = run(fe, n), run(fc, n)
            p("  %-48s %-12s %-5s %-28s %-5s %s" % (show(n), rule, se, show(e), sc, show(c)))
    p("=== ORACLE SELF-CHECKS %s ===" % ("PASS" if ok_all else "FAIL"))

    # P2 on the built form: every in-tree key unchanged, both functions, both name forms
    p2 = all(b[1] == a[1] and b[2] == a[2] for form in ("REL", "ABS")
             for b, a in zip(tables[(form, "today")], tables[(form, "proposed_G2")]))
    p("=== P2 proposed_G2 entry / cache-name diffs 0 / 0 in REL and ABS %s ===" % ("PASS" if p2 else "FAIL"))

    # P3 / P4 on the built form: the mandate's seven probes and case O's fixture key with no throw; an outside name
    # keeps its leading '/' in the entry and writes it '_' in the file; no probe key equals an in-tree key
    fe, fc = RULES["proposed_G2"]
    tree = [r for form in ("REL", "ABS") for r in tables[(form, "proposed_G2")]]
    tree_e, tree_c = {e for _, (_, e), _ in tree}, {c for _, _, (_, c) in tree}
    seven = [b"/media/stick/img.png", b"/mnt/stick/img.png", b"/t/bodies/earth.jpg", b"/home/x/.spacecrafter",
             b"/home/x/.spacecrafter/foo.png", b"foo.png", b"a/b.png", o_png]
    p34 = True
    for n in seven:
        (se, e), (sc, c) = run(fe, n), run(fc, n)
        outside = n.startswith(b"/") and SPC not in n
        ok = se == sc == "OK" and (not outside or (e[:1] == b"/" and c[:1] == b"_")) \
            and e not in tree_e and c not in tree_c and not c.startswith(b"-")
        p34 &= ok
        p("P3/P4 %-5s %-66s entry %-58s file %s" % ("PASS" if ok else "FAIL", show(n), show(e), show(c)))
    slash = sorted(show(e) for e in tree_e if e.startswith(b"/"))
    dash = sorted(show(c) for c in tree_c if c.startswith(b"-"))
    p("P3 in-tree entry keys starting with '/':", len(slash), slash[:3], "| in-tree cache names starting with '-':",
      len(dash), dash[:3])
    p34 &= not slash and not dash
    p("=== P3/P4 probes %s ===" % ("PASS" if p34 else "FAIL"))

    # the inputs, one name per line, for the compiled cross-check (cachekey_cpp.sh)
    for form, names in (("REL", [r for _, r in files]), ("ABS", [DATA + r for _, r in files]), ("PROBES", probes)):
        assert not any(b"\n" in n or b"\t" in n for n in names)
        with open(os.path.join(OUT, "inputs_%s.txt" % form), "wb") as f:
            f.write(b"".join(n + b"\n" for n in names))
    for rule, (fe, fc) in RULES.items():
        with open(os.path.join(OUT, "%s_PROBES.tsv" % rule), "wb") as f:
            for n in probes:
                (se, e), (sc, c) = run(fe, n), run(fc, n)
                f.write(b"\t".join([n, se.encode(), e, sc.encode(), c]) + b"\n")
    ok_all &= p2 and p34
    text = "\n".join(out) + "\n"
    with open(os.path.join(OUT, "oracle_run.txt"), "w") as f:
        f.write(text)
    sys.stdout.write(text)
    return 0 if ok_all else 1


sys.exit(main())
