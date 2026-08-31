#!/usr/bin/env python3
"""f70_dispositions.py - the per-literal disposition list D14's sweep is gated on.

Scope item 3 of task F70: before a single string literal is touched, every
non-ASCII literal in the CONVERT set is listed WITH ITS TRACED CONSUMER, because
a literal is not prose - somebody reads its bytes.

The traces below are hand-made and each names where it was read.  The generator
only joins them to the mechanical census; it does not invent a trace.  A file
that appears in the census with no trace row is an ERROR, not a default: a
literal nobody traced is exactly the one that breaks something.

DISPOSITIONS, and there are only two:

  ESCAPE        C-family source.  The characters are re-spelled as \\xNN escapes
                of the SAME UTF-8 bytes.  The compiled program's output does not
                move, so no consumer has to be adjudicated at all - which is why
                every risky class below lands here.  Precedent in this tree:
                sc_tui.cpp:640 and :1215 already do this by hand.
  TRANSLITERATE Not C-family (JSON / Python / CMake / text), where no byte-exact
                escape exists.  Applied only where the trace shows the bytes are
                DOCUMENTATION, and where any pinned expectation of them is itself
                in the CONVERT set and therefore moves with them.
"""

import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import f70_ascii as A                                            # noqa: E402

# path -> (consumer class, traced-at)
TRACES = {
 "src/coreModule/skyline.cpp": (
    "DRAWN IN THE DOME: degree signs concatenated into the tick labels of the "
    "ecliptic/meridian/equator lines",
    "observed: skyline.cpp:318,795,853,971,973,1024,1026,1031,1336,1522 - "
    "`oss << <value> << \"<degree>\"` feeding the line's label string"),
 "src/coreModule/skyDisplay.cpp": (
    "DRAWN IN THE DOME: the alt/az and declination readouts",
    "observed: skyDisplay.cpp:213,472,509,574,647,687,758,857"),
 "src/coreModule/skygrid.cpp": (
    "DRAWN IN THE DOME: grid meridian/parallel labels",
    "observed: skygrid.cpp:277,281,283,305 - sprintf into the label buffer"),
 "src/tools/utility.cpp": (
    "DRAWN IN THE DOME: printAngleDMS's degree separator, the shared "
    "angle-formatting helper",
    "observed: utility.cpp:260 `std::string degsign = \"<degree>\";`, emitted at "
    ":283 and :306"),
 "src/ojmModule/ojm.cpp": (
    "STDOUT diagnostic of the mesh loader",
    "observed: ojm.cpp:516 `std::cout<< \"***** shapes n<degree>\"`"),
 "src/inGalaxyModule/starManager.cpp": (
    "STDOUT histogram of the star-catalogue statistics (a French range 'a')",
    "observed: starManager.cpp:819,848"),
 "src/experimentalModule/SessionFile.cpp": (
    "LOG text quoting b31-design section numbers",
    "observed: SessionFile.cpp:249,268,269,274,276,290,385"),
 "src/bodyModule/ssystem_factory.cpp": (
    "LOG text (cLog) quoting a section number",
    "observed: ssystem_factory.cpp:551,598 `cLog::get()->write(\"B5 <S>6.9 pilot: ...\")`"),
 "src/interfaceModule/app_command_interface.cpp": (
    "USER-VISIBLE REFUSAL that also reaches the TCP wire: `debug_message` is "
    "logged AND, since 11.188, sent to a $DIAG subscriber as a $DIAG record. "
    "Two French messages containing 'etre' with a circumflex",
    "observed: app_command_interface.cpp:3977,4008; the $DIAG path at 11.188(b). "
    "CATALOGUE CHECKED: `_()` is not gettext here - Translator::translateUTF8 is "
    "an exact-match lookup in <localeDir>/<lang>.txt with identity fallback "
    "[observed: src/tools/translator.cpp:80-119,156-163], and a census of all 71 "
    "installed catalogues found ZERO keys carrying a non-ASCII byte, these two "
    "msgids included [measured 2026-08-31] - so the lookup misses today and would "
    "miss identically after any change"),
 "util/scedit/src/sc_check.cpp": (
    "LINT MESSAGES, pinned byte-for-byte by tests/lint-expected.txt, "
    "history-expected.txt and check-json-expected.txt",
    "observed: sc_check.cpp:528,537,562,567,638,648 vs tests/lint-expected.txt:23,25,27"),
 "util/scedit/src/sc_docindex.cpp": (
    "THE 'no documentation extracted' BAR TEXT, pinned by "
    "tests/ui-selftest-expected.txt:11,87 - a file the partition EXCLUDES",
    "observed: sc_docindex.cpp:23 `kNoDoc`"),
 "util/scedit/src/sc_tui.cpp": (
    "STATUS/DOC BAR TEXT, pinned by tests/ui-selftest-expected.txt (EXCLUDED)",
    "observed: sc_tui.cpp:118 'known ones -- there are more' at "
    "ui-selftest-expected.txt:86; :404 the em dash used as the empty-path placeholder"),
 "util/scedit/src/sc_docjson.cpp": (
    "--doc / --json answer text, pinned by tests/doc-expected.txt",
    "observed: sc_docjson.cpp:273"),
 "util/scedit/src/sc_editcore.cpp": (
    "EDITOR ERROR TEXT shown on the status bar",
    "observed: sc_editcore.cpp:278"),
 "util/scedit/src/sc_mcp.cpp": (
    "MCP TOOL DESCRIPTIONS - the text a model reads over the protocol; "
    "tests/mcp_gate.py asserts on it",
    "observed: sc_mcp.cpp:42,286,310,335"),
 "util/scedit/src/sc_tcpclient.cpp": (
    "CONNECTION-FAILURE TEXT shown in the editor",
    "observed: sc_tcpclient.cpp:154"),
 "util/scedit/tests/editcore_test.cpp": (
    "TEST ASSERTION LABELS (the message printed when a check fails)",
    "observed: editcore_test.cpp:404,872,908"),
 "util/scedit/tests/tcpclient_test.cpp": (
    "TEST ASSERTION LABELS",
    "observed: tcpclient_test.cpp"),
 "util/src_converter/obj3D.cpp": (
    "STDOUT diagnostics of the obj converter (French)",
    "observed: obj3D.cpp"),
 "util/src_converter/obj_to_ojm.cpp": (
    "STDOUT diagnostics of the obj converter (French)", "observed: obj_to_ojm.cpp"),
 "util/src_converter/extract_sphere.cpp": (
    "STDOUT diagnostics (French)", "observed: extract_sphere.cpp"),
 "util/src_ojmviewer/ojm.cpp": (
    "STDOUT diagnostics of the ojm viewer", "observed: ojm.cpp"),
 "util/new_parser_scripts/TextToHtml.cpp": (
    "GENERATED HTML BODY TEXT ('Parametre' with a grave accent) - the tool writes "
    "it into the documentation page it produces, and util/new_parser_scripts/"
    "resultat.html is a checked-in sample of that output which the partition EXCLUDES",
    "observed: TextToHtml.cpp:189"),
 "util/Atmosphere1/CMakeLists.txt": (
    "CMAKE CONFIGURE-TIME BANNER carrying an author's name",
    "observed: util/Atmosphere1/CMakeLists.txt:12 `message(\" Project Earth by "
    "Lartillot Jerome\")`"),
 "util/scedit/grammar/sc-grammar.json": (
    "GRAMMAR DOCUMENTATION PROSE that scedit DISPLAYS; the only byte-exact "
    "pins of it are tests/doc-expected.txt and tests/check-json-expected.txt, "
    "both themselves in the CONVERT set, so pin and pinned move together and "
    "ctest is the proof",
    "observed: sc-grammar.json doc/notes fields; ui-selftest-expected.txt "
    "(EXCLUDED) was checked and its 137 non-box characters come from C++ "
    "literals and already-escaped bytes, not from grammar prose"),
 "util/scedit/grammar/args/unit-1.json": ("GRAMMAR DOCUMENTATION PROSE (as above)",
                                          "observed: unit-1.json"),
 "util/scedit/grammar/args/unit-2.json": ("GRAMMAR DOCUMENTATION PROSE (as above)",
                                          "observed: unit-2.json"),
 "util/scedit/grammar/args/unit-3.json": ("GRAMMAR DOCUMENTATION PROSE (as above)",
                                          "observed: unit-3.json"),
 "util/scedit/grammar/args/unit-4.json": ("GRAMMAR DOCUMENTATION PROSE (as above)",
                                          "observed: unit-4.json"),
 "util/scedit/tests/fixture-grammar.json": (
    "FIXTURE GRAMMAR PROSE - a `why_it_exists` note, read by no assertion",
    "observed: fixture-grammar.json:4"),
 "util/scedit/tests/fake_engine.py": ("PYTHON DOCSTRING / helper prose",
                                      "observed: fake_engine.py:3,30,35,130"),
 "util/scedit/tests/mcp_gate.py": ("PYTHON DOCSTRING and check() failure labels",
                                   "observed: mcp_gate.py:7,121,152,179,228"),
 "util/scedit/tests/pty_gate.py": ("PYTHON DOCSTRING", "observed: pty_gate.py:8,9,18"),
 "util/scedit/tests/tcp_gate.py": ("PYTHON DOCSTRING", "observed: tcp_gate.py:7,9"),
}


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else "/home/claude/spacecrafter"
    part = A.read_partition(A.DEFAULT_PARTITION)
    rows = []
    missing = set()
    for path in A.tracked_files(root):
        d = A.decide(part, path)
        if not d or d[0] != "CONVERT":
            continue
        data = open(os.path.join(root, path), "rb").read()
        if A.classify_encoding(data) in ("ascii", "binary"):
            continue
        text = A.decode(data)
        ctx = A.contexts(path, text)
        line = 1
        hits = {}
        for i, ch in enumerate(text):
            if ch == "\n":
                line += 1
                continue
            if ord(ch) < 0x80 or ctx[i] not in ("string", "char"):
                continue
            hits.setdefault(line, set()).add(ch)
        if not hits:
            continue
        if path not in TRACES:
            missing.add(path)
            continue
        consumer, at = TRACES[path]
        disp = "ESCAPE" if A.family(path) == "c" else "TRANSLITERATE"
        lines = text.split("\n")
        for ln in sorted(hits):
            chars = " ".join("U+%04X" % ord(c) for c in sorted(hits[ln]))
            src = lines[ln - 1].strip()
            src = src.encode("ascii", "backslashreplace").decode()
            rows.append((path, ln, chars, disp, consumer, at, src[:160]))
    if missing:
        print("ERROR: literal-bearing CONVERT file(s) with no trace row: %s"
              % ", ".join(sorted(missing)), file=sys.stderr)
        return 1
    print("\t".join(["path", "line", "chars", "disposition", "consumer_class",
                     "traced_at", "source_line"]))
    for r in rows:
        print("\t".join(str(x) for x in r))
    print("# %d literal-bearing lines across %d files; ESCAPE %d, TRANSLITERATE %d"
          % (len(rows), len({r[0] for r in rows}),
             sum(1 for r in rows if r[3] == "ESCAPE"),
             sum(1 for r in rows if r[3] == "TRANSLITERATE")), file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
