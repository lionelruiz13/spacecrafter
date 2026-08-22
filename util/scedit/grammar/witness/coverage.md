# Witness mining — coverage arithmetic

Produced 2026-08-04 for scedit `INTENT.md` §5 item 9, deliverable C. Every number here
is re-derivable from `superscript-witness.json` in this directory; the scripts that
produced it re-implement `script.cpp:114` (line classification) and
`AppCommandInterface::parseCommand` (tokenization) rather than approximating them.

Corpus: `doc/superscript.sts`, md5 `f7f6985c141b6f7e93e01df870d4a9ef`, 44 067 bytes,
ISO-8859, **CRLF**
(1407 of 1407 lines end `\r\n`; the CR is why every blank line is dropped by the
script layer's `line[0] != '\r'` test rather than by its `line[0] != 0` test).

---

## 1. Line census — closes at 1407

| class | count | rule |
|---|---:|---|
| comment (first byte `#`) | 475 | `script.cpp:114` |
| blank (first byte CR, or empty) | 368 | `script.cpp:114` |
| command (everything else) | 564 | `script.cpp:114` |
| **total** | **1407** | |

The 564 command lines include the two that were **written as comments** but are
indented (1269, 1271) and therefore execute — see D-INDENT. Counting authorship
rather than execution, the comment layer is **477 lines**.

## 2. Comment-line coverage — closes at 477

Every comment line is consumed by exactly one class.

| class | count | meaning |
|---|---:|---|
| `claim` | 343 | documents a command, a key or a value — attached below |
| `structural.banner` | 50 | a `###…` rule line |
| `structural.section_title` | 45 | a section heading (`# AUDIO:`, `# BODY_TRACE`, …) |
| `commented_out_command` | 23 | a command line kept disabled (`#script action end`, `#domemasters action snapshot`, the six `#external_mplayer …`, …) |
| `data_or_block_example` | 9 | example *file* content, not script: the `personal`/`personeq` point files (`#2`, `#AD1 DE1`, `#-0.068 -1.570`, …) and `#(...)` inside the `struct` example |
| `structural.file_header` | 5 | lines 2, 4, 6, 8, 9: title, copyright, licence, LSS URL (lines 1, 3, 5, 7, 10 are the `###` banners around them) |
| `claim.indented_hash_executed` | 2 | 1269, 1271 — written as comments, executed as commands |
| **total** | **477** | |

**Unclassified: 0.** **Unattached claims: 0.**

Attachment, over all 477 lines:

| attachment | count |
|---|---:|
| to the next EXECUTED command line | 386 |
| to the next COMMENTED-OUT command line | 31 |
| no target (a banner or a section title closes the run first) | 60 |

Per class: **343/343 `claim` lines are attached** — the number the Definition of Done
turns on — as are 9/9 data examples and 19/23 disabled-command lines. The 60 unattached
break down as: 54 structural (of the 100 banner / section-title / file-header lines, 46
happen to sit directly before a target and carry it as a forward anchor, 54 do not),
4 disabled-command lines that close a section with nothing after them, and the 2
indented-hash lines, which are their own target. 54 + 4 + 2 = 60.

Attachment is *forward* — the file's own convention is a comment run above the line it
documents — plus two hand-curated layers, because a mechanical rule cannot see which
KEY a sentence is about:

- **62 claim lines carry a curated per-key attachment** (`attached_to_keys_curated`):
  the lines that answer a doc gap for a specific key — `image.alpha`, `image.scale`,
  `image.xpos`/`ypos`, `landscape.limited_shade`, `set.mode`, `set.heading`,
  `set.stall_radius_unit`, `heading.azimuth`, `moveto.lat/lon/alt`,
  `configuration.mode`, `personal.filename`, `deselect.constellation`,
  `select.pointer`, `wait.*`, `text.align/fader`, `timerate.*`, and the rest;
- **5 claim lines carry a curated FAMILY attachment** (`attached_to_family_curated`):
  1345-1349 document `families.reserved_variables`, 360 documents the flag value
  grammar — neither is about the command that follows it;
- **65 claim lines** additionally carry an automatic key detection
  (`keys_named_in_text`): the key's own spelling occurs in the sentence.

Claims per attached command, largest first: `flag` 93, `set` 39, `image` 30, `body` 17,
`date` 12, `media` 12, `audio` 11, `deselect` 9, `camera` 8, `struct` 8, `landscape` 7,
`dso` 7, then the tail. 45 sections.

## 3. Usage extraction

| quantity | value |
|---|---:|
| command lines tokenized | 564 |
| key/value pairs written | 1319 |
| distinct (command, key) | 412 |
| distinct (command, key, value) | 815 |
| distinct command spellings written | 52 |

Every distinct value is stored with its line list and count
(`superscript-witness.json` → `usage.<command>.keys.<key>.values`), which is the
completion-candidate and default-evidence data the D31 editor asked for.

## 4. Flagged gaps — 27 of 27 visited

`sc-grammar.json` carries 62 `flagged` arrays; 15 are non-empty and hold **27** items.
Every one has a `candidate_answers` row with a verdict and a code-cross-check status.

| verdict | count |
|---|---:|
| ANSWERED (a witness claim, code-cross-checked) | 8 |
| partial (values or purpose attested, the asked-for range/unit not) | 2 |
| no-witness-evidence | 17 |
| **total** | **27** |

Code-cross-check status of the 10 answered/partial rows: **consistent 6**,
**divergent 4** (G-BODY-3, G-CAM-2, G-IMG-2, G-SET-1 — each also carries a
`divergences` row).

Of the 17 no-witness-evidence rows, **7 are silent because the command or sub-action is
written nowhere in the file at all** — G-BODY-4 (`body action screenshot`/`dual_dump`),
G-CAM-3 (`camera action rotate`), G-DSO3D-1 (`z_reflection`), G-DSO3D-2
(`dso3d action restart`), G-DSO2D-1 (the whole `dso2d` command), G-MEDIA-2
(`media speed`), G-TRANS-1 (the whole `transition` command); that absence is recorded
in each row as the evidence it is. Three more (G-MEDIA-1, G-MET-1, G-SUN-1) are silent
on the asked question but carry the nearby usage the file DOES have, which is why they
have line anchors. The remaining seven are product decisions (lint severity, unit,
public-surface membership) that no usage document could settle.

Across all 27 rows the code-cross-check status is: **consistent 9, divergent 4,
unverifiable-from-code 14**.

## 5. Divergence table — 25 rows

**In what sense it is complete**, stated exactly, because "complete" is doing work here:

- the **usage layer is exhaustively checked, mechanically**: all 564 command lines, all
  1319 pairs, all 815 distinct (command, key, value) triples — every command against
  the registry, every key against its command's `args` (or against `families.flags` /
  `families.set_names`), every value against the enumerated `values` arrays the grammar
  carries. Every mismatch was then read in source before being written down here;
- the **comment layer is exhaustively READ** (all 477 lines, by hand, in file order)
  and every claim that asserts something CHECKABLE — a default, a range, a unit, a file
  name, a value domain, a key spelling, an encoding table — was cross-checked against
  code or against the installed field data. Claims that are purely descriptive
  (*"# Draw the Milky Way."*, *"# Create a comet"*) were classified and attached but
  **not** individually verified: verifying that `flag milky_way on` draws the Milky Way
  needs a render, and nothing here was run.

Disposition of the 25:

| disposition | count | ids |
|---|---:|---|
| doc-error-at-writing | 8 | D-NBSP, D-STALL, D-INDENT, D-BTHIDE, D-BINN, D-ANCHORSINI, D-FLAGVAL, D-SETHEAD |
| engine-drift | 4 | D-DATEDISP, D-MOVETOCITY, D-TESS, D-IMGTWICE (this one self-labelled legacy by the witness itself) |
| engine defect | 4 | D-VRCASE, D-ANCHOR, D-SATURN, D-GANYMED |
| doc-error + engine defect | 1 | D-CONFMOD |
| unclear (labelled, with the evidence that is missing) | 8 | D-SETDUR, D-SETMODE, D-ZROT, D-SPACECRAFT, D-WAITRESET, D-DSO3D-RESET, D-LOCKSKY, D-XPOS |

**Ten rows re-derive `claude/INTENT.md` §5.97 and §5.97 EXTENDED independently**
(D-NBSP, D-SETDUR, D-DATEDISP, D-MOVETOCITY, D-SETMODE, D-STALL, D-INDENT, D-ZROT,
D-SPACECRAFT, D-WAITRESET — every catalogued line reproduced, none missed, by a
tokenizer written from `parseCommand` rather than by re-reading the ledger; the ten
rows cover all sixteen catalogued line numbers, several rows carrying two or three).

**Fifteen rows are new** — D-TESS, D-IMGTWICE, D-CONFMOD, D-DSO3D-RESET, D-BTHIDE,
D-VRCASE, D-BINN, D-LOCKSKY, D-ANCHOR, D-ANCHORSINI, D-XPOS, D-FLAGVAL, D-SETHEAD,
plus **D-SATURN and D-GANYMED, two engine defects the witness's own documentation
exposed**. 10 + 15 = 25. The new ones come from two layers the earlier corpus runs did
not cover:
the **value domains** (a key can be legal while its value is not) and the **comment
layer** (a sentence can be wrong while every command line is legal).

One correction owed upstream: `§5.97(c)` and `SCRIPT_SURFACE SS-6` both quote line
1205 as `set stall_radius_unit 5.0`. The file's bytes read
`set stall_radius_unit = 5.0` — with a stray `=` that becomes the value, and `5.0` a
dangling key the parser drops (D-STALL).

## 6. Commands: written vs registered

| set | count |
|---|---:|
| registered entries in `families.commands` | 62 |
| written by the witness AND registered | 50 |
| **registered but NEVER executed by the witness** | **12** |
| written but not registered | 2 (`movetocity`, obsolete; `#`, the two indented comment lines) |
| appearing only in the comment layer, commented out | `script` action/speed forms (11 lines), `external_mplayer` (obsolete, 6), `domemasters` (3), `shutdown` (1), `configuration action save` (1), `set date_display_format` (1) — 23 lines |

**The never-exercised twelve** — the file that was "the only functional test" never
tested any of these:

`domemasters` · `dso2d` · `flyto` · `galaxy_stars` · `get` · `modulo` · `search` ·
`session` · `shutdown` · `sub` · `suntrace` · `transition`

Two of the twelve (`domemasters`, `shutdown`) are *taught* in the comment layer as
commented-out lines the reader is told to enable (*"hash has to be removed"*), so they
are documented-but-untested. The other ten appear nowhere at all. Cross-referenced
with the second witness (`new-parser-scripts.md` §4.5), **nine commands have no
documentation anywhere in the tree except the code**: `dso2d`, `flyto`,
`galaxy_stars`, `get`, `modulo`, `search`, `session`, `sub`, `transition`.

`flyto` deserves its own line: it is an ALIAS of `camera`, and the recorder cannot
round-trip it (`parse_model.recording_alias_loss`). Neither witness ever writes it.

## 7. What this pass did NOT establish

- Nothing was **measured**: no engine was run. Every cross-check is a read of source,
  of the merged grammar, or of the installed field data, each anchored by file:line.
- The dome/horizontal reading of `image scale` (*"the angular width of the image in
  degrees"*, :656/:693) is carried as `[superscript-attested]` and was **not**
  re-derived from the dome draw path — the viewport reading was.
- The heading convention (*"0 = South | 90 = East…"*, :616) is
  **unverifiable-from-code at this depth**: `Navigator::changeHeading` stores a roll in
  degrees and names no compass point. It is recorded with its in-file corroboration
  (:1159-1160) and the contrast against the dome-image convention (:692), not resolved.
- The 87 constellation abbreviations (:1090-1094) could not be checked against a sky
  culture: `~/.spacecrafter/sky_cultures/` is EMPTY on this laptop (the data package is
  not installed). The file the witness names is confirmed to be the file the engine
  reads (core.cpp:1437, :1472); its contents are not.

## 8. For the supervisor — what is here and how it merges

Four files, all in `util/scedit/grammar/witness/`. Nothing outside this directory was
written; no commit was made.

| file | what it is |
|---|---|
| `superscript-witness.json` | deliverable A: 477 comment claims (byte-verbatim, line-anchored, attached), the full usage table, 27 candidate answers, 25 divergences, the never-exercised set |
| `new-parser-scripts.md` | deliverable B: identification of `util/new_parser_scripts/`, witness-tier verdict, cross-check findings |
| `new-parser-witness.json` | the structured extraction backing B (37 blocks, 256 slots, 141 values, 471 description lines, 0 unparsed) |
| `coverage.md` | this file: the arithmetic |

Merge notes:

1. **Nothing here should be folded into `sc-grammar.json` unchanged.** A witness claim
   becomes a grammar `doc`/`values`/`default` only where this record marks the
   code-cross-check `consistent`; the six such rows are G-CONST-1, G-DATE-1, G-IMG-1,
   G-LAND-1, G-SET-2, G-SET-3 (plus G-SET-4 and G-BODY-3 as `partial`). The four
   `divergent` rows must not be merged as documentation — they are findings.
2. **`§5.97` and `SCRIPT_SURFACE SS-6` carry a wrong quote of line 1205** (the `=` is
   missing). Correcting it is an upstream edit in the parent ledger, which is
   READ-ONLY for this task — recorded here, not done.
3. **Two new engine defects** (D-SATURN, D-GANYMED) belong in the parent ledger's §5,
   not in scedit's. They are unreachable branches of `Core::setSelectedBodyName`
   (core.cpp:2173, :2177) caused by two misspelled body names, and they make
   `$body_selected` return 999 for Saturn and for Ganymede — which is what a script
   written from `superscript.sts:1349` tests against.
4. **Three lines of the reference script re-initialise the app or overwrite
   config.ini** (D-CONFMOD, lines 249/251/252). If the file is ever run end to end as
   a conformance corpus again, that must be known first.
5. **Suspended for Vixy, one item**: S-NP-1 in `new-parser-scripts.md` §2 — whether
   `util/new_parser_scripts/input_fr.txt` joins C2's anchor classes as a second
   usage-witness, and with what reliability statement. Everything extracted from it is
   tagged `[new-parser-fr-attested]` and kept in its own file until that is answered.
6. **New material for `SCRIPT_SURFACE.md` §3** (the pending divergence table): the 25
   rows of `superscript-witness.json.divergences`, and three answers to questions
   already open there — SS-2, SS-4 and SS-10 each get a second, independent
   attestation from `input_fr.txt` (`set … duration`, `moveto … name`,
   `wait action reset_timer`), and SS-11 gets the 2020 statement that `sun_trace` was
   *"un alias de la commande body_trace avec le soleil comme astre sélectionné"*.
