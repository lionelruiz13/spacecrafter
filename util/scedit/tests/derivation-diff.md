# scedit tokenizer — derivation diff against the engine

Constraint C1 (`claude/util/scedit/INTENT.md` §2): scedit's reading of a line
must be the engine's reading of that line, sharp edges included. This file is
the audit: every line of `AppCommandInterface::parseCommand`, every line of the
`executeCommand` pre-table region, and every clause of the contract file's
`parse_model` mapped to the scedit code that implements it — plus, in §5, every
engine behaviour scedit does NOT reproduce, each one raised as a question rather
than taken as a decision.

Engine at `master-beta @ b12c8cdd`. Line numbers are that revision's.

- Engine parser: `src/interfaceModule/app_command_interface.cpp:124-176`
- Engine dispatch (pre-table region): same file, `:191-239`
- Engine suggestion: `src/interfaceModule/app_command_init.cpp:337-387`
- Engine script layer: `src/scriptModule/script.cpp:110-120`
- Engine predicates: `src/tools/utility.hpp:160-181`, `:85` (`stringHash_t`)

**Evidence that this is a measurement and not a claim.** `tests/parse_oracle_test.cpp`
carries a VERBATIM copy of `parseCommand` and compares its `(command, args)`
against `scedit::tokenizeLine` over 49 053 exhaustively enumerated strings
(alphabets `{a,b,SP,TAB,"}` up to 6 bytes and `{a,SP,"}` up to 9 bytes — every
branch of the parser is reachable from those), 3 ISO-8859 high-byte lines, and
every line of the real corpus: **53 058 comparisons, 0 mismatches**
[measured 2026-08-04, `ctest -R parse_oracle`]. The mapping below explains the
agreement; the oracle proves it.

---

## 1. `parseCommand` line by line (`app_command_interface.cpp:124-176`)

| engine | engine code | scedit | note |
|---|---|---|---|
| :124 | signature `(command_line, command&, arguments&)` | `tokenizeLine(raw) -> Line` | The engine writes into caller state; scedit returns a value that also carries provenance (spans). Same information, plus what the TUI needs. |
| :126 | `std::string str = command_line;` | `sc_tokenizer.cpp` `std::string s = raw;` + `off` = identity offset vector | The offset vector is scedit-only: it is what makes the raw↔normalized map exact rather than recomputed. |
| :129-132 | `while (str[0]==' ' \|\| str[0]=='\t') str.erase(0,1);` | `while (k < s.size() && (s[k]==' ' \|\| s[k]=='\t')) ++k;` then erase `[0,k)` from `s` and `off` | Same set of bytes removed. The engine's `str[0]` on an exhausted string reads the NUL terminator (defined since C++11 for non-const `operator[]` at `size()`), which is why the loop terminates on an all-blank line; the bounded form is equivalent, not weaker. Only SP and TAB — a leading `\r` survives here, and that is load-bearing for CRLF files (see §3, script layer). |
| :135 | `found = str.find(" \" ");` | `std::size_t found = s.find(" \" ");` | identical |
| :136-139 | `while(found!=npos){ str.erase(found+2,1); found = str.find(" \" "); }` | same loop, mirrored on `off` | Re-searching from the start (not from `found`) is reproduced: it is what lets one erase create the next match. Pinned by `tokenizer_test.cpp` "two-pass normalization" (`x "  " y` → `x "" y`, two bytes erased). |
| :141 | `std::istringstream commandstr(str);` | `StreamSim stream(s)` | Reduced to the two operations used: `>>` and `.get()`. Whitespace set is the C locale's (`SP TAB LF VT FF CR`) — `std::locale::global()` is never changed; `src/main.cpp:242` sets `LC_TIME` only. |
| :142-143 | `std::string key, value; char nextc;` | locals per iteration | The engine's `key`/`value` persist across iterations; that only matters when an extraction FAILS, and scedit reproduces the consequence directly (see :148). |
| :145 | `commandstr >> command;` | `stream.extract(tok,b,e)`; on failure no `Command` token is produced | On failure the engine leaves `command` at the value `executeCommand:198` cleared it to — empty. scedit models the observable: `has_command == false`. Pinned: "whitespace-only line". |
| :146 | `transform(command..., ::tolower)` | `asciiLower(tok)` | `::tolower` under `LC_CTYPE="C"`: A–Z only; bytes ≥ 0x80 map to themselves. Cross-checked against the real `::tolower` by the oracle on ISO-8859 input (`\xc9`, `\xe9`). |
| :148 | `while (commandstr >> key >> value)` | `extract(key)` then `extract(value)`; if the key succeeded and the value failed, a `DanglingKey` token is recorded and the loop breaks | The engine's loop body never runs, so `arguments` never gets the key — the drop is silent. scedit drops it from `args` too and keeps it as a token so `--check` can say so. Pinned: "dangling only key", "dangling after a pair". |
| :149 | `if (value[0] == '"')` | `if (value[0] == '"')` | `value` is non-empty whenever the body runs (extraction yields ≥ 1 byte), so the index is safe in both. Only the FIRST byte opens a quote: `abc"def` is ordinary. Pinned: "quote mid-token". |
| :151 | `if (value[value.length()-1] == '"')` | same | For the one-byte token `"` this compares the byte with itself and takes the one-word branch. |
| :153 | `value = value.substr(1, value.length()-2);` | same expression | For length 1 the count underflows to `SIZE_MAX` and `pos == size()`, so `substr` returns `""`: **a lone `"` is an empty value, not an opening quote**. Reproduced literally rather than "fixed". Pinned: "lone quote is an empty value". |
| :156 | `value = value.substr(1, value.length()-1);` | same | |
| :158-162 | `while(1){ nextc = get(); if (nextc=='"' \|\| !good()) break; value.push_back(nextc); }` | same loop on `StreamSim` | `get()` returns the byte that terminated the token (a separator), so separators inside a quoted run are preserved verbatim. On EOF the engine's `char nextc` holds `(char)EOF`, but the `!good()` arm breaks before the `push_back`, so it is never appended — scedit breaks at the same point. The end position is recorded to build the value's raw span. Pinned: "multi-word quotes", "unclosed quote", "closing quote mid-token swallows the line". |
| :163-164 | closing braces | — | |
| :165 | `transform(key..., ::tolower)` | `asciiLower(key)` | Keys are folded; **values are not** (there is no transform on `value`). Pinned: "case folding". |
| :166 | `arguments[key] = value;` | `line.args[kt.text] = vt.text;` — `std::map`, same type | Last value wins on a repeated key; iteration order is byte-lexicographic, not line order. Both pinned ("duplicate key", "args.begin() is alphabetically first"). `Line::pairs` additionally keeps LINE order, which the engine has no use for and a diagnostic does. |
| :169-174 | `#ifdef PARSE_DEBUG` logging | — | Not compiled in the engine build; no observable. |
| :175 | `return 1; // no error checking yet` | `tokenizeLine` cannot fail | The engine's parser reports nothing; every diagnostic scedit emits is therefore a scedit-side reading of an engine behaviour, never a relayed engine error. |

## 2. `executeCommand` pre-table region (`:191-239`)

| engine | engine code | scedit | note |
|---|---|---|---|
| :193-196 | `recordable/debug_message/wait/commandline` init | — | Execution state; not a parse observable. |
| :198-199 | `command.clear(); args.clear();` | fresh `Line` per call | Why an unparseable line yields an EMPTY command rather than a stale one. |
| :202 | `parseCommand(commandline, command, args)` | `tokenizeLine` | §1. |
| :204 | `FilePath::fixScriptPath(...)` | — | Filesystem side effect, no parse observable. |
| :207-208 | `if (command.length() < 1 && command == "") return 0;` | `Line::has_command == false` → `BlockSkipState::feed` returns false and `sc_check` returns without a finding | A blank-after-strip line is a no-op, not an error. |
| :210 | log `Execute_command` | — | |
| :215-216 | `if (command=="comment") return commandComment();` (`:3160`, `swapCommand = true`) | `BlockSkipState::feed`: `command=="comment"` → `skipping_ = true`, returns false | Literal compare, BEFORE the table: `comment` is not in `m_commands`. |
| :218-219 | `if (command=="uncomment") return commandUncomment();` (`:3167`, `swapCommand = false`) | same, `skipping_ = false` | |
| :221-222 | `if (command=="struct") return commandStruct();` (`:4600`) | `feed` handles the `struct` comment case (see §5.1 for the rest) | `struct` is BOTH intercepted here and registered in `m_commands`. |
| :225-228 | `if ((swapCommand \|\| ifSwap->get()) && !unskippable) { log; return 1; }` | `feed` returns `skipping_` for every other command → `sc_check` skips the line entirely | Only `swapCommand` is modelled; `ifSwap` is §5.2. The three interceptions above act even while skipping — pinned by the block-state case list, which traverses the enter/leave pair twice, the second entry starting from the state the first exit produced. |
| :229 | `unskippable = false;` | — | Only ever true for the engine's own `terminateScript()` (`:178-182`), never for a script line. |
| :231-232 | `m_commands.find(command)` | `Grammar::command(name)` over the contract file's `families.commands` | Accepted set = 60 registered + `comment`/`uncomment` pre-table. Gate: the seed test re-derives 60 + 2 from the data. |
| :233-236 | unknown: `debug_message`, log, `searchSimilarCommand(command)`, `return 0` | `unknown-command` diagnostic + `nearestNeighbour(command, commandLookupList())` | Candidate list = `commandList`, built by walking `m_commands` (`app_command_init.cpp:105-107`) — a `std::map`, hence alphabetical; scedit sorts the contract file's command names to match, because `searchNeighbour`'s strict `<` keeps the FIRST minimum and the order is therefore part of the answer. `comment`/`uncomment` are absent from that list in both. |
| :239+ | `switch` to handlers | — | Handler semantics are the extraction sweep's territory, not the tokenizer's. |

### Levenshtein / `searchNeighbour` (`app_command_init.cpp:337-387`)

`levenshtein()` is a line-for-line transcription (same shorter-string swap, same
single-row update, same return). `nearestNeighbour()` reproduces `minDistance =
99999`, the strict `<`, and the absence of any threshold. The engine's early
`if (isObsoleteToken(source)) return;` is NOT inside `nearestNeighbour`: it is a
branch of the caller, and scedit takes the same branch one level up (an obsolete
name yields `deprecated` and no suggestion — `sc_check.cpp`, both the command
and the family-name paths).

## 3. `parse_model` clause by clause (`grammar/sc-grammar.json`)

| clause | scedit | status |
|---|---|---|
| `line_shape` | `tokenizeLine`: one command token then strict pairs | implemented |
| `command_case` | `asciiLower` at :146 | implemented, oracle-checked incl. ISO-8859 |
| `key_case` ("VALUE case is preserved") | `asciiLower` on keys only | implemented, pinned |
| `pair_extraction` (dangling key dropped) | `Line::has_dangling`, key absent from `args` | implemented, pinned, lint `dangling-key` |
| `duplicate_keys` (last wins, alphabetical iteration) | `Line::args` is `std::map`; `Line::pairs` keeps line order | implemented, pinned, lint `duplicate-key` |
| `quoting.value_position_only` | only the value branch inspects quotes | implemented — a `"` in a KEY is an ordinary byte and is folded with the rest of the key |
| `quoting.quote_char` (`"` only, no escapes) | no `'` or `\` handling anywhere | implemented, pinned ("single quotes are ordinary", "no backslash escape"), lint `unsupported-quoting` |
| `quoting.single_word` | :151-153 branch | implemented, pinned |
| `quoting.multi_word` (unclosed runs to EOL, no error) | :156-162 branch; `Token::quote_closed`, `Line::has_unclosed_quote` | implemented, pinned. Exposed as tokenizer state, deliberately NOT a lint — see §5.4 |
| `quoting.space_after_quote_normalization` | :135-139 replay on `(s, off)`; `Line::erased`, `rawOfNorm` | implemented, pinned, and the raw↔normalized map is tested through it |
| `comments.script_layer` (first byte `#`/0/CR/LF) | `classifyLine`, `splitScriptLines` | implemented, pinned incl. the CRLF case; lint `indented-comment` |
| `comments.live_channels` (no stripping on TCP/HTTP/pipe) | not applicable to a file checker | see §5.5 |
| `comments.block_form` (`comment`/`uncomment` skip state) | `BlockSkipState` | implemented, pinned twice through the pair |
| `comments.verify_next` (`script_mgr.cpp:94`) | not read | §5.6 — the contract file's own recorded unknown, unchanged by this slice |
| `pre_table_commands` | §2 `:215-222`; the accepted set includes `comment`/`uncomment`, the suggestion list does not | implemented |
| `unknown_command` (no threshold, always a suggestion) | `nearestNeighbour`, no cap | implemented — and the seed's own wording says "threshold-capped unlike the engine's"; superseded, see §5.7 |
| `flag_value_grammar` (toggle \| `isTrue` \| everything else silently OFF) | `isTrueValue`/`isFalseValue`, `value != "toggle"` (case-SENSITIVE, `W_TOGGLE` is compared with `==`) | implemented, pinned (`flag stars TOGGLE` is a silent OFF), lint `silent-off-value` |
| `flag_multi_pair` (only `args.begin()` applied) | `Grammar::isSinglePairCommand` + `args.begin()` | implemented, lint `single-pair-only`; the 10-command list is §6 |
| `recording_alias_loss` (`flyto` → `camera`) | `CommandData::alias_of` from the contract file | implemented, lint `alias-respelled` |

## 4. What scedit adds that the engine has no concept of

Not divergences — the engine has no reason to carry them, and the TUI cannot
work without them. All are pinned by `tokenizer_test.cpp`:

- **Raw spans.** Every token carries the half-open raw byte range it consumed,
  quotes included. Built by reading the surviving-offset vector, never by
  arithmetic on the normalisation rules — so a change to those rules cannot
  silently invalidate the map.
- **`erased` / `rawOfNorm` / `rawToNormalized` / `normalizedToRaw`.** The bytes
  the engine deleted still exist for the author; the map says which is which.
  A cursor sitting on an erased byte still resolves to the token that swallowed
  it (`tokenAtRawColumn`).
- **`tokenTouchingRawColumn`.** The completion anchor: the caret one past a
  token belongs to that token, which `contains` deliberately does not say.
- **`Line::pairs` in line order**, next to `args` in engine order. Both are
  needed: the engine acts on one, the author reads the other.

## 5. Engine behaviour NOT reproduced — questions for the supervisor, not decisions

**5.1 `struct loop` also moves the skip flag.** `commandStruct` sets
`swapCommand = true` when `struct loop <n>` evaluates to `n < 1` (`:4691-4693`)
and clears it on X. `n` goes through
`evalString` (`$`-substitution), and the whole branch is guarded by
`ifSwap->get() != true`, so it is not statically decidable in general.
scedit models only `comment`, `uncomment` and `struct comment <v>` (`:4664-4672`,
which reach `commandComment`/`commandUncomment` directly). **Consequence:**
lines inside a zero-iteration loop are analysed although the engine skips them —
a possible false-positive source. Not present in the corpus (`struct loop 2`,
`struct loop 4` only). **Question:** treat a literal `struct loop 0` as a skip
region, or leave it?

**5.2 `ifSwap` (the `struct if/else/end` skip state) is not modelled.** Same
skip test (`:225`), driven by `struct if A <cmp> B` with runtime comparison of
`evalDouble`-substituted operands (`:4604-4660`). Statically undecidable for
`$`-bearing operands. **Consequence:** same direction as 5.1 — scedit may lint a
line the engine skips. **Question:** worth modelling the *structure* (an `if`
region is one region, whichever arm runs) so that findings inside it are
demoted rather than dropped?

**5.3 `set`'s did-you-mean names the wrong key.** `evalCommandSet`'s unknown
branch calls `searchSimilarSet(args.begin()->first)` (`:2225`) — the
alphabetically first key, not the key that failed. scedit suggests for the key
that actually failed, because the suggestion is scedit's own diagnostic and not a
relay of the engine's log. **Recorded as a deliberate divergence**, the only one
in this slice. (It is also arguably an engine defect; out of scope to fix.)

A second, non-divergent consequence of the same region IS reproduced:
`commandSet` folds with `&&` (`:2121`) and `executeCommandStatus()` returns
false once `debug_message` is set (`:1171-1174`), so the first pair that fails
short-circuits every later `evalCommandSet` call — **the pairs after it,
alphabetically, are never applied at all**. scedit says so on the pair that
causes it (`superscript.sts:1167`: `set home_planet Mars duration 5` reports that
`home_planet` is never applied, because `duration` sorts first and fails).
scedit still reports the later unknown names as well: the engine not looking at
them does not make them right, and the author has to fix all of them.

**5.4 An unclosed `"` is not reported.** The engine consumes to end of line
without error, and for `... string "Hello World` that is exactly what the author
meant — a finding there would be a C3 false positive. scedit exposes it as
tokenizer state (`Token::quote_closed`) for the TUI to render, and emits nothing.
**Question:** does the TUI want a distinct id for the case where the swallowed
tail contains what looks like further key/value pairs?

**5.5 Live channels are not covered.** `parse_model.comments.live_channels`
records that TCP/HTTP/pipe/joypad strings get no comment stripping at all. The
tokenizer's `classifyLine` gate is the SCRIPT layer's rule, so a `#`-first line
is refused a parse. A future TCP consumer needs a `tokenizeCommand()` entry point
that skips classification. **Question / note for the TCP slice**: add that entry
point rather than letting the caller pre-strip.

**5.6 `script_mgr.cpp:94`.** The contract file's own recorded unknown (a second
`#` site). Not read in this slice; unchanged. `claude/util/scedit/INTENT.md` §5
item 3 already owns it.

**5.7 The `unknown-command` seed asks for a distance threshold** ("with
Levenshtein suggestion, threshold-capped unlike the engine's"). The dispatch
spec asks for the engine's own `searchNeighbour`, reproduced. scedit therefore
emits a suggestion always, exactly as the engine does — the seed's cap is NOT
implemented. Visible consequence in the corpus: `superscript.sts:303` suggests
`nebula_names` for `date_display_number`, which is nearest but not useful.
**Question:** cap it (and at what distance), or keep engine parity?

**5.8 `font`'s family check is unarmed.** The contract names the subfamily
(`font_targets`) and the engine reads the name from `args[W_TARGET]` (`:4152`),
but the acceptance test lives in `FontFactory::updateFont`, which was not read;
and `font action initial` legitimately carries no target at all (`:4155-4162`).
Arming it on an unread acceptance rule would be a guess (C2). Reported by
`scedit --rules`.

**5.9 Non-breaking space (0xA0) is not a diagnostic.** The corpus taught this
one: `superscript.sts:94` joins `true` and `albedo` with a 0xA0 byte, which is
not whitespace in the C locale, so `lighting` takes the value
`true\xa0albedo` and `albedo 1` is dropped. scedit reports the *consequence*
(`dangling-key`) correctly but not the *cause*. A new id (`invisible-separator`?)
would say it directly — but inventing an id is outside the seeds' vocabulary.
**Question for Vixy/supervisor: add the seed?**

## 6. Two tables that live in code and should live in the contract file

Both are read from the engine with a per-entry anchor, both are already
overridable by data, and both are named here so the extraction sweep can absorb
them without archaeology (I6: the structure, not the instance).

- **Where a subfamily name sits in the line** (`sc_grammar.cpp`
  `kBuiltinPlacements`). The file says `"subfamily": "flags"` but not that the
  flag name is the KEY. Entries: `flag` → applied key (`:1183-1185`), `set` →
  every key (`:2119-2121`), `color` → value of `property` (`:1874-1885`).
  Absent = unarmed, never guessed. Overridden by a command entry's
  `subfamily_position` (`"applied_key"` | `"every_key"` | `{"value_of": "..."}`)
  the moment the file carries one.
- **Which commands apply only `args.begin()`** (`kBuiltinSinglePair`): `flag`
  `:1183`, `define` `:4484`, `add` `:4498`, `sub` `:4511`, `multiply` `:4525`,
  `divide` `:4538`, `modulo` `:4551`, `tangent` `:4564`, `trunc` `:4577`,
  `sinus` `:4590`. Overridden by `"single_pair": true` on the command entry.

Everything else scedit knows — command names, family members, obsolete tokens,
orphans, known-defective entries, aliases, lint ids and severities — comes from
`grammar/sc-grammar.json` alone. `unknown-parameter`'s argument-key half arms
per command purely by the presence of `args` data, with no code change at merge.

## 7. C3 corpus run — every finding, dispositioned

Corpus: `doc/superscript.sts` (1407 lines, ISO-8859, CRLF) + all 9 `.sts` under
`claude/harness/`. **Boundary [measured 2026-08-04]: `~/.spacecrafter/scripts/`
is EMPTY on this machine**, so the shipped-scripts half of C3 cannot run here;
it re-arms by adding those files to `SCEDIT_CORPUS` in `CMakeLists.txt` once the
data package is installed. The harness scripts produce **zero** findings.

**Zero false positives.** A false positive would be a finding on a construct the
engine accepts with the author's plainly-intended meaning; every finding below is
a construct the engine reads differently from the author's evident intent, with
the engine site named. No rule was weakened to reach this.

| line | id | disposition | argument |
|---|---|---|---|
| 57 | duplicate-key | TRUE, benign | `halo true` appears at token 16 and again at token 22 of the comet body line. `arguments[key] = value` on a `std::map` (`:166`) keeps the last; both values are `true`, so the effect is nil and the construct is still real. |
| 94 | dangling-key | TRUE, high value | Byte 432 of the line is **0xA0 (non-breaking space)**, which is not whitespace in the C locale, so `>>` reads `true\xa0albedo` as ONE token: `lighting` takes that value and the trailing `1` becomes a key with no value, dropped at `:148`. `albedo 1` never reaches the engine. The cause (0xA0) has no seed id — see §5.9. Three further 0xA0 bytes exist in the file (lines 523, 1086, 1098), all inside column-1 comments, correctly silent. |
| 303 | unknown-parameter | TRUE | `flag date_display_number on/off/toggle`: `date_display_number` is in no family (the registered spellings are `datetime_display_number` / `datetime_display_position`, and they are `set` names, not flags). `setFlag` (`:309-315`) fails the `m_flags` lookup and the line has no effect. The suggestion (`nebula_names`) is the engine's own nearest neighbour, reproduced — see §5.7. |
| 306, 309 | unknown-parameter | TRUE | `set date_display_number 3` / `set date_display_position 120`: `parseCommandSet` returns `APP_FLAG_NONE` and `evalCommandSet` reports `command_'set': unknown argument` (`:2221-2226`). Old spellings; the suggestions are the right current ones. |
| 930 | deprecated | TRUE | `movetocity` is on `obsoletList` (`app_command_init.cpp:16`) and is not in `m_commands`: the engine takes the unknown-command path, `searchNeighbour` returns early on the obsolete test and logs "no longer used in software". |
| 1133, 1153 | inert-command | TRUE | `set mode "InGalaxy"`: `case SCD_NAMES::APP_MODE: break;` (`:2228`) — parses, reports success, calls nothing. Recorded in the contract file as `set_names.known_defective.mode`; scedit reads it from there. |
| 1167 | unknown-parameter | TRUE, high value | `set home_planet Mars duration 5`: `duration` is not a `set` name, and it sorts BEFORE `home_planet`, so the `&&` fold (`:2121`) short-circuits and **the whole line does nothing** — not just the typo'd half. See §5.3. |
| 1205 | dangling-key | TRUE | `set stall_radius_unit = 5.0`: `=` is not syntax here, so the pair is `stall_radius_unit` → `"="` and the real value `5.0` is dropped at `:148`. The preceding comment (`ancienne version`) confirms the line is a half-finished migration. |
| 1269, 1271 | indented-comment | TRUE | `  # should do it four times` / `  # but in fact do it just one`: the script layer tests `line[0] != '#'` (`script.cpp:114`), so these reach the parser, the leading spaces are stripped (`:129-132`) and `#` becomes the command — an unknown command, logged with a did-you-mean, once per loop iteration. |

Findings by id: `dangling-key` 2, `duplicate-key` 1, `unknown-parameter` 4,
`deprecated` 1, `inert-command` 2, `indented-comment` 2 — 12 total, all in
`doc/superscript.sts`, all engine/data findings rather than lint noise (C3's
"true findings in shipped content are recorded upstream instead").
