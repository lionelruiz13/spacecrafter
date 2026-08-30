# HOST-EVENTS — append-only dispatch-environment record

**Purpose (§11.174(e) fix (1), created 2026-08-30 at session-17 close; veto-reversible):**
dispatch-environment facts land HERE — reboots, driver moves, display provisioning,
dispatch-method changes, dispatch faults, GPU/VRAM state changes — so that transmission
stops depending on a lucky delta in a close report (§11.174(a): the 2026-08-29 fault
*"might have never been reported"* without one). Writers: the owner AND the supervisor,
each entry tagged. Readers: every §0/§0b warm-up (the canary automates the photometric
member; this file carries the events no instrument can measure after the fact).

**Convention:** append-only; one entry per event, newest last; format
`- YYYY-MM-DD[ HH:MM] [source-tag] event — consequence/pointer`. Corrections annotate,
never rewrite (maintenance invariant). Archival per the standing convention
(`archive/` drawer beside this file) when a span stops contributing.

## Events (seeded 2026-08-30 from the ledger — each line cites its source; pre-seed
## events are RECONSTRUCTED, not contemporaneous)

- 2026-08-23..26 [measured, §11.164(d)] NVIDIA driver bump `580.568.0 → 580.636.192`
  inside the bracket (2026-08-23 08:44, 2026-08-26 11:06] — later measured
  photometrically INERT from both sides (§11.172(e), §11.176(g)).
- 2026-08-27 07:32 [measured, §11.157(f)] host REBOOT; claude's graphical login
  session lost; foxy's seat owns :0/:1 thereafter.
- 2026-08-29 (day) [vixy, §11.174(a)(g)] sessions 14–15 (F42–F47, F48–F52) dispatched
  via `sudo -u claude bash --login` from foxy's console — NO Wayland display, no
  logind session, no runtime dir. THE DIM ERA: rendered photometry ~×0.37 on
  textured bodies, both paths, star channel untouched (§11.176(e)); recorded
  internally owner-side, not transmitted until the session-16 close report's delta.
- 2026-08-29 14:51:01 [measured, §11.174(f1)] F43's substitute stack born: hand-built
  headless gnome-shell + Xwayland **:2**/:3 under `XDG_RUNTIME_DIR=/tmp/rt-claude`
  (absent from loginctl). Serves the harness in BOTH eras (dim and bright).
- 2026-08-29 17:19:41 [measured, §11.174(f2)] the remmina fix: real claude logind
  sessions (459/460) + gnome-shell + Xwayland **:4** under `/run/user/1003`.
- 2026-08-30 [measured, §11.174(j)] the dim state DOES NOT REPRODUCE (six variables
  exonerated: driver, VRAM, dispatch env, server identity, windowing path, RDP
  connection state); surviving model = a latched state, trigger unenumerable
  post-hoc; live chase ENDED, watch inherited by the canary.
- 2026-08-30 [fable, §11.176(a)] ENVIRONMENT CANARY committed (`harness/f56_canary.sh`):
  display fingerprint + GPU snapshot + RDP state + dispatch-method fingerprint +
  photometric band **165.258/6.644 new · 160.142/6.603 old ±1.0/±0.15 on :2** —
  banked on :2, the **canonical-display fork (:2 substitute vs :4 real session)
  OPEN, owner's** (§11.174(f)); re-bank = one VALUES-block edit + one run.
- 2026-08-30 [fable, session-17 close] this file created; all prior entries seeded
  from the cited ledger nodes.
