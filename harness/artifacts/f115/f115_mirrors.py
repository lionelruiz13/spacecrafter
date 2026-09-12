#!/usr/bin/env python3
"""F115 - I2 guard on the three declarations f115_sizes.cpp cannot include.

f115_sizes.cpp includes the REAL headers for every old- and new-path UBO struct
except three, whose own headers pull SharedBuffer.hpp -> BufferMgr.hpp -> vulkan
and so cannot be compiled standalone:

    Ring::RingUniform          src/bodyModule/ring.hpp
    AtmExt::_uniform           src/bodyModule/atm_ext.hpp
      (== AtmExtModule::atmExtUBO, src/experimentalModule/bodyModules/AtmExtModule.hpp
       - its own comment calls itself "exact copy of the old AtmExt::_uniform",
       so BOTH are checked against the same mirror: if they ever diverge, this
       fails, which is the point)
    OjmContainer::uniformData  src/ojmModule/ojm_mgr.hpp

The header is the authority; the mirror in f115_sizes.cpp is a checked copy.
This script re-greps each struct body out of its header, normalises whitespace
and comments, and compares the FIELD LIST (type + name, in order) with the
mirror's. Any divergence is a hard failure: the sizes printed by the instrument
would then be measuring a struct the engine no longer has.

Run (from anywhere):  python3 f115_mirrors.py [--self-test]
--self-test mutates one mirror field in memory and asserts the check FAILS.
Exit 0 = every mirror agrees with its header.
"""
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", ".."))
SRC = os.path.join(ROOT, "src")
MIRROR_SRC = os.path.join(os.path.dirname(os.path.abspath(__file__)), "f115_sizes.cpp")

# struct-in-header -> (relative header path, the C++ name of the struct in the header,
#                      the name of the mirror struct in f115_sizes.cpp)
TARGETS = [
    ("bodyModule/ring.hpp", "RingUniform", "RingUniform"),
    ("bodyModule/atm_ext.hpp", "_uniform", "atmExtUBO"),
    ("experimentalModule/bodyModules/AtmExtModule.hpp", "atmExtUBO", "atmExtUBO"),
    ("ojmModule/ojm_mgr.hpp", "uniformData", "ojmContainerUniformData"),
]


def read(path):
    with open(path, "rb") as f:
        return f.read().decode("utf-8", errors="replace")


def extract_body(text, name):
    """Return the brace body of `struct <name> { ... }`, comments stripped."""
    m = re.search(r"struct\s+%s\s*\{" % re.escape(name), text)
    if not m:
        return None
    i = m.end()
    depth = 1
    start = i
    while i < len(text) and depth:
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
        i += 1
    return text[start:i - 1]


def fields(body):
    """(type, name) pairs, in declaration order. Comments and blank lines out."""
    body = re.sub(r"//[^\n]*", "", body)
    body = re.sub(r"/\*.*?\*/", "", body, flags=re.S)
    out = []
    for decl in body.split(";"):
        decl = " ".join(decl.split())
        if not decl:
            continue
        parts = decl.split()
        if len(parts) < 2:
            continue
        typ = parts[0]
        for nm in " ".join(parts[1:]).split(","):
            nm = nm.strip()
            if nm:
                out.append((typ, nm))
    return out


def main():
    selftest = "--self-test" in sys.argv
    mirror_text = read(MIRROR_SRC)
    rc = 0
    seen = 0
    for rel, hdr_name, mirror_name in TARGETS:
        hdr = os.path.join(SRC, rel)
        hbody = extract_body(read(hdr), hdr_name)
        mbody = extract_body(mirror_text, mirror_name)
        if hbody is None:
            print("FAIL  %-52s struct %s not found in the header" % (rel, hdr_name))
            rc = 1
            continue
        if mbody is None:
            print("FAIL  %-52s mirror struct %s not found in f115_sizes.cpp"
                  % (rel, mirror_name))
            rc = 1
            continue
        hf, mf = fields(hbody), fields(mbody)
        if selftest and seen == 0:
            mf = list(mf)
            mf[0] = ("MUTATED", mf[0][1])   # the check must notice this
        seen += 1
        if hf == mf:
            print("OK    %-52s %-18s %2d fields" % (rel, hdr_name, len(hf)))
        else:
            rc = 1
            print("FAIL  %-52s %s" % (rel, hdr_name))
            print("      header: %s" % hf)
            print("      mirror: %s" % mf)
    if selftest:
        if rc:
            print("SELF-TEST OK: the mutated mirror was rejected (exit would be 1)")
            return 0
        print("SELF-TEST FAILED: a mutated mirror passed")
        return 1
    print("f115_mirrors: %d mirror(s) checked, %s" % (seen, "all agree" if rc == 0 else "DIVERGENCE"))
    return rc


if __name__ == "__main__":
    sys.exit(main())
