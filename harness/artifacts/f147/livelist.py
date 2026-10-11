#!/usr/bin/env python3
"""F147 step 6: the live list, generated from data so that its item set can be compared
mechanically with the census's CARRIED set (check (c)). Prints the fenced block and the counts.
usage: livelist.py [--check]"""
import re, sys

# (group heading, [(item key, text, node pointer, tag)]) ; tag in N+ N- T D ; s42 added from the key (C66)
G = [
("THE VIEW AND THE CAMERA - where it looks, how it moves, what a view command means", [
 ("C01 #4", "§11.94(d) B30's fix: latch the view once tracking settles under frozen time (the proposal), or another convergence rule", "INTENT.md:1411", "N+"),
 ("C03 #1", "§5.49 which longitude `moveto lon 0` names: the mesh x-hat column or the map's centre; the owed `u_sub` render measurement first", "INTENT.md:343", "N-"),
 ("C15 #1", "§5.65 a sky lock turned on in the move's frame latches the PREVIOUS frame: lock what the operator now sees, or keep", "INTENT.md:375", "N+"),
 ("C28 #3", "§5.85 `align_with body` aligns nothing: what the command is FOR (its author's or a show's answer)", "INTENT.md:413", "N+"),
 ("C34 #4", "§5.106 what free flight should SHOW (landscape frozen, atmosphere floods); close-as-accepted or keep the design question", "INTENT.md:454", "N+"),
 ("C39 #1", "§5.109 a commanded `moveto alt` stands above the DRAWN surface (today) or the physical one; D21 is silent on the observer", "INTENT.md:457", "N+"),
 ("C52 #12", "§5.135 the OLD path rolls the offset with the heading (the tester saw it): correct the baseline, or let it retire with B8", "INTENT.md:516", "N+"),
 ("C55 #6", "`Camera::observedToBodyLocalPos` has no in-tree consumer (kept as §5.86's member): keep or delete", "INTENT/11.213.md:59", "N-"),
 ("C56 #7", "a body with NO direction (the zero vector): print it unlike 'at the pole'; a track on it (§5.148): refuse with a line, or a defined aim", "INTENT.md:530", "N+"),
 ("C61 #8", "§5.153 a view duration under 0.2 s: snap, clamp as today, or run at the commanded time", "INTENT.md:535", "N+"),
 ("C63 #2", "§5.156 `flag track_object on`: old snaps, the Camera eases (56-65 deg transient): snap now, smooth later, or a named divergence", "INTENT.md:538", "N+"),
 ("C63 #3", "§5.155 `look_at azimuth`: the two conventions are 107.66 deg apart at rest; which one `look_at` MEANS", "INTENT.md:537", "N+"),
 ("C63 #4", "§11.244(f) an already-accepted divergence in the identical phase: a predicted residual (as applied), or reverted", "INTENT/11.245.md:56", "N+"),
 ("C64 #4", "§11.246(g) the travel's definition is still two computations kept equal (manager count 2): the shape", "INTENT/11.247.md:14", "N-"),
 ("C65 #2", "§5.163 the anchored descent goes under the datum (-12.5 km after three steps): a clamp, or not", "INTENT.md:545", "N+"),
 ("C65 #7", "§5.165 the Camera's latitude passes the pole (-90.5): a clamp, or not", "INTENT.md:547", "N+"),
 ("C66 #13", "H2 a RETURN to the aimed body: `switchToBody` both ways (default), position and view continuous; + Q-E, does a reference change hold the view", "INTENT/11.313.md:46", "D"),
 ("C66 #14", "H3 the agent-added rule of `3c87a51d` goes (its premise never existed): strike or keep", "INTENT/11.313.md:49", "D"),
 ("C66 #15", "H8 the galaxy boundary: maintain the frame above the isolated node, the galaxy's influence radius, the root (default: no implicit change above the first system)", "INTENT/11.313.md:56", "D"),
 ("C66 #17", "H4-H7: one saved aimed body; `set home_planet` unconditional on the new path; the session does not save `aimed` yet; free-move unchanged", "INTENT/11.313.md:43", "D"),
 ("C66 #35", "H6-b the Camera's inertia branch goes: a timed re-aim restarts at zero velocity (a feel change)", "INTENT/11.314.md:72", "D"),
 ("C66 #45", "M3 `moveTo -> moveRel` forwarding (out of F137)", "DECISIONS_PENDING.md:57", "N+"),
 ("C66 #49", "H7 the fov limits: `setMaxFov` reaches the projector only (default `loadCamera` reads the same keys)", "DECISIONS_PENDING.md:48", "N+"),
 ("C66 #51", "H10 the partial `moveto` (lat, lon or alt alone) re-sends the drawn readout to both engines (default per-component movers)", "DECISIONS_PENDING.md:58", "N+"),
]),
("THE TWO ENGINES' WIRING (B42) AND THE FOV SLICE IN FLIGHT", [
 ("C66 #24", "the interface's NAME `SkyView` (default) / `SkyProjection` / `ViewedSky`, and `OldSkyView`, `CameraSkyView`, `Core::drawnView`, the Renderer accessors", "INTENT/11.315.md:68", "D"),
 ("C66 #25", "D2' the Renderer holds the centre AND the 2D MVP AND the label law's px entry, or the centre only", "INTENT/11.315.md:5", "D"),
 ("C66 #26", "D5 the grid included in the slice (0 new members), the illuminates' cull not", "INTENT/11.315.md:5", "D"),
 ("C66 #27", "the old half: ONE adapter {prj, nav} forwarding verbatim, or two interfaces on the two classes", "INTENT/11.315.md:5", "D"),
 ("C66 #28", "the pointer value-fed `drawPointer(dt, view, earthEquPos, onScreenSize)`, or excluded, or a (view, prj, nav) signature", "INTENT/11.315.md:5", "D"),
 ("C66 #29", "the obliquity / galactic literals in 2 homes (3 after): when the one-constant dedup commit lands", "INTENT/11.315.md:132", "N-"),
 ("C66 #32", "D2 the Renderer holds the screen centre beside `getViewportRadius()`", "INTENT/11.314.md:17", "D"),
 ("C66 #33", "D3 the old-path gate re-cut: the old pin bit-identical, `projector.cpp` / `navigator.cpp` untouched", "INTENT/11.314.md:51", "D"),
 ("C66 #34", "D4 'fovs per frame' read as the S7 count + a mid-zoom capture (not a canary member)", "INTENT/11.314.md:16", "D"),
 ("C66 #37", "F143 (h) `getOnScreenSize` returns a RADIUS where the old notes read a DIAMETER: a focused read, or your word", "INTENT/11.310.md:81", "N-"),
 ("C66 #46", "b42 Q2 the scope: the SPACE only, the body / environment seams listed (default yes)", "DECISIONS_PENDING.md:35", "N+"),
 ("C66 #47", "b42 Q6 the mode holder's NAME", "DECISIONS_PENDING.md:37", "N+"),
 ("C66 #48", "b42 Q9 the frame `dragView` measures in", "DECISIONS_PENDING.md:38", "N+"),
 ("C66 #50", "H8 the anchor verdicts `return oldOk || newOk` + the validate-and-continue WARNING (default: both must accept, the refusal at its anchor)", "DECISIONS_PENDING.md:49", "N+"),
]),
("WHAT IS DRAWN - regimes, thresholds, textures, light", [
 ("C05 #1", "B12 content slice: the star's spots authored or procedural", "b12-design.md:30", "N+"),
 ("C05 #2", "B12 content slice: is a star enterable, and what is seen inside", "b12-design.md:30", "N+"),
 ("C05 #3", "B12 content slice: a resolved star's halo alternates with its surface, or coexists", "b12-design.md:30", "N+"),
 ("C05 #4", "B12 content slice: the chromosphere module's `module =` word (B28, before it is built)", "b12-design.md:30", "N+"),
 ("C11 #1", "A41 the early-visibility gate: 2 px (the constant's name) or 3.07 px (what ships); `RAYMARCH_MIN_SCREEN_SIZE` rides it", "INTENT.md:1386", "N+"),
 ("C11 #2", "A42 the texture-level swap: since R29 the swap DISTANCE and the body list, the tester's", "INTENT.md:1387", "T"),
 ("C34 #2", "A44 the ring's shadow caster is the one SCALED shadow radius: D21 or the 2026-07-18 extent contract", "INTENT.md:1389", "N+"),
 ("C42 #5", "the 7.73 deg old/new attitude divergence on the Moon (two faces drawn): expected or not (rendering paths)", "INTENT/11.167.md:47", "N+"),
 ("C44 #5", "§5.114 every illuminate draws with green and blue exchanged: the one-line fix (user-visible)", "INTENT.md:468", "N-"),
 ("C51 #8", "§5.126 `halo = on`: nothing on the old path, a halo on the new; which predicate is canonical, and should the data surface lowercase", "INTENT.md:500", "N+"),
 ("C51 #17", "§5.129 a portrait window draws the dome (H-W)/2 px low: intended geometry (a tilted dome) or a slip (EntityCore)", "INTENT.md:505", "N+"),
 ("C64 #5", "camera INSIDE a body: new draws nothing, old the body; and `ModularBody::loaded` still a latch (removing it its own decision)", "INTENT/11.248.md:25", "N-"),
 ("C65 #5", "§5.171 bigTexture 256 px and the `0.2` literal (409.6 px) were one threshold and are two: your re-ruling of the two literals (B45 item 3)", "INTENT.md:553", "N+"),
]),
("THE DSO LAYER (B5, the design note `claude/b5-dso-design.md`)", [
 ("C62 #1", "V-D1 DSO content engine-instantiated or AUTHORED (527 lines of `mode in_galaxy` to migrate either way)", "INTENT/11.240.md:37", "N+"),
 ("C62 #2", "V-D2 the catalogue-to-AU factors: a measurement to ratify (gates slices 2 and 4)", "INTENT/11.240.md:37", "N+"),
 ("C62 #3", "V-D3 `mode in_sandbox`: no node, no behaviour to preserve - its fate", "INTENT/11.240.md:37", "N+"),
 ("C62 #4", "V-D4 Tully's selection after the port (three readings priced)", "INTENT/11.240.md:37", "N+"),
 ("C62 #5", "V-D5 OjmMgr's full-screen depth clear: keep it (faithful) or measure first (correct)", "INTENT/11.240.md:37", "N+"),
 ("C62 #6", "V-D6 the far edge (hard cut or fade), the peg's appearance and the onset ramp - with the tester", "INTENT/11.98.md:11", "N+"),
 ("C62 #7", "V-D7 Dso3d's labels: one state, two operator words - which word", "INTENT/11.240.md:37", "N+"),
 ("C62 #24", "the slices: slice 1 dispatchable once you have read the note; slice 3 before 2 and 4", "INTENT/11.240.md:42", "N-"),
]),
("ORBITS, BODIES AND THE TREE", [
 ("C24 #2", "§5.75 the trail drops a sample on a date jump: rounding or accumulating changes shipped trails' sample counts (policy)", "INTENT.md:395", "N+"),
 ("C55 #5", "the four-body selector (old-path ulps at a pinned clock): leave it unless you want it attributed (an offer)", "INTENT/11.215.md:36", "N-"),
 ("C56 #5", "§5.140 what `orbit_lon` MEANS (planetographic, or today's +90 deg); unblocks §5.21's two halves", "INTENT.md:522", "T"),
 ("C59 #1", "§5.149 the constructor seeds every solver at JD 0: shape (i)-(iv), or none", "INTENT.md:531", "N+"),
 ("C59 #5", "§5.151 Sedna's shipped orbit: three malformed keys in his data (forward-only)", "INTENT.md:533", "T"),
 ("C59 #6", "`isSystemCentered()` answers by POSITION at load time: was a declared flag the intent", "INTENT/11.229.md:52", "N-"),
 ("C62 #9", "§5.154 the trail walker's stale seed: fix shape (1) re-converge after the walk (candidate), (2), (3), or (4) nothing", "INTENT/11.241.md:50", "N+"),
 ("C62 #10", "the `IterativeEll` fold (modelled, unmeasured): its leg before §5.154's fix, or after", "INTENT/11.241.md:56", "N-"),
 ("C64 #3", "§5.159 a body parked under the tree ROOT never gets a position: the root publishes for it, or no fixed point is parked there", "INTENT.md:541", "N+"),
 ("C66 #16", "H9 = §5.192 galactic.ini x/y/z are parsecs by the data: the file's unit (the data author's word); the loader owns the conversion", "INTENT/11.313.md:56", "T"),
]),
("TIME", [
 ("C11 #4", "§5.64 + §5.76 `timerate action pause` does not stop the clock (and slowing a paused clock runs time backward): the pause semantics", "INTENT.md:373", "N+"),
 ("C15 #2", "§5.69 `keep_time` truncated to 8 bits: honour the documented seconds (texture residency changes)", "INTENT.md:383", "N+"),
]),
("THE SCRIPT AND COMMAND SURFACE", [
 ("C20 #1", "§5.70 the turn ramp records `look delta_az ...`, a command no channel replays: what it should record", "INTENT.md:385", "N-"),
 ("C30 #6", "§5.87 `select constellation_star` with an unresolved abbreviation acts on the previous selection: unselect, no-op, or today's", "INTENT.md:417", "N+"),
 ("C42 #1", "§5.110 a composed selection answers exactly like nothing-selected: which of the three contracts", "INTENT.md:459", "N+"),
 ("C42 #2", "§5.113 where the truthiness test belongs: each read, a fail-loud singleton, or both", "INTENT.md:466", "N+"),
 ("C44 #7", "`[parallel-script]` (your request): the tester declined it (R25) - withdrawn, or kept as yours", "INTENT/11.207.md:238", "N+"),
 ("C47 #2", "§5.116 the surface cannot say no: refuse, or named default + log (D9 both ways; §5.117 / §5.118 its family)", "INTENT.md:472", "N+"),
 ("C48 #4", "the `#!` generic channel (~1661 tails into 35 shipped scripts): yes, no, or which subset", "INTENT.md:1081", "N+"),
 ("C48 #8", "`[script-trigger]` (`@requires` the binding): triage and order", "FEATURE_REQUESTS.md:195", "N+"),
 ("C49 #8", "nested refusals reach nobody: a nested call inheriting the outer origin (a change to §11.184's rule)", "INTENT/11.188.md:224", "N+"),
 ("C50 #8", "§5.117 severity versus STREAM: a promotion to L_ERROR moves 12 corpus lines to stderr", "INTENT/11.196.md:246", "N+"),
 ("C51 #10", "the `anchor.ini` grammar, a named and absent third contract (`camera` / `flyto` wait on it)", "INTENT/11.200.md:27", "N-"),
 ("C51 #11", "`type = BODY` on every composed node (§11.89(c)'s transitional call): keep or end", "util/scedit/INTENT.md:679", "N+"),
 ("C52 #13", "D37's implementation (a hidden star darkens, ambient by `ambient_light`): when", "INTENT/11.207.md:37", "N-"),
 ("C54 #6", "§5.137 a reloaded authored body leaves the DRAWN universe (inside §11.55(i)'s 'keep current state')", "INTENT.md:519", "N+"),
 ("C66 #5", "B49 C1 `*` with a name: play's rules verbatim, extension required (default)", "INTENT/11.312.md:112", "D"),
 ("C66 #6", "B49 Q2 the key token: modifier prefixes + SDL_SCANCODE suffix (default), the sheet's letters, or SDL's names", "INTENT/11.312.md:113", "D"),
 ("C66 #7", "B49 Q6 the dead `UI::handleKeysReleased`: delegate to it (-36, default) or leave both", "INTENT/11.312.md:18", "D"),
 ("C66 #8", "B49 C3 = §5.190 the native CTRL+UP release asymmetry: your one-line fix, or left to the registry", "INTENT/11.312.md:112", "N+"),
 ("C66 #9", "B49 Q1 the lookup above the TUI / video gates (a global binding fires while the TUI is open), default above", "INTENT/11.312.md:30", "D"),
 ("C66 #10", "B49 Q5 an error = `reportScriptError` AND the failing status, or the status alone", "INTENT/11.312.md:33", "N+"),
 ("C66 #11", "B49 K-4's side effects accepted as legacy (the show's wait cut, speed reset, `end` ends the show, un-pause without audio)", "INTENT/11.312.md:5", "D"),
 ("C66 #19", "F137 F2 K-3 half-built: a plain KP release whose press ran a script also stops a show's held order - accept until the registry", "INTENT/11.299.md:138", "D"),
]),
("THE TCP AND CONTROL CHANNELS", [
 ("C20 #2", "§5.72 the `$LOGON` channel promises logs and carries other clients' answers: its semantics", "INTENT.md:389", "N+"),
 ("C26 #7", "the truncation POLICY: whether and how a too-long answer is MARKED", "INTENT.md:391", "N+"),
 ("C48 #10", "the control channel and a script it plays: carry the script's lifecycle at all", "INTENT.md:1067", "N+"),
 ("C48 #11", "... its own §5 row, or §5.72 + §5.117 read together", "INTENT.md:1067", "N+"),
 ("C48 #12", "... WHO MAY MOVE THE DOME: the socket authenticates nobody, MCP `run_command` reaches whatever listens", "INTENT.md:1067", "N+"),
]),
("STARTUP, FAULTS, MEMORY AND LOGS", [
 ("C06 #1", "A40 a quit while the drawing worker owes a frame: keep waiting, stop waiting (reaches a teardown fault), or fix the fault then stop (+ §5.61)", "INTENT.md:1385", "N+"),
 ("C07 #1", "§5.60 the 2.68 GB staging buffer where a device limit is enforced: the player's sizing / a fallback path (D13)", "INTENT.md:365", "N+"),
 ("C30 #2", "§5.77 / F36 a startup failure on the console twice when `print_log = true`; and the pre-log `out` accumulator", "INTENT.md:399", "N+"),
 ("C30 #7", "§5.88 a missing star catalogue reported nowhere: which of the three contracts", "INTENT.md:418", "N+"),
 ("C30 #8", "§5.89 an unknown `viewing_mode`: abort, named default + log, or refuse to start", "INTENT.md:420", "N+"),
 ("C52 #14", "N7 the config migration writer comments out, never deletes (§5.112's direction): when", "INTENT.md:464", "N-"),
 ("C56 #3", "§5.144 RSS +68 MB/h under the tester's corpus: one 8-12 h leg (cache or leak), yes / no", "INTENT.md:526", "N+"),
 ("C59 #3", "§5.152 a second instance aborts (exit -6): join / detach before the early return, or the normal shutdown", "INTENT.md:534", "N+"),
 ("C59 #20", "EntityCore's uncapped `EntityCore-logs-<epoch>.txt` pile (read-only submodule, his stratum)", "INTENT/11.230.md:29", "N-"),
 ("C61 #6", "§5.142's ROOT: (O2) cap the shadow casters, (O2') the receive array out of the body block, (O1) carve at first draw, (O5) a named refusal", "INTENT/11.236.md:84", "N+"),
 ("C61 #10", "§5.146 SharedBuffer throws (your shape): your one-line EntityCore edit, or dispatched with your submodule push", "INTENT.md:528", "N+"),
]),
("LOCALE AND TEXT", [
 ("C49 #9", "§5.121 one static translation table for two locales: the architecture (one map per translator, one translator, an order)", "INTENT.md:489", "N+"),
 ("C49 #10", "F74's field-consequence launch (`set sky_locale zh_CN` on a pre binary): on record or not", "INTENT/11.195.md:223", "N+"),
 ("C49 #13", "§5.119's rider: should `text` route to `media->updateTextFont`, and with which size", "INTENT.md:485", "N+"),
 ("C54 #4", "§5.111's unmatched set: which translation for the body NAME (UI or sky), the type slot, `\" (orbit centre)\"`", "INTENT.md:461", "N+"),
 ("C54 #5", "§5.136 `app_locale` inert when it differs from `sky_locale`: one map per translator, one translator, or an explicit order", "INTENT.md:518", "N+"),
]),
("SESSION STATE", [
 ("C12 #3", "B31 C4: the non-body selection's catalogue key (D34's unanswered half)", "INTENT.md:1408", "N+"),
]),
("TO THE TESTER, THROUGH YOU", [
 ("C24 #4", "the oort-SHADOW onset in free mode / Sun-referenced (L2: not tested yet), state-stamped", "INTENT/11.207.md:56", "T"),
 ("C50 #7", "`comet-particles.sts`'s provenance (no generator in the tree)", "SCRIPT_SURFACE.md:392", "T"),
 ("C51 #16", "§5.128 a show setting `zoom_offset` mid-show: does it expect the view to snap back to `init_view_pos`", "INTENT.md:504", "T"),
 ("C52 #20", "R19's other half: does any of his shows depend on today's 999", "INTENT/11.207.md:231", "T"),
 ("C54 #7", "the `basis/` shows' `script action pause` with `flag_skip_pause = false`: honour or skip under unattended play", "INTENT/11.218.md:72", "T"),
 ("C55 #8", "`doc/superscript.sts:1529` `Ganymed=503` -> `Ganymede=503` (his file)", "INTENT/11.214.md:5", "T"),
 ("C55 #9", "the §11.214(k)(2) sentence on `$body_selected` (his own installation)", "INTENT/11.214.md:46", "T"),
 ("C55 #10", "the new path's nav string on two lines with `\" / \"` (a readout-format expectation)", "INTENT/11.213.md:60", "T"),
 ("C56 #9", "`fscripts/panorama5.sts:102` a `struct if end` that closes nothing", "INTENT/11.218.md:62", "T"),
 ("C57 #9", "`06old.sts` lands 156 of 170 bodies (13 duplicate names + TDRS 3 with no `coord_func`)", "INTENT.md:1134", "T"),
 ("C66 #55", "T1 the halos and the lighting (which fov, which moment, with or without rings)", "DECISIONS_PENDING.md:25", "T"),
 ("C66 #56", "T2 SHIFT+keypad or joypad for Saturn (SS-44)", "DECISIONS_PENDING.md:25", "T"),
 ("C66 #57", "T3 `flag_manual_zoom` in his config", "DECISIONS_PENDING.md:25", "T"),
]),
("THE INTERN, THE REPOSITORY AND THE DEPLOYMENT MAP", [
 ("C52 #3", "R23 where the outside content-installation procedure lives, who owns it, may the code repo name it", "INTENT/11.234.md:27", "N+"),
 ("C53 #2", "R6's other half: do pull requests still target `2023-master`", "INTENT/11.232.md:22", "N+"),
 ("C61 #2", "`doc/developer-entry.md` §9 'Engineering principles': the text, or permission to reconstruct it from the ledger's uses", "INTENT/11.234.md:27", "N+"),
 ("C61 #3", "the remote form for the intern's second clone: a key / access, or the HTTPS form", "INTENT/11.234.md:27", "N+"),
 ("C61 #4", "the measurement build (`RelWithDebInfo -O2`) is not the shipped one (`Release -Ofast -O3`): re-point and re-bank, or state it", "INTENT/11.234.md:17", "N+"),
 ("C63 #8", "map Q2: does the DEVELOPER's criterion inherit the TESTER's both-paths exclusion (48 rows carried both ways)", "INTENT/11.243.md:9", "N+"),
 ("C63 #9", "map Q1 (what 'deployment' names), Q3 ('once ready'), Q4 (`R1`'s three meanings, a rename)", "INTENT/11.243.md:8", "N+"),
 ("C63 #10", "map [V1] 732 lines: should the tiers leave the file", "INTENT/11.243.md:53", "N+"),
]),
("THE HOST, THE HARNESS AND THE LEDGER'S OWN SHAPE", [
 ("C44 #6", "`f23_b33_control`'s S1 / S2 skylock legs red: the B33 intent (your stratum) before anyone re-baselines", "INTENT/11.171.md:34", "N+"),
 ("C48 #5", "LLM assistance: which mode first, a local-only default, the checker a hard gate in agent mode", "FEATURE_REQUESTS.md:409", "N+"),
 ("C50 #3", "an anchor can resolve while its sentence is false: a content-vs-anchor audit, on your word", "fable-dispatch/archive/F75.md:1", "N-"),
 ("C50 #10", "the one marker call `§11.76 -> §11.75`: a marker at §11.75, or §2.0's span covers it", "INTENT/11.197.md:255", "N+"),
 ("C50 #11", "the malformed marker span at `INTENT/11.192.md:29` (2 `[`, 1 `]`): yours or the next editor's", "INTENT/11.192.md:29", "N-"),
 ("C50 #12", "strict-credit v2's six priced candidates (C1-C6): enact any", "INTENT/11.197.md:301", "N+"),
 ("C51 #2", "`lock-enabled` true on the desktop: one gsettings line", "HOST-EVENTS.md:225", "N+"),
 ("C52 #11", "the 38 `sc-*` scratch trees and logs under /home/claude (sc-f84 4.1 G ... sc-f123): remove or keep", "harness/artifacts/f147/scratch-trees-2026-10-11.txt:1", "N-"),
 ("C61 #5", "every inline §5 row carries its own `§5.N` on its line (a convention)", "INTENT/11.234.md:25", "N+"),
 ("C63 #11", "the canary's coverage: members for release, kernel, toolchain, interpreter, soname + a driver gate (queued): veto to keep it small", "INTENT/11.244.md:13", "N-"),
 ("C63 #13", "Q-83 / Q-84: is 'information maximisation' your name for the generator; is the oracle-spent reason yours", "~/shared/QUEUE/Q-83.md:77", "N+"),
 ("C63 #26", "the executors' `Co-Authored-By: Claude Fable 5.1` line: one of the 'redundant self co-authors' your tool drops, or kept", "INTENT/11.232.md:10", "N-"),
 ("C65 #9", "`CLAUDE.md` 'not group-writable is a defect' also flags git's own 0444 objects: a rewording", "INTENT/11.264.md:12", "N+"),
 ("C65 #10", "section 0 step 6 (executors commit) against section 0b (a) (no agent commits; the checkpoint in the entry): rule (text proposed in census §7.4)", "INTENT/11.264.md:9", "N+"),
 ("C65 #11", "the three older FIFO runners onto `f124_run.sh`; the premise filler (`harness/artifacts/s34/mint.py`) a tool or evidence", "INTENT/11.264.md:20", "N+"),
 ("C66 #39", "sizing a round against the wall-clock session window (four max-effort agents spent it in 25 min)", "~/shared/QUEUE/Q-30.md:96", "N-"),
]),
]

TAGS = {"N+": "[N+]", "N-": "[N-]", "T": "[T]", "D": "[D]"}


def render():
    out = ["```", "YOURS — ONE LIST (held set per node; the entry's section in brackets)",
           "tags: [N+] the item's own node states the ask · [N-] the node holds only the open state, the ask's text is §3's alone",
           "      [T] routed to the tester, through you · [D] a default that queued or running work builds unless you rule",
           "      [s42] already in the session-42 block (stays there; not to be doubled)"]
    for head, items in G:
        out.append(head + ":")
        for key, text, ptr, tag in items:
            s42 = " [s42]" if key.startswith("C66 ") else ""
            out.append(f"  {text} -> {ptr} [{key}] {TAGS[tag]}{s42}")
    out.append("```")
    return "\n".join(out)


def keys():
    return [k for _, items in G for k, _, _, _ in items]


if __name__ == "__main__":
    print(render())
    if "--check" in sys.argv:
        car = set(l.split("\t")[0] for l in open("/home/claude/spacecrafter/claude/harness/artifacts/f147/carried.tsv", encoding="utf-8") if l.strip())
        ks = keys()
        dup = sorted(k for k in set(ks) if ks.count(k) > 1)
        print(f"check (c): list {len(ks)} lines, {len(set(ks))} distinct; CARRIED set {len(car)}; missing {sorted(car - set(ks))}; extra {sorted(set(ks) - car)}; duplicates {dup}")
        from collections import Counter
        c = Counter(t for _, items in G for _, _, _, t in items)
        s = sum(1 for k in ks if k.startswith("C66 "))
        print("tags", dict(c), "s42", s, "groups", len(G))
