#!/usr/bin/env python3
"""f58_gaptable.py — the §11.169 gap table over the F58 universe.

Input : artifacts/f58/f58_sites_raw.tsv (produced by f58_census.py; the universe's
        boundary and enumeration method are stated in artifacts/f58/f58_universe.md)
Output: artifacts/f58/f58_gap_table.tsv  (one row per site)
        artifacts/f58/f58_gap_summary.md (sites by schema + element coverage)

Every verdict below is a READING of the source at `master-beta @ d6aec251`, made by the
F58 executor.  The script does the bookkeeping, not the judging: its job is to render
the verdicts and to FAIL LOUDLY if the verdict set does not cover the census exactly.
That assertion is this classification's discriminating check — a site added, removed or
mistyped makes the script exit non-zero rather than silently produce a shorter table.

---------------------------------------------------------------------------
ELEMENT TESTS (§11.169(b), made operational so each verdict is checkable)
---------------------------------------------------------------------------
Scored on the EMISSION EVENT, not the source line: §11.169(b) says "one log entry
carrying all elements satisfies both", so where one refusal produces a fixed pair of
lines from one chokepoint (`Could not execute: <commandline>` at
app_command_interface.cpp:1173 + the message at :1174) the pair is one record.

ERROR schema
  WHAT  Y the record identifies the specific offending element - an embedded runtime
          value (file name, body name, key, index) or a named argument/parameter word
        P names only the command/operation and a generic fault class
        N names neither, or MISIDENTIFIES which element is at fault
  CONS  Y states an effect beyond the refusal itself: what stays unchanged, what was
          not added/read/written, what the run does next
        P the effect is implicit in the wording but never stated
        N absent
  PREV  Y states an action the reader can take so it does not happen again
        P states a constraint, a single guess, or names the missing element without
          its form
        N absent

ACTING-DEFAULT schema (D12)
  CAUSE Y states what made the implicit decision necessary
  CONT  Y states what was decided - the value or behaviour actually adopted
        N absent, OR the stated content is WRONG (a record that misstates its own
          decision is worse than silence: it is a false belief with a citation)
  OVR   Y states how to make the decision explicit

SELF-CONTAINMENT (binds the action element of both schemas; scored only when the
action element is Y or P)
  SC    Y valid values enumerated, or their SHAPE stated when not enumerable
        P a partial set, or a single suggestion
        N neither

CLASSES
  ERR    a user-facing error record
  ACT    an acting-default record (D12)
  BOTH   one record wearing both faces (§11.169(b)'s double face)
  SILENT a site where a schema applies and NOTHING is emitted
  DELIB  known-deliberate, cost-forced silence - §5.115's bulk-script class, carried
         as its own category and never counted as a naive gap
  INFO   progress/state reporting; neither schema applies
  TRACE  internal debug trace; outside §11.169(b)'s user-facing floor by nature
  NOISE  a record emitted where nothing went wrong and nothing was decided
  CHAN   a channel mechanism site: it emits ANOTHER site's record and owns no content
"""
import collections
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ART = os.path.join(HERE, "artifacts", "f58")
RAW = os.path.join(ART, "f58_sites_raw.tsv")

# ===========================================================================
# 1. The arm-A refusal channel: 157 `debug_message` sites in
#    app_command_interface.cpp, grouped by verdict signature.
#    Every group's signature and rationale is stated; the union is asserted equal
#    to the census's debug_message line set.
# ===========================================================================
# signature: (class, WHAT, CONS, PREV, CAUSE, CONT, OVR, SC, rationale)
AG = ("ERR", "P", "N", "N", "-", "-", "-", "-",
      "names the command and a generic fault class ('unknown argument', 'malformed "
      "command'); the offending token is visible only through the companion "
      "'Could not execute: <commandline>' line, and no consequence or action is stated")
AY = ("ERR", "Y", "N", "P", "-", "-", "-", "P",
      "names the missing/short element, which is a partial action (add THIS argument) "
      "but never its form or valid values; no consequence stated")
AF = ("ERR", "Y", "N", "N", "-", "-", "-", "-",
      "names the offending value (file, image, HP number, object) and nothing else: "
      "no consequence, no action, no valid-value shape")
AE = ("ERR", "P", "N", "N", "-", "-", "-", "-",
      "reports that an operation failed without naming a reason, a consequence or an "
      "action ('error <op>', 'Error loading X.')")
AI = ("ERR", "N", "N", "N", "-", "-", "-", "-",
      "an internal-assertion message with no user-facing content ('unexpected error "
      "in command_X', 'system error'): the reader learns nothing at all")
AS = ("ERR", "Y", "N", "P", "-", "-", "-", "P",
      "the vocabulary paths: the record is completed by searchNeighbour's "
      "'<token> is unknown. Did you mean <x> ?' [app_command_init.cpp:368-387], which "
      "names the token and offers ONE Levenshtein guess - a guess is not an "
      "enumeration and the distance is unbounded, so a wildly wrong token still gets "
      "a confident suggestion")
AB = ("ERR", "N", "N", "P", "-", "-", "-", "P",
      "MISIDENTIFIES the fault: an unknown flag NAME is reported as a bad flag "
      "ARGUMENT, because the assignment that would have named it is commented out at "
      "app_command_interface.cpp:311 - and the emit left behind at :312 writes the "
      "then-empty debug_message, i.e. a blank line, before the caller's wrong message")
AD = ("ERR", "Y", "Y", "Y", "-", "-", "-", "Y",
      "GOLD: names the offending value, states what stays unchanged, gives an action, "
      "and states the valid domain or its shape - all four elements in one record")
AP = ("ERR", "P", "N", "Y", "-", "-", "-", "P",
      "states the command's correct form as the action, without the argument's own "
      "valid values or shape")
AV = ("ERR", "Y", "N", "Y", "-", "-", "-", "Y",
      "enumerates the valid values inside the message ('expected save or load')")
AL = ("ERR", "Y", "N", "P", "-", "-", "-", "P",
      "states the PRECONDITION the user violated (must be on a body) as the action; "
      "written in French inside an otherwise-English channel and wrapped in _() over a "
      "French literal, so the translation table is keyed on French here and on English "
      "everywhere else")
AC = ("ERR", "P", "N", "N", "-", "-", "-", "-",
      "reports a parse failure without stating the accepted format - the exact case "
      "§11.169's self-containment criterion names (a value space too large to "
      "enumerate still owes its grammar)")

GROUPS = [
    (AG, [1195, 1230, 1233, 1318, 1321, 1403, 1424, 1431, 1471, 1505, 1509, 1527,
          1549, 1552, 1573, 1580, 1607, 1625, 1876, 1882, 1965, 1972, 2114, 2239,
          2286, 2325, 2333, 2335, 2347, 2349, 2392, 2464, 2563, 2611, 2619, 2621,
          2636, 2782, 2791, 2845, 2858, 2925, 2929, 3200, 3220, 3264, 3298, 3392,
          3774, 3792, 3794, 3914, 3941, 4041, 4059, 4071, 4136, 4145, 4160, 4193,
          4263, 4327, 4451, 4723, 4745, 4012, 4026]),
    (AY, [1344, 1356, 2259, 2284, 2297, 2310, 2358, 2370, 2405, 2586, 2607, 2652,
          2672, 2762, 2793, 2868, 2943, 3413, 3513, 4166, 4182, 4200, 4212, 4236,
          4252, 4272, 4283, 4306, 4350, 4367, 4381, 4394, 4412]),
    (AF, [1958, 2267, 2306, 2417, 2423, 2429, 2435, 2441, 2447, 2814, 2916, 2920,
          2956, 2980, 2986, 3042, 3134, 3241, 3528, 4177]),
    (AE, [1455, 1545, 4226, 4244, 4259, 4297, 4320, 4342, 4357, 4420, 4428, 4436,
          4447]),
    (AI, [2452, 4490, 4503, 4516, 4530, 4543, 4556, 4569, 4582, 4595]),
    (AS, [233, 2222]),
    (AB, [327, 1186]),
    (AD, [4110, 4115, 4125, 4130]),
    (AP, [3248]),
    (AV, [1323]),
    (AL, [3848, 3879]),
    (AC, [3825, 3837]),
]

# per-site notes on top of the group rationale
DM_NOTE = {
    233: "the only refusal path that does NOT reach executeCommandStatus (returns 0 at "
         ":236), so the 'Could not execute' companion line is absent here",
    327: "companion blank line at :312; see AB",
    1186: "companion blank line at :312; see AB",
    2222: "searchSimilarSet is passed args.begin()->first, not the offending setName - "
          "with several pairs on one `set` line the suggestion is about the wrong token",
    2814: "the message is written TWICE: explicitly at :2815 and again by "
          "executeCommandStatus at :1174",
    4177: "the message is written TWICE: explicitly at :4178 and again by "
          "executeCommandStatus at :1174",
    4012: "a fuller record for the same failure exists at ssystem_factory.cpp:953 "
          "(WHAT+CONS+PREV+SC) - but it goes to LOG_FILE::INTERNAL at L_ERROR while "
          "this one goes to LOG_FILE::SCRIPT at L_DEBUG: the script author reads the "
          "poorer half",
    4026: "same split as :4012 - the full record is at ssystem_factory.cpp:997/:1005/"
          ":1021 on the INTERNAL channel",
    3042: "the valid HP range is a bounded integer domain (guarded by num > NR_OF_HIP "
          "at illuminate_mgr.cpp:126) and is not stated",
}

# ===========================================================================
# 2. Every other live site, one entry each.
#    key "<file>:<line>" -> (class, WHAT, CONS, PREV, CAUSE, CONT, OVR, SC, note)
# ===========================================================================
def E(cls, what="-", cons="-", prev="-", cause="-", cont="-", ovr="-", sc="-", note=""):
    return (cls, what, cons, prev, cause, cont, ovr, sc, note)


ACI = "src/interfaceModule/app_command_interface.cpp"
ACE = "src/interfaceModule/app_command_eval.cpp"
ACN = "src/interfaceModule/app_command_init.cpp"
SCR = "src/scriptModule/script.cpp"
SMG = "src/scriptModule/script_mgr.cpp"
IO = "src/tools/io.cpp"
APP = "src/appModule/app.cpp"
MKF = "src/appModule/mkfifo.cpp"
SOL = "src/bodyModule/solarsystem.cpp"
SSF = "src/bodyModule/ssystem_factory.cpp"
PRO = "src/bodyModule/protosystem.cpp"
CKC = "src/mainModule/checkConfig.cpp"
INI = "src/tools/init_parser.cpp"
AST = "src/tools/app_settings.cpp"
HSM = "src/starModule/hip_star_mgr.cpp"
ZAR = "src/starModule/zone_array.cpp"
NEB = "src/coreModule/nebula_mgr.cpp"
CST = "src/coreModule/constellation_mgr.cpp"
ILL = "src/coreModule/illuminate_mgr.cpp"
LOG = "src/tools/log.cpp"
MAIN = "src/main.cpp"

V = {}


def put(f, lines, entry):
    for ln in (lines if isinstance(lines, (list, tuple)) else [lines]):
        V["%s:%d" % (f, ln)] = entry


# --- arm A: app_command_interface.cpp, non-refusal sites -------------------
put(ACI, [170, 172], E("TRACE", note="argument-hash dump, L_DEBUG"))
put(ACI, 210, E("INFO", note="the per-command echo; §5.115's named amplifier - every "
                             "executed command is echoed here, twice with app.cpp:770 "
                             "on the TCP path"))
put(ACI, 226, E("INFO", note="a command skipped by an if-block: expected behaviour, "
                             "neither schema applies"))
put(ACI, [234, 312, 1173, 1174, 2815, 4178],
    E("CHAN", note="emits another site's record; owns no content of its own. :312 is "
                   "the one that writes a variable known to be empty"))
put(ACI, 676, E("ERR", "Y", "P", "Y", "-", "-", "-", "Y",
                "the flag surface's best record: names the fault, implies the "
                "consequence ('cannot be toggled'), gives the action and enumerates "
                "the valid values ('pass on or off') - but it is written to "
                "LOG_FILE::INTERNAL at L_WARNING while the script author reads "
                "LOG_FILE::SCRIPT"))
put(ACI, 1148, E("ERR", "N", "P", "N", "-", "-", "-", "-",
                 "'no effect with unknown case ' - the trailing space is where the "
                 "case was meant to go; the record names neither the flag nor the "
                 "value, and lands on INTERNAL at L_DEBUG"))
put(ACI, 1205, E("ERR", "P", "Y", "N", "-", "-", "-", "-",
                 "'No tcp : i can't send ' - the `get` command's answer is dropped; "
                 "the consequence is stated, the requested value is not named and no "
                 "action is given"))
put(ACI, [1979, 1980], E("INFO", note="the `print` command's own user output, written "
                                      "TWICE (SCRIPT + INTERNAL) at L_WARNING - a "
                                      "deliberate user message carrying the severity "
                                      "of a fault"))
put(ACI, 2559, E("INFO", note="heading from/to"))
put(ACI, [3550, 3566, 3569, 3576, 3581, 3593, 3646, 3649, 3656, 3661, 3673],
    E("BOTH", "Y", "P", "N", "Y", "P", "N", "-",
      "the media locale chain: each miss is both an error (the requested localised "
      "asset is absent) and an acting default (the chain silently falls through to "
      "the next candidate). The candidate tried is named, the FINAL outcome of the "
      "chain never is, and no action is offered"))
put(ACI, [3564, 3584, 3588, 3596, 3603, 3644, 3664, 3668, 3676],
    E("INFO", note="'succesfull' [sic] - the media chain's positive legs"))

# --- arm A: app_command_eval.cpp ------------------------------------------
put(ACE, 139, E("ERR", "Y", "P", "N", "-", "-", "-", "-",
                "'<name> is a reservedVar so you can't define it' goes to std::cout "
                "ONLY - never to any log file, so a script author has no record of it"))
put(ACE, 213, E("ACT", "-", "-", "-", "Y", "N", "N", "N",
                "'...so define to null from ops' - the CONTENT IS WRONG: the call that "
                "follows is define(mArg, mValue) [ :214 ], which sets the variable to "
                "mValue when mValue parses as a number [ :146-152 ] and only to 0 when "
                "it does not. The record states a decision the code did not take"))
put(ACE, 259, E("INFO", note="'Not variable available' [sic]"))
put(ACE, 264, E("INFO", note="variable dump"))
put(ACE, 342, E("ACT", "-", "-", "-", "Y", "Y", "N", "N",
                "'Unknown reserved variable <v>. Default 0.0 is returned.' - cause and "
                "content present, override absent, and the valid values ARE enumerable: "
                "m_reservedVar holds 24 names [ :37-64 ] and not one is printed"))
put(ACE, 368, E("ACT", "-", "-", "-", "Y", "Y", "N", "N",
                "'No setter with reserved variable <v>. Do nothing.' - the read-only "
                "reserved variables are enumerable and not enumerated"))
put(ACE, 372, E("ACT", "-", "-", "-", "Y", "Y", "N", "N",
                "same as :342 on the write side"))

# --- arm A: app_command_init.cpp ------------------------------------------
put(ACN, 29, E("ERR", "Y", "N", "N", "-", "-", "-", "-",
               "'<token> is no longer used in software' names the dead token but not "
               "its replacement, and it RETURNS EARLY [ :374-375 ] so the obsolete "
               "token is the one case that gets no suggestion at all"))
put(ACN, 385, E("ERR", "Y", "N", "P", "-", "-", "-", "P",
                "'<token> is unknown. Did you mean <x> ?' - one nearest-neighbour "
                "guess with NO distance threshold (minDistance starts at 99999 and is "
                "never bounded [ :371-383 ]), so an arbitrary token still receives a "
                "confident suggestion. A guess is not an enumeration"))

# --- arm A: script.cpp / script_mgr.cpp -----------------------------------
put(SCR, [49, 70, 72, 75], E("TRACE", note="token/print dump"))
put(SCR, 104, E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                "'Unable to open script: <file>' - no reason (missing? unreadable?), "
                "no consequence, no action"))
put(SCR, 174, E("INFO"))
put(SCR, 183, E("ERR", "P", "N", "N", "-", "-", "-", "-",
                "'End of script detected but not at end of the execution stack' - an "
                "internal-state complaint in the user's channel"))
put(SMG, [65, 66], E("INFO", note="the same line written twice, to two sinks"))
put(SMG, [111, 134, 145, 203, 233, 263], E("INFO"))
put(SMG, 200, E("ERR", "Y", "P", "N", "-", "-", "-", "-", "'Already recording script'"))
put(SMG, 235, E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                "'Error opening script file for writing: <file>' - no reason, no action"))
put(SMG, 254, E("TRACE"))
put(SMG, 278, E("ERR", "Y", "N", "P", "-", "-", "-", "N",
                "'one script is leaking a lock !' names the cause but gives the reader "
                "no way to find WHICH script or lock"))
put(SMG, 360, E("ERR", "P", "N", "N", "-", "-", "-", "-",
                "'Unable to read script directory' - the directory is not named, and "
                "it is configurable"))

# --- arm A: io.cpp --------------------------------------------------------
put(IO, 370, E("ACT", "-", "-", "-", "P", "N", "N", "N",
               "'ServerSocket data setOutput too big' - the next statement is "
               "data.resize(MAX_BUFFER) [ :372 ]: the answer is SILENTLY TRUNCATED and "
               "the record never says so, never gives the size or the limit, and the "
               "truncated answer still goes out to the client as if whole"))
put(IO, 722, E("ERR", "Y", "Y", "Y", "-", "-", "-", "Y",
               "GOLD: names what could not be delivered, why nobody received it, and "
               "the exact action ($LOGON on the feedback channel)"))
put(IO, [560, 563, 566, 571, 575, 583],
    E("TRACE", note="ungated printf on stdout for EVERY HTTP request (marked //Debug "
                    "in source but compiled in): the request line, url, file, path, "
                    "parameters and command"))

# --- arm A: app.cpp / mkfifo.cpp ------------------------------------------
put(APP, [124, 128, 265, 475, 513, 654, 731, 741, 745, 747, 748, 927, 935], E("INFO"))
put(APP, 210, E("INFO", note="an advisory about an EXPLICIT config choice: no error "
                             "occurred and nothing was decided implicitly, so neither "
                             "schema applies - listed so the boundary is visible"))
put(APP, 305, E("ACT", "-", "-", "-", "Y", "N", "P", "Y",
                "'shadow_resolution MUSTN'T exceed <max>' - the clamp at :304 sets "
                "shadowRes = SHADOW_MAX_SIZE and the record NEVER SAYS WHAT IT USED. "
                "The sibling three lines below does say it"))
put(APP, 308, E("ACT", "-", "-", "-", "Y", "Y", "P", "Y",
                "'...MUST be a multiple of <n> - fallback to shadow_resolution of <v>' "
                "- cause, content and the value's shape all present; the override is "
                "implied (set a conforming value) but the key's file/section is not "
                "named"))
put(APP, [761, 770], E("INFO", note="the TCP/mkfifo intake echo. App::updateFrom"
                                    "SharedData DISCARDS executeCommand's return "
                                    "[ :762, :771 ], so no refusal ever travels back "
                                    "to the client that sent the command"))
put(MKF, [70, 76, 83, 106, 128], E("INFO"))
put(MKF, [80, 87, 95], E("ERR", "P", "N", "N", "-", "-", "-", "-",
                         "reports a bare numeric errno instead of strerror(errno): the "
                         "reader must look up an integer to learn what failed - a "
                         "self-containment failure of the most mechanical kind"))
put(MKF, 113, E("ERR", "Y", "N", "N", "-", "-", "-", "-", "platform limitation"))

# --- arm B: ssystem ------------------------------------------------------
put(SOL, 96, E("INFO"))
put(SOL, 99, E("ERR", "Y", "N", "N", "-", "-", "-", "-",
               "'<body> body could not be added to Earth orbit.' - no reason, no "
               "consequence, no action"))
put(SOL, 102, E("ERR", "Y", "Y", "N", "-", "-", "-", "-",
                "'...position may be inacurate' [sic] - one of the few loader records "
                "that states a consequence"))
put(SSF, 69, E("ERR", "P", "Y", "N", "-", "-", "-", "-",
               "'no default objMgr loaded, system aborded' [sic]"))
put(SSF, 276, E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                "'Warning: No name found for planet <x> in <file>' - the 'Warning:' "
                "prefix duplicates the level tag the log already writes"))
put(SSF, 371, E("ACT", "-", "-", "-", "Y", "Y", "Y", "Y",
                "GOLD: which file won, over what, that the loser was NOT read, and how "
                "to fall back"))
put(SSF, 422, E("ERR", "Y", "Y", "N", "-", "-", "-", "-",
                "names the errc message and what was not generated; no action"))
put(SSF, 520, E("ACT", "-", "-", "-", "Y", "Y", "Y", "Y",
                "GOLD: the config key that was ignored, why, the value actually in "
                "force, and BOTH ways to take control back"))
put(SSF, 551, E("BOTH", "Y", "Y", "N", "Y", "Y", "N", "-",
                "states cause, content and consequence (the old cloud is unaffected); "
                "no action - the reader is not told how to get a Solar node"))
put(SSF, 598, E("INFO"))
put(SSF, 645, E("ERR", "Y", "Y", "Y", "-", "-", "-", "Y",
                "GOLD: the offending line, that it is ignored, the required form, and "
                "the comment escape"))
put(SSF, [696, 698], E("TRACE", note="parameter dump on stdout"))
put(SSF, 708, E("ERR", "Y", "Y", "Y", "-", "-", "-", "Y",
                "GOLD: which section, why, that the rest of the file still loads, and "
                "the required keys WITH their units"))
put(SSF, 953, E("ERR", "Y", "P", "Y", "-", "-", "-", "Y", "GOLD (INTERNAL channel)"))
put(SSF, 967, E("BOTH", "Y", "Y", "Y", "Y", "Y", "Y", "Y",
                "the audit's single best record: it wears both faces at once - the "
                "error (a body named in the observer state is absent from the reloaded "
                "data), the decision taken instead (which body is referenced now), and "
                "two explicit ways to override"))
put(SSF, 984, E("INFO"))
put(SSF, [997, 1005], E("ERR", "Y", "Y", "Y", "-", "-", "-", "Y",
                        "GOLD: names the value, why it is refused, that NOTHING was "
                        "written, and a worked example of the accepted form"))
put(SSF, 1021, E("ERR", "Y", "Y", "N", "-", "-", "-", "-",
                 "names the errc message and that the system was NOT saved; no action"))
put(SSF, 1084, E("ERR", "N", "N", "N", "-", "-", "-", "-",
                 "THE BANKED INSTANCE, §11.170(f): 'New path has no body '' to "
                 "re-reference the camera on'. The user asked for a home planet and is "
                 "told about a camera; the empty name is printed as two quotes; the "
                 "1 AU displacement and the fictitious anchor registered under the "
                 "empty name are stated nowhere. Fails all three error elements"))
put(PRO, 151, E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                "'Unable to open file <planetfile>' - no reason, no action, and see :153"))
put(PRO, 153, E("INFO", note="'(solar system loaded)' is emitted UNCONDITIONALLY, "
                             "including on the branch where :151 has just reported the "
                             "file could not be opened [ :150-153 ]: the log says the "
                             "load failed and then says it loaded"))
put(PRO, [170, 184, 209, 214],
    E("ERR", "Y", "N", "N", "-", "-", "-", "-",
      "a failed removal reported at L_INFO - an error carrying the severity of "
      "progress, in the channel a script author reads for faults"))
put(PRO, 473, E("ERR", "P", "N", "N", "-", "-", "-", "-",
                "'Unknown planet_name in bodyTraceBodyChange' - the offending name is "
                "in hand and is not printed"))
put(PRO, 517, E("INFO"))
put(PRO, 523, E("ERR", "Y", "Y", "N", "-", "-", "-", "-", "'can not add body with no name'"))
put(PRO, 530, E("ACT", "-", "-", "-", "Y", "Y", "Y", "P",
                "'No parent specified for <b>, assume parent is the center body "
                "(Specify 'none' for no parent)' - all three elements present; the "
                "override is partial because it teaches the 'none' spelling only, not "
                "how to name a real parent"))
put(PRO, 534, E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                "'can't find parent for <b>' - what happens to the body is not stated"))
put(PRO, 542, E("ACT", "-", "-", "-", "Y", "Y", "N", "N",
                "'No valid body type specified for <b>, assume 'Asteroid'' - the "
                "textbook §11.169 gap: cause and content present, override absent, and "
                "the valid body types are a small closed enum that is never printed"))
put(PRO, 547, E("ERR", "Y", "Y", "N", "-", "-", "-", "-",
                "name collision: says what it could not do, not what the reader does"))
put(PRO, 565, E("INFO"))
put(PRO, [569, 582], E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                       "'ERROR : can't find position function <f> for <b>' - the "
                       "'ERROR : ' prefix duplicates the level tag and the trailing "
                       "\\n double-spaces the log; the body is silently skipped "
                       "(return at :571/:584) and the record does not say so"))
put(PRO, 615, E("TRACE", note="stdout duplicate of :616"))
put(PRO, 616, E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                "'Error when creating orbit from <b>' - duplicated to stdout at :615"))
put(PRO, 637, E("ERR", "Y", "Y", "Y", "-", "-", "-", "Y",
                "GOLD, and the audit's reference specimen for a loader error: the "
                "offending key AND its value, that the body is NOT added, the full "
                "enumeration of valid values, and two alternative actions"))
put(PRO, 894, E("ERR", "N", "N", "N", "-", "-", "-", "-", "'Undefined body' - names nothing"))
put(PRO, 897, E("ERR", "N", "N", "N", "-", "-", "-", "-",
                "'Failed to create body' - names neither the body nor the reason"))

# --- arm B: config -------------------------------------------------------
put(CKC, 486, E("INFO"))
put(CKC, [568, 606], E("ACT", "-", "-", "-", "N", "Y", "N", "N",
                       "'key <k> has been removed from config.ini' - this record "
                       "reports a MUTATION OF THE USER'S OWN FILE. The content is "
                       "stated; WHY the key was removed is not, and how to keep it is "
                       "not. An acting default that edits the file the user edits"))
put(CKC, 571, E("ACT", "-", "-", "-", "N", "Y", "N", "N",
                "same, at section granularity"))
put(INI, [62, 70, 83], E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                         "'can't find/read/open config file <f>' - no consequence "
                         "(what runs instead?) and no action"))
put(INI, [95, 120, 147, 174],
    E("ACT", "-", "-", "-", "Y", "Y", "N", "N",
      "'can't find the configuration key \"<k>\", default <empty string|0> returned' - "
      "cause and content present, override absent: the file to edit is not named, the "
      "section is not separated from the key, and the value's shape is not stated. "
      "This is the single most-reached acting-default record in the tree"))
put(INI, [106, 133, 160, 187],
    E("NOISE", "-", "-", "-", "N", "N", "N", "-",
      "'Init_parser def_str <section>:<key>' is written at L_WARNING UNCONDITIONALLY "
      "on entry to the 3-argument getters [ :104-108 etc. ], BEFORE and REGARDLESS of "
      "whether the key is found: a warning that reports no fault, states no decision "
      "and fires on the success path. It also pays §5.115's cost per call"))
put(INI, [91, 113, 140, 167], E("TRACE", note="printf gated on the DEBUG macro"))
put(AST, 121, E("INFO"))
put(AST, list(range(252, 277)), E("TRACE", note="AppSettings::display() path dump on stdout"))

# --- arm B: star ---------------------------------------------------------
put(HSM, [79, 90], E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                     "'convertToSpectralType/ComponentIds: bad index: <i>, max: <n>' - "
                     "an internal bound report in the user's error channel"))
put(HSM, 125, E("ACT", "-", "-", "-", "P", "P", "N", "N",
                "'Star color not define' [sic] - fires inside a 128-iteration loop over "
                "stars.ini's color_NNN keys [ :112-127 ] and names NEITHER the index "
                "NOR the file; the fallback (v3fNull, i.e. black) is not stated. A "
                "missing stars.ini produces 128 of these, each composed with "
                "init_parser.cpp:95's own warning for the same key: 256 unactionable "
                "lines from one missing file"))
put(HSM, 134, E("ERR", "N", "N", "N", "-", "-", "-", "-",
                "'ColorTable index Error' - neither the index nor the bound"))
put(HSM, 188, E("ERR", "Y", "N", "N", "-", "-", "-", "-", "allocation failure, console only"))
put(HSM, [448, 458, 510, 522, 571], E("INFO"))
put(HSM, 466, E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                "duplicate catalogue level; console only"))
put(HSM, [487, 498], E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                       "a REQUIRED CONFIG KEY is missing and the record goes to "
                       "std::cerr only - it never reaches any log file"))
put(HSM, [492, 503], E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                       "exception text forwarded to the console only"))
put(HSM, [526, 576], E("BOTH", "Y", "N", "N", "Y", "N", "N", "-",
                       "'Warning <file> not found.' on std::cerr: star names/sci-names "
                       "are silently not loaded and the record never says so"))
put(HSM, [536, 586], E("BOTH", "Y", "N", "N", "Y", "N", "N", "-",
                       "'bad line: \"<line>\"' on std::cerr: the line is quoted (good) "
                       "but what is wrong with it, that it was skipped, and the "
                       "expected format are all absent - and no log file receives it"))
put(HSM, 1255, E("ERR", "Y", "P", "N", "-", "-", "-", "-",
                 "'limit of 127 variable stars reached' - the limit is named, what "
                 "happens to star 128 is not"))
put(HSM, 1354, E("ERR", "Y", "Y", "N", "-", "-", "-", "-",
                 "'...- Feature disabled' - a rare loader record that states its "
                 "consequence"))
put(HSM, [1368, 1392], E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                         "'VariableStar error parsing <record>' - the record is quoted, "
                         "the expected format is not"))
put(ZAR, [161, 210], E("INFO"))
put(ZAR, [102, 107, 120, 127, 134, 147, 154, 166, 173, 179, 186, 192, 199, 204, 212,
          249, 442, 448, 487, 505, 510, 518, 541],
    E("ERR", "P", "N", "N", "-", "-", "-", "-",
      "THE STAR CATALOGUE LOADER REPORTS EVERY FAULT TO A CONSOLE AND NONE TO A LOG. "
      "23 fault records over printf/fprintf/std::cerr ('bad file', 'no star catalogue "
      "file', 'unsupported version, ', 'no memory, ', 'bad file type, ', "
      "'initialization failed', mmap failure) against exactly 2 cLog calls in the whole "
      "file, both L_INFO progress lines. Several are sentence FRAGMENTS meant to "
      "concatenate on one console line, so they are not even self-contained records"))

# --- arm B: nebula / sky-culture ------------------------------------------
put(NEB, 49, E("ERR", "P", "N", "N", "-", "-", "-", "-",
               "'DSO: error while loading pictogram texture' - no file, no reason"))
put(NEB, 138, E("ERR", "Y", "N", "N", "-", "-", "-", "-", "delete target not found"))
put(NEB, 369, E("ACT", "-", "-", "-", "Y", "Y", "N", "-",
                "'replacing user added nebula with name <n>' - the replacement is "
                "stated, how to avoid it is not"))
put(NEB, 373, E("ACT", "-", "-", "-", "Y", "Y", "N", "-", "'dso: hide nebula with name <n>'"))
put(NEB, 395, E("INFO"))
put(NEB, 399, E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                "GARBLED BY A LITERAL SPLICE: the source reads "
                "`\"NGC data file \" + cat + \" not found\"\"Loading NGC data... \"` - "
                "two adjacent string literals concatenate at compile time, so the "
                "emitted line is 'NGC data file <cat> not foundLoading NGC data... ' "
                "and ends by announcing the load it is refusing"))
put(NEB, 423, E("ERR", "P", "P", "N", "-", "-", "-", "-",
                "'Nebula: <i> items loaded, <d> dropped' at L_INFO is THE ONLY REPORT "
                "THAT CATALOGUE RECORDS WERE DROPPED. The drops happen silently at "
                ":412 and :419 (data_drop++ with no record); the aggregate names no "
                "dropped line, no line number and no reason, so a corrupted catalogue "
                "is reported as a number in a progress line"))
put(CST, [162, 184, 644], E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                            "'No constellation Color/label/ArtIntensity with shortName "
                            "<n>' - the valid short names are enumerable (the loaded "
                            "culture's list) and are not offered"))
put(CST, [208, 254, 279, 553, 904],
    E("ERR", "Y", "N", "N", "-", "-", "-", "-",
      "'can't open/not found <file>' - the file is named; no reason, no consequence "
      "(which culture is now empty?), no action"))
put(CST, 235, E("ERR", "P", "N", "N", "-", "-", "-", "-",
                "'loadLinesAndArt on line <n> of <file>' - says WHERE and never WHAT: "
                "the reader is given a coordinate and no fault"))
put(CST, 291, E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                "'Error parsing constellation art record <record>' - the record is "
                "quoted, the expected form is not"))
put(CST, 308, E("ERR", "Y", "N", "N", "-", "-", "-", "-", "art references an unknown constellation"))
put(CST, [908, 956], E("INFO"))

# --- arm C ---------------------------------------------------------------
put(ILL, 62, E("ERR", "P", "N", "N", "-", "-", "-", "-",
               "'Error loading texture illuminateTex' - no path, no reason"))
put(ILL, 288, E("ERR", "Y", "N", "N", "-", "-", "-", "-",
                "'illuminate: error when loading user texture <file>'"))
put(LOG, 95, E("ERR", "Y", "N", "P", "-", "-", "-", "P",
               "'(EE): Couldn't open file log!\\n Please check file/directory "
               "permissions' - the ONLY diagnostic in the tree that cannot use the log, "
               "correctly on the console. It does not name the path it tried, and the "
               "next statement is a bare `throw;` OUTSIDE any catch [ :96 ], which "
               "calls std::terminate: an unwritable log directory aborts the process"))
put(LOG, [207, 209], E("CHAN", note="writeConsole's two sinks"))
put(MAIN, [72, 74, 81, 107, 158, 171], E("INFO"))
put(MAIN, [95, 98], E("ERR", "P", "N", "N", "-", "-", "-", "-", "uname() failure"))
put(MAIN, [115, 116, 124, 133], E("INFO", note="--help / --version console output"))
put(MAIN, [134, 135], E("ERR", "P", "N", "Y", "-", "-", "-", "P",
                        "'<argv0> don't use command line argument(s)' [sic] + 'Try "
                        "`<argv0> --help' for more information.' - the action is "
                        "present and points at a real enumeration"))
put(MAIN, 173, E("ERR", "P", "N", "N", "-", "-", "-", "-",
                 "'Error deleting file.lock' - no path, no errno, no consequence"))

# ===========================================================================
# 3. SILENT sites (S1/S2/S3 of the universe statement).  No grep finds these;
#    each is a READ result and carries its own citation.
# ===========================================================================
SILENT = [
    # (file, line, class, WHAT,CONS,PREV, CAUSE,CONT,OVR, SC, note)
    ("src/interfaceModule/app_command_interface.cpp", 1154, "SILENT",
     "N", "N", "N", "N", "N", "N", "N",
     "convertStrToFlagValues: `if (value==W_TOGGLE) TOGGLE; else if "
     "(Utility::isTrue(value)) ON; else OFF;` [ :1156-1160 ]. Utility::isTrue accepts "
     "exactly true|TRUE|on|ON|1 [utility.hpp:160-171], so EVERY other value - 'yes', "
     "'enabled', 'Off', a typo, a number - becomes OFF. Nothing is emitted and "
     "executeCommandStatus reports SUCCESS. `flag atmosphere yes` turns the atmosphere "
     "OFF and records a success"),
    ("src/interfaceModule/app_command_eval.cpp", 116, "SILENT",
     "N", "N", "N", "N", "N", "N", "N",
     "evalDouble falls through to Utility::strToDouble [ :125 ], whose catch-all "
     "returns default_value = 0 [utility.cpp:399-406]; evalInt casts that [ :130-133 ]; "
     "Utility::strToInt leaves 0 on a failed extraction [utility.cpp:448-456]. A "
     "non-numeric argument therefore becomes 0 in silence, and `set` routes ~40 targets "
     "through it [app_command_interface.cpp:2155-2222]. `set moon_scale big` sets the "
     "Moon's scale to 0 and reports success"),
    ("src/navModule/anchor_manager.hpp", 201, "SILENT",
     "N", "N", "N", "N", "N", "N", "N",
     "SS-6's live half, measured in the field (§5.97c / §11.177(d)): "
     "`bool setRotationMultiplierCondition(float v) noexcept { if (v>1.0) "
     "rotationMultiplierCondition = v; return true; }` [ :201-206 ] - a value <= 1.0 is "
     "DROPPED and the function returns true anyway; the caller "
     "[ssystem_factory.hpp:964-966] discards even that bool. Reached from `set "
     "stall_radius_unit` [app_command_interface.cpp:2217] and from config.ini "
     "[navigation] stall_radius_unit [ssystem_factory.hpp:978]. The engine keeps 5.0 "
     "and says nothing. Outside the universe's file list by one hop and included "
     "because the ledger banks it"),
    ("src/coreModule/illuminate_mgr.cpp", 176, "DELIB",
     "N", "N", "N", "N", "N", "N", "N",
     "THE BANKED ILLUMINATE CLAMP (F53 acceptance): `if (angular_size<1.0) "
     "angular_size=defaultSize;` [ :176-177 ] - a silent acting default with none of "
     "the three elements. Classified DELIB and not a naive gap: §5.115 pre-classifies "
     "the bulk-script sites as known-deliberate and cost-forced, and this is exactly "
     "that path - §11.170(a2) measured ~100k individual illuminate commands in one "
     "authored show, so a per-call log line here is the log-volume defect itself. The "
     "fix this row belongs to is §5.115(2)'s O(1)-per-site dedup-with-counter, not a "
     "cLog call"),
    ("src/coreModule/illuminate_mgr.cpp", 126, "DELIB",
     "N", "N", "N", "N", "N", "N", "N",
     "IlluminateMgr::load's three silent early returns - `num>NR_OF_HIP` [ :127-128 ], "
     "`_size<=0` [ :129-133 ], `_size<=1` [ :135-139 ] - each refuses a script command "
     "with no record at all. Same bulk-volume argument as :176; the `_size<=1` return "
     "is the precondition §11.170(2) had to discover by measurement because nothing "
     "states it"),
    ("src/coreModule/sky_localizer.cpp", 31, "SILENT",
     "N", "N", "N", "-", "-", "-", "N",
     "THE SKY-CULTURE ENUMERATOR HAS NO DIAGNOSTIC AT ALL: zero cLog calls, zero "
     "console writes in the whole file. The directory walk [ :31-45 ] skips any entry "
     "that is not a directory and any directory whose info.ini is missing or unreadable, "
     "in silence; a culture that will not appear in the list produces no record of why. "
     "The one channel it ever had is the commented-out std::cerr at :14"),
    ("src/coreModule/nebula_mgr.cpp", 412, "SILENT",
     "N", "N", "N", "-", "-", "-", "N",
     "the NGC catalogue's per-record drops: `data_drop++` at :412 (extraction failed) "
     "and :419 (loadDeepskyObject refused) emit NOTHING - neither the record, nor the "
     "line number, nor the reason. Only the aggregate at :423 survives, at L_INFO"),
    ("src/appModule/app.cpp", 762, "SILENT",
     "N", "N", "N", "-", "-", "-", "N",
     "App::updateFromSharedData discards executeCommand's return on BOTH transports "
     "[ :762 mkfifo, :771 tcp ], and ScriptMgr::update discards it too "
     "[script_mgr.cpp:311, :330]. So a refused command stops nothing and tells nobody: "
     "the script runs on with the state the failed line was supposed to set, and the "
     "TCP client that sent it receives no answer. This is the CONSEQUENCES element that "
     "157 arm-A records are missing, and it is the same sentence for all of them"),
]

# ===========================================================================
# 4. Render + the completeness assertion
# ===========================================================================
COLS = ["id", "arm", "family", "file", "line", "channel", "severity", "sink", "class",
        "WHAT", "CONS", "PREV", "CAUSE", "CONT", "OVR", "SC", "text", "note"]


def main():
    rows = []
    with open(RAW) as fh:
        header = fh.readline().rstrip("\n").split("\t")
        for line in fh:
            f = line.rstrip("\n").split("\t")
            rows.append(dict(zip(header, f)))
    live = [r for r in rows if r["commented"] == "False"]

    dm_lines = set()
    for sig, lines in GROUPS:
        for ln in lines:
            if ln in dm_lines:
                sys.exit("DUPLICATE debug_message line in GROUPS: %d" % ln)
            dm_lines.add(ln)

    census_dm = set(int(r["line"]) for r in live if r["channel"] == "debug_message")
    if dm_lines != census_dm:
        sys.exit("GROUPS do not cover the census's debug_message sites.\n"
                 "  missing from GROUPS: %s\n  not in census: %s"
                 % (sorted(census_dm - dm_lines), sorted(dm_lines - census_dm)))

    census_other = set("%s:%s" % (r["file"], r["line"])
                       for r in live if r["channel"] != "debug_message")
    if set(V) != census_other:
        sys.exit("V does not cover the census's non-debug_message sites.\n"
                 "  missing from V: %s\n  not in census: %s"
                 % (sorted(census_other - set(V))[:20], sorted(set(V) - census_other)[:20]))

    sigof = {}
    for sig, lines in GROUPS:
        for ln in lines:
            sigof[ln] = sig

    out = []
    n = 0
    for r in live:
        n += 1
        key = "%s:%s" % (r["file"], r["line"])
        if r["channel"] == "debug_message":
            sig = sigof[int(r["line"])]
            cls, what, cons, prev, cause, cont, ovr, sc, note = sig
            extra = DM_NOTE.get(int(r["line"]))
            if extra:
                note = note + " || " + extra
        else:
            cls, what, cons, prev, cause, cont, ovr, sc, note = V[key]
        out.append(dict(id="F58-%s%03d" % (r["arm"], n), arm=r["arm"], family=r["family"],
                        file=r["file"], line=r["line"], channel=r["channel"],
                        severity=r["level"], sink=r["sink"], **{
                            "class": cls, "WHAT": what, "CONS": cons, "PREV": prev,
                            "CAUSE": cause, "CONT": cont, "OVR": ovr, "SC": sc,
                            "text": r["text"], "note": note}))
    for i, s in enumerate(SILENT):
        f, ln, cls, what, cons, prev, cause, cont, ovr, sc, note = s
        out.append(dict(id="F58-S%03d" % (i + 1), arm="S", family="silent", file=f,
                        line=str(ln), channel="(none)", severity="(none)",
                        sink="(none)", **{"class": cls, "WHAT": what, "CONS": cons,
                                          "PREV": prev, "CAUSE": cause, "CONT": cont,
                                          "OVR": ovr, "SC": sc,
                                          "text": "(nothing is emitted)", "note": note}))

    with open(os.path.join(ART, "f58_gap_table.tsv"), "w") as fh:
        fh.write("\t".join(COLS) + "\n")
        for r in out:
            fh.write("\t".join(str(r[c]).replace("\t", " ") for c in COLS) + "\n")

    # ---------------- summary ----------------
    byclass = collections.Counter(r["class"] for r in out)
    L = []
    L.append("# F58 — the §11.169 gap table: summary\n")
    L.append("Universe + method: `f58_universe.md`. Table: `f58_gap_table.tsv`. "
             "Verdicts and element tests: `f58_gaptable.py` (self-asserting against the "
             "census — it exits non-zero if the verdict set and the site set differ).\n")
    L.append("Code `master-beta @ d6aec251`, product code read-only.\n")
    L.append("## Sites by class\n")
    L.append("| class | sites | meaning |")
    L.append("|---|---:|---|")
    MEAN = {"ERR": "user-facing error record", "ACT": "acting-default record (D12)",
            "BOTH": "one record wearing both faces", "SILENT": "schema applies, nothing emitted",
            "DELIB": "known-deliberate, cost-forced silence (§5.115's bulk class)",
            "INFO": "progress/state; neither schema applies",
            "TRACE": "internal debug trace; outside the user-facing floor",
            "NOISE": "emitted where nothing went wrong and nothing was decided",
            "CHAN": "emits another site's record; owns no content"}
    for c, k in byclass.most_common():
        L.append("| %s | %d | %s |" % (c, k, MEAN[c]))
    L.append("| **total** | **%d** | |" % len(out))

    scored = [r for r in out if r["class"] in ("ERR", "ACT", "BOTH", "SILENT", "DELIB")]
    L.append("\n## Element coverage over the %d schema-applicable sites\n" % len(scored))
    L.append("Y = present · P = partial · N = absent · – = the schema's other face.\n")
    L.append("| element | Y | P | N | – | Y-rate over sites where it applies |")
    L.append("|---|---:|---:|---:|---:|---:|")
    for el, label in [("WHAT", "WHAT (error)"), ("CONS", "CONSEQUENCES (error)"),
                      ("PREV", "PREVENTION (error)"), ("CAUSE", "CAUSE (acting default)"),
                      ("CONT", "CONTENT (acting default)"), ("OVR", "OVERRIDE (acting default)"),
                      ("SC", "self-containment of the action")]:
        c = collections.Counter(r[el] for r in scored)
        appl = c["Y"] + c["P"] + c["N"]
        rate = ("%.3f" % (c["Y"] / appl)) if appl else "—"
        L.append("| %s | %d | %d | %d | %d | %s |" % (label, c["Y"], c["P"], c["N"],
                                                      c["-"], rate))

    err = [r for r in scored if r["class"] in ("ERR", "BOTH")]
    act = [r for r in scored if r["class"] in ("ACT", "BOTH")]
    full_err = [r for r in err if r["WHAT"] == "Y" and r["CONS"] == "Y" and r["PREV"] == "Y"]
    full_act = [r for r in act if r["CAUSE"] == "Y" and r["CONT"] == "Y" and r["OVR"] == "Y"]
    zero_err = [r for r in err if r["WHAT"] == "N" and r["CONS"] == "N" and r["PREV"] == "N"]
    L.append("\n## Whole-schema results\n")
    L.append("| | sites | all three elements | zero elements |")
    L.append("|---|---:|---:|---:|")
    L.append("| error schema (ERR + BOTH) | %d | %d | %d |"
             % (len(err), len(full_err), len(zero_err)))
    L.append("| acting-default schema (ACT + BOTH) | %d | %d | %d |"
             % (len(act), len(full_act),
                len([r for r in act if r["CAUSE"] == "N" and r["CONT"] == "N" and r["OVR"] == "N"])))
    L.append("\nThe %d error records carrying all three elements:\n" % len(full_err))
    for r in sorted(full_err, key=lambda r: (r["file"], int(r["line"]))):
        L.append("- `%s:%s`" % (r["file"], r["line"]))
    L.append("\nThe %d acting-default records carrying all three elements:\n" % len(full_act))
    for r in sorted(full_act, key=lambda r: (r["file"], int(r["line"]))):
        L.append("- `%s:%s`" % (r["file"], r["line"]))

    L.append("\n## Where the records go (the channel question)\n")
    sinks = collections.Counter(r["sink"] for r in scored)
    L.append("| sink | schema-applicable sites |")
    L.append("|---|---:|")
    for s, k in sinks.most_common():
        L.append("| `%s` | %d |" % (s, k))
    sev = collections.Counter(r["severity"] for r in scored)
    L.append("\n| severity as emitted | schema-applicable sites |")
    L.append("|---|---:|")
    for s, k in sev.most_common():
        L.append("| `%s` | %d |" % (s, k))

    L.append("\n## Per-file\n")
    L.append("| file | sites | ERR | ACT | BOTH | SILENT | DELIB | INFO | TRACE | NOISE | CHAN |")
    L.append("|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|")
    perfile = collections.defaultdict(collections.Counter)
    for r in out:
        perfile[r["file"]][r["class"]] += 1
    for f in sorted(perfile):
        c = perfile[f]
        L.append("| `%s` | %d | %s |" % (f, sum(c.values()),
                 " | ".join(str(c[k]) for k in ["ERR", "ACT", "BOTH", "SILENT", "DELIB",
                                                "INFO", "TRACE", "NOISE", "CHAN"])))
    with open(os.path.join(ART, "f58_gap_summary.md"), "w") as fh:
        fh.write("\n".join(L) + "\n")

    print("gap table: %d rows (%d census sites + %d silent sites)"
          % (len(out), len(live), len(SILENT)))
    print("classes:", dict(byclass))


if __name__ == "__main__":
    main()
