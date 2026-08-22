# `util/new_parser_scripts/` — identification, verdict, extraction

Produced 2026-08-04 for scedit `INTENT.md` §5 item 9 (witness mining), deliverable B.
Write surface: `util/scedit/grammar/witness/` only. Nothing outside it was touched.

---

## 1. What it is

**A documentation toolchain for the script command surface, plus its source text and
one generated output.** It is not a parser for `.sts` scripts and has no connection to
the engine's own parser.

Twelve files, three roles:

| role | files | evidence |
|---|---|---|
| the tool | `main.cpp`, `TextToHtml.{cpp,hpp}`, `FileReader.{cpp,hpp}`, `FileWriter.{cpp,hpp}`, `Makefile` | `main.cpp` hard-codes `source = "input_fr.txt"`, `sourceCSS = "style.css"`, `destination = "resultat.html"` and runs `TextToHtml` over the text |
| the input | `input_fr.txt` (28 763 B), `style.css` | `README.txt`: *"Parser (texte -> HTML/CSS)"* |
| the output | `resultat.html` (148 478 B) | produced by the tool from the two inputs |

Authorship and date, verbatim from `README.txt`:

```
Auteur: Nicolas Barile <n.barile.57@gmail.com> pour association-sirius.org
Mise à jour le 3/06/2020
```

`README.txt` also states the block grammar of `input_fr.txt` in full (`NAME`,
`ARGUMENT`, `PARAMETER`, `$`, `EXEMPLE`, `@@`, `NAME END`, `IMG`) — the file is a
*declared* format, not an ad-hoc note, which is what makes machine extraction safe.

Nothing in the repository references the directory: `grep -rn "new_parser\|input_fr\|resultat.html"`
over `*.txt *.md *.cpp *.hpp *.am` outside it returns nothing. It is a standalone tool
that was run by hand.

## 2. Witness-tier verdict: **YES — it is documentation of the same surface, and it is
richer per key than `superscript.sts`. But it is NOT `[superscript-attested]`.**

Why it qualifies as documentation:

- it is *per command AND per key AND per value*: 37 command blocks, 98 `ARGUMENT`
  slots, 158 `PARAMETER` slots, 141 enumerated values, 471 description lines,
  10 example lines, 0 lines my parser could not place;
- it states things the code does not: which keys are **required**
  (*"nécessite name"*, *"obligatoire"*), which are optional, and **defaults**
  (*"par défaut, step est fixé à 2"*, *"vaut 10 par défaut"*, *"par défaut, la
  répétition n'est pas active"*) — exactly the C6/D31 material;
- its key vocabulary checks out against the merged grammar at a high rate:
  **125 keys present / 10 absent** on the commands with `args_complete`,
  **69 flag names present / 1 absent**, **29 set names present / 2 absent**
  (13 more unverifiable because `body`/`camera` carry `args_complete:false`).

Why it is a **separate tier** and stays labelled as one:

- C2's amendment of 2026-08-04 names `doc/superscript.sts` and nothing else. Vixy's
  reliability statement (*"it might have (rarely) been wrong if it wasn't understood
  when documented"*) was made about that file, by the person who owns it.
  `input_fr.txt` is a different document by a different author, and no statement about
  its reliability exists.
- It carries at least one claim that is simply **wrong against code** (§4, `print`),
  and one **internal contradiction** (`timerate step` default 2 vs `multiplier step`
  default 10, in the same file).

Claims extracted from it are therefore tagged **`[new-parser-fr-attested: line]`**,
never `[superscript-attested]`, and the file is stored separately
(`new-parser-witness.json`).

> **SUSPENDED FOR VIXY (S-NP-1)** — does `util/new_parser_scripts/input_fr.txt`
> join the anchor classes of C2 as a second usage-witness (tier: below code, beside
> superscript.sts), and with what reliability statement? It is the only per-key,
> per-value documentation of the command surface that exists in the tree. It is also
> French-language: if it becomes an anchor, the C6 doc lines still have to be written
> from code + cross-check, not translated — that is assumed here, not decided.

## 3. Extraction

Full structured extraction: **`new-parser-witness.json`** (same directory), same rules
as the superscript record — every description line kept **verbatim** with its line
anchor, nothing translated, nothing summarised, plus a per-slot cross-check row
against `sc-grammar.json`.

French is kept as written. A translation is an interpretation, and the zero-knowledge
doc bar (C6) is to be met from code plus a cross-checked witness, not from a paraphrase.

## 4. What the cross-check found

### 4.1 Four commands documented that the engine does not register

| block | line | what the doc says | engine |
|---|---|---|---|
| `sun_trace` | 855 | *"permet de représenter la course du soleil en direct sous le dôme"*, *"cette commande est un alias de la commande body_trace avec le soleil comme astre sélectionné"* | the registered spelling is `suntrace` (`ACP_CN_SUNTRACE`, base_command_interface.hpp:384) |
| `sky_draw` | 829 | *"affiche une série de points sur le dôme"*; `action clear` = *"supprime tous les points affichés"*; `points` = *"une série de points sous la forme XXXYYY ou (XXX,YYY) représentent une position sur le dôme exprimée en pourcentage"*, three digits per coordinate, several points per argument | **no such command and no such flag.** `#define ACP_FN_SKY_DRAW "sky_draw"` (base_command_interface.hpp:477) is registered nowhere — this is the `sky_draw` orphan the scedit seed pass recorded (INTENT journal 2026-08-03) |
| `multiplier` | 595 | a time-rate command: `rate`, `action increment/decrement`, `step` *"vaut 10 par défaut"* | not registered; the live command is `timerate`, which the same file documents separately at line 919 with `step` *"fixé à 2"* |
| `look` | 506 | `delta_az`, `delta_alt` | the command is `look_at` and its keys are `delta_azimuth` / `delta_altitude` |

**`sun_trace` is the highest-value line in this file.** It states the intent behind
`suntrace` — *an alias of `body_trace` with the Sun as the selected body* — which is
precisely the behaviour §5.91 / SCRIPT_SURFACE **SS-11** reports as broken
(`suntrace pen on` traces whatever body was last traced, because the pen branch reads
`args[W_SUN]`, the value of a key spelled `sun`). It does not decide SS-11 — that is
Vixy's — but SS-11 was asked with no documentary evidence at all, and now there is
some, from 2020, in a second hand.

**`sky_draw`** answers, at the intent level, what the orphaned flag define was for.

### 4.2 Ten keys, one flag name and two set names documented that do not exist

| command | slot | line | engine |
|---|---|---|---|
| `body_trace` | `hidde` | 118 | the key is `hide` (a typo in the doc) |
| `date` | `relativeYear`, `relativeMonth` | 217, 219 | the keys are `relative_year`, `relative_month` |
| `date` | `sideral` | 223 | the key is `sidereal` |
| `moveto` | `name` | 585 | no `name` key on `moveto` — **the same belief as `superscript.sts:912`** (`moveto … name marseille`, SS-4) |
| `print` | `value`, `name` | 662, 665 | `print` takes ANY key; each pair becomes one log line `[key] text` (app_command_interface.cpp:1971-1981). See §4.3 |
| `select` | `pointeur` | 714 | the key is `pointer` (French spelling in the doc) |
| `text` | `latitude` | 898 | the key is `altitude` |
| `wait` | `action` (`$ reset_timer`, *"reset le script"*) | 943 | `wait` reads only `duration` / `loading` / `video_termination` — **the same belief as `superscript.sts:1366`**, SS-10 |
| `flag` | `sky_draw` | 398 | not a registered flag name (the orphan above) |
| `set` | `flight_duration`, `duration` | 737, 743 | not registered set names — **`set … duration` is the same belief as `superscript.sts:1167`**, SS-2 |

**Three of these are independently attested by both witnesses**: `set … duration`,
`moveto … name`, `wait action reset_timer`. Two documents, two authors, the same three
spellings. That materially changes the disposition question SS-2/SS-4/SS-10 put to
Vixy: a spelling that two independent documents teach is less likely to be a slip and
more likely to be a feature that went away.

### 4.3 One claim decided against this witness by code

`print` (line 659-667) documents a fixed `value` (message, *"obligatoire"*) and `name`
(tag, *"facultatif"*). The code takes **any** key and logs one line `[key] text` per
pair (app_command_interface.cpp:1971-1981; grammar `commands.print.key_grammar`).
`superscript.sts:1029-1032` has it right — *"Output is the console and logs formatted
`"[argument] value"`"*, written `print "A=" a`. Tier discipline applied: code wins,
the divergence is recorded, and the doc line for `print` comes from the code.

### 4.4 Claims that check out and fill gaps

| claim | line | code cross-check |
|---|---|---|
| audio volume is 0…128 | 4 | `SDL_MIX_MAXVOLUME` is 128 (`/usr/include/SDL2/SDL_audio.h:1085`); `Audio::setVolume` clamps to [0,128] (src/mediaModule/audio.cpp:260-268) — **consistent** (and the same as `superscript.sts:15`) |
| `volume increment` raises it *"de 5"* | 22 | `#define STEP_VOLUME 5` (src/mediaModule/media.cpp:32) — **consistent** |
| audio volume default *"il est fixé à 85"* | 4 | **off by one**: `master_volume = SDL_MIX_MAXVOLUME/3*2` (src/mediaModule/audio.cpp:62) is integer arithmetic — 128/3 = 42, ×2 = **84**. The doc computed 128×2/3 ≈ 85. Recorded as a divergence, not fixed |
| `timerate step` default 2 | 935 | **consistent but incomplete**: 2 for `increment`/`decrement`, 1.05 for `sincrement`/`sdecrement` (grammar `timerate.step`) |
| `select pointer` on by default | 716 | **consistent** (grammar `select.pointer`; same as `superscript.sts:1098`) |
| `domemasters action snapshot` writes into `snapshot/`, `action record` into `vframes/`, and re-running `record` stops the capture | 252-256 | **not cross-checked here** — `domemasters` is never exercised by `superscript.sts` either (it appears only as commented-out lines), so these three facts are the only documentation of that command in the tree. Left `[new-parser-fr-attested]`, unverified |

### 4.5 Coverage of the command surface, both witnesses together

- documented by `input_fr.txt`: 33 registered commands (+4 unregistered spellings);
- **not** documented by it: 29 registered commands — `add camera comment constellation
  define divide dso2d dso3d flyto font galaxy_stars get heading look_at mode modulo
  multiply random screen_fader search session sinus struct sub suntrace tangent
  transition trunc uncomment`;
- of the 12 commands `superscript.sts` never EXECUTES, `input_fr.txt` documents three:
  `domemasters` (line 247), `shutdown` (861) and `suntrace` — the last under the
  unregistered spelling `sun_trace` (855). Two of those three (`domemasters`,
  `shutdown`) also survive in `superscript.sts`'s comment layer, as commented-out
  lines it tells the reader to enable (*"hash has to be removed"*);
- **documented by neither witness**, and therefore with no documentation anywhere in
  the tree except the code itself: `dso2d`, `flyto`, `galaxy_stars`, `get`, `modulo`,
  `search`, `session`, `sub`, `transition` — nine commands.

## 5. What was NOT done

- `resultat.html` was not parsed. It is generated from `input_fr.txt` by the tool in
  the same directory; treating it as a second source would double-count one witness.
  It was not diffed against a fresh run either — the tool was not built or executed
  (nothing here needs it, and building it is outside the write surface).
- No French text was translated into a C6 doc line. That step waits on S-NP-1.
- `style.css`, `TextToHtml.cpp`, `FileReader/FileWriter` were read only far enough to
  establish the roles in §1.
