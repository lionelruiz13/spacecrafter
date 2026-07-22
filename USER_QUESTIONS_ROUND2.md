# Questions for testers — Round 2

Last updated: 2026-07-21.

Round 1 (Q1–Q27, answered, your answers kept inline) lives in
`USER_QUESTIONS.md`. Round 2 now has its own file so the live questions
aren't buried under a few hundred lines of answered history.

**Status: ANSWERED 2026-07-22 (tester, relayed by Vixy) — propagated to the ledger `INTENT.md` §11.70 + §13 rows (same folder since the 2026-07-22 repo split).** The live items (R1, R3–R11, R13) were answered inline below and the no-longer-questions (R2, R12) confirmed; the answers here are the raw record, the ledger is what each answer decides (§11.70). Originally: ten+ items live (R1, R3–R11, R13).
Two are no longer questions — R2 and R12, kept at the bottom: worth a
skim, because one of them states a behavior you can still veto, and the
other tells you what your earlier answer produced.

Same rules as round 1, and they're short: reply inline, one line per
question is enough. "I don't have a strong opinion" is a valid and useful
answer — it tells us the choice is safe to make on engineering grounds
alone. If a question doesn't make sense as asked, say so; that's
information too. Questions about what your *scripts* do are decisive
rather than advisory: existing show scripts are treated as immutable, so
"my scripts rely on that" closes a question by itself. The `(ref: …)` tag
under each question is the stable key for matching your answer back to
the internal tracker; you can ignore it.

## First: your two questions back

**You asked (in Q17): "Is eclipse shadows the shadows casted on a body?"**
Yes. That's the one you're thinking of: the Moon darkening as it enters
Earth's shadow, or Io's small black dot crossing Jupiter's cloud tops.

**You asked: "And ring shadows is the size of the shadow of Earth at Moon
distance?"** No — sorry, that was our wording, not your reading. "Ring
shadows" means Saturn: the dark band the rings lay across Saturn's globe,
and the shadow the globe throws across the rings (the one that sweeps
along the ring plane through the seasons). It has nothing to do with
Earth or the Moon. Because that guess was wrong, we've kept only two
things from your answer — that the two must be separate, and that eclipse
shadows default on — and we're re-asking the ring half below (**R1**).

## The live questions

**R1. Ring shadows (Saturn's rings on the globe, and the globe on the
rings) — do you want to be able to switch them off during a show, and if
so, on or off when the software starts?** Two clarifications since we
asked: the switch you were answering about turns out to be a
development-only switch that disappears when the old rendering path is
removed, so what we're really asking is whether you want an *operator*
control at all. And to be sure we're talking about the same thing: ring
shadows are the dark band the rings lay across Saturn's globe, and the
shadow the globe throws across the rings — nothing to do with Earth or the
Moon. Eclipse shadows (a body's shadow falling on another body) will be on
at start-up either way.
*(ref: A28)*
The shadow of the planet on the rings should be activated everytime. Only desactivated for test purpose.

**R3. Of the three kinds of camera anchor, which do you actually use?**
&nbsp;&nbsp;(1) a fixed point in space (x/y/z in AU),
&nbsp;&nbsp;(2) a point attached to a body,
&nbsp;&nbsp;(3) a point on an orbit around a body — and with or without "follow the body's rotation"?
Any combination is a fine answer, including "only (2)". This decides how
the anchor system is rebuilt, which is why it's worth a second ask.
*(ref: B4)*
We use mostly (3), but we are lacking (2), having an object attached to a body keeping its angle to the planet which is very important for future scripts. (1) has no real interest as it doesn't exist in the solar system. It is only useful in the "Universe" mode as nothing moves. For the "Galaxy" mode, the stars will move through time in the future.

**R4. The two-body trick (Q12): you said your scripts do it — what for?**
A shell or zone you fly into, a transparent/gaseous body, a cutaway view,
something else? We're replacing it with two per-body radii, and we can
only claim the replacement is complete if we know what the original was
buying you.
And the second half: **in free flight, when you reach the lowest allowed
altitude, should you stop dead and hold there, or slow down more and more
without ever quite arriving?**
*(ref: B10)*
A zone we fly into and also for transparent bodies, if the behavior change, we will adapt the scripts.
We should stop if going to the lowest altitude. Slowing down will be annoying as it'll take time to go out. 

**R5. Selection when several objects are almost on top of each other
(Q11): you said "the brightest/biggest/main object".** Those three can
disagree — a large dim moon next to a small bright one, or a planet next
to a moon that currently looks bigger. When they disagree, which one
should win?
*(ref: A17)*
The biggest should win because it'll be the brightest in 99% of cases due to surface magnitude. 

**R6. Descending from very far away (the other half of Q6).** Coming down
from a galactic viewpoint, should the descent aim at whatever body you
last selected, or just bring you into the general vicinity and let you fly
the rest yourself?
*(ref: A18)*
Last selected.

**R7. The planet grid (the lat/lon lines drawn on a body's surface).**
Two leftovers from Q16:
&nbsp;&nbsp;(a) today it appears/disappears together with the planet's rotation axis — should it get its own on/off instead?
&nbsp;&nbsp;(b) in the old version it only appeared once you were more than ~10 km above the surface. Do you want that back, or should it be visible at any altitude?
*(ref: A4)*
Planet rotation axis is the toggle to make it appear onto the planet. It must stay associated.
Visible at any altitude.

**R8. Colors and reloading (leftover from Q15) — the stakes changed since
we asked, so here is the honest framing.** If a script reloads a body
(re-issues its definition mid-show) after you changed one of its colors at
runtime, today's software either keeps your color or resets it to the
body's default. Since asking, a general rule was set: the new rendering
path reproduces whatever today's software does, exactly — so your answer
no longer decides what gets built now. It's still worth answering: tell
us what you'd *prefer*, and if that differs from what the software
currently does, it becomes a logged change request for later instead of
being lost.
*(ref: A3 → B29)*
It must reset to the body default or color asked in the script. The user must reload the rule to be applied to it as well.

**R9. Landscapes (leftover from Q18).** You said the landscape changes to
a specific one when you arrive, and stays on whatever the user picks
afterwards. Two things that decides:
&nbsp;&nbsp;(a) should a body's data file be able to name its landscape (`landscape = moon_landscape` in the body's entry), or should the engine keep deciding by built-in rules?
&nbsp;&nbsp;(b) after the user picks a landscape by hand, how long does that choice last — until they land somewhere else, until they pick another one, or until the end of the session?
*(ref: A27)*
Decide by built-in rules.
Until they land somewhere else or if the user decide to change landscape by script.

**R10. "Linked to Earth or Moon or Sun behavior" (Q24) — what do you
expect to change?** Since asking, the plan changed in your favor: the
list of special behaviors those bodies get will be extracted from the
software's own code, so you don't need to enumerate anything from memory.
Your answer now serves as the completeness check. When you link a body to
"Earth behavior" for a comparison, tell us what you *expect* to change —
lighting, atmosphere, landscape, the fact that the sky/horizon works,
which body the "moon" in the sky is, day-length handling, anything else.
We'll compare your expectations against the extracted list; anything you
expect that we didn't extract is exactly the gap we can't find ourselves.
*(ref: A29 → B27)*
Nothing will have to change if we go back.

**R11. The view offset for tilted domes (Q3).** Is it set once for your
installation (in the config, never touched again), or do scripts change it
during a show?
*(Heads-up on stakes: since asking, an engine rule settled that both
channels — config file and live command — get built either way, so your
answer no longer changes the design. It still tells us which usage to
test hardest, which is why the question stays.)*
*(ref: A24 → B17)*
Yes, it can change during a show.

**R13. Anchored, then out to the galaxy and back — should the anchor
survive the round trip?** When you lock the view onto a body (anchor) and
fly outward, the anchor is kept and you see the solar system shrink to a
point — that part works and stays. The open question is the way *back*:
today, as you descend again, the view quietly re-anchors to the solar
system as a whole rather than staying locked to the body you left from.
So if you anchored on Earth, flew out, and came straight back, you would
*not* return to being anchored on Earth. Is that acceptable, or do you
expect the anchor to hold the whole way out and back (so "go out and
return" always lands you exactly where you started)? If it should hold,
does that apply to every anchored body, or only some? (This one is finer
than the others — it only matters if you actually fly anchored out to
galactic distance and back during a show; if you never do, say so and we
stop worrying about it.)
*(ref: A14 → B20 — deferred here from the B20 investigation, INTENT §11.66(e))*
The anchor must hold when we get back to this mode. So memorize it for each mode.

## No longer questions

**R2. A body is removed from the simulation while the camera is involved
with it — answered meanwhile by the design authority, and you can still
veto.** Both situations now resolve the same way: the camera falls back
to the **parent body**. Drop a moon you were following → you follow its
planet; drop a body you were standing on → you're attached to its parent.
This matches both possible readings of your earlier "(a) is better",
which is why it could be accepted without re-asking. If that is *not*
what you meant, say so now — it costs nothing to revisit before release,
and much more after.
*(ref: A9 — resolved 2026-07-21)*
Yes.

**R12. Withdrawn — your Q26 answer was right, and it decided the design.**
Not a question any more, but you should know what it produced: rather than
choose between "the data file decides everything" (your answer) and "every
capability must be reachable at runtime" (an engine principle it conflicted
with), there will be **two ways to describe a body**. The legacy file
format keeps working exactly as it does today, with your rule intact — if
it's not in the data, it isn't activated. Alongside it, a second format
lets anyone who wants to compose a body out of parts do things the legacy
format can't express. Nothing you have breaks, and nobody is prevented from
going further. The software will also write out each of your legacy files
translated into the new format, with a `.disabled` extension so it does
nothing — there purely so you can see what your own data looks like in the
new model. Since then the details firmed up: these files live in the
`modularSystem/` folder inside the spacecrafter home directory, named like
the original plus `.disabled` (e.g. `mysystem.ini.disabled`), and adopting
one is literally just removing `.disabled` from the name.
*(ref: A2 — resolved 2026-07-21)*
Ok.

---

*Traceability note: this document is derived, not authoritative — the live
decision ledger is `INTENT.md` §13 (same folder). If that file
and this one ever disagree (e.g. an item gets resolved in one but not
updated here), the ledger wins; tell us and we'll reconcile.*
