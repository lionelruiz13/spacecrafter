- **F110 — THE INTERN'S ENTRY PATH, FOLLOWED BY HAND. Three gaps only you can fill, quoted from
  the document's own lines, and one finding that is a decision rather than a defect.** The
  rehearsal ran the whole path from a plain clone (§11.234): the clone, the build, the D14 gate,
  the smoke suite on both arms, scedit's `cmake`/`ctest`, the hook, the harness README's first
  command. Everything measurable was measured and corrected in `doc/developer-entry.md`; these
  four were NOT written, because they are yours.

  ```
  YOURS — nothing here substitutes them
  [Y5] R23 · the content-installation procedure ─ his clone runs EMPTY and nothing tells him why
       doc says  "by default only limited catalogues are loaded, and the correct ones are loaded
                 by 'an outside installation procedure' [owner, R23]" and, next sentence,
                 "This repository does not document that procedure, and where it lives and who
                 owns it is the one thing still to ask the owner."  INSTALL sec.5 agrees: the
                 tree ships NO content.
       measured  a tree install is the binary, the shaders and eleven metadata files
                 (§11.204(f)); a first launch then prints ten "Completed copy of ..." lines over
                 ten EMPTY directories (§11.204(i), §5.132) — so the log cannot tell him either
       ask       where the procedure lives, who owns it, and whether it may be named in the code
                 repo. One sentence in INSTALL sec.5 closes it. Until then his first run is a
                 program with no sky and no way to find out that this is expected.
       →         §11.232(c)4(i) · §11.234(l) · R23 · §5.74

  [Y6] §9 · "Engineering principles" is a placeholder addressed to you, and he meets I1-I7 in week 1
       doc says  "**Placeholder -- to be written by Calvin Ruiz, the project owner.** ... The
                 ledger argues from a set of engineering invariants it cites by number, I1 to I7
                 ... **Neither repository states what they say.** The text is the owner's and
                 lives outside both, so it is not restated here: a paraphrase of a principle you
                 cannot check against its author is worse than an empty section. Until he fills
                 this in, read an I<n> citation as a pointer to him."
       ask       the text, or permission to reconstruct it from the ledger's uses FOR YOUR
                 CORRECTION (I2 alone appears 33 times, so the uses are dense enough to draft
                 from — but a drafted principle is exactly the thing the section refuses to do
                 unchecked). Either answer closes it; silence leaves him citing a pointer.
       →         §11.232(c)4(ii) · §11.234(l)

  [Y7] the remote form ─ he cannot run the document's second clone at all
       doc says  "git clone -b CC-harness <same-remote-url> claude", the remote being
                 git@github.com:lionelruiz13/spacecrafter.git [observed: git remote -v]
       measured  the rehearsal substituted a LOCAL path and says so at the step; it is not a
                 substitute he has. An SSH form needs a key on that account.
       ask       a deploy key / collaborator access, or the HTTPS form written into the document.
                 NB the choice is not cosmetic: the rehearsal measured that a clone over the git
                 transport does NOT carry unreachable objects, which is why the stale sha in this
                 document read "bad object" for him and "exists" for me (§11.234(e)).
       →         §11.232(c)4(iii) · §11.234(l)

  [Y8] THE BUILD EVERY MEASUREMENT USES IS NOT THE BUILD THE DOCUMENT SHIPS — your call, not a defect
       measured  build-claude (the binary every number in this ledger is taken on, 6d63e6c1) is
                 configured RelWithDebInfo and compiles `-O2 -g -DNDEBUG`; install_src.sh's
                 documented path configures Release and compiles `-ggdb3 -Ofast -Wall -O3`.
                 CMakeLists.txt branches on Debug (:101), Release (:118) and LocalRelease (:134,
                 :142) ONLY, so RelWithDebInfo takes none of them, CMAKE_CXX_FLAGS stays empty
                 and CMake's own RELWITHDEBINFO default applies. Both read from each build's own
                 flags.make; sizes 191 022 536 B against 218 127 672 B.
       why it    §2.0 D11 prices everything in 1 ms/frame. Every cost number this project has
       matters   published was measured on -O2 while a user runs -Ofast -O3.
       ask       re-point build-claude at Release and re-bank, or keep -O2 and state it as the
                 measurement platform. NOT touched here: re-configuring build-claude would
                 invalidate the canary's photometric band and every baseline standing on it.
       →         §11.234(g)

  ALSO YOURS, one line each, no answer needed today
  [Y9] whether every inline §5 row should carry its own "§5.N" on its line — the convention
       change §11.233(h) raised. The RESOLVER now exists either way
       (claude/intent_resolve.py, and the entry document tells him to use it), so this is a
       preference about the file's shape, not a blocker. → §11.233(h) · §11.234(k)
  ```


  ```
  YOURS — from the later deliveries (supervisor, at close)
  [Y10] §5.142 · the ROOT you asked for has numbers, and four decisions are yours (§11.236(k))
        fact    1344 B per 06.sts body = 320 old-eager + 1024 new-at-load; only 88 B is a per-body
                requirement (128 carved → 8192 bodies/MiB); 768 B is ONE shadow-receive array
                embedded in FOUR blocks (a ringed ray-capable body carries it three times); 448 B
                are matrices derivable from their neighbour field; a body nobody draws holds 1344
                carved bytes containing 4 written ones — 06.sts's 1013 all do
        ask     (O2) cap the shadow-caster array — VISIBLE (the weakest occluder dropped), not
                as-if; (O2') the receive array out of the per-body block — an arena in your
                EntityCore; (O1) carve at first draw — moves the wall from load to camera;
                (O5) a NAMED refusal after any of them. The options table with its arithmetic is
                §11.236(i); the pool stays 1 MiB, by your word
        facts   shaders/src/receivedShadowsDecl.glsl:20 re-declares MAX_SHADOW_CASTERS = 8 with
                nothing tying it to bodyShaderInterface.hpp:47 (an I2 double — live the day O2
                edits one of them); the field's max_shadow_cast = 8 against the code default 4
                (context.hpp:194) leaves half of every receive array unfillable on a default config
        →       §11.236(g)(h)(i)(k) · §5.142 · §11.233(b) correction

  [Y11] §5.115 · the 1 GiB bound is LIVE ─ five veto points, each cheap to reverse (§11.237(m))
        V1      the RULE: the channel holding the MOST bytes rotates — neither "the channel being
                written" (11 987 rotations for 11 987 lines with the excess intact) nor "all five";
                measured, one line to revert
        V2      an in-session rotation spends one of the eight launch slots
        V3      WHAT THE BOUND COSTS IN HISTORY — built as your words, "never hold more than
                1 GiB": at 193 MB/h the first crossing comes after ~5.5 h and the gigabyte is gone
                in ~40 s. "Keep the LAST 1 GiB" is a different shape (a 128 MiB live-file bound
                per channel), named and NOT built — say the word if that is what you meant
        V4      no config key (F108's argument)  ·  V5 the number
        also    a directory left over budget by the OLD build rotates during startup before the
                console exists — the lines are now replayed to it (the second commit, unasked,
                measured 0/6 → 6/6)
        →       §11.237(d)(e)(i)(m) · §5.115 (size bound FIXED, density half OPEN)

  [Y12] §5.153 · a commanded view duration UNDER 0.2 s is not honoured by the drawn path ─ minted
        at F114's acceptance: Camera::lookTo floors every plan at 0.2 s (Camera.cpp:932); at
        `duration 0.1` old lands at 0.105 s and the two paths are 38.35° apart at that instant.
        Snap, clamp as today, or run the plan at the commanded time — perceptual, yours
        →       §11.235(j)1 · §5.153

  [Y13] §5.149 · the constructor's JD-0 seed ─ carried: the pointer you could not resolve this
        morning is the inline row INTENT.md:515 (python3 claude/intent_resolve.py 5.149); the
        shapes (i)–(iv) are at §11.229(h1); today the two-step fix pays for it
  [Y14] §5.146 · SharedBuffer THROWS on a refused allocation ─ your shape is recorded; the edit is
        EntityCore, and a pin naming an unpushed submodule commit breaks the intern's
        --recurse-submodules clone THIS WEEK (§11.204(b)) — your one line, or dispatched in the
        same act as your EntityCore push                                    → §11.233(f)
  [Y15] §5.150 · two halves NOT minted at F111's acceptance ─ the special-orbit half (Europa
        3.5e-08 AU through the static ephemeris cache, walked identically by the OLD path) stays
        a caveat at §5.84; the trail walker (3.3 arcsec one frame after an unhide, new-path-only,
        no old shape to restore) is a rider with its engine leg owed. Say the word if either
        deserves its own row                                                → §11.239(f)(h)
  [Y16] scratch trees ─ sc-f110 2.6 G · sc-f116 557 M · sc-f112 532 M · sc-f114 185 M ·
        sc-f111 276M · sc-f115 176 K join the list (21 G across all sc-* before sc-f111); the
        two registered worktrees sc-f89/tree and sc-f107/tree still stand

  VETO POINTS taken (implemented-and-live, each cheap to reverse; silence = endorsed)
  [V1] the canary re-bank on the 09:51:57 boot (the block's own prescription); the full band
       reproduced on a FOURTH boot
  [V2] archival pass 21 at open; F113 kept as the WITHDRAWN record of my misread (ids never reused)
  [V3] F110: eleven doc hunks (+98/−24, each traced to a rehearsal step); f85_links.py's commit-sha
       class; claude/intent_resolve.py as the ledger's resolver (the doc names it)
  [V4] F114: the two sites; Camera::oldLocalToLocal as the ONE home of the init-view conversion,
       ssystem_factory.cpp rerouted through it (outside the section's file list — I2)
  [V5] F116: the rule (V1 of [Y11]) and the startup replay
  [V6] F112: ONE implementation with two entry points; sc_gpu.py as a third home with the NUMBER
       banked in the canary; gpu.headroom a GATE on both arms at 6144 MiB FREE; b22_live_run.sh's
       kill routed; F90_SKIP_CANARY=1 now leaves a trace; the doc's §5 item 2 corrected
  [V7] F111: the const_cast (a named compromise); the pair's return value ignored; §5.150 flipped
       on its iterative half only
  [V8] §5.153 minted; B41 (the preload signal) opened in §13.B from your words; the §3 nodes of
       sessions 26–28 answered by §11.232/§11.233; §0.5's instance bullet rewritten from §11.238(l)
  [V9] this block

  HELD OPEN, not absorbed
  [H1] the DSO design pass — dso3d + ojmMgr as ModularBody, tully a Milky Way BodyModule, the
       as-if rule the reach/visibility criterion (§11.233(c)) — dispatchable, M–L, first among the
       design items
  [H2] B41 the preload signal — the zoom half has no anchor (zoomToBothPaths does not know the
       body); decision-free once one is chosen (§11.235(l))
  [H3] the trail walker's engine leg (it fires on an edge; S) · §5.150's special half as a caveat
  [H4] the rewrite tool's class gap: citations in the CODE tree (developer-entry.md's harness shas,
       scedit's anchor_pin 54a2b844 in four grammar fragments) are not repointed; with your Saturday
       run done, the (7) tool items are dispatchable (purge-path.sh's map, list_code_trailers' skip)
  [H5] [Y8] the build type — every D11 number on -O2 while the documented build ships -Ofast
  [H6] the intern's three gaps [Y5]–[Y7] — the only items with a clock (this week)
  [H7] §5.115's density half; the EntityCore log pile (395 files today); the drawn orbit line has
       no observable (a readout of orbitPoint[] first, §11.239(l)4)

  FACTS, no decision asked
  [F1] six for six delivered AND supervisor-verified same session, every delivery re-run by my own
       hand (F91 on every engine binary — d607cfdc, 95087b68, 42d7982c — table 1fe630a4 each time);
       code fcc277c9 → 7be23468 (five executor commits: the doc, the zoom pair, the log bound ×2,
       the sampler); binary 6d63e6c1 → 42d7982c
  [F2] the host: rebooted 09:51:57; your rewrite + push at 11:03 (0 unpushed then); a resident 27B
       model until 11:32, then your java (21 GiB RSS, up to 2.9 GiB VRAM) — its 15:48–16:21 VRAM
       crossing cost F112 33 minutes and one resume, and the precondition that refused it was MINE
       (used ≤ 4000: the wrong shape; the derived gate reads FREE ≥ 6144); 25+ canary runs, green
       after the 11:34 re-bank throughout
  [F3] SIXTEEN dispatcher defects (5 value, 10 structure, 1 intent), all output-side, none reaching a
       delivery: the intent one is the §5.142 misread (Q-04's second instance in five days; the
       prevention: every clause of an incoming ruling located in the inferred structure before the
       first propagating edit); two values quoted after my own writes had moved them (Q-67's new
       sub-class); one commit whose message described edits its tree lacked (Q-56's third); the rest
       structure asserted without the read (a grep read as a census, twice; a shared object that was
       two; a launch precondition typed instead of derived)
  [F4] for the intern: his plain clone shows none of claude/; c5be42b is ABSENT for him (a network
       clone carries no unreachable objects); the resolver exists; the canary's per-host trap is
       now said in the document; PRs target 2023-master as the doc says, on your "as is"
  ```

