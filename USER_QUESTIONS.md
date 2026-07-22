# Questions for testers close to development

Last updated: 2026-07-21.

**Status:** the questions below (Q1–Q27) were answered on 2026-07-21 —
answers are kept inline under each one, exactly as given. They have been
propagated into the internal tracker: 13 decisions closed, 11 narrowed, 2
of the answers turned out to be requests larger than the question that
produced them and were logged in `FEATURE_REQUESTS.md`. **Round 2 lives
in its own file, `USER_QUESTIONS_ROUND2.md`** — it starts by answering
the two questions you asked back, then asks only what is still open (one
question had to be re-asked because it was badly written the first time —
that's on us, and it's explained there).

## What this is

Every question below corresponds to a real, currently-open decision in the
engine rework — not a hypothetical or a wish-list item. Each one is blocking
something concrete from being finalized (a behavior, a command, or the
retirement of an older code path). Nothing here was invented for this
document; each question traces back to a row in the internal tracker
(`INTENT.md`, §13 — same folder since the 2026-07-22 repo split), given here in parentheses as
`(ref: Axx)` / `(ref: Bxx)` purely so an answer can be matched back to the
right item without ambiguity — you don't need to read that file to answer.

**The `(ref: …)` tag is the stable key, not the question number.** Numbering
can change between revisions of this document; the refs don't.

If you're unsure, "I don't have a strong opinion" is a valid and useful
answer — it tells us the choice is safe to make on engineering grounds
alone. What's *not* useful is guessing at an answer you don't actually hold;
say what you'd genuinely want, not what sounds right.

Roughly half of these are "do you actually use this / would you notice?"
questions and answer in a word. The rest ask for a judgment call where your
experience of what a planetarium audience sees is worth more than ours.

## How to answer

Reply inline, one line per question is enough (a sentence of "why" is
welcome but not required). If a question doesn't make sense to you as
asked, say so — that's information too, it usually means the phrasing
missed the real use case.

Several questions ask what your *scripts* do. Those answers are decisive
rather than advisory: existing show scripts are treated as immutable, so
"yes, my scripts rely on that" closes the question by itself.

---

## Camera, tracking and view

**Q1. If a body is removed from the simulation while the camera is involved with it, what should happen?**
Two cases, and they may deserve different answers:
(a) the camera is *following/tracking* it — switch to following its parent body (drop a moon, keep following its planet), or stop following anything?
(b) the camera is *anchored on* it (you're standing on or attached to it) — fall back to the parent, or to some fixed neutral position?
*(ref: A9)*
(a) is better

**Q2. When the camera changes what it's attached to — switching reference body, or entering/leaving free flight — what should happen to where you're pointing?**
Keep looking at the same absolute direction in the sky, keep the same framing relative to the new body, or re-center on the new body? And should it snap instantly or ease over a short move? (Dome comfort matters here — instant re-orientation on a dome is a different experience than on a flat screen.)
*(ref: A11)*
Keep looking at the same absolute direction in the sky

**Q3. The camera used to support a "view offset / zoom offset" — shifting what's on screen off-center without moving the camera itself. Do you still use this, or is it fine to drop?**
If you do use it: for what — off-axis or tilted dome geometry, keeping the horizon low in the dome, something else?
*(ref: A24)*
tilted dome geometry

**Q4. There used to be a way to lock the sky in place so it doesn't move when the observer moves (`flag_lock_equ_pos`, the equatorial-mount companion). Do you still use this, or can it be retired?**
Note the equatorial *viewing mode* itself (config `viewing_mode`) is ported and stays either way — this question is only about the sky-lock on top of it.
*(ref: A25)*
Yes, we still use it.

**Q5. How do you use named camera anchors and saved camera positions?**
The old anchor file supports three kinds: a fixed point in space (x/y/z in AU), a point attached to a body, and a point on an orbit around a body — plus "follow the body's rotation". Which of these do you actually use, do you edit `anchor.ini` by hand or only create anchors from scripts, and do you rely on saving/restoring camera positions between sessions?
*(ref: B4)*
We edit anchor.ini if we need and we create anchors from scripts. But we don't need to save them between sessions (even if it could be useful to have a command that saves it in the anchor.ini).

## Navigation and transitions between bodies

**Q6. When flying freely toward a planet and pressing "move down," should "down" always mean straight toward the planet's center, or toward the specific point on the surface you're currently looking at?**
Related: coming down from very far away (say from a galactic viewpoint), should the descent aim at whatever body you last selected, or just land you in the general vicinity and let you fly the rest?
And at the other end of the scale — how does altitude control feel when you're *very* close to the ground? The control multiplies your current altitude, which works beautifully across ten orders of magnitude and then goes dead as you approach zero (which is why the start-up altitude is set a little above the ground rather than at it). Do you hit that, and how do you work around it today?
*(ref: A18, and the near-surface regime it borders)*
I would say "toward the specific point on the surface you're currently looking at". When we are close to the surface, we know where we are and we have to stop coming down as it doesn't move.

**Q7. Should the software ever change your reference body *by itself* while you're anchored (not in free flight)?**
Today: automatic hand-off up and down the chain (planet → star → solar system → galaxy → universe) happens **only in free flight**. When you're anchored — e.g. after `set home_planet Earth` — you stay anchored to that body whatever your altitude, because an automatic hand-off in between two script commands was observed to race the script and land the camera somewhere unintended. Is anchored-stays-anchored the behavior you'd expect?
*(ref: A13)*
Yes. If we set the new body we are attached to, we are not anchored anymore.

**Q8. If you *are* anchored to a body and go out to galactic distances anyway, what should be on screen?**
The galaxy/universe view (as the old altitude-based mode switch would have given you), or the solar-system view seen from very far, with your anchor kept?
*(ref: A14)*
The solar-system view seen from very far, with my anchor kept (in case we go back afterward).

**Q9. Where should the hand-off between "I'm at this planet" and "I'm in this system" happen, in terms you'd notice?**
There are two thresholds: (a) how far from a body you have to be before it stops being your reference, and (b) how small a whole system has to look on screen before it collapses from "individual bodies" to "one dot" (currently: about 16 pixels across). Do those transitions feel right where they are, too early, too late? Is a visible pop at that point acceptable, or should it cross-fade?
*(ref: A15)*
It would be better to cross-fade, if possible without costing too much.

**Q10. Those transition altitudes are computed once, when the app starts, from the date loaded at that moment.** If you then jump the date by months while flying (or run a very long session), they can be off by roughly 10% because the bodies have moved. Is that something you'd ever hit — do you jump dates significantly *while* navigating, as opposed to setting the date and then flying?
*(ref: A19)*
Yes, I sometimes do jump dates while navigating.

**Q11. When another star system is far away but drawn resolved (you can see its individual bodies as more than a dot), should you be able to select, point at, and track bodies inside it?**
Or is selection only ever meaningful in the system you're currently in? Do you run shows with more than one system loaded at once?
*(ref: A17)*
If a body can be seen, we must be able to select it. But the selection is always the brightest/biggest/main object if there are many objects too close to the selection.

**Q12. Have you ever created a second, throwaway body just to be able to get *inside* the body you actually cared about?**
Background — this one is already decided, we're checking the fix covers the real cases: altitude is measured from the body's **surface**, and that stays the default. But sometimes you want it measured from the **center** instead, so you can descend past the surface and be inside the object. Until now the only way was the patch of declaring two bodies — the one you want, plus another to anchor on — so a per-body flag is being added to say "measure this one from the center" directly.
The replacement being built is two per-body radii, both defaulting to the body's own radius so nothing changes unless you set them: a **datum radius** (where altitude zero sits — set it to zero and altitude is measured from the center, so you can descend inside) and a **ground radius** (how close free flight can get before it stops). They're independent, so you can measure altitude from a gas giant's cloud deck while still descending below it, or keep altitudes exactly as they are today while stopping free flight a few km up so it can't be driven into a mountain.
What we'd like to know: **do you have scripts doing that two-body trick?** If so, roughly what for (a shell or zone you fly into, a transparent/gaseous body, a cutaway view…), and would these two radii reproduce what those scripts achieve — or were the two bodies buying you something more (a different size, a different anchor point, different visibility)?
And on the ground radius specifically: is a stop-and-hold at that height what you'd want in free flight, or would you rather it slow you asymptotically without ever quite arriving?
*(ref: A23 — resolved; this is a completeness check on the replacement)*
Yes, but for a script command, it would be better "radius ground" and "radius datum".

## Display and visibility

**Q13. When you hide a body from view, should it keep moving/updating in the background (so it's exactly where it should be if you show it again later), or freeze in place until you show it again?**
The case that decides this: hide a body early in a show, run twenty minutes of simulated time, show it again — should it be where it should be now, or where it was when you hid it?
*(ref: A10)*
It should be where it is now.

**Q14. When you turn trails off for a body, should the software keep silently recording its path in the background — so the full trail is instantly there if you turn trails back on — or should it stop recording, so a re-enabled trail starts fresh?**
*(ref: A1)*
Start fresh when enabled again to avoid performance issues.

**Q15. How would you expect to set colors — for orbits, trails, body names, and the planet grid?** One setting at a time per object, one setting that applies to everything at once, or something else?
Follow-ups that decide the design: should a "set them all" command override colors you've already set on individual bodies, or leave those alone? And should a per-body color you set at runtime survive a body being reloaded?
*(ref: A3)*
A "set them all" command should override colors of the selected kind of bodies we want to change.

**Q16. The latitude/longitude grid drawn on a planet: what should it be?**
Three separate points, answer any of them:
(a) The old version also drew tropics and polar circles; today's is meridians and parallels only. Do you want tropics/polar circles back? (They're the pedagogically interesting lines — they show the obliquity directly.)
(b) The old version only appeared once you were more than ~10 km above the surface, and its individual lines were switched on by the corresponding *sky* line flags (turn on the sky's tropic line, get the planet's tropic circles). Was that deliberate in how you use it, or just how it happened to be wired?
(c) There is still no independent on/off for it — it rides the "planet axis" flag. Should it get its own toggle?
*(ref: A4)*
Tropics and polar circles onto the body have to be drawn if the flags related to them are activated.

**Q17. Eclipse and body shadows are a single on/off (a config key plus a command toggle); off means no eclipse shading at all.** Is one global toggle enough, or do you need finer control (e.g. eclipse shadows on but ring shadows off)? And should it default to on?
*(ref: A28)*
Unclear question. Is eclipse shadows the shadows casted on a body ? And ring shadows is the size of the shadow of Earth at Moon distance ? It must be separated and default value for ring is off but eclipse must be on.

**Q18. Should each body be able to declare its own landscape in its data?**
Today, which landscape you get when you land is decided by built-in rules; there's no way to attach a landscape to a body in a data file. Is that a gap you hit?
*(ref: A27(2))*
By default, it changes to a specific landscape when we get there. But if the user changes it afterward, it'll switch to the wanted one.

**Q19. Standing on Mars, "the moon" in the sky for lighting/decor purposes used to be Earth's Moon; it is now the moons of the body you're actually on.** This is intentional — but if any of your shows depended on the old behavior, say so now.
*(ref: A27(4))*
Great. Old behavior has no way to be kept.

## Physical accuracy vs. legacy behavior

**Q20. Should a comet's coma and tail size be computed from its physical properties (size, reflectivity), or from the separately-entered `apparent_magnitude` + `slope` parameters as before?**
Context: those two parameters are what drive the coma diameter and tail length (via the classic comet-tail formula), not just the brightness. No comet in the shipped data uses them — comets with tails come from scripts. So the practical question is: **do your scripts load comets with `apparent_magnitude`/`slope`?** If yes, that route stays. If no, the physical route is more consistent across all comets, but tails won't necessarily match sizes you remember.
*(ref: A5)*
apparent_magnitude and slope can be forced in some cases to fit the appearance. 

**Q21. Flying very close to the Sun currently renders it as a shaded, planet-like disc instead of a glow. Should the Sun's surface glow when viewed up close?**
And if so, how far does it need to go: uniformly glowing disc, or actual surface appearance (limb darkening, granulation, spots)?
*(ref: A8)*
Actual surface appearance. But it could be glowing around like an atmosphere around a planet (with red glow, if possible not uniform to match the prominences).

**Q22. Stars other than the Sun currently render as points of light / halos only. A detailed close-up view (a real surface, like planets have) is designed but not connected to anything yet. Do you want to be able to fly up to a star and see a surface, or is a point of light always enough?**
Related, same decision: do you ever show a system with more than one star, where each star would need its own halo color/texture?
*(ref: A7)*
We would like the possibility to select a star and get there by switching to the "galaxy" mode and then to the "stellar_system" mode (if existing or created from scratch like it is if there is no specific one).

**Q23. About 28 moons in the shipped data carry the same copy-pasted axial-orientation values (`rot_obliquity = 15.5`, `rot_equator_ascending_node = 213.7`) — clearly placeholders, not measurements.**
The consequence is that those moons' poles point the wrong way and their surface features face the wrong direction — by up to ~155° for the Uranian moons, not a small error. The list is mostly small/irregular moons (Amalthea, Himalia, Elara, Carme, Pasiphae, Puck, Nereid, Proteus, the Uranian and Neptunian irregulars…) — but **Iapetus is in it**, and Iapetus is the one whose two-tone surface an audience actually looks at.
Should this data be corrected moon by moon (real work, real precision), or is a placeholder orientation acceptable for bodies rendered a few pixels across? Are there specific ones you show close up?
*(ref: A12)*
This data must be corrected moon by moon. Especially Iapetus which is important to look at.

**Q24. Some behavior is still keyed to body *names*: a body called "Earth", "Moon" or "Sun" gets special-cased handling.** Do you ever run shows where that would misfire — renamed bodies, a fictional or exoplanet system, a second star, a system whose central body isn't named "Sun"? If yes, which cases specifically?
*(ref: A29)*
Some bodies can be linked to Earth or Moon or Sun behavior to do some comparisons.

## Image quality vs. hardware

**Q25. When should a body switch to its high-resolution texture?**
Old rule: when the disc is wider than 180 pixels. New rule: when it covers more than 20% of the viewport width — which on a large dome is a much bigger disc, so mid-sized planets look less sharp than they used to (roughly in the 180–400 px range). The trade is crispness against VRAM and loading hitches, and it depends on your projector resolution and machine. What's the right side of that trade for your setup, and have you noticed the difference?
*(ref: A26)*
20% is fine and a better choice.

## Data authoring and workflow

**Q26. Do you ever need to give a body a feature the engine wouldn't infer from its data?**
Today, what a body draws (rings, trail, tail, grid, axis…) is deduced from the keys present in its entry. There is a way to declare a feature explicitly instead, but only one case is wired up so far, and the general syntax isn't fixed. Do you hit cases where you'd want to force a feature on (or off) explicitly — and would you rather write that in the body's data entry or issue it as a runtime command?
*(ref: A2)*
No. If it's not in the data, it will never be activated ever.

**Q27. There's a "reload the whole star system" capability built into the engine, but no command/button wired up to trigger it yet (useful e.g. after editing data files, without restarting the app). Would you want this exposed as a command?**
If yes: what should it do to your current state — keep the camera where it is and keep the date, or reset to the start-up state?
*(ref: A20)*
Yes, create a command and keep to the current state.

---

## What we deliberately did not ask you

So the omissions are visible and you can object if we misjudged: the
remaining open items are internal-architecture choices with no user-visible
surface — how draw batches are grouped internally (`A6`), whether an unused
type label stays in the code (`A16`, `A22`), where implementation code lives
relative to headers (`A21`), and a handful of coordinate/threading items.
If you think one of those actually *does* have a user-visible surface, say
so — that's exactly the kind of misjudgment this section exists to catch.

## Anything else

This document only asks about decisions already open. If you want to
*request* something, that's a different channel: `FEATURE_REQUESTS.md` at
the repository root, format documented in the file. Requests there aren't
promised, but they aren't lost either — each gets triaged into the tracker
or explicitly declined with a reason.

---

# Round 2 — moved to its own file (2026-07-21)

Round 2 now lives in **`USER_QUESTIONS_ROUND2.md`**, next to this file, so
the live questions aren't buried under this file's answered history. It
starts by answering the two questions you asked back, asks only what is
still open (R1, R3–R11), and marks what got resolved in the meantime as
no-longer-questions (R2 — including one behavior you can still veto — and
R12). Nothing was dropped in the move; the `(ref: …)` keys are unchanged.

---

*Traceability note: this document is derived, not authoritative — the live
decision ledger is `INTENT.md` §13 (same folder). If that file
and this one ever disagree (e.g. an item gets resolved in one but not
updated here), the ledger wins; tell us and we'll reconcile.*
