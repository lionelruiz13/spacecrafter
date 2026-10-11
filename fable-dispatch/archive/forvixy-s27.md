- **Session-27 decision items (2026-09-07, the SATURDAY-TOOL / TWO-STEP-SOLVER / DUMP-CHANNEL round —
  F103 · F104 · F105, three for three). Q-70's shape: one decision per node, its held set at the node,
  anchors by ID and symbol, correlated items together. Nothing asked in-session (Monday). The first
  node is the one Saturday's operation needs.**

  ```
  YOURS — nothing here substitutes them
  [Y8] §5.147 · the rewrite COLLAPSES 13 commits ─ decide BEFORE Saturday's supervised-by.sh run;
       irreversible after the force-push
       fact    master-beta carries the same 13 commits TWICE: cebebf44 (GPG-signed by GitHub, Lionel's
               "Update install_dependancies_ubuntu.sh", 2026-04-30) beside its unsigned twin b8dddd6c —
               same tree, author, date, subject, same parent 1ddd32f0 — rejoining at ba9df31f;
               `git commit-tree` drops gpgsig, so the rewrite rebuilds the signed chain INTO the
               unsigned one: 3829 → 3816 commits, range 94 → 81, reproduced twice byte-identically,
               while the tool's content assertion PASSES (the tip tree is unchanged); the 13 old
               shas are cited in INTENT/11.203.md only, in 0 trailers, and are ABSENT from the map
       (i)     ACCEPT AND MAP — widen build_map's twin search to the whole new history so the map
               carries cebebf44 → b8dddd6c and the twelve, step E repoints them; the signature
               stays lost; cheap, each has exactly one fingerprint twin
       (ii)    REFUSE AND RESOLVE FIRST — the tool STOPs on any unpaired commit; the duplicated
               chain is resolved deliberately before any rewrite (the tool's own philosophy)
       today   the tool WARNS (a §2(f) block at the map, READ BEFORE PUBLISHING at the end, Undo
               printed) and proceeds
       ask     (i) or (ii) — one word
       →       §11.224(h) · §5.147 · DEPLOYMENT-MAP R5
       ANSWERED [vixy 2026-09-07, ~17:50] → §11.227: "the rewrite should only rewrite the commit
       message (and author) on commit Claude is the author, from the last pushed change to the
       HEAD … the commit themselves shouldn't be affected, so their signature shouldn't move" —
       NEITHER (i) nor (ii): the collapse is the TOOL's defect (filter-branch rebuilds every
       commit in the range; commit-tree drops gpgsig); a commit-filter that leaves unselected
       commits untouched keeps cebebf44's signature and the 13 shas (its parent 1ddd32f0 is
       already pushed) — F106, dispatched the same session; the one unavoidable case (a signed
       commit DOWNSTREAM of a rewritten one) is empty here and will be reported by name
       DELIVERED 18:4x → §11.228 (F106), accepted by my own run on the scratch pair: 3829 stays 3829,
       cebebf44 keeps its sha and its gpgsig, 15 of 15 side-chain shas kept, the merge keeps both
       parents, 0 signature warnings; on the live pair 83 of 98 commits will change sha (82 yours as
       Claude + 30ea1f8d on top of one of them), 15 keep their object — NOTHING is left for you to
       decide before the run; §5.147 FIXED

  [Y4] the tool you operate Saturday ─ what changed, in the order you will meet it
       state   RUNNABLE NOW, and was not before this round: build_map's `[ ] && printf` returned 1
               on its last iteration and `set -e` killed every run AFTER the code rewrite and
               BEFORE rollback (D1: 73 of 138 trailers left dangling, in silence); build_repair_map
               re-derived "already dangling" AFTER the rewrite and aborted every run on a pair with
               a pre-existing dangling trailer (D2) — this pair has 2 (e5f0889b, 5ad587af, twins
               unique). Both fixed, each with a mutant on record
       B1      a renamed branch now STOPs the preview (exit 1, `--dry-run` too) with a §2(f) block;
               `--branch-alias=master-beta=<new-name>` is the one flag that proceeds; the rename
               and the tool are now order-INDEPENDENT (F92's checklist needs no reordering)
       maps    claude/sha-maps/<UTC>-<code8>-<harness8>/{code,harness,repair}.tsv + a written-once
               README, committed by the closing commit (unconditional once a map exists) — your
               "commit tracking" was READ as this map; your correction may differ
       today   `--dry-run` on the live pair: 94 code / 805 harness selected, 138 distinct trailers,
               2 already dangling, no STOP; deterministic (two runs, byte-identical maps)
       skip    ONE pin the parser SKIPS silently: harness d1f8fea's trailer reads
               `Code: master-beta @ 3ffea018 (the mutation is not in it)` — outside the grammar; the
               map resolves it after the run; the parser's silence is a residue (queue 7)
       →       §11.224 · README.md §sha-maps · harness/README.md §supervised-by.sh · §11.212(g)

  [Y2] §5.145 FIXED on your ruling as corrected ─ two veto points, each cheap to reverse
       scope   read as the iterative CLASS (all four advancing branches of eccentricAnomaly + both
               comet steps); Eris's branch alone was built as the MUTATION: indistinguishable on
               the engine (120/120), so the two forms differ in COST only — 2.36 µs/frame worst
               case = 0.24 % of D11 for the class form, the narrow form's share smaller
       old     orbit.cpp is shared: the old path converges twice as fast per frame after a date
               jump — strictly more exact on the comparison baseline (as-if)
       count   still yours: two steps sufficed for Eris by four orders (1.1987° → 1.06e-05°); the
               alternative shape — iterate to a criterion — already exists in the dead warp()
               methods (§11.225(j1))
       →       §11.225 · §5.145 · §11.223(b) · §11.76(b)

  [Y9] §5.148 · tracking a ZERO-VECTOR body poisons the old path's view state ─ 66 NaNs, on the
       baseline binary too
       fact    `select` + `flag track_object on` on any of the 19 dist-0 records (18 star systems +
               Universe; now also the 8 anchors under Universe, refused a frame by F105) writes nan
               into helioToEye / matLocalToEye / matJ2000ToEye / matEarthEquToEye / headingVector /
               moveAim; the navigator step that first produces it is not read
       ask     with §11.216(i)'s question, one answer: what a body with no position answers to a
               track — refuse with a §2(f) line at the tracking seam, or a defined aim
       →       §11.226(f) · §5.148 · §11.216(i)

  [Y3] §5.146 ─ ANSWERED [vixy 2026-09-12] → §11.233(f): "it should throw whenever it can't allocate" —
       the shape; EntityCore, coupled to your submodule push before the intern's clone (§11.204(b));
       was: carried: minted on your word "edit it" (SharedBuffer's bind/release guards + the
       contract line at BufferMgr::releaseBuffer, EntityCore)              → §11.223(c) · §5.146
  [Y1] §5.142 · the pool policy ─ carried (grow / refuse / degrade)          → §11.222(h)
  [Y5] §5.144 · the 8–12 h leg ─ carried  ·  [Y6] §5.140 · orbit_lon ─ carried
  [Y7] scratch trees ─ sc-f103 (959 M: the clone pair + the P4 maps' object store), sc-f104 (898 M:
       the pre/post/mutant binaries), sc-f105 (955 M) join the list; sc-f98 (1.8 G) still holds the
       only complete arm-C applog

  VETO POINTS taken (implemented-and-live, each cheap to reverse; silence = endorsed)
  [V1] F103  `--branch-alias` in the `--author-fix` shape; the STOP at step A'; the maps' location
             and naming; the closing commit unconditional once a map exists; D1/D2 fixed under the
             forced-expansion rule (both blocking for your run)
  [V2] F104  the class scope ([Y2]); ITERATIVE_STEPS_PER_CALL = 2 in iterative_orbits.hpp, one home
  [V3] F105  useNow() returns bool; a use in a NEVER-published frame is REFUSED with one D12 line
             per body (the 8 keep their honest zero); the header's two fields; a dump a use for
             120/120 (the 2 anchors under Earth/Moon now move with the frame)
  [V4] two §5 mints at acceptances (§5.147 [Y8] under §5.131's precedent — a repository-state
             property in the defect register; §5.148 [Y9])
  [V5] archival pass 19; this block

  HELD OPEN, not absorbed
  [H1] §11.225(j2)/(j3): the new path's orbit-line sampler walks the position solver's own seed
       (180 dates through iterativeLastE), and Eris's pre-fix staleness (+32 rad) is unattributed
       — a leg, next round (S)
  [H2] §11.226(h): after `body action reload` the two anchors under Earth/Moon are refused for
       ONE frame (their creator uses them before the walk publishes) — measured self-healing; the
       line names both ways to be there
  [H3] the guard cannot tell "outside the walk" from "not walked yet" — needs the system's
       identity inside ModularBody: your nested-modular-bodies line (§11.223(e))
  [H4] instrument residues: purge-path.sh rewrites the same way and writes NO map (README corrected
       at acceptance); list_code_trailers skips an unparseable `Code:` line silently; the scan's
       EVENT lexicon carries no FIXED (by ruling), so B1's FIXED marker is invisible to it
  [H5] §5.84's own trigger still unmeasured; §11.215(g)'s four bodies' ulps predicted to change,
       not re-measured (a 3 h soak)

  FACTS, no decision asked
  [F1] three for three delivered AND supervisor-verified same session, every delivery re-run by my
       own hand; code 4cd00139 → 7ceea976 (four executor commits); binary b5f08778 → e411b838
  [F2] exactly ONE readout moved across the round at the un-moved launch state: Eris's parked
       position (1.1987° → 1.06e-05°); the F91 table's md5 is now 1fe630a4 (c125adf0 retired)
  [F3] eleven dispatcher defects this round (3 value-class, 8 structure-class), all output-side,
       all caught before a delivery; three more caught AT THE MINT by the instrument
  [F4] the host: same boot, `:2` alive with sessions 14/15, RAM 52 GiB at open and close; 41
       launches, no HOST-EVENTS entry owed
  ```

