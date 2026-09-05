# Questions for testers — Round 3 (the final pass)

Last updated: 2026-08-31.

**Status: ~~DRAFT — compiled 2026-08-31, NOT yet sent.~~ ANSWERED 2026-09-05 — the
replies below are the MAIN TESTER's (Lionel RUIZ), transmitted by the owner
[vixy 2026-09-05: *"Reading the claude code's Edit side, I can tell those came from
the main tester/user"*]; committed inline 13:38:30 under the owner's git identity and,
on his word, the commit's AUTHOR was amended to the tester's history identity
(`Lionel RUIZ <lionel.ruiz@live.fr>`, 151 commits in the code repo): `c5be42b` →
**`6ffb017`**, committer = the supervising session (the transport). Provenance tag for
every reply: `[stated: tester, via owner commit 6ffb017]`. Propagation into the
ledger (§13/§5/DECISIONS rows, DEPLOYMENT-MAP, back-markers) = F88; until it lands,
the replies are authoritative and the rows are STALE.** This file is the
send-time view of the ledger-owned final-pass list (INTENT §11.116(c):
tester items accumulate into ONE pass before testing deployment; the
ledger wins on divergence). Before it goes out, the developer applies the
per-question routing filter — "what the tester knows better how to
answer" (§11.161(c1)) — to the candidate section at the bottom, and may
strike or add items. Compiled from: the ledger final-pass members
(§11.149(g), §11.116(c)), DEPLOYMENT-MAP.md T3/T4, §11.162, §11.163(j),
§11.177(h)(i), §11.173(b)(c), §5.115/§5.21/§5.98, SCRIPT_SURFACE.md.

Rounds 1 and 2 (answered, your answers kept inline) live in
`USER_QUESTIONS.md` and `USER_QUESTIONS_ROUND2.md`. Numbering continues
from round 2: these are R14 onward. The `(ref: …)` tag stays the stable
key; you can ignore it.

## How this round is different (and why)

We measured how the first two rounds actually worked, question by
question, and changed the format accordingly — the summary is: your
answers were never wrong inside your own domain; what got lost was
*parts* of multi-part questions. So:

- **One decision per item.** No four-part questions this time. A few
  items are conditional ("only if R14 = X") — skip freely.
- **History questions are separated** and live in `SCRIPT_SURFACE.md`
  (they are the ones that can't be answered by fixing a file). Answer
  them one at a time or say "don't know" — both are useful.
- **There is an explicit "anything you would add" slot at the end.**
  Last rounds you volunteered 21 proposals inside answers to other
  questions, and 4 became tracked work items. This time the slot is
  yours by design instead of displacing an answer.
- **A proposal must arrive with its interest visible — what it enables,
  simplifies, or removes from your hands — never as a bare change.**
  (The developer's rule, added for this round: a change whose interest
  isn't visible is *rightly* dismissed as a useless one. R25 was
  reframed under it.)
- Same rules as always: one line per answer is enough; "no strong
  opinion" is a valid answer; if a question doesn't make sense as asked,
  say so — that's information; what your *scripts/shows* do is decisive,
  not advisory.

---

## The questions

**R14. An invisible star: does it still light the scene?**
When a star is hidden, should the bodies it lights go dark, or stay lit
by a star you can't see? Today: they stay lit (an invisible body can be
used as a "light rig"). Worth knowing before you answer: on the shipped
data this is invisible either way — every drawable body descends from the
Sun, so hiding the Sun hides *everything* and the two options give the
exact same picture. The question only bites in a scene you author (a body
declared with `parent = none` beside the star) or in a future multi-star
system.
*(ref: D37 → B39, §11.149(d) — delegated to you by the developer because
"it changes the behavior of spacecrafter under identical use")*

If we hide the star, it won't light the scene.

**R15. Only if R14 = "go dark": what does the darkened scene look like —
a faint ambient glow, or genuinely black?**
*(ref: D37's attached question)*

It depends on the ambient_light value.

**R16. Do you run shows with more than one star system loaded at once?**
(Asked in round 1 inside Q11 and lost in the shuffle — our fault, it was
the fourth part of a four-part question. It matters: it decides how much
of the multi-system machinery needs to be reachable for you.)
*(ref: Q11 second part → A30, §11.177(h))*

No. Only one stellar_system at once.

**R17. Do you show (or plan to show) systems with more than one star,
where each star would need its own halo colour/texture?**
(The other lost sibling, from Q22. Also decides where R14 becomes
observable at all.)
*(ref: A7 residual)*

Yes. Binary stars must be shown appropriately.

**R18. Have you ever authored a body with `coord_func = location_orbit`
in a data file?** If yes: was its `orbit_lat` written as real degrees, or
tuned by eye until it looked right? (Background: that path currently
distorts the authored latitude; we can fix it exactly, but if any of your
values were tuned by eye against the distortion, an exact fix would move
your body — so the fix waits on this answer.)
*(ref: §5.21, §11.163(j))*

No.

**R19. Do any of your shows test `$body_selected` against 600 (Saturn) or
503 (Ganymede) — or deliberately work around getting 999 for those two?**
(Background: selecting either body answers 999 today because of two
misspellings inside the engine's table; every other body matches its
documented number. The fix is two spellings, free once you confirm
nothing relies on today's 999.)
*(ref: §5.98 / SCRIPT_SURFACE SS-17)*

Satun must be corrected to Saturn and Ganymed to Ganymede.

**R20. The script log: how far back do you actually reach?**
Background you should have: this is the log that has already produced
gigabyte-scale files at client sites, because it is the only one kept
across launches and nothing ever caps it. It is becoming bounded — every
log channel gets a retention cap (this also *restores* the debug channels
you lost in the 2020 room-tradeoff, since bounded room dissolves that
tradeoff). Everything is settled except one number, and it's yours: when
you go back into old script logs, how far back do you need — days,
weeks, a number of launches? "The last N days" or "N launches" both work
as an answer.
*(ref: §5.115, §11.173(b))*

8 launches

**R21. Content census — which of these do your real shows actually
load?** (Tick/cross per line, one word each; this bounds which engine
work can block you at all and which never will.)
- sky cultures beyond the default (constellation sets by culture)
- deep-sky 3D content (nebulae/galaxies as volumes, the 3D universe view)
- full star catalogues (millions of stars, beyond the default package)
- other star systems / `stellar_systems` content
- videos (flat, domemaster, VR360)
- very large body populations (comet swarms, asteroid belts, 100+ moons)
- runtime-loaded images / audio as show content
- joystick or other hardware controls
- a window/display taller than it is wide (portrait setup — one of the
  two configurations the developer names as barely tested; knowing
  whether it exists anywhere in the field decides if it needs testing
  at all)
*(ref: the content census, §11.162 / DEPLOYMENT-MAP T3; the portrait line's harness half is now measured — §11.202 / §5.129: a window taller than wide draws the dome 128 px low with an unwritten band above it, everything else about portrait being unchanged, so a "no" here retires a known cost rather than an unknown one. The question body is unchanged.)*

All have been tested, but sometimes long ago, so maybe some features could have altered the way it shall work.

**R22. On your installation, does `search` find constellations and named
stars?** (On our test install the sky-culture data is empty — 2922
zero-byte files — so search finds nothing and we cannot tell what your
deployment actually carries.)
*(ref: §5.74 field-content family, routing ratified §11.161(c1))*

Search is deprecated.

**R23. Which star catalogue package does your installation run, and does
your `stars.ini` name its files correctly?** (Ours silently runs 26 561
stars instead of millions — a name mismatch between `stars.ini` and the
catalogue files on disk would do that in the field too, silently.)
*(ref: §5.90)*

By default, only limited catalogs are loaded. Correct catalogs are loaded in an outside installation procedure.

**R24. Is `~/.spacecrafter/stars/` a directory you actually put catalogue
files in?** (Decides whether it must be honored as a search path.)
*(ref: §5.90, second half)*

We should but for now it is in another directory.

**R25. Running more than one script at a time — a resubmission, with
the interest stated first this time.**

*What it unlocks.* Two more ideas from the developer's own list sit
behind this one — his proposals, like this whole item, not something
you asked for; they are described here because their use would land in
your hands, and you are the judge of whether it's worth anything.
**Dynamic binding**: a
joystick or console has few buttons and they are fixed — with
`script action bind on <key-or-button> launch <script>`, one button can
mean a different thing in each show, or each moment of a show, and a
binding can end with the script that made it (nothing to un-bind by
hand). Where that lands in practice: a single button that cycles
between modes; navigation handled by a script — which you already do
in part (the script section that offers the planets and warps to the
one selected). The difference the binding makes: today such a script,
launched, *is* the running script; bound to a button it is simply at
hand — pressed whenever, running beside whatever is playing, gone when
done. **Triggers**: a script that fires by itself — when the camera
attaches to a body, crosses an altitude, when a body comes into view.
Where that lands: the two-appearance body you already build — a
different look from far and from close, done today by giving the
miniature/preview skin a second design. That works until it doesn't:
the swap keys on apparent size, not distance, so far-but-zoomed shows
the wrong face — the developer has already warned you this usage can
break at any time. A trigger firing on the distance or visibility
change, driving the `skin_tex`/`skin_use` swap you already have, is
the same effect as a supported mechanism instead of a side effect of
the miniature system. (R29 below asks which bodies do this today —
it matters beyond this proposal.)
Both of these mean a script starting *while your show is already
playing* — and today's engine has no defined answer for that situation
(scripts have always been serial; what nesting does today is accident,
not contract). This proposal is the prerequisite that gives it one:
scripts get names, a named script can run beside another, controls
address the named one, and ending a script ends what it started.

*What stops being managed by hand, even before any buttons.* A
background strand — an ambient time-lapse, a repeating camera drift, a
music/subtitle track — no longer has to be hand-woven into the main
timeline with computed waits; it is its own named script, and stopping
the show stops the whole tree without you keeping the list of what was
started. And an unclosed `struct if` in a called script can no longer
silently skip the rest of the *calling* script — the same silent-skip
shape SS-24 just found inside superscript.sts, no longer able to cross
a file boundary.

*What it costs you: nothing until you use a name.* A script launched
as today keeps name `""` and exactly today's global-control semantics;
the execution policy defaults to `legacy`, which is a contract naming
today's nesting behavior as-is. Existing shows byte-untouched, habits
intact; it stays one vocabulary (`script action …`), not a family of
new commands.

*Your two earlier objections* were raised against an older draft and
are answered in the current text: (1) "which script do the global
controls target?" — the unnamed default keeps them global, named
scripts are addressed by name, tree-wise, and the special name `*`
addresses every running script at once (one word still stops
everything); (2) "hard to track" —
tracked/detached is explicit per launch, with the termination cascade.
One honest gap: how resume/speedup compose down a tree is designed but
not yet written as clauses.

**Question: with the interest visible — worth having?** (If your two
objections still stand against the current text, or the cost still
looks unbalanced, say so with the reason — a "no because…" is exactly
as useful, and it lands on the current version, not the draft you saw.)
*(ref: FEATURE_REQUESTS [parallel-script] + [script-binding] +
[script-trigger]; §11.173(c); prerequisite chain [vixy 2026-08-31];
R21's hardware-controls line informs the binding half)*

No need for that.

**R26. [confirm] Free flight ↔ attached: the proposed default.**
Switching into or out of free flight will hold view continuity — no jump,
no warp; your heading is then recovered over a smooth,
minimum-acceleration transition. The developer's proposed default is
**heading stays stable across the swap**, flagged by him "to confirm with
user/tester". Is stable heading what you'd expect as the default?
*(ref: D15(b), §11.149(a))*

Stable heading when switching or at least change of camera orientation smoothly.

**R27. The RA/DE readout: the catalog number, or the pointing number?**
When the app displays a body's RA/DE, two conventions exist and they
only differ where you'd notice on the Moon (~1° — the Moon's parallax;
everything farther is identical): the number **as seen from where the
observer stands** (what you'd point a telescope with — today's
long-standing behavior), or the number **as seen from the Earth's
centre** (what a catalog or almanac prints). Which do you rely on when
you read RA/DE off the screen? (Routed to you because you read
coordinates professionally and this convention was inherited, not
chosen. The *zero point* of RA is not part of the question — that is
fixed by definition at the vernal equinox and is being verified against
catalog values of known stars.)
*(ref: §11.4 decision (2), §11.158(f); routing per §11.161(c),
exercised [vixy 2026-08-31])*

The RA/DE must be the value from our position.

**R28. The tilted-dome view offset: what happens when it meets a
rotated heading?** Background: the view offset exists for tilted-dome
geometry (your round-1 answer) and can change mid-show (R11). Its plain
behavior is field-proven — an installation used it, issues got reported
and fixed — and it stays exactly as it was, on principle: what the
field may silently depend on is never changed. The corner nobody
exercised is the *combination* with a rotated heading (heading is one
of the two configurations the developer names as barely tested), and
there the two paths disagree: the old software turns the offset
direction *with* the heading; the new path keeps it fixed to the
physical dome. If you know the installation that used the offset (or
serve any tilted-dome setup): did it ever rotate the heading during a
show — and if so, which behavior did it depend on? If the combination
was never used to your knowledge, say exactly that — it downgrades this
to an engineering call and we stop pressing.
A second, plainer question about the same offset, from a measurement made
this week: when a show changes the offset *while the view is following a
body*, the old software snaps the view back to the show's starting
direction (`init_view_pos`) — and at narrow fields of view the body it was
following can leave the image entirely; the new path keeps looking at the
body. Did any show that changes the offset mid-show rely on that snap-back
(the view returning to where the show started), or was the view expected
to stay on what it was looking at? Either answer settles it; "never
changed it mid-show" settles it too.
*(ref: §11.92(d), B17 residual; routing + the exercise-boundary
testimony [vixy 2026-08-31]. **POINTER 2026-09-04, F81 §11.201 → §5.128
(measured, record-only): the plain offset carries a SECOND corner beside
the heading one — old's aim site compensates a fixed 90° while its draw
applies fov/2, so a mid-show `set zoom_offset` lands the aimed body
0.0000°/13.5000°/21.0000° off the drawn centre at fov 180/90/40 and
throws the old view 29.4474° back onto `init_view_pos`, while the new
path keeps its aim. That raises a question this text does not yet ask —
"when a show changes the offset mid-show, does it EXPECT the view to
return to init_view_pos?" — whose answer decides whether only the
scaling is wrong or the whole aim half is. RECORDED, not inserted into
the question body: what this file asks a human is the supervising
session's call, not the executor's.**)*

The offset is screen dependant. The change of heading shouldn't shift the offset position.
I saw that it does now but it shouldn't.

**R29. Which bodies deliberately look different from far than from
close?** The shipped Sun and Moon carry a miniature/preview skin that is
a *different picture* from their full-resolution map (the Moon visibly
changes tone when crossing the swap distance — measured). Until now this
was classed as a data inconsistency, to be fixed by regenerating the
previews from the full maps. If it is instead your intended
two-appearance design (the far/close effect from R25), regenerating
would destroy it. Which is it — slip or design — and if design, on
which bodies? (Your answer also decides at what distance the swap
should engage, and R25's trigger is the supported replacement either
way.)
*(ref: A43 + A42, §11.127(c)(d), §11.172(c)(d); premise reframed by
[vixy 2026-08-31]: the tester authors distinct normal/miniature skins)*

It's a design. 

---

## For your information — deliberate changes, with a revise/revert offer

These are decided and (where noted) already built. The developer's own
instruction: *"the main user/tester must be informed — so that he can
revise/revert the decision if needed."* Silence = accepted; any of these
can be reopened at your word, before release, at low cost.

**N1. VIEW_HORIZON is not being rebuilt on the new path.** It existed to
work around the old system's up-vector flip (looking up while moving in
latitude could spin the whole view); the new camera avoids up-vectors, so
the problem it solved is gone. The config value stays recognized and the
old path is untouched.
*(ref: D15(a))*

**N2. Free-flight transitions: the requirement.** Entering and leaving
free flight must be transparent in both directions — view continuity
under a dome has the same structure as a VR headset, so every transition
optimizes for minimal acceleration. Stated as a requirement; the
implementation is not yet built (R26 above is its one open default).
*(ref: D15(b))*

**N3. Tracking through transitions — built.** Continuous tracking is
preserved and smooth ("replicate the body tracking function of advanced
telescopes"): four places where the sky-lock and tracking state could
silently disagree between the two paths are now mirrored, verified in
both directions on the shipped command surface.
*(ref: D15(c), task F38)*

**N4. Two new config keys — built.** `[navigation] attached = true` and
`flag_lock_sky_position = false`, both defaulting to today's behavior, so
startup/reinit fully initialize the camera state. An older spacecrafter
version reading a config carrying them survives (measured on both parser
routes).
*(ref: D15(d), task F38)*

**N5. Changing reference body keeps your whole framing — including roll.**
Your round-1 answer (Q2: "keep looking at the same absolute direction")
is implemented literally: after `set home_planet X` the image does not
jump *or roll*. The old path re-derived roll from the mount (north
snapped back up), so this is a visible difference on the new path; the
price is that after a chain of switches the horizon can end up tilted —
the remedy is an explicit `set heading 0`, and the heading readout now
reports the drawn view truthfully.
*(ref: D28/A38, §11.113(g))*

Just switch to the planet, keeping the old values (that we will change by script anyway).

**N6. `camera action move_to … duration 0` is guarded on the new path.**
The old path produces a broken camera (NaN) on that input; the new path
refuses it cleanly. No shipped script uses duration 0 — this is only
visible if you write one.
*(ref: §5.83)*

**N7. Warning before the next data/software delivery — please read.**
The first launch of a *newer version* over your existing `config.ini`
rewrites that file by itself: comments destroyed, keys lowercased, and
any key the new version doesn't know in a known section *deleted* — no
command needed, it happens on version mismatch alone. If your config
carries hand-written comments or custom keys, copy it aside before
upgrading. (This mechanism is years old and both-paths; you're being
told now because a delivery is the trigger.)
Measured 2026-09-05 on your own installed `config.ini`: a version bump deletes
**nothing** from it and adds two keys — but on the same file with two comment
lines, one hand-added key and one extra section, it deletes **all four** and
both comments, and the result is byte-identical to the run that had nothing
added. Whatever is not in the new version's list does not survive.
*(ref: §5.112, §11.163(f), §11.204(j))*
*[Paragraph RESTORED 2026-09-05 by the supervisor: the reply below was pasted
onto a copy of this file taken before F84 added it; the measurement and its
citation are the ledger's (§11.204(j)) and the reply is kept verbatim.]*

Put a # in front of the deprecated lines would be better

---

## Judged live, not on paper — the two items that need your eyes

These two are part of the same final pass but can only be answered while
operating the program; every judgment gets a state stamp (fov, anchor,
flags, scene — one command) so the configuration is never lost again.

**L1. The system-collapse cross-fade.** From far enough away a whole
system collapses to a single dot; you asked for a cross-fade instead of a
pop (Q9), and it exists: the collapse threshold sits at ~16 px on screen,
the fade spans ~8 px of the approach, and a residual ~10% brightness step
remains at the faint end. Judge side-by-side: does the transition feel
right — too early, too late, band too narrow/too wide, is the residual
step acceptable?
*(ref: A15, §11.82)*

It would be better to have no residual threshold

**L2. Oort-cloud shadow onset.** Your 2026-07-24 observation: "I think
the oort shadow are showing too early." The configuration that produced
it was reconstructed: free flight, Sun-referenced, fov 340, climbing in
altitude. (In the anchored-Earth ladder the new path shows the cloud
*later* than the old one, so the question is live only in your
configuration.) The same scene gets re-flown with you watching,
state-stamped: early against what expectation?
*(ref: §11.98(c) → §11.116(b))*

I didn't test it yet

---

## Your other open file

`SCRIPT_SURFACE.md` (same folder) is still yours and still open — the
script-surface entries are not repeated here. The two that decide entire
defect classes if you answer them: **SS-12** (what should a session
*recording* contain — what you typed, or what the engine did? both
currently coexist) and **SS-11** (should `suntrace` aim at the Sun, as
the 2020 reference says it was meant to?). The history questions
(SS-2, SS-4, SS-5, SS-9, SS-10 — "did this ever work / what was it
for") sit there too, each answerable alone, "don't know" welcome.

## Anything you would add

Deliberately empty and yours: commands you're missing, spellings you'd
prefer, behaviors you've always wanted, anything the questions above
made you think of. Last rounds this content arrived squeezed inside
other answers and four of them became real tracked items — it gets its
own room now.

---

## Not included, so the omission is visible

Three items from this section's first draft moved UP into the questions
at the developer's routing call (2026-08-31): the RA/DE origin (→ R27),
the tilted-dome offset behavior (→ R28), and the far/close texture
question (→ R29, which now also gates the Sun/Moon preview data fix).
One item resolved without you and is stated here so you can object: the
*zero point* of the RA readout — the new path's RA was off by a constant
90° — is fixed by astronomical definition (RA = 0 at the vernal
equinox; no convention freedom exists), so it is corrected against
catalog values of known stars rather than asked. The small residual
beyond the 90° (~1 arcsecond) gets attributed during the fix, not
absorbed.

Still held back as the developer's decisions: the free-flight `moveto`/
`get status position` defaults (§11.144 riders — become FYI items here
once he confirms), free-flight environment semantics (§5.106), the
quit-during-render fork (A40), the dot-vs-disc pixel gate (A41), the
texture-swap *distance* (A42 — R29's answer feeds it), the ring-shadow
layer contract (A44), and the script-semantics defect batch
(DEPLOYMENT-MAP T1.6). If any of these looks like it should have been
YOUR question, say so — that's exactly what this section is for.

---

*Traceability note: this document is derived, not authoritative — the
live ledger is `INTENT.md` (same folder; §11.116(c) owns the final-pass
list, §13 the open items). If they ever disagree, the ledger wins; tell
us and we'll reconcile.*
