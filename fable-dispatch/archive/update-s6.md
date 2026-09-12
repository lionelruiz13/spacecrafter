**Update [Fable 2026-07-31, supervising session 6]:** round of 3 per the sizing lesson:
**F16 → F17 → F18**, minted below from session 5's own close queue (F16 before any
hunt — §5.51 masks the instrument; B7-hunt-4 after F16; G4 batch). Third slot kept on
the G4 batch over the next B31 slice: the session file carries two carve-outs behind
late-August decisions (D28/D21) and takes a revisit wave regardless, while §5.52 is a
user-visible hole in D3's common case. Warm-up: both trees clean (code `a958c05e`,
harness `e006927` — the harness moved past session-5 close: cadence corrections
§11.122(o)/§11.123(o)(o2) + INTENT archival pass 1, all verified, no code change);
binary present (mtime 03:22, consistent with F9's build order; first executor rebuilds
regardless). DECISIONS_PENDING open set at session start: **D15, D21 (both SCHEDULED
last week of August, §11.116(a)), D37 (awaiting Vixy)**.
**Round outcome (session 6 close, 2026-07-31):** F16 → §11.124, F17 → §11.125,
**F19** → §11.126 — all three delivered AND supervisor-verified same day. **The round
restructured itself mid-flight**: F17's hunt caught the row's first reproducible
teardown crash (reload + composed OJM + quit, ~96 % under load) and attributed it
single-variable to F16's own §5.51 fix having made two long-standing lifetime
violations REACHABLE; the supervisor decision (recorded at F17's WIP + §3): fix-first
— F19 minted into slot 3, F18 deferred with its section intact. Net: **§5.50, §5.51,
§5.55, §5.57, §5.58 CLOSED** (the teardown-order class fixed at the class, audit
in-entry); **B7's crash class CLOSED** (27/30 → 0/30; the row is now the HUNG class
alone = §5.59, reproducible on demand, its fork Vixy's as **A40**); §5.56 re-derived
and open; §5.59 open with its fix BUILT, MEASURED, WITHDRAWN (I7 bar not met — it
converts the hang into a crash); NEW §5.52–§5.54 (F9, pre-existing), §5.60 (device
limit, Vixy), §5.61 (EntityCore lost wakeup, recorded read-only). TSan is UNUSABLE on
this driver (§11.125(e), N=0 stated). Executor quality this round: two corrections of
predecessor records at source (§11.124(c) LeakSan, §5.55's signal face), one
fix-refused-on-its-own-bar — all three the discipline operating, all endorsed.
Remaining dispatchable, next round: **F18** (minted, head of queue), **next B31
slice** (session file §3.2 — mint must state D28/D21 carve-outs), **B7-hunt-5 only
after A40** (and its mix gains the line-family flags — the §11.124(k) hole is now
closable), **F18's G4/B12-content interaction** waits on Vixy's b12-design §7 set.

