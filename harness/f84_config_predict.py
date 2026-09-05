#!/usr/bin/env python3
"""F84 -- predict, from source alone, what a VERSION BUMP does to a config.ini.

INTENT Sec.5.112: `main.cpp:252` -> `CheckConfig::checkConfigIni` returns
immediately iff the file's `main:version` equals the build's; otherwise it
populates the schema, runs checkMigration2020 / checkUselessSection /
checkUselessKey, stamps the version and re-saves through the
comment-destroying, key-lowercasing writer of Sec.5.42.

This script extracts the schema MECHANICALLY from the two source files -- no
hand-typed key list -- and reports what the rewrite will do to a given
config.ini.  It is written to be run BEFORE the launch that measures it, and
its output committed first, so the measurement has something that could refute
it.

    ./f84_config_predict.py <config.ini> [--src /home/claude/spacecrafter]

Order of operations, taken from checkConfig.cpp:493-524 and reproduced here:
  1. each check*Settings() pushes its section into sectionSettings and its
     keys into sectionKeySettings, and insertKeyFromTmpSettings ADDS any
     schema key the file lacks (checkConfig.cpp:542-550) -- additions happen
     BEFORE any deletion;
  2. checkMigration2020 divides astro:milky_way_fader_duration by 1000 if it
     exceeds 1000 (:531-539);
  3. checkUselessSection deletes every key of an UNKNOWN section, then the
     section (:553-574);
  4. checkUselessKey deletes every key of a KNOWN section that is not in
     sectionKeySettings (:577-611);
  5. main:version is stamped and the file saved (:523-524) -- iniparser_dump_ini
     re-emits `%-30s = %s`, so comments die and strlwc lowercases every section
     and key (Sec.5.42).
"""
import re
import sys
from pathlib import Path


def parse_defines(hpp: Path):
    """#define SCK_FOO "bar" / #define SCS_FOO "bar"  ->  {macro: value}"""
    out = {}
    pat = re.compile(r'^\s*#define\s+(SC[KS]_[A-Za-z0-9_]+)\s+"((?:[^"\\]|\\.)*)"')
    for line in hpp.read_text(encoding="utf-8", errors="surrogateescape").splitlines():
        m = pat.match(line)
        if m:
            out[m.group(1)] = m.group(2)
    return out


def parse_schema(cpp: Path, defs):
    """-> (sections:list[str], section_keys:set[str], defaults:dict[str,str])

    Walks checkConfig.cpp in source order, honouring the same structure the
    code has: tmpSettings[...] accumulates, insertKeyFromTmpSettings(SCS_X)
    flushes it into sectionKeySettings under section X.  Commented-out lines
    are skipped, which is what makes the extraction faithful (several keys in
    this file are commented out and must NOT count).
    """
    sections, keys, defaults = [], set(), {}
    pending = {}
    ctor_pat = re.compile(r'sectionKeySettings\.push_back\("([^"]+)"\)')
    tmp_pat = re.compile(r'^\s*tmpSettings\[\s*(SCK_[A-Za-z0-9_]+)\s*\]\s*=\s*"((?:[^"\\]|\\.)*)"')
    sec_pat = re.compile(r'sectionSettings\.push_back\(\s*(SCS_[A-Za-z0-9_]+)\s*\)')
    ins_pat = re.compile(r'insertKeyFromTmpSettings\(\s*(SCS_[A-Za-z0-9_]+)\s*\)')
    unknown = []
    for line in cpp.read_text(encoding="utf-8", errors="surrogateescape").splitlines():
        if line.lstrip().startswith("//"):
            continue
        m = ctor_pat.search(line)
        if m:
            keys.add(m.group(1).lower())
        m = tmp_pat.match(line)
        if m:
            macro, val = m.group(1), m.group(2)
            if macro not in defs:
                unknown.append(macro)
            else:
                pending[defs[macro]] = val
            continue
        m = sec_pat.search(line)
        if m:
            macro = m.group(1)
            if macro not in defs:
                unknown.append(macro)
            elif defs[macro].lower() not in sections:
                sections.append(defs[macro].lower())
        m = ins_pat.search(line)
        if m:
            macro = m.group(1)
            if macro not in defs:
                unknown.append(macro)
            else:
                sec = defs[macro].lower()
                for k, v in pending.items():
                    keys.add(f"{sec}:{k.lower()}")
                    defaults[f"{sec}:{k.lower()}"] = v
            pending = {}
    if unknown:
        print(f"WARNING: {len(unknown)} unresolved macros: {sorted(set(unknown))}",
              file=sys.stderr)
    return sections, keys, defaults


def parse_ini(path: Path):
    """-> list of (section, key, value) in file order, plus counters.

    Deliberately NOT configparser: the file is read the way iniparser reads it
    (a `[sec]` line opens a section, `k = v` adds a key), so the census matches
    what the app will see.
    """
    entries, sections = [], []
    cur = None
    ncomment = 0
    for raw in path.read_bytes().decode("latin-1").splitlines():
        s = raw.strip()
        if not s:
            continue
        if s[0] in "#;":
            ncomment += 1
            continue
        if s.startswith("["):
            cur = s.strip("[]").strip()
            sections.append(cur)
            continue
        if "=" in s and cur is not None:
            k, v = s.split("=", 1)
            entries.append((cur, k.strip(), v.strip()))
    return entries, sections, ncomment


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    src = Path("/home/claude/spacecrafter")
    for i, a in enumerate(sys.argv):
        if a == "--src":
            src = Path(sys.argv[i + 1])
    cfg = Path(args[0])

    defs = parse_defines(src / "src/mainModule/define_key.hpp")
    sections, keys, defaults = parse_schema(src / "src/mainModule/checkConfig.cpp", defs)
    entries, file_sections, ncomment = parse_ini(cfg)

    print(f"# f84_config_predict -- {cfg}")
    print(f"# schema extracted from {src}/src/mainModule/checkConfig.cpp")
    print(f"schema.sections            = {len(sections)}")
    print(f"schema.section_keys        = {len(keys)}")
    print(f"file.sections              = {len(file_sections)}")
    print(f"file.keys                  = {len(entries)}")
    print(f"file.comment_lines         = {ncomment}")
    upper = [f"{s}:{k}" for s, k, _ in entries if f"{s}:{k}" != f"{s}:{k}".lower()]
    print(f"file.keys_needing_lowercase= {len(upper)}  {sorted(upper)}")
    print()

    unknown_sec = [s for s in file_sections if s.lower() not in sections]
    print(f"PREDICT unknown sections   = {len(unknown_sec)}  {unknown_sec}")

    del_by_section, del_by_key = [], []
    for s, k, _ in entries:
        sk = f"{s.lower()}:{k.lower()}"
        if s.lower() not in sections:
            del_by_section.append(sk)
        elif sk not in keys:
            del_by_key.append(sk)
    print(f"PREDICT deleted (unknown section) = {len(del_by_section)}")
    for x in del_by_section:
        print(f"    - {x}")
    print(f"PREDICT deleted (known section, key off-schema) = {len(del_by_key)}")
    for x in del_by_key:
        print(f"    - {x}")
    print(f"PREDICT deleted TOTAL = {len(del_by_section) + len(del_by_key)}")
    print()

    present = {f"{s.lower()}:{k.lower()}" for s, k, _ in entries}
    added = sorted(k for k in keys if k not in present and k != "main:version")
    print(f"PREDICT added (schema key the file lacks) = {len(added)}")
    for x in added:
        print(f"    + {x} = {defaults.get(x, '(no default)')}")
    print()

    mw = [v for s, k, v in entries
          if s.lower() == "astro" and k.lower() == "milky_way_fader_duration"]
    if mw:
        try:
            f = float(mw[0])
            print(f"PREDICT checkMigration2020 milky_way_fader_duration = {mw[0]}"
                  f" -> {'DIVIDED by 1000' if f > 1000 else 'UNCHANGED (<= 1000)'}")
        except ValueError:
            print(f"PREDICT checkMigration2020 unparsable value {mw[0]!r}")
    else:
        print("PREDICT checkMigration2020 -- key absent")
    print(f"PREDICT comment lines after = 0 (was {ncomment})")
    net = len(entries) - len(del_by_section) - len(del_by_key) + len(added)
    print(f"PREDICT key count {len(entries)} -> {net}")
    print(f"PREDICT sections {len(file_sections)} -> "
          f"{len(file_sections) - len(unknown_sec)}")


if __name__ == "__main__":
    main()
