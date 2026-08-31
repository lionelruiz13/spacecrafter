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
    cd build && ctest --output-on-failure     # 14 gates, see "Verification"

`-Wall -Wextra` are set on scedit's OWN targets (library, TUI layer, binary,
test binaries) and on nothing else: the vendored trees under `third_party/`
compile exactly as their authors ship them, because a warning there is neither
ours to fix nor evidence about our code. The build is at zero warnings, and
since 2026-08-31 that is a statement which can be false — before that date the
project set no warning flags at all, so "clean build" proved only that it
compiled.

## Use

    scedit [--grammar FILE] [--list FAMILY]          # default: validate the contract
    scedit [--grammar FILE] [--rules] --check FILE...
    scedit [--grammar FILE] --history FILE...        # the error pane's list, no tty
    scedit [--grammar FILE] --doc [CMD [KEY|NAME]]   # the documentation, as JSON
    scedit [--grammar FILE] --search WORDS...        # which page answers this?
    scedit [--grammar FILE] --check --json FILE...   # the findings, as JSON
    scedit [--grammar FILE] --mcp                    # MCP server on stdio
    scedit [--grammar FILE] [--tcp [[HOST:]PORT]] FILE   # the editor, with a live engine
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

**`--history FILE...`** — print the list the editor's error pane shows: every
`#!` tail spacecrafter left in the file and every finding scedit makes, in line
order. Same reader as the pane (`EditCore::errorHistory`), so a harness can
measure what an author would see without a terminal. One entry per line, seven
TAB-separated fields:

    file  line  source  id  severity  message  relation

`source` is `spacecrafter` or `scedit`. `id` is the lint id, or the literal
`#!` for an engine tail. `severity` is empty for an engine tail — the engine
states none, and scedit will not print its own opinion in the engine's column.
`relation` is filled for engine tails only: *agrees with scedit's `<id>`*,
*scedit finds no `<id>` here now* (fixed since the last run, or the two
disagree), or *not a class scedit checks*. A field never contains a tab: the two
prose fields are written with tabs and newlines turned into spaces, so a message
cannot break the shape. Exit codes are `--check`'s: 0 nothing to list, 1 entries
printed, 2 a file could not be read.

The shape is a contract, not a print: `claude/harness/f63_scedit_agree.py`
consumes it to compare scedit's reading of a script with the verdict the engine
wrote into that same file.

**`--tcp [[HOST:]PORT]`** — open the editor with a running spacecrafter at the
other end: send the line under the caret, play the file, watch what the engine
says. Default `127.0.0.1:7805`, the shipped `io:tcp_port_in`; `--tcp 7805` and
`--tcp dome:7805` both work. The argument is optional and is taken only if it
parses as an endpoint, so `scedit --tcp show.sts` opens `show.sts` with live
mode on the default engine — a script whose name is a bare number would be
taken as a port, which is the whole of the ambiguity. Without `--tcp` scedit
opens no socket, starts no thread, and shows nothing about live mode. With
`--mcp` instead of a file, it sets where the `run_command` tool sends by
default. See "Live mode" below.

**`--doc`, `--search`, `--check --json`, `--mcp`** — the same answers, addressed
to a program instead of to a reader. See "For machines" below.

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
| **F5**, or Ctrl-E | show or hide the error pane |
| **F3**, or Ctrl-N | go to the next error; **F4**, or Ctrl-P, the previous one |
| Ctrl-S, or F2 | save |
| Ctrl-U | re-read the file from disk, losing this buffer's unsaved edits |
| Ctrl-Q, Esc, or F10 | quit; with unsaved changes, once to warn and again to discard |

With `--tcp`, five more — and only with `--tcp`: no socket key exists otherwise.

| key | what it does |
|---|---|
| **F6**, or Ctrl-T | connect to the engine, or disconnect from it |
| **F7**, or Ctrl-L | send the line under the caret, as a command |
| **F8**, or Ctrl-R | play this file on the engine (saving it first if needed) |
| **F9**, or Ctrl-W | show or hide the feed |
| F11 / F12, or Ctrl-B / Ctrl-F | older / newer lines in the feed |

Ctrl-S and Ctrl-Q are the terminal's own flow-control pair, so the editor turns
flow control off while it runs and puts it back on exit. F2 and F10 do the same
two things for terminals where that does not take.

### Mouse

Click to put the caret where you clicked, in the text or on a row of the error
pane. Wheel to scroll — and the view stays where you scrolled it: it only chases
the caret again when you next press a key. The wheel over the live feed scrolls
the feed, because that is what the pointer is on.

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

A **`#!` tail** is a comment the ENGINE wrote: when spacecrafter runs a script
and finds a block fault, it writes the diagnosis at the end of the faulty line
(the opener, for a `struct if`/`struct loop` never closed), replaces it when
the verdict changes, and removes it once the fault is gone
(`parse_model.comments.machine_tail`; the writer's contract is
`src/scriptModule/script_annotator.hpp`). scedit never writes one. With the
caret anywhere on such a line, row 4 shows `spacecrafter wrote #! …` followed
by how that relates to scedit's own reading of the line — *agrees with
scedit's `end-without-if`*, or *scedit finds no `end-without-if` here now*
(fixed since the last run, or the two disagree — worth reporting), or *not a
class scedit checks*. That relation is the one place the editor compares its
reading of a line with the engine's actual verdict on it (constraint C1,
measured on the file rather than on the parser). Inside the tail the bar
says what a `#!` is and who owns it.

### The error pane

**F5** opens a list of every place something is wrong with the script: each
`#!` tail spacecrafter left in the file, and each finding scedit makes, in line
order. **F3** and **F4** step through it — they open the pane too, so wanting
the next error is enough, you need not know the pane is there — and a **click**
on a row puts the caret on that entry, at the byte it is about. `>` and the
terminal's inversion mark the row you are standing on. The pane costs six rows
and most files are clean, so it starts closed; the error COUNT is on the status
line at all times, which is what tells you it exists.

A row reads `E    12 │ sc  unknown-command: …` for scedit's own findings and
`!    12 │ #!  …` for the engine's. The engine's rows carry no severity because
the engine states none, and scedit will not print its own opinion in the
engine's column; with the caret on such a line the doc bar's fourth row says how
the two readings relate.

A line carrying **both** a tail and a finding gives **two rows**, the engine's
first. They are two claims by two authors about one line — what happened when
spacecrafter RAN it, and what scedit reads NOW — and the case where they differ
is exactly the C1 signal worth seeing. The engine's row comes first because
there is at most one per line and it records a run that happened.

**What "history" means here — an interpretation, stated so it can be vetoed.**
The requirement is Vixy's, verbatim [2026-08-30]: *"RECOGNIZE `#!` machine
annotations as navigable errors — shown on the doc bar when the caret is on the
line, otherwise listed in an error history with click-to-warp-cursor"*. This
pane reads "history" as **the current buffer's set**, not a log of past editing
sessions. The argument: the engine's channel already IS the log — a `#!` stays
in the file until the fault is fixed and spacecrafter reaches a natural end — so
the file itself carries what the engine found last time it ran, and a second
store could only be a copy of it that goes stale. Nothing is remembered across
an open, and nothing outlives its cause: fix the fault, the row goes. If the
word was meant to carry more than that — every diagnostic this editing session
has seen, kept after it was fixed — say the word; it is a different feature and
a different store.

The keys, the placement, the toggle and the default-closed are scedit's own
calls, made rather than asked, and equally open to veto.

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

## Live mode

    scedit --tcp show.sts          # the engine on 127.0.0.1:7805
    scedit --tcp dome:7805 show.sts

spacecrafter listens on a TCP line protocol (`io:enable_tcp`, shipped **true**,
`io:tcp_port_in` = **7805**) and executes a line arriving there exactly as it
executes a line of a script file. With `--tcp`, the editor can use it: **F6**
connects, **F7** sends the line under the caret, **F8** plays the whole file,
**F9** shows the feed.

**Nothing is sent unless you press a key.** There is no auto-connect, no
reconnect after a drop, no keep-alive and no replay. `run_command` (below) is
the same client, and the same rule.

### What comes back, and what does not

Three things reach a client, and no others:

- the answer to `get status …` and to `search name …`
  (`src/interfaceModule/app_command_interface.cpp:1309-1326,1440` - the only
  callers of `ServerSocket::setOutput` in the tree);
- the replies to `$NOTICE` / `$LOGON` / `$LOGOFF`, and to `$DIAGON` / `$DIAGOFF`
  (`src/tools/io.cpp`, `computeNormalString`);
- **a refusal, if you asked for refusals.** Since engine `be2ddd81`
  (INTENT 11.188), a connection that sends `$DIAGON` receives one record per
  diagnostic the engine produces about a command it read on the control socket.
  scedit sends it on connect, so `flag stars onn` now comes back as

      $DIAG|tcp#7|Unrecognized or malformed command name|flag stars onn

  Four fields: the marker, the **origin** (`tcp#<id>`, the engine's never-reused
  connection id), the engine's own **message**, and the **command line** it is
  about. The command line is last because it may itself contain a `|`.

The pane draws these **red**: on a feed where everything else is an answer, the
one line that says something went wrong should not look like the rest.

**The silence that remains is still silence.** A command that WORKED sends
nothing, so an empty feed is still not a report of success. Nor is it a report
of failure in three cases worth knowing: a refusal produced *inside* another
command (`media action play ...` runs `audio filename ...`, and the inner one is
the one that fails) carries no origin and does not arrive; a script's start and
end are still unannounced; and an engine older than `be2ddd81` has no `$DIAGON`
at all, treats it as an unrecognised command, and tells you nothing - scedit
connects to it perfectly well and the feed is simply as quiet as it always was.
To find out what happened, read a state back - `get status position` - or look
at the engine's log, which still has every refusal it ever had (INTENT 5.117:
the wire got a COPY, not the original).

**Two subscriptions, and the older one did not move.** `$LOGON` is the feed of
command answers, and it carries **other clients' answers** too: that
subscription's greeting promises the logs, and what it actually delivers is
every command answer the engine produces (INTENT 5.72). `$DIAGON` is the
separate, opt-in diagnostic link. They are separate because the `$LOGON` wire is
also spoken by a closed-source client (masterput), so it was frozen
byte-identical rather than extended - an engine that did not receive `$DIAGON`
from you sends you exactly the bytes it sent before this feature existed, and
that is a measured claim, not a design intention
(`claude/harness/f69_feedback.py` leg iv).

Note that a diagnostic caused by ANOTHER client arrives here too, tagged with
that client's origin. Reading one as "my command failed" without looking at the
origin is the mistake that field exists to prevent.

The feed keeps the last **500 lines**; older ones are dropped and the header
says how many, because a bounded buffer that discards in silence is one you
cannot trust. Lines scedit wrote itself — what it sent, what it did — are dim
and marked `>`; the engine's answers are plain; its refusals are red.

### The engine writes into your file, and neither side loses

When a played script reaches its natural end, spacecrafter **rewrites the
file**: a `#!` tail on each faulty line, and the tails of lines that are now
clean removed (`src/scriptModule/script_annotator.hpp`). So the file under your
buffer changes, written by another program, while you may have been typing.

Nothing announces this. The engine has no end-of-script event on any channel a
client can see — measured: over a whole play that produced findings, the wire
carried not one byte after the subscription confirmation
(`claude/harness/f67_tcp_live.py`, leg D). So, **after a play and only then**,
scedit re-reads that file at most **once a second**, for at most **five
minutes**, until it changes. That is the only clock in the editor. It is a
convenience, not the guarantee — the guarantee is that scedit compares the file
with what it read **before every save, always**, play or no play.

What happens then depends on you, and never on scedit:

- **buffer clean** → the file is reloaded, the error pane opens, and the
  engine's findings are in it (`F3` walks them). Nothing of yours can be lost:
  you had not changed anything.
- **buffer modified** → **nothing happens to either side.** The status line says
  the engine rewrote the file and names the two ways out, and a save is
  REFUSED with the same sentence. **Ctrl-U** reloads (your edits go);
  **Ctrl-S a second time** saves anyway (the engine's tails go). One of the two
  has to lose, and the editor will not choose for you.

Playing a modified buffer saves it first — the engine opens the FILE, not your
buffer — and if that save is refused, the play does not happen either, with the
save's reason. Playing a buffer that has never been written anywhere is refused
for the same reason.

A UX call, veto open (scedit's, as the keys are): the second Ctrl-S is what
takes the destructive branch, on the model of the quit warning above it, and the
refusal message names both losses before either can be chosen.

## For machines

The editor shows a human what the contract file says about the thing under the
cursor. These four modes say the same thing to a program — a harness, a script,
or a language model through the MCP server below — and they exist because the
alternative is a model answering from its recollection of a planetarium's script
language. Nothing here is a second reader of the grammar: every answer comes
from `Grammar` and `DocIndex`, the two objects the doc bar reads
(`src/sc_docjson.hpp`). **No model is ever called.** One thing opens a network
connection: the MCP tool `run_command`, which sends a command to a live engine
over the same client the editor uses (see "Live mode"). Everything else — the
documentation, the search, the checker — reads a file and nothing else.

**The honest null survives.** A `doc` field that is JSON `null` means *no
documentation has been extracted for this name*, and it is the one field that
carries that distinction, because it is the one the file draws. Two argument
keys are null today (`dso3d z_reflection`, `suntrace sun` — both flagged, the
code does not support a sentence) and so are all 184 names of the five families
whose doc pass has not run. `"present": true, "doc": null` says the name exists
and its documentation does not. Every other field is the file's own text, empty
where the file says nothing.

### `--doc [<command> [<key> | <family name>]]`

    scedit --doc                       # the catalogue
    scedit --doc image                 # one command: its doc, keys, key specs, family
    scedit --doc image filename        # one argument key
    scedit --doc flag stars            # one family name (flag/set/color/font)

One JSON document on stdout. A command page carries `doc`, `registration` (the
engine site the command is registered at), `alias_of`, `args_complete`,
`args_source`, `keys`, the full spec of each key under `args`, `key_grammar`
where the command's keys are not a fixed list, and — for the four commands that
name a family — `family` and its `members`. A key or family-name page carries
`doc`, `value_domain`, `values` (verbatim, prose entries included) with the
subset that may be OFFERED as a completion under `completable`, `value_docs`,
`default` (a sentence today, see D31), `default_literal`, `required`, `source`
and `notes`. An alias answers with its own `doc` and `registration` and the
canonical command's keys, which is how both readers resolve it.

The catalogue is the two-level shape a model is given as context: every command
with its one-liner, and under `flag`, `set`, `color` and `font` the names that
family accepts. Argument keys are not in it — they are one `--doc` away and
would quadruple it.

A name that is not in the vocabulary is answered, not guessed at: exit **2**,
and on stdout an object with `error`, `message`, `vocabulary` (which vocabulary
was searched, and whether the file claims it is complete) and `did_you_mean` —
the checker's own suggestion, so `--doc` and `--check` cannot disagree about
what the engine's nearest name is.

### `--search <words>...`

Ranked pages for a few words of a request, best first, `--scope all` (default:
commands, argument keys and family names) or `--scope commands`, `--limit N`.
Each result carries `page`, which is exactly the arguments to pass to `--doc`.

The score is published, and there is no inference and no synonym anywhere in it:

    score(page) = |words(query) & words(page)| / (1 + sqrt(|words(page)|))

where `words` lowercases, cuts on anything outside `[a-z_]` and keeps runs of
three letters or more, and a page's words are its own name (underscores read as
spaces) plus its doc line. A page that shares no word with the query is not an
answer and is not returned — the empty result set is a real answer, and exit is
still 0. Ties keep enumeration order: commands in the contract file's order,
then each command's keys and family names.

The formula is not invented here. It is the model-free baseline of
`claude/harness/f64_doc_router.py`, which measured the documentation-router role
on 340 real (comment → command) questions mined from `doc/superscript.sts`.
Porting it — rather than writing a nicer one — is what lets the measurement and
the product be about the same ranking: `claude/harness/f66_search_parity.py`
asks scedit all 340 questions and compares its top-ranked command against the
baseline question by question. **340/340 agree**, and both sides score
**80/340 = 23.5%**, F64's recorded number. That gate can fail on any question,
in either direction, and the one stated difference is the empty-answer rule
above (21 of the 340 share no word with any command; the baseline picks the
file's first command there, scedit picks nothing, and those 21 were hits zero
times).

### `--check --json`

The findings of `--check` as objects — `file`, `line`, `severity`, `id`,
`message` and `span` (the raw byte range on the line, `null` when the finding is
about the line as a whole) — plus `counts` and, with `--rules`, the unarmed
rules. The gcc-shaped text of plain `--check` is a contract three recorded gates
pin and is untouched: one finding set, two printers. Exit codes are unchanged:
0 clean, 1 findings, 2 usage or I/O error.

### `--mcp` — the Model Context Protocol server

    scedit --mcp        # newline-delimited JSON-RPC 2.0 on stdin/stdout

Binding it to a harness, e.g. Claude Code:

    claude mcp add scedit -- /path/to/scedit --grammar /path/to/sc-grammar.json --mcp

Four tools, and their descriptions are written for a reader who knows nothing —
they are what the outside model sees before it decides to call one:

| tool | what it answers |
|---|---|
| `doc_lookup` | `{command?, name?}` → one page, or the catalogue when called with no arguments |
| `doc_search` | `{query, scope?, limit?}` → the ranked pages, each with the `page` to look up |
| `check_script` | `{text? \| path?, label?}` → the findings, over a buffer or a file on disk |
| `run_command` | `{command, host?, port?, wait_ms?}` → **runs one line on a LIVE engine** and returns what it says |

`run_command` is the only one that changes anything: the dome moves, the show
changes, and there is no undo. Its description says so, and says the things a
caller cannot work out for itself - that most commands answer with silence,
that a reply may be an answer to another client's command (the `$LOGON`
subscription), and that playing a script returns when the script STARTS because
nothing announces that it ended. It returns **two** lists: `replies`, and
`diagnostics` - the refusals, split into `origin` / `message` / `subject`, off
the `$DIAGON` link this connection also subscribes to (INTENT 11.188). The
`note` field is computed from what actually arrived, because what silence MEANS
changed with that link and a note written once and left would now be wrong: an
empty `diagnostics` says the command was not refused at the top level, and no
more than that - success is silent, a refusal nested inside another command
carries no origin and does not arrive, and an older engine sends nothing at all.
Its
`wait_ms` is bounded at both ends (100–10000): asking for 1 ms and then
reporting a reply as absent is not a measurement. `--tcp` before `--mcp` sets
where it sends by default:

    claude mcp add scedit -- /path/to/scedit --grammar /path/to/sc-grammar.json --tcp 7805 --mcp

A tool is declared in exactly one place, `registeredTools()` in
`src/sc_mcp.cpp`: name, description, input schema, handler. The protocol code
names no tool and knows no tool's arguments — `run_command` was added as
exactly one entry in that vector plus one field on `ToolContext`, with no line
of protocol code touched, which is what that seam was written to make possible.

An unknown TOOL is a protocol error (`-32602`); an unknown argument VALUE — a
command that does not exist, a file that cannot be read, a missing `query` — is
a tool result with `isError: true` carrying the same explanation a human gets,
because that is the one the calling model can act on.

**The specification was fetched, not recalled.** `https://modelcontextprotocol.io/
specification/`, whose own "latest" pointer resolved to revision **2026-07-28**
when this was written (fetched **2026-08-31**). That revision has no
initialization handshake: it is stateless, every request carries
`_meta["io.modelcontextprotocol/protocolVersion"]` and
`_meta["io.modelcontextprotocol/clientCapabilities"]`, and a server MUST
implement `server/discover`. The handshake era it calls "legacy" (`initialize` +
`notifications/initialized`, revision **2025-11-25** and earlier) is what
deployed clients still speak — Claude Code 2.1.251 opens with
`"protocolVersion": "2025-11-25"`, measured. So the server is **dual-era**,
which is the specification's own name for this case: a request carrying the
modern `_meta` is served per 2026-07-28 (with `resultType`, with the version
validated and `-32022` listing what is supported), anything else is served per
2025-11-25. Nothing is inferred from an earlier request on the same connection.

Implemented: `initialize`, `notifications/initialized` (and any other
notification, which is ignored in silence, as a notification must be),
`server/discover`, `ping`, `tools/list`, `tools/call`, and the JSON-RPC error
shapes (`-32700`, `-32600`, `-32601`, `-32602`, `-32022`).

Deliberately NOT implemented, listed because a silent omission cannot be told
from a bug: resources, prompts, logging, completions; pagination (`cursor` /
`nextCursor`) and result caching (`ttlMs`, `cacheScope`) — the tool list is
fixed at build time and short; `subscriptions/listen` and
`notifications/tools/list_changed` (the server declares `listChanged: false`);
progress notifications and `notifications/cancelled` (every call answers
synchronously in microseconds); multi-round-trip results (`input_required`) and
the client features they need — elicitation, sampling, roots; `outputSchema` and
`icons` on the tools; JSON-RPC batches (an array is refused with `-32600` rather
than half-answered); extensions (tasks, apps); and the authorization framework,
which the specification itself says stdio servers should not implement — a stdio
server takes its credentials from the environment, and this one needs none. The
engine's control socket has no authentication of any kind, at either end: a
`run_command` call reaches whatever is listening on that host and port, and
binding this server to a machine that can reach a dome is a decision about who
may move that dome.

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
                          documentation bar, live findings, the error history
    src/sc_tui.hpp        the terminal: draws the above, forwards events,
                          decides nothing

`sc_editcore` is headless on purpose. A terminal cannot be asserted on, and this
can: `tests/editcore_test.cpp` reaches every behaviour the editor has without a
tty, and `--ui-selftest` then proves those answers actually reach a screen. The
error pane is the shape of that rule: the list and the warp are core calls with
their own tests, `--history` prints the same list for a machine, and the pane
only draws it and turns a click into the one call the keyboard also makes. It
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

Fourteen `ctest` gates, all green on a clean build (`-Wall -Wextra`, 0 warnings):

| gate | what it measures |
|---|---|
| `tokenizer` | 189 checks: constructed lines, one per sharp edge of the parse model, each with its expected tokenization — the comment cut included (quoted `#`, unclosed quote, glued `#`, indented `#`) — plus the block structure (`struct if`/`loop` openers, closers, closers that close nothing, the `comment`-block guard) |
| `parse_oracle` | scedit's reading vs a **verbatim copy of the engine's `parseCommand`** — carrying the ruled comment cut in the exact form the engine receives it — over exhaustively enumerated short strings ({a, b, space, tab, `"`} to 6 bytes, {a, space, `"`} to 9, {a, space, `"`, `#`} to 8), ISO-8859 high-byte lines and every line of the real corpus: 119 337 comparisons |
| `editcore` | 266 checks over the headless editor: the byte-preserving buffer, the cursor→token map across quoting and the space-after-quote normalisation, every completion context, the documentation bar including its honest blanks, and the live findings with their spans (a finding points at its bytes; an opener never closed is reported on ITS line), and the error history: both sources in line order, a warp landing on the stated byte, an edit that removes an entry, and the shapes that carry a `#!` and are NOT tails; and the write-back rule against a real file on disk — the change seen, a clean buffer reloading with the engine's tail listed and the caret kept, a dirty one's save refused with the file byte-identical afterwards, each of the two explicit choices doing exactly what it says, a deleted file answered differently from a changed one |
| `roundtrip` | `doc/superscript.sts` — 1606 lines (rewritten upstream 2026-08-26, `f0c8ef83`), ISO-8859, CRLF — opened in the editor and saved untouched: **same MD5**. Plus one edit that must change exactly the line it was made on |
| `ui_selftest` | 21 frames the editor actually DRAWS, rendered off-screen at a fixed size - the error pane among them, with both sources mixed, after one warp and after two, and empty on a clean buffer - each with four masks of the caret's row: the ghost text is DIM, the look-alike-space marker lands on the column the finding names, every finding's span is UNDERLINED at exactly its bytes, and the caret is the standard SGR inversion on exactly one cell. Four of the twenty-one are live mode, with a fifth mask (`feed ELDLE`) that reads off the PIXELS which lines scedit wrote (`L`, dim cyan), which the engine answered (`E`) and which it REFUSED (`D`, red) - so the claim that a refusal looks different is proved from the screen and not from the FeedKind the renderer was handed; the other seventeen are byte-identical to what they were before live mode existed, which is how "no `--tcp`, nothing on the screen" is checked rather than asserted |
| `seed_gate` | the contract file validates (counts re-derived from the data, not asserted) — **and every fact in the four `grammar/args/` fragments is still byte-identical in the merged file**, which is what keeps the granular source and the merged contract from drifting apart |
| `lint_rules` | `tests/lint_cases.sts` — one construct per armed id (15 ids), proving the rule fires with the right id, severity and shape; plus a section that must stay silent (comments in every position included), and a last section for what is only known at the end of the file |
| `history_list` | `--history` over the lint fixture and `tests/history_cases.sts` produces exactly the recorded rows. Two inputs on purpose: the fixture's scedit rows include the 27 the `--check` record already pins, so one reader's two printers cannot drift apart silently; history_cases is F63-SHAPED — every `#!` in it is a sentence the ENGINE wrote — and three of its ten cases are deliberate ABSENCES (a `#!` in quotes, in a column-0 comment, in an indented one) |
| `check_json` | `--check --json` over the lint fixture produces exactly the recorded objects — the same comparator as the text record above, with `--json` added to its mode, so the two printers of one finding set cannot drift apart |
| `doc_queries` | ten recorded (arguments → stdout + exit code) answers of `--doc` and `--search`, each line of `tests/doc-queries.txt` saying what it pins: both flagged keys answering `doc: null`, a v1 family name beside a v2 one, an alias, both not-found vocabularies with their did-you-mean and exit 2, the ranking, and a query that matches nothing. stderr must stay empty for every one |
| `mcp_protocol` | 85 checks from a stdlib-only Python client (`tests/mcp_gate.py`) that spawns `scedit --mcp` - a second implementation on purpose: both protocol eras, every tool with good and bad arguments, the honest null arriving as JSON `null` through the whole chain, the catalogue's counts, the five refusals (parse error, no method, unknown method, unknown tool, unsupported version), and `run_command` driven against the stand-in engine with both sides asserted - the answer verbatim, the stand-in's own record of what arrived, a silent command with the note that says exactly how much that silence is worth, a REFUSED command coming back in `diagnostics` split into origin/message/subject with the raw record kept, and the no-engine path as a tool error naming what to check |
| `tcp_client` | 30 gate checks over 112 leg checks: `sc_tcpclient` against `tests/fake_engine.py`, a stand-in whose framing rules are each read from a named line of `src/tools/io.cpp`. One leg per process, and after each one the gate asserts what the stand-in RECEIVED - so a leg cannot pass by agreeing with itself. Endpoint parsing and its refusals, a port nothing listens on, subscribe/ask/answer/disconnect/reconnect, a two-line command refused with nothing sent, latin-1 and 0xA0 bytes arriving as bytes, the bounded feed counting what it drops, another client's answer arriving because we subscribed, the engine closing the connection - and the DEDICATED diagnostic link: both subscriptions sent in order, a `$DIAG|` record split into its four fields (a subject containing the separator included), a malformed one SHOWN rather than dropped, and a second `$LOGON`-only onlooker proving from the other end that it received the answer and not one byte of the diagnostic channel |
| `pty_keys` | the editor's live KEYS, pressed on a **pseudo-terminal**, with the stand-in engine asserting what arrived: `--tcp` alone connects to nothing, Ctrl-T subscribes, Ctrl-L sends the caret's line verbatim and sends NOTHING from a comment line, Ctrl-R plays the file by absolute path, the engine's own words reach the feed pane on screen, Ctrl-Q exits 0 having unsubscribed. This is the seam between the two gates on either side of it — one pins what is DRAWN, the other what the core and client DO — and it is what makes "F8 plays the file" a measurement rather than a reading |
| `corpus_gate` | `--check` over the real corpus produces exactly the recorded findings |

`tests/lint-expected.txt`, `tests/corpus-expected.txt`,
`tests/history-expected.txt`, `tests/check-json-expected.txt`,
`tests/doc-expected.txt` and `tests/ui-selftest-expected.txt` are a
**record, not a silencer**: every line in the first two is dispositioned in
`tests/derivation-diff.md` §7 with the engine site named, every row of the third
in a comment beside the bytes that produce it in `tests/history_cases.sts`, and
anything that appears without being recorded fails the gate. That is constraint C3 — zero false positives before a rule ships — and it
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
  in file size and will be one day. The error pane rides that same pass and
  adds only a `find("#!")` per line before it will tokenize one.
- **The error pane never writes a `#!`, and never will from here.** The channel
  is the engine's: scedit shows a tail, relates it to its own reading, and warps
  to it. Deleting a tail by hand is an ordinary edit like any other — the editor
  simply has no command that composes or rewrites one, which is what keeps the
  round-trip guarantee above worth anything.
- **The pane has no filter and no sort.** Line order, both sources, all of it.
  A file with a thousand duplicate-key findings will list a thousand rows; "show
  me only the engine's", "only errors", "group by id" are one accessor away
  (`EditCore::errorHistory` is a plain vector) and are not built because nobody
  has asked for them yet.
- **The FUNCTION keys are not pressed by any gate — their control twins are.**
  `pty_keys` presses Ctrl-T/Ctrl-L/Ctrl-R/Ctrl-W/Ctrl-Q on a real
  pseudo-terminal; F6/F7/F8/F9/F10 arrive as terminal-dependent escape
  sequences, and encoding one terminal's table in a gate would be measuring that
  table. Each pair is one `||` in the same branch of `src/sc_tui.cpp`, which is
  read rather than measured. The twins exist for exactly the terminals where the
  F-keys do not arrive.
- **Nothing measures the editor against a real engine and a real terminal at the
  same time.** `pty_keys` uses the stand-in; `claude/harness/f67_tcp_live.py`
  uses the real engine headlessly. The two halves have never been held together
  in one run.
- **The engine cannot be asked whether a command worked.** Not a scedit
  limitation: the wire carries answers to `get`/`search` and nothing else, so
  after `flag stars on` there is nothing to show but the fact that it was sent.
  Routed to the project's owner rather than worked around here.
- **A play's write-back is watched for five minutes.** A show longer than that
  finishes with scedit no longer looking; the file is still compared before the
  next save, so nothing can be lost — but the pane will not light up on its own.
  The bound exists because there is no end-of-script event to wait for.

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
documentation bar, the error pane and its `--history` twin, the FTXUI front end
(`src/sc_tui.hpp`), the machine surface (`--doc`, `--search`, `--check --json`
and the MCP server over the same readers), live mode (`--tcp`: the client, the
feed, the play, the `#!` write-back that loses neither side, and the
`run_command` tool over the same client), and the fourteen gates above.

Not yet: the stellar-system-file grammar (second contract file) and `$`-variable
semantics for the `reserved_variables` family. The router half of the
LLM assistance — a model that picks the page to show, and whether the checker
gates what such a model writes — is Vixy's to triage; scedit's side of it is the
tools above, and it calls no model. Roadmap,
decisions and the open-question ledger: `claude/util/scedit/INTENT.md` (harness
repo).
