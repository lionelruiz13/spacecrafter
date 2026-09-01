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
- 2026-08-31 [measured, fable session 18 — the scedit round on **TravellingFoxDev**, the
  LAPTOP; every entry above is LovelyFoxDev's] host identity: `claude` holds a REAL logind
  session (id 5, seat0, tty2, Active=yes, `wayland-0`, `XDG_RUNTIME_DIR=/run/user/1003`);
  `:2` = XWAYLAND0 **1920x1080@143.88**, GTX 1660 Ti 6144 MiB, driver 580.173.02; the F28
  XAUTHORITY recipe (`/run/user/$(id -u)/.mutter-Xwaylandauth.*`) applies — §0.5's
  `/tmp/rt-claude` note does NOT. Consequence: `f56_canary.sh --no-scene` FAILS here by
  construction (`display.geometry` 1920x1080 vs banked 2448x1332; `compositor.absent`: no
  headless gnome-shell) — the bank is the desktop's; per-host re-banking is the owner's
  fork (§11.174(f)), reported not done; the functional launches (F61–F63) proceeded, none
  photometric.
- 2026-08-31 11:44–12:03 [measured, F61 ×4 + F62/F63 logs] **SCREEN LOCK ⇒ 1 Hz FRAME
  CLOCK.** With the claude session's screensaver ACTIVE (`org.gnome.ScreenSaver.GetActive`
  true, `LockedHint=yes`, `lock-enabled true`, `idle-delay 300`) the engine logs `Frame
  stall detected` every **1000 ms** for the whole run (105 stalls/run; 101 of 104
  intervals exactly 1000 ms) on HEAD `2b8ec034` AND on the pre-fix control `a3437670`
  alike — the compositor throttling a blanked output, not the binary; each stall makes
  the watchdog send itself SIGUSR1 (fps.cpp:150-156). Screensaver deactivated: **1 stall
  in the whole run**, F61 16/16 (prediction stated before the run, held). Instrument
  consequence: any fixed-`sleep` timing or per-frame claim taken on a locked session is
  wrong by up to 1 s per frame; F61's SIGUSR1 leg now waits for stall quiescence and
  pairs the watchdog's own WARNINGs away (or says it could not). HOST FACT for the
  owner: `gdbus … org.gnome.ScreenSaver.SetActive false` from inside the session
  DISMISSED THE LOCK (`LockedHint` yes→no) — used once as the discriminating control
  (self-reverting after idle-delay, kept awake by `SimulateUserActivity` for the run's
  length only); keeping the display awake during dispatch (idle-delay 0 / lock off) vs a
  per-launch wake is the OWNER's call — decision flag in the session-18 report. Auto-suspend
  inert: `sleep-inactive-ac-timeout 0`, on AC.
- 2026-08-31 [stated: vixy, session-19 trigger line; setting measured same minute]
  **AUTO LOCK-SCREEN DISABLED** on TravellingFoxDev — the owner's answer to the
  session-18 display item, option (a)'s LOCK half: `org.gnome.desktop.screensaver
  lock-enabled` = **false** [measured]. The BLANK half is NOT enacted:
  `idle-delay` = 300, `idle-activation-enabled` = true — the screen still blanks
  at 5 min idle, and the 1 Hz entry above attributes the throttle to "the
  compositor throttling a blanked output" (keyed on screensaver-active, not
  LockedHint). CONSEQUENCE, until a blank-only run discriminates (named check:
  idle past 300 s, read `GetActive`, count `Frame stall` over one run): the 1 Hz
  hazard is treated as LIVE, and every live-launch executor keeps the F67 wake
  mitigation (`SetActive false` + `SimulateUserActivity`, recorded per run) —
  it keys on GetActive, which covers both attributions. Full record §11.186(a).
- 2026-08-31 [stated: vixy, in-conversation mid-round; measured same minute]
  **IDLE-DELAY DISABLED TOO** — `org.gnome.desktop.session idle-delay` = **0**
  (`lock-enabled` false, `GetActive` false at the read). Both arms of the 1 Hz
  hazard (lock AND blank) are now removed; the lock-vs-blank attribution
  question closes as MOOT (neither state can occur on idle — epistemically
  undiscriminated, operationally dead; reopen only if a throttle recurs).
  Cause of the earlier partial enactment, owner-stated: *"you pointed
  explicitly the session lock in your report ... so I reduced the scope to
  just the lock screen"* — the session-18 report's HEADLINE said "a locked
  screen runs the engine at 1 Hz" while the mechanism line said "blanked
  output"; headline label != operative variable, a supervisor report-shaping
  defect, tallied session 19. Standing consequence: executors keep a
  CHECK-AND-RECORD of `GetActive` at launch (defense in depth, cost ~0); the
  per-minute wake loop is no longer load-bearing.
- 2026-08-31 [measured, F68 canary run, §11.187(i)] **X SERVER FOR `:2`
  RESTARTED** 2026-08-30 23:54:52 (vs the F67-era start), compositor
  UNCHANGED — canary NOTE `xserver.restarted`, first seen this round; canary
  exit still 2 (the two desktop-bank fail-by-construction members). No claim
  rides it; echoed here because a display-stack birth-time change is exactly
  what this channel exists to carry (§11.121(m) retroactive shape).
- 2026-08-31 [measured, F69 §11.188(j)] **CORRECTION: the `xserver.restarted`
  entry above records a NON-EVENT.** The canary's start-epoch probe reads
  `stat -c %Y /proc/<pid>` — a directory MTIME, not a start time; pid 14079
  answers 2026-08-30 23:54:51 on both `ps -o lstart` and btime+starttime while
  that mtime moved 19 h with no restart (probe committed,
  `harness/artifacts/f69/xserver-epoch-probe.txt`). The X server for `:2` did
  NOT restart; the earlier entry stands as the record of what the instrument
  said, this one as why it was wrong. Same proxy backs the GATING
  `compositor.restarted` check — false-red abort hazard on the desktop until
  the probe is fixed AND `BANK_XSERVER_START`/`compositor.start_epoch` are
  re-banked there (one VALUES edit with argument, §0.5; queued for the next
  desktop round, owner-visible in fable-dispatch §3).
- 2026-08-31 20:57 [stated: vixy ("I forgot to disarm the scheduled shutdown");
  boot measured 20:57:30] **SCHEDULED SHUTDOWN mid-round, host rebooted** —
  killed the supervising session AND the F70 executor mid-sweep (CP1 was
  committed `df39166`; the conversion diff survived UNCOMMITTED in the code
  tree; loss bounded per §0.6). `/tmp` WIPED: all staging binaries gone
  (`/tmp/f70-pre`, `/tmp/f68-pre`) — pre binaries are rebuildable from their
  named commits. Persisted across the boot: `idle-delay` 0, `lock-enabled`
  false [measured]. **DISPLAY STACK MOVED: `:2` no longer exists** — sockets
  are now `X0`/`X1`, both answering 1920x1080 under
  `/run/user/1003/.mutter-Xwaylandauth.ZA0TU3`; claude holds logind session 2
  (seat0, tty2). The F28 recipe generalizes (mutter auth + the session's
  display) but the NUMBER is per-boot: `DISPLAY=:0` here, verified by
  xdpyinfo before use. Retro-note: the previous boot was 2026-08-30 23:31 —
  the 23:54:51 X start §11.188(j) measured is that boot's, consistent; the
  canary's per-pid mtime proxy remains the refuted instrument, and its
  display-target member will now also mismatch on this host (still exit-2
  fail-by-construction territory, desktop bank).
- 2026-09-01 [measured: F76 executor (report), re-verified at the socket by
  the supervisor same evening] **DISPLAY `:2` EXISTS AGAIN ON THIS LAPTOP AND
  IT IS NOT OURS** — `/tmp/.X11-unix/X2` (and `X3`) are owned by **foxy**
  (Xwayland pid 90210, started 18:09:36, auth under `/run/user/1000`; gdm
  holds X1024/X1025 since the same minute — a second graphical login on
  seat0/tty3). claude's sockets remain `X0`/`X1` (2026-08-31 20:57 boot,
  `DISPLAY=:0` verified by xdpyinfo). CONSEQUENCE: the canary's `:2`
  display-target member now REACHES a live server that belongs to another
  user — exit moved **2 → 3** (fail members `display.reachable`,
  `compositor.absent`; supervisor's own run 22:07, artifacts f56/canary/
  20260901-220758). Still fail-by-construction vs the desktop bank; nothing
  re-banked (§11.174(f) fork stands). Standing rule unchanged: HOST-EVENTS
  is the per-host/per-boot display authority — and NEVER launch at `:2` on
  this laptop; it is another user's session.
