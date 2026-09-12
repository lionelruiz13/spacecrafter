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
  intervals exactly 1000 ms) on HEAD `c589d1b2` AND on the pre-fix control `02bc028e`
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
  committed `a55e28a`; the conversion diff survived UNCOMMITTED in the code
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
- 2026-09-04 20:08 [measured: fable session 21 warm-up — the FIRST desktop round since
  session 17, host **LovelyFoxDev**] host BOOTED **18:45:08**; foxy's seat0/tty2
  session 2 owns `:0`/`:1` from 18:45; claude holds REAL logind sessions **14** (user,
  seatless = remote) + 15 (manager) under `/run/user/1003` — `gnome-remote-desktop` in
  the runtime dir, an RDP connection ESTAB on :3389 — with a real `/usr/bin/gnome-shell`
  (pid 43595) + Xwayland **`:2`**/`:3` (pid 43800, born **19:50:18**, auth
  `/run/user/1003/.mutter-Xwaylandauth.5KBYU3`); `xdpyinfo` **2448x1332**. F43's
  substitute stack (`/tmp/rt-claude`, headless `--virtual-monitor 2448x1332`) is GONE
  with `/tmp` and nobody rebuilt it ⇒ the §11.174(f) fork's two branches have collapsed
  to ONE claude-owned display — the real session, now on `:2` (the number the substitute
  held; the real session was `:4` on 2026-08-29). CANARY `--no-scene` on `:2`: **exit 2,
  `compositor.absent`** (the banked command line matches no live process) + NOTE
  `xserver.restarted` (19:50:18 vs banked 2026-08-29 14:51:26 — read through the refuted
  /proc-mtime probe, §11.188(j)); the dims member PASSES (2448x1332 == bank);
  artifacts `harness/artifacts/f56/canary/20260904-201433`. GPU RTX 5090, driver
  **580.159.03** (the 2026-08-26 entry above recorded 580.636.192 — a different
  number; the driver class is photometrically inert per §11.176(g), recorded not
  judged), VRAM 1191/32607 MiB. Host: 24 threads, affinity 0-23 (the §0.5 "12-core
  affinity" self-cap was this host's earlier state; the session hook now says safe
  -j24), RAM 59 GiB / 52 avail. **GitHub SSH from this host: `Permission denied
  (publickey)`** — fetch/push impossible here; local contains origin on both repos
  (+58 code / +604 harness commits over the stale origin refs). The working tree carries
  sessions 19–20's LAPTOP commits with no sync daemon running (reflog lists them as local
  commits ⇒ the home was file-synced by the owner's hand before this session; `build-claude/`
  was NOT: binary dated Aug 26, 209 compile steps pending ⇒ rebuilt at `8d41fbe3`, -j24,
  20:11, md5 `c8e12950`). Config/ssystem md5 pristine (`03fbee59`/`545a51ef`); no
  spacecrafter process. `lock-enabled`/`idle-delay` not re-measured here (the 2026-08-31
  settings were the laptop's; executors keep the GetActive check-and-record).
- 2026-09-04 21:10 [measured: F79 executor, task delivery; INTENT §11.199, artifacts
  `harness/artifacts/f79/`] **THE CANARY IS RE-BANKED ON THIS HOST AND GREEN** —
  `f56_canary.sh --no-scene` exit **0** and the FULL canary (scene arm) exit **0**,
  0 fail 0 note, first green on LovelyFoxDev since F43's substitute stack died with
  `/tmp` at the 18:45:08 boot. Banked values now, all [measured] on claude's REAL
  logind session (14/15) serving `:2`:
  `BANK_XDG_RUNTIME_DIR=/run/user/1003` · `BANK_XAUTH_GLOB=/run/user/1003/.mutter-Xwaylandauth.*`
  · `BANK_COMPOSITOR_CMD=/usr/bin/gnome-shell` · `BANK_COMPOSITOR_START=1788544217`
  (pid 43595) · `BANK_XSERVER_START=1788544217` (pid 43800) ·
  `BANK_HOST_BOOT=2026-09-04 18:45:08`; `BANK_DISPLAY=:2` and `BANK_DIMS=2448x1332`
  UNCHANGED (measured equal). The photometric band is UNCHANGED — six `f51_run.sh
  --samples 2` runs returned 165.258/6.644 new and 160.142/6.603 old on **72 of 72**
  gated members to the last printed digit, spread **0.000**, so the substitute ->
  real-session compositor change is photometrically INERT. The dwell frame is
  BYTE-IDENTICAL (md5 `5215565b`) to 2026-08-30's, across both that compositor change
  AND a different binary (`fa00deae` at `3ccfc6d8` -> `c8e12950` at `8d41fbe3`).
  **CORRECTION TO THIS FILE'S 2026-09-04 20:08 ENTRY ABOVE:** Xwayland `:2` pid 43800
  was born **19:50:17**, not 19:50:18 — ":18" is the `/proc`-directory-mtime probe's
  reading, and that probe is the very thing §11.188(j) refuted and F79 replaced
  (`btime` + `starttime`/CLK_TCK now; `ps -o lstart=` agrees to the second on four
  live pids). The compositor (43595) started in the SAME second, 19:50:17.
  **PER-BOOT, BY DESIGN:** a real session's epochs and display NUMBER die with the
  boot, so the canary WILL red at exit 2 after the next reboot. That is the protocol
  working — report it and re-bank in one VALUES-block edit with its argument; never
  widen the band, never demote a member (§11.191(c)). This file stays the per-host,
  per-boot display authority. `GetActive` **(false,)** at all seven launch-time reads
  of this task; no `spacecrafter` process before any of them; config/ssystem md5
  `03fbee59`/`545a51ef` pristine in == out on every run.
- 2026-09-04 [stated: vixy, in-session during F79; measured same minute by the
  supervisor; cross-project record `~/shared/QUEUE.md` Q-61] **THREE STANDING FACTS
  FOR THIS AND EVERY HOST.** (1) `/tmp` is SESSION-LIFETIME: *"/tmp get cleared at
  the end of the session — nothing tell when it will persist and when it's not
  traced"* — assert it gone after the working session; the reboot wipes recorded
  above were instances of a broader rule; nothing under `/tmp` may be a bank, a
  display stack or a record. (2) Exactly THREE trees migrate between devices —
  `~/spacecrafter`, `~/shared`, `~/.claude`; everything else is per-device, which is
  why this host's `build-claude/` was stale at open and why (3b) below differs from
  the laptop. (3) SSH era planned: *"I would leave :2 open and set DISPLAY to it.
  Spacecrafter use SDL2 which uses X11 which work through DISPLAY"* — the canonical
  display stays THIS RDP-created real logind session on `:2`, kept logged in; F79's
  bank carries over unchanged as long as it lives. (3a) [measured]: `Linger=no`, so
  an ssh login alone provisions no graphical session and `/run/user/1003` (with the
  `.mutter-Xwaylandauth.*` cookie) lives only while the RDP session does — a
  logout/reboot = owner re-provision + canary re-bank, recorded here. (3b)
  [measured, gsettings via `/run/user/1003/bus`, session 14]: `idle-delay` = **0**
  (the blank arm dead on idle, as on the laptop) but **`lock-enabled` = true** — the
  laptop's 2026-08-31 change never reached this host (gsettings is not carried);
  cannot fire on idle at idle-delay 0, CAN fire from an explicit lock/suspend path,
  and an unattended ssh-era session would then sit in the F67 1 Hz throttle;
  `idle-activation-enabled` true (moot), `sleep-inactive-ac-type` 'nothing',
  GetActive false, LockedHint no, Remote=yes Type=wayland. Setting `lock-enabled`
  false here is one gsettings line and the owner's call — flagged, not done.
- 2026-09-05 12:23 [measured: F86 executor, mid-task, from the submodule's own
  reflogs; reported not absorbed per §11.174(h)] **AN EXTERNAL WRITER AMENDED AND
  PUSHED THE `EntityCore` SUBMODULE COMMIT WHILE A DISPATCHED TASK WAS RUNNING.**
  At **12:23:23** `src/EntityCore`'s HEAD moved `7ce58350` -> `84f5d94b` by
  `commit (amend)` (reflog verbatim), author preserved (`Claude Opus 5`,
  2026-08-26) and committer re-stamped (`Claude`, 2026-09-05); at **12:23:44**
  `refs/remotes/origin/main` logged `update by push` to the same SHA. **The TREE
  is identical** — `HEAD^{tree}` = `7ce58350^{tree}` = `6ee9f6a7`, `git diff`
  empty — so no build is affected and every pre/post comparison of that task
  stands (its release binaries were built at 12:22 and 12:41 with byte-identical
  submodule sources). **Two consequences that outlive the event.** (1) The code
  working tree now carries one unstaged entry, `M src/EntityCore` (gitlink
  `7ce58350` -> `84f5d94b`), which F86 did not create and did not resolve: moving
  a submodule pin is an owner decision and EntityCore is read-only to executors.
  A warm-up that expects `git status` clean on the code repo will see it. (2)
  **§5.131 is NOT discharged by the push**: `master-beta` still records the
  amended-away `7ce58350`, no remote ref contains it, so `git clone
  --recurse-submodules` fails exactly as F84 measured until the pin is bumped —
  one `git add src/EntityCore` + commit in THIS repository. Recorded at §5.131,
  DEPLOYMENT-MAP R2/R5 and §11.205(i).
- 2026-09-07 01:29 [measured: supervisor, session 26 (Claude Fable 5.1), the FIRST round run with
  the owner over ssh from the laptop — his note verbatim: *"First attempt over ssh from the laptop
  (TravellingFoxDev) - I have no physical access to the desktop (LovelyFoxDev) - precising in case
  it changes something."*] **THE SSH MOVE CHANGED NOTHING THAT REACHES A LAUNCH.** At open
  (21:24:37) `loginctl` still listed claude's sessions 14 (user) and 15 (manager) — the RDP-created
  real logind session of 2026-09-04 19:50 — `XAUTHORITY=/run/user/1003/.mutter-Xwaylandauth.5KBYU3`
  present, `DISPLAY=:2` answering `xdpyinfo` at **2448x1332**, canary `--no-scene` **exit 0**
  (`artifacts/f56/canary/20260906-212529`, 30 members), `uptime -s` **2026-09-04 18:45:08**, RAM
  52 GiB available of 59 at open and at close. Four executors ran 40+ launches on `:2` through the
  night (F99 8, F100 16, F101 11, F102 0, plus the supervisor's own 9), every one with the canary
  green, `/proc/*/comm` clear and **no `/tmp/spacecrafter.lock` at any check** — Q-61(3) holds as
  stated: the display lives with the logged-in session, not with the owner's seat. Nothing
  re-banked; nothing re-provisioned; no HOST-EVENTS entry was owed by any executor.
- 2026-09-11 20:15–20:28 [measured: supervisor, session 28 (Claude Fable 5.1), warm-up —
  every value from the command beside it, Q-67] **THE HOST REBOOTED 9 MINUTES BEFORE THE
  ROUND OPENED, THE OWNER RE-PROVISIONED THE SAME CLASS OF DISPLAY, AND THE CANARY DID
  EXACTLY WHAT ITS BLOCK SAYS: RED BY DESIGN, THEN RE-BANKED, THEN GREEN ON BOTH ARMS.**
  `uptime -s` **2026-09-11 20:06:16** (the 2026-09-04 18:45:08 boot of sessions 21–27
  ended; `/tmp` wiped with it — nothing carried there). At 20:13:30 `loginctl` shows
  claude's sessions **5** (user, `Type=wayland`, `Service=gdm-password`, `Remote=yes`) +
  **6** (manager) — the RDP-created REAL logind session the owner ruled on 2026-09-04,
  re-provisioned by him two minutes before the trigger line; `/run/user/1003` with the new
  cookie `.mutter-Xwaylandauth.AF5BV3`; `/usr/bin/gnome-shell` pid **12247** (kernel start
  1789150410 = 20:13:30) → Xwayland **`:2`** pid **12735** (1789150411 = 20:13:31), the
  inherited `DISPLAY=:2` / `XAUTHORITY` already pointing at them; `xdpyinfo` **2448x1332**.
  Canary `--no-scene` at 20:19:47: **exit 2, 1 fail 1 note** — FAIL `compositor.restarted`
  (the gating member), NOTE `xserver.restarted`; display, dims, runtime dir, cookie glob
  and owner uid all EQUAL to the bank (`artifacts/f56/canary/20260911-201947`). RE-BANKED
  in one VALUES-block edit with its argument (`f56_canary.sh`, the three per-boot members
  only — `BANK_COMPOSITOR_START` 1789150410, `BANK_XSERVER_START` 1789150411,
  `BANK_HOST_BOOT` 2026-09-11 20:06:16 — the 2026-09-04 values struck not deleted; the
  band untouched). Then `--no-scene` **exit 0** (30 members, `20260911-202614`) and the
  FULL canary **exit 0, 0 fail 0 note** (`20260911-202616`): 165.258/6.644 new and
  160.142/6.603 old, every delta **0.0** — the band reproduced on a THIRD boot and a third
  session generation. Config/ssystem md5 `03fbee59`/`545a51ef` in == out; no
  `spacecrafter` in `/proc/*/comm` before either launch; no lock file. GPU RTX 5090,
  driver 580.159.03, 730 MiB used; RAM 53 GiB available of 59; `-j24`. Decision flag per
  §11.174(h): the re-bank is the response the block prescribes for a per-boot red on the
  ruled display class — your correction may differ (e.g. a display you would rather bank
  on) — say the word and the block is one edit again. The scratch trees under
  `/home/claude/sc-f*` (18, `sc-f84` 4.2 G … `sc-f91` 6.4 M) survived the boot.
