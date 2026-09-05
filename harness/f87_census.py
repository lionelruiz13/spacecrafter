#!/usr/bin/env python3
"""F87 -- the string-literal census of `src/experimentalModule/`, and the old path's
msgid inventory it is measured against (INTENT §5.111, §11.209).

WHY AN INSTRUMENT AND NOT A GREP (§11.146's denominator rule, restated for literals):
this tree writes its requirements INTO its comments, and those comments quote the very
literals they explain -- `ModularObject.cpp:66` names the label `"Az/Alt/coA"` in prose
beside the line that prints it. A raw `grep -c '"'` therefore counts the explanation as
a site. Every count here is taken on COMMENT-STRIPPED text (`f44_census.strip_comments`,
which replaces comment bytes with spaces and preserves line numbers, so every `file:line`
below is valid against the original file).

WHAT IT MEASURES, in three passes:

  (1) OLD-SIDE INVENTORY -- every `_(...)` site in `src/` outside `experimentalModule`
      and outside EntityCore, split into literal msgids and non-literal arguments
      (`_(englishName)`, `_(getTypePlanet(...))` -- gettext on a runtime value).
      This is the set the new path is allowed to draw from: §11.52(b) makes old the
      baseline, so a msgid that is not old's is an INVENTED string and out of bounds.

  (2) NEW-SIDE CENSUS -- every string literal in `src/experimentalModule/`, classified
      by the SINK of the statement that holds it (statement-level, not line-level: a
      `cLog::get()->write("..." "...")` spanning five lines is ONE diagnostic, and a
      line-level classifier reports its continuation lines as unclassified prose --
      measured: 335 false candidates by the line-level filter, 8 by this one).

  (3) THE CROSS -- for every user-surface literal, whether an identical msgid exists on
      the old side, and whether it is wrapped in `_()` on the new side.

THE TRANSLATION CHANNEL IS NOT GETTEXT.  `_()` is `Translator::translateUTF8`
(`src/tools/translator.cpp:45`), a lookup in a std::map loaded from
`<localeDir>/<lang>.txt` -- lines of the form `"key";"value"` -- with IDENTITY FALLBACK
for a key that is absent. There is no `.po`/`.mo` anywhere in either repository. The
catalogue is INSTALLED FIELD DATA (`~/.spacecrafter/language/fr.txt`, from
`spacecrafter-data`), frozen by §2.0 D9. Two consequences the census reports on:
  - a msgid copied byte-exact from old gets EXACTLY old's answer, translated or not;
  - `--catalogue` therefore tests EXACT KEY EQUALITY, never a substring grep: `grep -c
    '"SA "'` would answer 1 on any line that merely contains those bytes, and the key
    `"SA "` is in fact ABSENT from fr.txt.

Usage:
    f87_census.py [--old] [--new] [--cross] [--catalogue PATH] [--json OUT]
    f87_census.py --self-test
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from f44_census import strip_comments  # noqa: E402  (the §11.146 stripper, reused)

SRC = "/home/claude/spacecrafter/src"
NEW = os.path.join(SRC, "experimentalModule")
EXTS = (".cpp", ".hpp", ".h", ".c", ".cxx", ".hxx", ".inl")
CATALOGUE = os.path.expanduser("~/.spacecrafter/language/fr.txt")

LIT = re.compile(r'"(?:[^"\\\n]|\\.)*"')
# `_("literal")` -- the msgid form.  The identifier guard keeps `foo_(` out.
MSGID = re.compile(r'(?<![A-Za-z0-9_])_\(\s*("(?:[^"\\\n]|\\.)*")\s*\)')
CALL = re.compile(r'(?<![A-Za-z0-9_])_\(')


def sources(root, skip=()):
    for dirpath, dirnames, filenames in os.walk(root):
        if any(s in dirpath for s in skip):
            continue
        dirnames[:] = [d for d in dirnames if not any(s in d for s in skip)]
        for f in sorted(filenames):
            if f.endswith(EXTS):
                yield os.path.join(dirpath, f)


def mask_chars(stmt):
    """Blank C++ CHAR literals, preserving length and newlines.

    Measured hazard (this census, first run): `out << '"' << a.name << '"';`
    [observed: CameraAnchors.cpp:758, ModularBody.cpp:1055] makes a naive
    double-quote scanner read `' << a.name << '` as a STRING literal -- two char
    literals become one phantom string that looks like untranslated prose.  Three
    such phantoms were in the first census run; the mask removes them.
    """
    out = list(stmt)
    i, n = 0, len(stmt)
    while i < n:
        c = stmt[i]
        if c == '"':
            i += 1
            while i < n:
                if stmt[i] == '\\':
                    i += 2
                    continue
                if stmt[i] == '"':
                    i += 1
                    break
                if stmt[i] == '\n':
                    break
                i += 1
            continue
        if c == "'":
            j = i
            i += 1
            while i < n:
                if stmt[i] == '\\':
                    i += 2
                    continue
                if stmt[i] == "'":
                    i += 1
                    break
                if stmt[i] == '\n':
                    break
                i += 1
            for k in range(j, min(i, n)):
                if out[k] != '\n':
                    out[k] = ' '
            continue
        i += 1
    return "".join(out)


BLOCK_AFTER = set(') ; { } : \n'.split(' ')) | {'\n', ''}
CTRL = re.compile(r'\b(if|for|while|switch|else|do|catch)\s*$')
SIGN = re.compile(r'([A-Za-z_~][A-Za-z0-9_:~]*)\s*\([^;]*\)\s*'
                  r'(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?$')


def statements(stripped):
    """Yield (start_line, text, scope_stack) for each `;`-terminated statement.

    Two hazards this handles, both measured in this tree:

    * A `;` inside parentheses (a `for` header, a lambda argument) does not end a
      statement, and neither does one inside a string or char literal.
    * A `{` is a BLOCK opener only when what precedes it is `)`, `;`, `{`, `}`, `:`
      or a control keyword.  After `=`, `(` or `,` it opens a BRACED INITIALIZER --
      splitting there tore the six-line `banner` initializers of
      `SessionFile.cpp:433` and `ModularSystem.cpp:1749` into fragments whose call
      name had been left behind, so 56 file-header lines looked unclassifiable.

    `scope_stack` is the condensed text preceding each open block, innermost last;
    the enclosing FUNCTION is the outermost entry that parses as a signature.
    """
    line = 1
    depth = 0            # () and [] depth
    start = 0
    startline = 1
    stack = []           # open blocks: (is_block, condensed-prefix)
    i = 0
    n = len(stripped)
    while i < n:
        c = stripped[i]
        if c == '\n':
            line += 1
            i += 1
            continue
        if c == '"' or c == "'":
            q = c
            i += 1
            while i < n:
                if stripped[i] == '\\':
                    i += 2
                    continue
                if stripped[i] == q:
                    i += 1
                    break
                if stripped[i] == '\n':
                    line += 1
                    break
                i += 1
            continue
        if c in '([':
            depth += 1
        elif c in ')]':
            depth -= 1
        elif c == '{' and depth <= 0:
            prefix = " ".join(stripped[start:i].split())
            raw_prefix = stripped[start:i]
            prev = prefix[-1:] if prefix else ''
            attached = bool(raw_prefix) and not raw_prefix[-1].isspace()
            # INITIALIZER (does not open a scope, does not split a statement):
            #   `= {`, `( {`, `, {`, `[ {`, `return {`, or a brace-init glued to a
            #   type/variable name -- `banner{`, `TextureInfo{`, `Vec3f{`.
            # Everything else at paren-depth 0 opens a BLOCK.
            is_init = (prev in ('=', '(', ',', '[')
                       or re.search(r'\breturn$', prefix) is not None
                       or (attached and (prev.isalnum() or prev in ('_', '>'))))
            if not is_init:
                yield startline, stripped[start:i], [s for _, s in stack if _]
                stack.append((True, prefix))
                i += 1
                start = i
                startline = line
                continue
        elif c == '}' and depth <= 0:
            if stack:
                yield startline, stripped[start:i], [s for _, s in stack if _]
                stack.pop()
                i += 1
                start = i
                startline = line
                continue
        elif c == ';' and depth <= 0:
            yield startline, stripped[start:i + 1], [s for _, s in stack if _]
            i += 1
            start = i
            startline = line
            continue
        i += 1
    if start < n:
        yield startline, stripped[start:], [s for _, s in stack if _]


def enclosing_function(scope):
    """The outermost scope entry that parses as a function signature."""
    for s in scope:
        if CTRL.search(s):
            continue
        m = SIGN.search(s)
        if m:
            return m.group(1)
    return ""


LOG_SINKS = ("cLog", "std::cerr", "std::cout", "SDL_Log", "fprintf", "printf", "puts(",
             "putLog")   # putLog = the Vulkan log channel, this module's second one
# The three object-readout producers: the ONLY functions in this module whose text
# reaches the user's screen / the `get status object` answer (§11.158's R1/R2).
READOUT_FUNCS = ("getInfoString", "getShortInfoString", "getShortInfoNavString")
KEYish = re.compile(r'(==|!=)\s*"|\.\s*(at|find|count|contains)\s*\(\s*"'
                    r'|appendEntry\s*\(\s*"|\[\s*"|insert\s*\(\s*\{?\s*"'
                    r'|\bcase\b|^\s*return\s+"[A-Za-z_][A-Za-z0-9_]*"\s*;')
PATHish = re.compile(r'\.(vert|frag|geom|comp|spv|json|ini|txt|obj|ojm|png|jpg|dat)\b|/')


def classify(stmt, func=""):
    s = " ".join(stmt.split())
    func = func.rsplit('::', 1)[-1]     # `Thing::getInfoString` -> `getInfoString`
    if s.startswith('#') or '#include' in s or '#pragma' in s or '#define' in s:
        return 'PP'
    if any(k in s for k in LOG_SINKS):
        return 'LOG'
    if re.search(r'\bdiagnose\s*\(', s):
        return 'DIAG'          # the data-diagnostic channel (§2(f) / §11.184)
    if re.search(r'\bannotate\s*\(|\bbanner\b|IniLine::COMMENT_CHAR', s) \
            or func.startswith('save'):
        return 'ARTIFACT'      # text written INTO a file the app produces
    if re.search(r'\bthrow\b', s):
        return 'THROW'
    if re.search(r'\b(static_assert|assert)\s*\(', s):
        return 'ASSERT'
    if re.search(r'\boss\s*<<|\bos\s*<<|\boss<<', s):
        return 'READOUT' if func in READOUT_FUNCS else 'OSTREAM'
    return 'OTHER'


def subclassify(text, stmt):
    if PATHish.search(text):
        return 'PATH'
    if KEYish.search(stmt):
        return 'KEY'
    if ' ' not in text.strip():
        return 'TOKEN'
    return 'UNCLASSIFIED'


def old_inventory():
    """Pass 1: the old path's msgid inventory (everything `_()` may legally say)."""
    lits, nonlits = [], []
    for path in sources(SRC, skip=("EntityCore", "experimentalModule")):
        raw = open(path, encoding='utf-8', errors='replace').read()
        s = strip_comments(raw)
        for ln, line in enumerate(s.split('\n'), 1):
            covered = []
            for m in MSGID.finditer(line):
                lits.append({"file": rel(path), "line": ln, "msgid": json_unquote(m.group(1))})
                covered.append((m.start(), m.end()))
            for m in CALL.finditer(line):
                if any(a <= m.start() < b for a, b in covered):
                    continue
                nonlits.append({"file": rel(path), "line": ln, "text": line.strip()[:160]})
    return lits, nonlits


def new_census():
    """Pass 2: every literal in experimentalModule, statement-classified."""
    out = []
    for path in sources(NEW):
        raw = open(path, encoding='utf-8', errors='replace').read()
        s = strip_comments(raw)
        for startline, stmt, scope in statements(s):
            stmt = mask_chars(stmt)
            func = enclosing_function(scope)
            # line of each literal = startline + newlines before it inside the stmt
            for m in LIT.finditer(stmt):
                ln = startline + stmt[:m.start()].count('\n')
                text = json_unquote(m.group(0))
                cls = classify(stmt, func)
                wrapped = bool(re.search(
                    r'(?<![A-Za-z0-9_])_\(\s*' + re.escape(m.group(0)),
                    stmt))
                rec = {"file": rel(path), "line": ln, "text": text,
                       "sink": cls, "wrapped": wrapped, "func": func,
                       "stmt": " ".join(stmt.split())[:200]}
                if cls == 'OTHER':
                    rec["sub"] = subclassify(text, stmt)
                out.append(rec)
    return out


def json_unquote(q):
    """C string literal -> its bytes, for the escapes this tree actually uses."""
    body = q[1:-1]
    return (body.replace('\\n', '\n').replace('\\t', '\t')
                .replace('\\"', '"').replace("\\'", "'").replace('\\\\', '\\'))


def rel(p):
    return p.replace(SRC + "/", "src/").replace(SRC, "src")


def load_catalogue(path):
    """Parse <lang>.txt with the ENGINE'S OWN rule (translator.cpp:96-110)."""
    cat = {}
    raw = open(path, 'rb').read().decode('utf-8')
    for line in raw.split('\n'):
        line = line.rstrip('\r')
        if not line or line[0] in '#\r\n':
            continue
        f = line.find('";"')
        if f == -1:
            continue
        key, val = line[1:f], line[f + 3:len(line) - 1]
        if val:
            cat[key] = val
    return cat


SELF_TEST = r'''
std::string Thing::getInfoString(const Navigator *nav) const
{
    cLog::get()->write("a diagnostic that "
                       "spans three lines "
                       "and is ONE statement");
    oss << _("Distance: ") << x << " " << _("AU");
    oss << ("Magnitude: ") << y;
    if (name == "halo") return 1;
    out << '"' << a.name << '"';
    const std::vector<std::string> banner{
        " a file header line",
        " and its second line",
    };
}
std::string Thing::saveOrbit() const
{
    os << "coord_func = surface_point" << std::endl;
}
'''

SELF_TEST_EXPECT = {'ARTIFACT': 3, 'LOG': 3, 'OTHER': 1, 'READOUT': 4}


def self_test():
    s = strip_comments(SELF_TEST)
    got = []
    for startline, stmt, scope in statements(s):
        stmt = mask_chars(stmt)
        func = enclosing_function(scope)
        for m in LIT.finditer(stmt):
            got.append((classify(stmt, func), json_unquote(m.group(0)), func,
                        bool(re.search(r'(?<![A-Za-z0-9_])_\(\s*' + re.escape(m.group(0)), stmt))))
    seen = {}
    for g in got:
        seen[g[0]] = seen.get(g[0], 0) + 1
    rw = sorted(g[1] for g in got if g[0] == 'READOUT' and g[3])
    ru = sorted(g[1] for g in got if g[0] == 'READOUT' and not g[3])
    # Four discriminations pinned here, each one a way the first draft was wrong:
    #  1. the three-line `cLog` call is ONE diagnostic, not three prose fragments;
    #  2. `out << '"' << a.name << '"';` contributes ZERO literals -- naive scanning
    #     reads the two CHAR literals as one phantom string;
    #  3. the six-line braced `banner` initializer stays ONE statement, so its lines
    #     are ARTIFACT (file-header text) and not unclassifiable prose;
    #  4. `saveOrbit`'s `os <<` is ARTIFACT, `getInfoString`'s `oss <<` is READOUT --
    #     same operator, different surface, and only the second is translatable.
    # The two BARE readout literals are the separator `" "` and the label
    # `"Magnitude: "`: the census reports both and does NOT decide between them --
    # a separator is not a label, and that call is made by reading, in §11.209.
    ok = (seen == SELF_TEST_EXPECT and len(got) == 11
          and rw == ['AU', 'Distance: '] and ru == [' ', 'Magnitude: '])
    print("self-test: got %s\n           expected %s ; TOTAL=%d (expect 11)"
          % (dict(sorted(seen.items())), dict(sorted(SELF_TEST_EXPECT.items())), len(got)))
    for g in got:
        print("   %-9s %-20s wrapped=%-5s %r" % (g[0], g[2], g[3], g[1]))
    print("SELF-TEST", "PASS" if ok else "FAIL")
    return 0 if ok else 1


def main(argv):
    if '--self-test' in argv:
        return self_test()
    want_old = '--old' in argv or not any(a in argv for a in ('--old', '--new', '--cross'))
    want_new = '--new' in argv or not any(a in argv for a in ('--old', '--new', '--cross'))
    want_cross = '--cross' in argv or not any(a in argv for a in ('--old', '--new', '--cross'))
    cat_path = CATALOGUE
    if '--catalogue' in argv:
        cat_path = argv[argv.index('--catalogue') + 1]

    lits, nonlits = old_inventory()
    census = new_census()
    old_ids = {}
    for r in lits:
        old_ids.setdefault(r["msgid"], []).append("%s:%d" % (r["file"], r["line"]))

    if want_old:
        print("=== PASS 1 -- OLD-SIDE `_()` INVENTORY (src/ minus experimentalModule, minus EntityCore)")
        print("    literal msgid sites: %d   distinct msgids: %d   non-literal `_()` sites: %d"
              % (len(lits), len(old_ids), len(nonlits)))
        byfile = {}
        for r in lits:
            byfile[r["file"]] = byfile.get(r["file"], 0) + 1
        for f in sorted(byfile):
            print("      %4d  %s" % (byfile[f], f))
        print("    non-literal `_()` (gettext on a runtime value):")
        for r in nonlits:
            print("      %s:%d: %s" % (r["file"], r["line"], r["text"]))

    if want_new:
        print()
        print("=== PASS 2 -- NEW-SIDE LITERAL CENSUS (src/experimentalModule/, comment-stripped)")
        bysink = {}
        for r in census:
            k = r["sink"] if r["sink"] != 'OTHER' else 'OTHER/' + r.get("sub", "?")
            bysink[k] = bysink.get(k, 0) + 1
        print("    total literals: %d" % len(census))
        for k in sorted(bysink, key=lambda x: -bysink[x]):
            print("      %5d  %s" % (bysink[k], k))
        print("    READOUT sites (the object-readout surface -- the ONLY translatable one):")
        for r in census:
            if r["sink"] == 'READOUT':
                print("      %s:%d  %-22s wrapped=%-5s %r"
                      % (r["file"], r["line"], r["func"], r["wrapped"], r["text"]))
        for lbl in ('OSTREAM',):
            rows = [r for r in census if r["sink"] == lbl]
            if rows:
                print("    %s sites (an ostream that is NOT a readout):" % lbl)
                for r in rows:
                    print("      %s:%d  %-22s %r" % (r["file"], r["line"], r["func"], r["text"]))
        print("    OTHER/UNCLASSIFIED (no sink rule matched -- hand-adjudicated in §11.209):")
        for r in census:
            if r["sink"] == 'OTHER' and r.get("sub") == 'UNCLASSIFIED':
                print("      %s:%d  %-22s %r" % (r["file"], r["line"], r["func"], r["text"]))
        print("    `_()`-WRAPPED literals in this module: %d"
              % sum(1 for r in census if r["wrapped"]))

    if want_cross:
        print()
        print("=== PASS 3 -- THE CROSS: user-surface literals vs the old inventory vs the catalogue")
        cat = load_catalogue(cat_path) if os.path.exists(cat_path) else None
        print("    catalogue: %s -- %s"
              % (cat_path, ("%d entries" % len(cat)) if cat is not None else "ABSENT"))
        hdr = "      %-6s %-16s %-8s %-9s %-9s %s"
        print(hdr % ("line", "literal", "wrapped", "old-msgid", "in-cat", "fr answer"))
        for r in census:
            if r["sink"] != 'READOUT':
                continue
            t = r["text"]
            has_old = "YES" if t in old_ids else "no"
            in_cat = "-" if cat is None else ("YES" if t in cat else "no")
            ans = "" if (cat is None or t not in cat) else repr(cat[t])
            print(hdr % ("%s:%d" % (os.path.basename(r["file"]), r["line"]),
                         repr(t), str(r["wrapped"]), has_old, in_cat, ans))

    if '--json' in argv:
        out = argv[argv.index('--json') + 1]
        cat = load_catalogue(cat_path) if os.path.exists(cat_path) else {}
        with open(out, 'w') as fh:
            json.dump({"old_msgids": old_ids, "old_nonliteral": nonlits,
                       "new_census": census, "catalogue_entries": len(cat)}, fh, indent=1)
        print("\nwrote %s" % out)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
