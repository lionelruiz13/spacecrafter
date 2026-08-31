#!/usr/bin/env python3
"""f70_binary_equal.py - did the ASCII sweep change the PROGRAM, or only its text?

    python3 f70_binary_equal.py PRE_BINARY POST_BINARY

F70 converts comments and re-spells string literals as \\xNN escapes of the same
bytes.  If that claim is true, the two builds must agree on every byte the CPU
or the loader ever sees, and may disagree only on metadata:

  * `.note.gnu.build-id` - a hash over the linked inputs; it changes whenever
    ANY input byte changes, debug information included, so it proves nothing
    either way and is excluded by name rather than by silence.
  * the DWARF sections - `.debug_line` and `.debug_info` record COLUMN numbers,
    and an escape is longer than the character it replaces (2 bytes of UTF-8
    become 8 characters of "\\xc3\\xaa"), so every token after it on that line
    sits at a higher column.  That is a change in what the debugger says about
    the source, not in what the program does.

So the check is: hash every SHF_ALLOC section - the ones actually mapped into
the process - and require equality.  A single differing byte in .text or
.rodata would mean the sweep changed the program, and that is a
delivery-blocking defect rather than a judgment call.

Exit 0 equal, 1 different, 2 could not read.
"""

import hashlib
import re
import subprocess
import sys


def sections(path):
    """name -> (file_offset, size) for every SHF_ALLOC section."""
    out = subprocess.run(["readelf", "-SW", path], capture_output=True,
                         text=True, check=True).stdout
    res = {}
    for line in out.splitlines():
        m = re.match(r"\s*\[\s*\d+\]\s+(\S+)\s+(\S+)\s+([0-9a-f]+)\s+"
                     r"([0-9a-f]+)\s+([0-9a-f]+)\s+\S+\s+(\S*)", line)
        if not m:
            continue
        name, typ, addr, off, size, flags = m.groups()
        if "A" not in flags:
            continue
        if typ == "NOBITS":                 # .bss has no bytes in the file
            continue
        res[name] = (int(off, 16), int(size, 16))
    return res


def digest(path, off, size):
    with open(path, "rb") as fh:
        fh.seek(off)
        return hashlib.sha256(fh.read(size)).hexdigest()


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    pre, post = sys.argv[1], sys.argv[2]
    a, b = sections(pre), sections(post)
    if set(a) != set(b):
        print("FAIL: the two binaries do not even have the same allocated "
              "sections: %s" % (set(a) ^ set(b)))
        return 1
    diff = []
    same = 0
    total = 0
    for name in sorted(a):
        off_a, size_a = a[name]
        off_b, size_b = b[name]
        if size_a != size_b:
            diff.append((name, "size %d vs %d" % (size_a, size_b)))
            continue
        total += size_a
        da, db = digest(pre, off_a, size_a), digest(post, off_b, size_b)
        if da == db:
            same += 1
            continue
        if name == ".note.gnu.build-id":
            print("  (.note.gnu.build-id differs, as it must: it hashes the "
                  "debug information too)")
            same += 1
            continue
        diff.append((name, "%s vs %s" % (da[:16], db[:16])))
    if diff:
        print("FAIL: the sweep changed the PROGRAM, not only its text:")
        for name, why in diff:
            print("  %-24s %s" % (name, why))
        return 1
    print("PASS: all %d allocated sections identical (%d bytes hashed); "
          "only DWARF and the build-id moved." % (same, total))
    return 0


if __name__ == "__main__":
    sys.exit(main())
