# scedit — script & stellar-system-file editor for spacecrafter

## What it is for

A spacecrafter show is a `.sts` script: one command per line, each command a
word followed by key/value pairs. The engine reads such a line with a parser
that has several sharp edges and **no error reporting at all** — a misspelt flag
is a silent no-op, a trailing key with no value is silently dropped, a value the
engine does not recognise silently means "off". An author finds out in the dome.

scedit exists to move that discovery to the desk. It reads a script *exactly*
the way the engine will, and reports every place where what the engine will do
differs from what the author plainly meant — with the engine's own source line
named in each message. Around that core it grows into the editor Vixy asked for
(`claude/FEATURE_REQUESTS.md` [2026-07-29]): a mouse-driven TUI, autocomplete
with ghost-text preview, and a documentation line for the command and for the
key under the cursor, so that *someone with no knowledge of scripting can
understand and modify any script through this editor*.

Second artefact class, same tool: stellar-system data files (`ssystem.ini` and
the composed new-format files).

**One rule runs ahead of the engine, by ruling.** A `#` outside a `"…"` run
starts a comment that runs to the end of the line — an indented `#` is a
whole-line comment, a `#` inside quotes is text. Vixy ruled the behaviour
(2026-08-30) and the order (2026-08-31): scedit models the corrected engine
first, and spacecrafter is brought into phase with the identical parser code.
Until that engine commit lands, this is the one place where "read exactly the
way the engine will" means the *ruled* engine (`parse_model.comments.mid_line`).

**Name note:** "TUI" inside spacecrafter means the in-app dome text menu
(`ui_tuiconf`, channel 8 in `claude/capability-surface.md`). This tool is
*scedit* everywhere, to keep the two unambiguous.

## Build

Standalone, deliberately not wired into the spacecrafter build (the `util/`
sibling pattern). C++17, no system dependency: everything it needs is vendored
under `third_party/` (see "Vendoring").

    cd util/scedit
    cmake -B build && cmake --build build
    cd build && ctest --output-on-failure     # 8 gates, see "Verification"

## Use

    scedit [--grammar FILE] [--list FAMILY]          # default: validate the contract
    scedit [--grammar FILE] [--rules] --check FILE...
    scedit [--grammar FILE] [--edit] FILE            # the editor
    scedit [--grammar FILE] --ui-selftest            # render fixed frames, no tty

`--grammar FILE` — the contract to read. Default `grammar/sc-grammar.json`,
resolved relative to the working directory; when that fails, and only when the
default was not overridden, it is looked for beside the binary as well, so
`scedit some/show.sts` works from wherever the scripts live. A path given
explicitly is never second-guessed: if it does not exist, that is an error.

**default action — validate the contract.** Re-derives every family count from
the data and compares it against `_meta.expected_counts`, checks in-family
uniqueness, subfamily links, per-entry completeness (`registration`, `doc`,
`args_complete`), the argument-token count and roles, and lint-id uniqueness.
This is the *seed gate*: it catches an accidental edit of the contract file, not
a divergence from the engine (that is the extraction passes' job).

**`--list FAMILY`** — print one family, one name per line, for shell use.
Families: `commands`, `flags`, `set_names`, `color_names`, `obsolete_tokens`,
`reserved_variables`, `font_targets`.

**`--check FILE...`** — analyse scripts. Diagnostics go to **stdout** (they are
the product); tool failures go to stderr. Output is gcc-shaped, per decision D6:

    file:line: severity: message [-Wid]

Every id is a name from the contract file's `lint_seeds`, and so is its
severity — retuning a severity is a data edit, not a code change. Findings are
printed in line order; two of them are only known at the end of the file and
are reported where the fault IS, not where it surfaces: a `struct if` or
`struct loop` never closed is reported **at its opener**. Example:

    doc/superscript.sts:94: error: column 433 holds a no-break space, the ISO-8859
    spelling (byte 0xA0), not a space: the engine separates words on space, tab, CR,
    LF, VT and FF only, so what is written on either side of this byte is read as ONE
    word [-Winvisible-separator]

**`--rules`** — with `--check`, first print the rules this build does **not**
emit, and why. An unarmed rule is visible rather than silent: a check that
cannot be grounded in the contract at zero false positives is not armed at all,
and this is where you see which ones and what they are waiting for.

## The editor

    scedit doc/superscript.sts

An editor whose whole purpose is that *you do not have to know the scripting
language to change a show*. The line under your caret is explained as you move
through it, and what can be completed is shown before you press anything.

### Keys

| key | what it does |
|---|---|
| arrows, Home, End, PageUp/PageDown | move the caret (columns are BYTES, see below) |
| any character | insert it; a character the file cannot hold is refused, with a message |
| Enter | split the line — the new line ending is the one this file already uses |
| Backspace / Delete | remove the byte before / under the caret; at a line edge, join |
| **Tab** | insert the grey text; if it is already typed in full, show the next candidate |
| Shift-Tab | show the previous candidate |
| Ctrl-S, or F2 | save |
| Ctrl-Q, Esc, or F10 | quit; with unsaved changes, once to warn and again to discard |

Ctrl-S and Ctrl-Q are the terminal's own flow-control pair, so the editor turns
flow control off while it runs and puts it back on exit. F2 and F10 do the same
two things for terminals where that does not take.

### Mouse

Click to put the caret where you clicked. Wheel to scroll — and the view stays
where you scrolled it: it only chases the caret again when you next press a key.

### The documentation bar

Four lines under the text, driven by where the caret is:

1. **where you are** — `` `set` `star_scale` ``, `` `date` `load` = `current` ``,
   `command `flag`` — and, in brackets, what would complete here and how many
   candidates there are;
2. **the sentence** the contract file holds for exactly that thing, with a note
   saying what it documents (a command, a key, a value, or "any key of
   `<command>`" when the file explains the command's key grammar but has no line
   for this particular name). Where the file has none — `null`, or a family
   whose documentation pass has not run — the bar says **"no documentation
   extracted"** in grey and invents nothing. That is constraint C2 on a screen;
3. **the value domain**: what kind of value the key takes, the values it names
   (verbatim, prose entries included), the default, and whether it is required;
4. **the findings on this line**, in full, with their id — or the engine source
   line the sentence above came from.

In the text itself, every finding is **underlined at exactly the bytes it is
about** (the misspelt word, the dropped key, the whole `struct if` that is
never closed); the look-alike-space byte keeps its red marker. Both come from
the finding's own span — there is no second reading of the bytes in the
renderer. A comment — from its `#` to the end of the line — is drawn dim, like
the ghost: text the engine does not read. With the caret inside one, the bar
says `comment` and nothing completes.

### What the grey text means

Grey text at the caret is **exactly what Tab would insert** — never a hint,
never an example. If several candidates share what you have typed, Tab cycles
and the grey text follows, so the promise stays true. There is grey text
wherever a completion exists: a command name, a key valid for this command, a
name from the family a command draws on (`flag`'s 97 flags, `set`'s 43 settings,
`color property`'s 46 colours), and an enumerated value — including in an empty
value slot, where the first candidate is offered.

A key list that scedit knows to be **partial** says so (`body`, `camera`,
`flyto`: their remaining keys belong to the stellar-system contract, item 4).
Their known keys are still offered, marked "known ones — there are more", and
scedit never calls one of their unlisted keys wrong.

### Bytes, not characters

Script files are ISO-8859 and their bytes are significant — `--check`'s
`invisible-separator` rule exists because 0xA0 in a column changes what the
engine reads. So the editor never decodes the file: the caret moves over BYTES,
the buffer holds bytes, and saving writes the bytes back. A file you open and
save without editing is byte-identical (there is a gate for it); a file you edit
on one line is byte-identical everywhere else.

For the screen only, one byte becomes one cell: printable ASCII as itself,
0x80–0xFF decoded as ISO-8859-1, and everything invisible given a visible
marker — `·` in red for the no-break space 0xA0, a dim `»` for a tab, a dim `?`
for a control byte. A line ending stays a line ending and is not drawn. Typing a
character above U+00FF (from a UTF-8 terminal) is refused rather than written,
because there is no byte for it in this file.

### Exit codes

| code | meaning |
|---|---|
| 0 | clean — no findings (or the validation/listing succeeded) |
| 1 | findings were reported (or the contract failed validation) |
| 2 | usage error, or a file could not be read / parsed |

## The grammar file

`grammar/sc-grammar.json` is the single machine-readable contract for
spacecrafter's command surface. Nothing in the code hard-codes a command, a
flag or a family member: a name that is not in the file does not exist for
scedit.

    _meta                        schema version, provenance, expected counts,
                                 count deltas against the dated census
    parse_model                  how a line becomes (command, args) — including
                                 every sharp edge, each with its engine line
    families.commands            63 registered names (59 canonical + 4 aliases:
                                 flyto, div, mul, mod) + comment/uncomment, each with a
                                 doc line, its argument keys and their specs
    families.flags / set_names / color_names / obsolete_tokens /
    reserved_variables / font_targets
    argument_token_vocabulary    the 238 W_* spellings, with the role each plays
    lint_seeds                   diagnostic id, severity, rule, engine source

An **alias** entry (`flyto`, `div`, `mul`, `mod`) carries `alias_of` and its
own `doc`/`registration`, and nothing else: the engine registers a second name
on the same handler, so the keys and every claim about them are the canonical
entry's, held once. Both readers resolve an alias ONCE, at load
(`Grammar::load`, `DocIndex::load`), and the validator refuses an alias that
names a missing or chained target or carries keys of its own. A recording
keeps the spelling the author typed (the engine's enum-to-name map has no
consumer — `parse_model.recording_alias_loss`), so an alias is not a finding.

Two shapes coexist in the families on purpose. A family whose documentation
pass has run carries objects (`[{name, doc, value, default, required, source}]`
— `set_names` today, per decision D7); one whose pass has not run carries plain
names, and says so in its own `_schema_note`. Consumers must accept both.

Per-command, the field that governs `--check`'s argument vocabulary is
**`args_complete`**. It answers one question — *is the key list here the WHOLE
vocabulary this command accepts?* — because "I listed some keys" and "I listed
all of them" are different claims and only the second licenses calling a key
unknown. Four handlers hand their whole parsed map to another module; two of
them (`dso3d`, `landscape`) have that module's keys extracted and answer `true`,
two (`body`, `camera`, and `flyto` which is `camera`) answer `false` because
their downstream vocabulary is the stellar-system contract's to state. Those
last three are silent on their keys **by construction, not by omission**, and
`--rules` says so.

### Authority chain

**Until the engine emits this file, SOURCE WINS on any divergence:**

1. `src/interfaceModule/app_command_init.cpp` — registration (which names exist)
2. `src/interfaceModule/base_command_interface.hpp` — token spellings
3. `src/interfaceModule/app_command_interface.cpp` — the parser and the handlers

Every doc line, value domain and default in the file carries a `file:line`
anchor into one of those, or into the downstream module that actually reads the
key. Nothing is written from recall or inference: where the code cannot answer,
the entry is `null` and the question is listed in that entry's `flagged` array
for Vixy. `UNEXTRACTED`/`null` are the honest states (constraint C2).

The granular source of the per-key data is `grammar/args/unit-{1,2,3,4}.json`,
the four fragments of the per-handler extraction sweep. They stay in the tree
and each carries its own count gate (every `args[` line in its range mapped to
exactly one key); the merged contract cites them in `_meta.merge_provenance`.
**A correction goes into the fragment first, then into the merged file** — and
the seed gate compares the two on every run, so forgetting the second half is a
test failure rather than a slow divergence.

Target state, already seamed in the engine: `AppCommandInit` keeps
`commandList`/`flagList`/`colorList`/`setList` copies "to futur exploitation",
and `SessionFile::CommandSurface::forEach*` enumerates flags, values and colours
at runtime — so this file can become a build artefact the engine emits, and the
authority chain above collapses into one link.

## How the editor is put together

Three layers, and the middle one is where everything happens:

    src/sc_document.hpp   the buffer: bytes in, the same bytes out
    src/sc_editcore.hpp   the interaction: cursor -> token, completion,
                          documentation bar, live findings
    src/sc_tui.hpp        the terminal: draws the above, forwards events,
                          decides nothing

`sc_editcore` is headless on purpose. A terminal cannot be asserted on, and this
can: `tests/editcore_test.cpp` reaches every behaviour the editor has without a
tty, and `--ui-selftest` then proves those answers actually reach a screen. It
consumes four contracts through their headers and owns none of them —
`sc_tokenizer` (the engine's reading of a line, and the raw-column↔token map),
`sc_grammar` (the structural answers, `argKeysAreExhaustive` among them),
`sc_docindex` (the prose half of the same contract file), `sc_check` (findings,
recomputed over the whole buffer after an edit, never over a line alone —
`comment`/`uncomment` make a line's meaning depend on the lines above it).

## Vendoring

| what | version | archive sha256 | how |
|---|---|---|---|
| nlohmann/json | v3.11.3 | prefix `9bea4c8066ef4a1c…`, 919 975 B | single header, `third_party/nlohmann/` |
| FTXUI | v5.0.0 | `a2991cb222c944aee14397965d9f6b050245da849d8c5da7c72d112de2786b5b` | pruned source tree, `third_party/ftxui/` |

FTXUI's archive is
`https://github.com/ArthurSonzogni/FTXUI/archive/refs/tags/v5.0.0.tar.gz`
(236 755 B). What is vendored: `include/` and `src/` byte-for-byte, minus the 42
`*_test.cpp` / fuzzer files, plus `LICENSE`, `CHANGELOG.md` and upstream's
README under its own name. What is NOT vendored: `examples/`, `doc/`, `tools/`,
`cmake/`, `.github/` and upstream's top-level `CMakeLists.txt` — it ends with an
unconditional `add_subdirectory(examples)`, so it cannot build a tree without
them. `third_party/ftxui/CMakeLists.txt` is therefore **scedit's**, and its
source lists are copied verbatim out of upstream's (lines 31–133): explicit
lists, never a glob, so a file that disappears is a build error rather than a
silently dropped translation unit.

## Verification

Eight `ctest` gates, all green on a clean build:

| gate | what it measures |
|---|---|
| `tokenizer` | 189 checks: constructed lines, one per sharp edge of the parse model, each with its expected tokenization — the comment cut included (quoted `#`, unclosed quote, glued `#`, indented `#`) — plus the block structure (`struct if`/`loop` openers, closers, closers that close nothing, the `comment`-block guard) |
| `parse_oracle` | scedit's reading vs a **verbatim copy of the engine's `parseCommand`** — carrying the ruled comment cut in the exact form the engine receives it — over exhaustively enumerated short strings ({a, b, space, tab, `"`} to 6 bytes, {a, space, `"`} to 9, {a, space, `"`, `#`} to 8), ISO-8859 high-byte lines and every line of the real corpus: 119 337 comparisons |
| `editcore` | 175 checks over the headless editor: the byte-preserving buffer, the cursor→token map across quoting and the space-after-quote normalisation, every completion context, the documentation bar including its honest blanks, and the live findings with their spans (a finding points at its bytes; an opener never closed is reported on ITS line) |
| `roundtrip` | `doc/superscript.sts` — 1606 lines (rewritten upstream 2026-08-26, `f0c8ef83`), ISO-8859, CRLF — opened in the editor and saved untouched: **same MD5**. Plus one edit that must change exactly the line it was made on |
| `ui_selftest` | the frames the editor actually DRAWS, rendered off-screen at a fixed size, with four masks of the caret's row: the ghost text is DIM, the look-alike-space marker lands on the column the finding names, every finding's span is UNDERLINED at exactly its bytes, and the caret is the standard SGR inversion on exactly one cell |
| `seed_gate` | the contract file validates (counts re-derived from the data, not asserted) — **and every fact in the four `grammar/args/` fragments is still byte-identical in the merged file**, which is what keeps the granular source and the merged contract from drifting apart |
| `lint_rules` | `tests/lint_cases.sts` — one construct per armed id (15 ids), proving the rule fires with the right id, severity and shape; plus a section that must stay silent (comments in every position included), and a last section for what is only known at the end of the file |
| `corpus_gate` | `--check` over the real corpus produces exactly the recorded findings |

`tests/lint-expected.txt`, `tests/corpus-expected.txt` and
`tests/ui-selftest-expected.txt` are a **record, not a silencer**: every line in
the first two is dispositioned in `tests/derivation-diff.md` §7 with the engine
site named, and a finding that appears without being recorded there fails the
gate. That is constraint C3 — zero false positives before a rule ships — and it
is why the expected files are edited deliberately and never regenerated blind.
The same discipline applies to the rendered frames.

`tests/fixture-grammar.json` is a tiny contract file that exists only to reach
one code path the real one cannot arm yet (see below). It says so itself, in its
own `_meta`, and it is not a source of facts about spacecrafter.

## What the editor cannot do yet, and why

Stated rather than hidden — the `--rules` discipline, applied to the editor.
`DocIndex::dormantFeatures()` reports the first two at runtime.

- **The default is shown, never typed for you.** D31 asks for the default value
  of an empty value field to be greyed and offered. All **324** argument specs
  at HEAD state their default as an English SENTENCE (`absent -> 0`, `absent or
  empty -> the next form is tried`), so there is no literal a machine may type
  on the author's behalf without reading English and guessing — and guessing is
  what constraint C2 forbids. The mechanism is written and arms itself from the
  data: an explicit `default_value` string in a spec becomes the first
  completion candidate and the ghost on an empty field. Count at HEAD: **0**.
  35 of the 324 reduce to a bare token by pattern (`absent -> 0` ×29,
  `-> 1` ×4, `-> no`, `-> 180`) and are the obvious first batch for whoever
  fills the field — as data, written down, not as a regex over English.
  Meanwhile an empty value slot with an ENUMERATED domain does complete, from
  `values` — that half is live.
- **184 of the 227 family names have no documentation of their own.** `flags`,
  `color_names`, `obsolete_tokens`, `reserved_variables` and `font_targets` are
  still the v1 shape (a plain array of names). For those, the bar shows what the
  command says about its keys in general and labels it as such; it never lets
  that stand in for a line about the name itself.
- **`--check` prints no column.** `scedit::Diagnostic` carries a `span` since
  2026-08-31 (the editor underlines it), but D6's printed shape is
  `file:line: severity: message` and the recorded expected files pin it; adding
  gcc's `:col:` is a one-line change waiting for a decision, not for code.
- **`values` mixes values with prose.** An arg spec's `values` array holds both
  literal values (`current`, `toggle`) and descriptions of the rest of the
  domain (`<file name>`, `anything else = off`), and nothing in the schema
  separates them. scedit offers only entries that are a bare `[A-Za-z0-9_]+`
  word (166 of the 234 distinct entries at HEAD) and SHOWS all of them. One
  known false positive survives that rule: `xRRGGBB`, a shape rather than a
  value. A `completable: true` marker in the schema would end the guessing.
- **The whole buffer is re-analysed after every keystroke** — 2.0 ms on
  `doc/superscript.sts` (measured 2026-08-04 on the then-1407-line file) in a
  Release build, so it is not worth making incremental yet, but it is linear
  in file size and will be one day.
- **No TCP mode**: `§5` item 6, blocked on a spacecrafter rebuild.

`tests/derivation-diff.md` is the audit that makes engine fidelity (C1) a
measurement instead of a claim: `parseCommand` line by line against the scedit
code that implements it, every contract clause mapped, and every engine
behaviour scedit does *not* reproduce raised as a question rather than taken as
a decision.

## Status

Landed: the contract file (schema v2, per-key argument data merged), the
tokenizer library (`src/sc_tokenizer.hpp` — also the editor's cursor→token
engine), `--check` with its lint rules, the headless editor core
(`src/sc_editcore.hpp`) with its byte-preserving buffer, completion and
documentation bar, the FTXUI front end (`src/sc_tui.hpp`), and the eight gates
above.

Not yet: the stellar-system-file grammar (second contract file), `$`-variable
semantics for the `reserved_variables` family, and the TCP client mode. Roadmap,
decisions and the open-question ledger: `claude/util/scedit/INTENT.md` (harness
repo).
