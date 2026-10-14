#!/usr/bin/env python3
"""build_read_table.py -- writes claude/harness/f75/read_table_f158.json from the READ below.

The decisions are the reading (one line each, made from read-view-manual.txt,
read-cites-manual.txt, read-proposals.json and the code at both pins); this
script only attaches what a reader must not retype: the texts at the old pin
and at the new one, the number of references per referent, and checks that
every new line exists at 9d461bd5. The JSON it writes is the table; this file
is its build record.

Classes of reading (11.331 (4)):
  F  the cited construct is a whole FUNCTION; same qualified name, one
     definition, at the target: its def line .. closing brace there
  M  the construct moved or was rewritten IN PLACE (a call's arguments, a
     signature): the same statement / block at the target
  C  agent prose left the code (3c5952ad / 57102bf4 / a49df0f1 / b57d0642):
     the cited range keeps the code it held, plus the code a removed comment
     introduced when the comment ended the range; a pure comment is replaced by
     the code it introduced when that code is the sentence's subject
  S  the construct moved into another function of the same file (F151's
     scriptEnded) or was split between the two paths
  X  the anchor did not hold its sentence's subject AT ITS OWN PIN (F75's new
     class): re-pointed to where the subject lives, and ROUTED
  D  the referent left and the sentence cites the referent ITSELF ("the
     comment at ... records", "recorded in", "two comments spell"), or its
     subject lives only in another file: declared [NOT AT HEAD] at its pin
  U  a declared reference whose construct is at the new pin: re-anchored and
     its marker removed
"""
import collections, json, sys
sys.path.insert(0, '/home/claude/spacecrafter/util/scedit/tests')
import anchor_gate as ag

TO = "9d461bd5"
A = "/home/claude/spacecrafter/claude/harness/artifacts/f158"
OUTP = "/home/claude/spacecrafter/claude/harness/f75/read_table_f158.json"
B, S = "b45d3b58", "8d41fbe3"
SKY = "29e93df9 (the frame-thread sky layers drawn through one SkyView) rewrote its first line"

# (repo, elem, pin, class, new-or-None, marker-or-None, reason)
READ = [
 ("src/bodyModule/ssystem_factory.hpp", "263-272", B, "C", "193-196", None,
  "the seam comment block and setFlagMoonScale at the pin; the comment left (3c5952ad); setFlagMoonScale is 193-196"),
 ("src/bodyModule/ssystem_factory.hpp", "263-268", B, "C", "193-196", None,
  "a pure comment heading the four display-scale seams; it left (3c5952ad); the note's subject (`flag moon_scaled off` puts the scale back to 1) is setFlagMoonScale, the function it introduced, 193-196; the note's 5 s figure is not visible in these lines (commandDisplayScale -> setScaling)"),
 ("src/bodyModule/ssystem_factory.hpp", "477-478", B, "C", "341-344", None,
  "the comment inside createTexSkin introducing the new-path mirror; it left (57102bf4); the try block it introduced is 341-344"),
 ("src/bodyModule/ssystem_factory.hpp", "515-526", B, "C", "371-372", None,
  "the navigation-radius seam comment and setBodyDatumRadius; the comment became the one-line what-for at 371 (3c5952ad); the declaration is 372"),
 ("src/bodyModule/ssystem_factory.hpp", "743-765", B, "C", "530-531", None,
  "the save contract comment and saveCurrentSystem; the comment became the one-line what-for at 530 (3c5952ad); the declaration is 531"),
 ("src/bodyModule/ssystem_factory.hpp", "756-765", B, "C", "530-531", None,
  "the refusal paragraph of the same comment and saveCurrentSystem; the what-for at 530 keeps the path-separator half; the .disabled refusal is implemented in saveCurrentSystem's body in the .cpp (present at 9d461bd5); the declaration is 531"),
 ("src/coreModule/backup_mgr.cpp", "59-73", B, "C", "54", None,
  "the write-half comment and the observerMoveTo restore call; the comment left (3c5952ad); the call is 54"),
 ("src/coreModule/backup_mgr.cpp", "82-90", B, "C", "63-65", None,
  "the read-half comment and the three observatoryGet* reads; the comment left (3c5952ad); the reads are 63-65"),
 ("src/coreModule/cardinals.cpp", "78-94", B, "M", "78-94", None,
  "the four cardinal letters printed in Cardinals::draw; the print calls rewritten in place, prj -> view (29e93df9); same lines"),
 ("src/coreModule/cardinals.cpp", "82-94", B, "M", "82-94", None,
  "the four letters' print calls, rewritten in place prj -> view (29e93df9); same lines"),
 ("src/coreModule/cardinals.cpp", "95-118", B, "M", "95-118", None,
  "the intercardinal block, its print calls rewritten in place prj -> view (29e93df9); same lines"),
 ("src/coreModule/constellation.cpp", "150-161", B, "M", "149-160", None,
  "Constellation::drawName from its def line to the printHorizontal call; the signature and the view calls rewritten (29e93df9), one line up"),
 ("src/coreModule/core.cpp", "1092-1097", B, "C", "1036", None,
  "the both-trees comment and the selectObject(searchObjectByEnglishName) statement; the comment left (3c5952ad); the statement is 1036"),
 ("src/coreModule/core.cpp", "2523-2534", B, "C", "2610-2621", None,
  "Core::setViewOffset from its def through the clamp and the comment that introduced the out-of-range report; the comment left (3c5952ad); def through the report is 2610-2621"),
 ("src/coreModule/core.hpp", "170-173", B, "C", "171-173", None,
  "a merge comment and setInitialSrtLocale; the comment left (57102bf4); the function is 171-173"),
 ("src/coreModule/core.hpp", "199-200", B, "C", "199", None,
  "a merge comment and setSrtLanguage; the comment left (57102bf4); the function is 199"),
 ("src/coreModule/core.hpp", "203-204", B, "C", "202", None,
  "a merge comment and setInitFov; the comment left (57102bf4); the function is 202"),
 ("src/coreModule/core.hpp", "224-226", B, "C", "222-223", None,
  "the doc comment and setFlagLockSkyPosition; the comment became the one-line what-for at 222 (a49df0f1); the declaration is 223"),
 ("src/coreModule/core.hpp", "510-512", B, "C", "470", None,
  "the doc comment and setExperimentalPath; the comment left (a49df0f1); the declaration is 470"),
 ("src/coreModule/coreLink.hpp", "836-852", B, "C", "794-799", None,
  "a pure comment heading the observer place readouts; it left (3c5952ad); the note's subject (the place a move starts from is the drawn path's) is the readout it introduced, observatoryGetLatitude asking drawnPlace, 794-799"),
 ("src/coreModule/coreLink.hpp", "850-852", B, "D", None, "the comment that recorded it left the code",
  "the note cites the RECORD itself (recorded in ...): the no-clamp paragraph of that comment; it left (3c5952ad) and no line of coreLink.hpp says it at 9d461bd5"),
 ("src/coreModule/coreLink.hpp", "943-950", B, "C", "879-881", None,
  "the free-flight comment and cameraSetFreeMode; the comment left (3c5952ad); the function is 879-881"),
 ("src/coreModule/coreLink.hpp", "952-961", B, "C", "883-886", None,
  "the descent comment and cameraDescend; the comment became the one-line what-for at 883 (3c5952ad); the function is 884-886 and now calls Camera::multAlt (78455b88 renamed Camera::descend)"),
 ("src/coreModule/nebula.cpp", "408-418", B, "M", "408-418", None,
  "Nebula::drawName from its def line to the printGravity180 call; signature and calls rewritten in place prj -> view (29e93df9); same lines"),
 ("src/coreModule/nebula_mgr.cpp", "203", B, "M", "203", None,
  "the drawName call, its first argument prj -> view (29e93df9); same line"),
 ("src/coreModule/nebula_mgr.cpp", "206", B, "M", "206", None,
  "the drawName call, its first argument prj -> view (29e93df9); same line"),
 ("src/coreModule/nebula_mgr.cpp", "211-222", B, "M", "211-222", None,
  "the drawHint branch, its two calls rewritten prj, nav -> view (29e93df9); same lines"),
 ("src/coreModule/nebula_mgr.cpp", "212", B, "M", "212", None,
  "the drawHint call, prj, nav -> view (29e93df9); same line"),
 ("src/coreModule/nebula_mgr.cpp", "222", B, "M", "222", None,
  "the drawHint call, prj, nav -> view (29e93df9); same line"),
 ("src/coreModule/skyDisplay.cpp", "226", B, "M", "225", None,
  "the uMat choice (getMatLocalToEye for AL, getMatEarthEquToEye otherwise), prj -> view (29e93df9), one line up"),
 ("src/coreModule/skyDisplay.cpp", "419-1031", B, "M", "418-1030", None,
  "SkyCoords::draw through SkyOrthodromy::draw (five draw functions), each signature rewritten (29e93df9), one line up; same span"),
 ("src/coreModule/skyDisplay.cpp", "594-596", B, "M", "593-595", None,
  "SkyMouse::draw's pointer read (SDL_GetMouseState then getCursorPosEqu), prj -> view (29e93df9), one line up"),
 ("src/coreModule/skyline.cpp", "427", B, "M", "427", None,
  "the ANALEMMALINE frame choice proj_func = &...::projectEarthEqu, Projector -> SkyView (29e93df9); same line"),
 ("src/coreModule/skyline.cpp", "557", B, "M", "557", None,
  "SkyLine_Vernal's frame choice proj_func = &...::projectEarthEqu, Projector -> SkyView (29e93df9); same line"),
 ("src/coreModule/skyline.cpp", "1372-1440", B, "M", "1372-1440", None,
  "SkyLine_Precession::draw from its def to the end of its circle loop; the signature rewritten (29e93df9); same lines"),
 ("src/executorModule/solarSystemModule.cpp", "210", B, "M", "198", None,
  "the skyDisplayMgr->draw call passing the current and the previous selection; its view argument rewritten (29e93df9), 12 lines up"),
 ("src/executorModule/solarSystemModule.cpp", "298", B, "S", "280-283", None,
  "the star preDraw call SPLIT by path (c2d24732): the new path's call at 281, the old one, re-indented, at 283; the if/else holding both is 280-283"),
 ("src/executorModule/stellarSystemModule.cpp", "205", B, "M", "205", None,
  "the skyDisplayMgr->draw call passing the current and the previous selection; its view argument rewritten (29e93df9); same line"),
 ("src/experimentalModule/BodyModule.hpp", "49", B, "D", None, "the comment that spelled the colour form left the code",
  "the note cites the two engine COMMENTS themselves (two engine-side comments spell ...); this one left (3c5952ad); no comment of BodyModule.hpp spells the form at 9d461bd5"),
 ("src/experimentalModule/BodyModule.hpp", "264", B, "D", None, "the comment that spelled the colour form left the code",
  "same note, the second comment; it left (3c5952ad)"),
 ("src/experimentalModule/Camera.cpp", "1057-1082", B, "M", "726-742", None,
  "Camera::descend RENAMED Camera::multAlt (78455b88); from its def to the end of the system-reference branch, its comments left; 726-742"),
 ("src/experimentalModule/CameraAnchors.cpp", "243-285", B, "C", "181-215", None,
  "the orbit-anchor parent resolution block; its comments left (3c5952ad); the code is 181-215"),
 ("src/experimentalModule/CameraAnchors.cpp", "251-273", B, "C", "183-203", None,
  "the parentless orbit-centre branch; its comment left (3c5952ad); the code is 183-203"),
 ("src/experimentalModule/ModularBody.cpp", "794-797", B, "C", "586", None,
  "the tex_ring comment and the rings test in deduceBodyModuleList; the comment left (3c5952ad); the test is 586"),
 ("src/experimentalModule/ModularSystem.cpp", "773-778", S, "X", "678-687", None,
  "MIS-ANCHORED AT ITS OWN PIN: at 8d41fbe3 these lines compute a subsystem's on-screen size, not the parent lookup the note states (resolves by name against what is loaded, then at 1063-1072 there); the subject at 9d461bd5 is loadBody's parent resolution, 678-687 (default parent, findBody, the warning when not found). Routed"),
 ("src/experimentalModule/ModularSystem.cpp", "1301-1310", S, "C", "830-838", None,
  "the relation / bound_to_surface conflict diagnostic; the comment after it left (3c5952ad); the diagnostic block is 830-838"),
 ("src/experimentalModule/ModularSystem.cpp", "1615-1626", S, "C", "1002-1003", None,
  "the D16 type= comment and the isFamily / famType declaration it introduced; the comment left (3c5952ad); the two lines are 1002-1003. The note's sentence is ROUTED: the code discriminates by the type VALUE (moduleTypeFromName), `body =` only flags a mistyped family"),
 ("src/experimentalModule/ModularSystem.cpp", "2036", S, "C", "1289-1290", None,
  "a comment line explaining the sidereal_time emptiness test; it left (3c5952ad); the test it explained (find, then end or empty) is 1289-1290"),
 ("src/experimentalModule/SessionFile.hpp", "211", B, "M", "57", None,
  "DIRECTORY = \"sessions\", moved up with a trailing what-for comment; 57"),
 ("src/experimentalModule/ShadowService.hpp", "110-121", B, "C", "18-23", None,
  "the class head through the static enabled switch; the comments left (3c5952ad); 18-23"),
 ("src/experimentalModule/moduleLoader/LayeredMeshLoader.cpp", "52", S, "C", "48", None,
  "a comment line on reading the surface model from the body; it left (3c5952ad); the statement it introduced, cfg.moonClass from getSurfaceModel, is 48"),
 ("src/experimentalModule/moduleLoader/RingLoader.cpp", "8-45", B, "C", "8-36", None,
  "isLikely, isLoaderOf and load up to the near-only routing comment; the comments left (3c5952ad); the routing they explained, addNearComponent, is 36; 8-36"),
 ("src/experimentalModule/orbitModules/LocationOrbitLoader.hpp", "3", S, "U", "10", None,
  "DECLARED by F119 because the pin could not move; the construct (the read of the parent key) is at the new pin: ModularBody *parent = findBodyOnce(params parent), 10 (01adbb30 gave it the nullptr check). Marker removed: its claim would be false at the new pin"),
 ("src/interfaceModule/app_command_init.cpp", "99-111", B, "C", "99-101", None,
  "the canonical-first comment and the reverse-map loop; the comment left (3c5952ad); the loop is 99-101"),
 ("src/interfaceModule/app_command_interface.cpp", "1266-1284", B, "X", "1251-1269", None,
  "MIS-ANCHORED AT ITS OWN PIN: at b45d3b58 these are setFlag's switch cases, not the single-pair read of the seed's rule (the census cites commandFlag reading args.begin()); commandFlag is 1343-1361 at b45d3b58 and 1251-1269 at 9d461bd5. Routed"),
 ("src/interfaceModule/app_command_interface.cpp", "1281-1285", B, "D", None, "the case comment left the code",
  "the note cites the case COMMENT itself (the case comment records ...); it left (3c5952ad); the plain assignment it explained stays"),
 ("src/interfaceModule/app_command_interface.cpp", "1475-1477", B, "D", None, "the handler comment left the code",
  "the note cites the handler's own COMMENT (the handler's own comment states ...); it left (3c5952ad)"),
 ("src/interfaceModule/app_command_interface.cpp", "1877-1880", B, "C", "1905-1908", None,
  "readColor's doc comment; it left (3c5952ad); the note's subject (the two colours with no getter answer false) is the two false returns, 1905-1908"),
 ("src/interfaceModule/app_command_interface.cpp", "2971", B, "S", "262", None,
  "the W_END branch's imageDropAllNoPersistent moved into scriptEnded (f571664e), reached from cancelScript on every end path; 262"),
 ("src/interfaceModule/app_command_interface.cpp", "2972", B, "S", "263", None,
  "the W_END branch's swapCommand reset moved into scriptEnded (f571664e); 263"),
 ("src/interfaceModule/app_command_interface.cpp", "2973", B, "S", "264", None,
  "the W_END branch's ifSwap reset moved into scriptEnded (f571664e); 264. Sat in a MARKED string, so the gate counted it declared"),
 ("src/interfaceModule/app_command_interface.cpp", "3024-3029", B, "D", None, "the comment left the code, the return it explained stays",
  "the note cites the COMMENT itself (the comment at ... records why); it left (b57d0642)"),
 ("src/interfaceModule/app_command_interface.cpp", "4200-4314", B, "X", "4537-4653", None,
  "MIS-ANCHORED AT ITS OWN PIN: at b45d3b58 these lines run from commandBody's screenshot branch into commandFont, not the single-pair arithmetic handlers the census cites; define .. sinus are 4647-4763 at b45d3b58 and 4537-4653 at 9d461bd5. Routed"),
 ("src/interfaceModule/app_command_interface.hpp", "185-187", B, "C", "164-165", None,
  "applyColor's doc comment and declaration; the comment became the one-line what-for at 164 (index is used by star_table alone, 57102bf4); the declaration is 165"),
 ("src/starModule/hip_star_mgr.cpp", "828-834", B, "M", "832-838", None,
  "the onlySelected test and the two zone-draw loops, their prj, nav arguments -> frame (c2d24732); 832-838"),
 ("src/starModule/zone_array.cpp", "293", B, "M", "294", None,
  "the drawStar call with color_table[BV index] (inside zones), prj -> frame.rolloff() (c2d24732); 294"),
 ("src/starModule/zone_array.cpp", "382", B, "M", "393", None,
  "the drawStar call with color_table[BV index] (border zones), prj -> frame.rolloff() (c2d24732); 393"),
 ("src/tools/ini_line.hpp", "16", S, "D", None, "the grammar comment left the header, the rule lives in the reader",
  "the cited line stated the rule '#' starts a comment anywhere on the line; the grammar comment left (3c5952ad); line 16 at 9d461bd5 is COMMENT_CHAR, which grounds the character but not the anywhere; the rule is implemented in another file"),
 # found by the reading-flip and written-context checks of the FIRST sweep (11.331 (4b)); each one's file
 # reading is now a FILE_OVERRIDES row of the gate, so the map reads the file its sentence names
 ("src/scriptModule/script_mgr.cpp", "479", B, "X", "345", None,
  "MIS-READ SINCE F75: parse_model._source's `script_mgr.cpp:81-107 + :335` (73cc7b80) named commander->terminateScript() in script_mgr.cpp; the resolver read the bare number in app_command_interface.cpp and F75 moved it there (335 -> 479), so at b45d3b58 it is past the end of script_mgr.cpp (401 lines). The subject, the natural end's terminateScript call, is 345 at 9d461bd5. Routed"),
 ("fscripts/panorama5.sts", "97-98", B, "K", None, None,
  "MIS-READ SINCE F75: the `:97-98` of lint_seeds[12] continues fscripts/panorama5.sts (field data, the two siblings declare it); the resolver read it in app_command_interface.cpp (R0 only). Number kept, declared by its string's marker"),
]

props = {(p["repo"], p["elem"], p["pin"]): p for p in json.load(open(A + "/read-proposals.json"))}
gone = json.load(open(A + "/gone.json"))
refs = collections.Counter((g["repo"], g["elem"], g["pin"]) for g in gone)
paths = collections.defaultdict(set)
for g in gone:
    paths[(g["repo"], g["elem"], g["pin"])].add(g["contract"].split("/")[-1] + " " + g["jp"])
# a class-X correction may name a RESOLVING reference: count it from the map
amap = json.load(open("/home/claude/spacecrafter/claude/harness/f75/f158/anchor_map.json"))
for path, rws in amap.items():
    for r in rws:
        k = (r["repo"], r["elem"], r["pin"])
        if k not in refs and not r["own"] and r["raw"] in ("clean", "moved"):
            paths.setdefault(("X?",) + k, set()).add(path.split("/")[-1] + " " + r["jp"])

def txt(ls, n):
    return ls[n - 1].strip()[:160] if 1 <= n <= len(ls) else "(no line %d: past the end of the file at the pin)" % n

rows, seen = [], set()
for p in props.values():
    if p["kind"] == "function":
        READ.append((p["repo"], p["elem"], p["pin"], "F", p["new"], None,
                     "the whole function %s (def line .. closing brace) is %s at %s; %s"
                     % (p["qname"], p["new"], TO, SKY)))
for repo, elem, pin, cls, new, marker, reason in READ:
    k = (repo, elem, pin)
    if k in seen:
        raise SystemExit("two readings for %r" % (k,))
    seen.add(k)
    if k not in refs:
        if cls != "X" or ("X?",) + k not in paths:
            raise SystemExit("not a GONE referent: %r" % (k,))
        refs[k] = len(paths[("X?",) + k]); paths[k] = paths[("X?",) + k]
    lp, lt = ag.lines_at(pin, repo), ag.lines_at(TO, repo)
    if lp is None:
        lp = lt = ["(not a file of this repository)"] * 100000
    a = int(elem.split("-")[0]); b = int(elem.split("-")[1]) if "-" in elem else a
    row = dict(repo=repo, elem=elem, pin=pin, cls=cls,
               action={"D": "notathead", "U": "unmark", "K": "keep"}.get(cls, "lines"),
               refs=refs[k], cited_by=sorted(paths[k]),
               at_pin=[txt(lp, a), txt(lp, b)] if b != a else [txt(lp, a)],
               reason=reason)
    if new:
        n1 = int(new.split("-")[0]); n2 = int(new.split("-")[1]) if "-" in new else n1
        if not (1 <= n1 <= n2 <= len(lt)):
            raise SystemExit("new lines out of range: %r -> %s" % (k, new))
        row["new"] = new
        row["at_to"] = [lt[n1 - 1].strip()[:160], lt[n2 - 1].strip()[:160]] if n2 != n1 else [lt[n1 - 1].strip()[:160]]
    if marker:
        row["marker"] = marker
    rows.append(row)
missing = set(k for k in refs if k in seen or k[0] != "X?") - seen
if missing:
    raise SystemExit("UNREAD referents: %r" % sorted(missing))
rows.sort(key=lambda r: (r["repo"], int(r["elem"].split("-")[0])))
json.dump(rows, open(OUTP, "w"), indent=1, ensure_ascii=True)
c = collections.Counter(r["cls"] for r in rows)
cr = collections.Counter()
for r in rows:
    cr[r["cls"]] += r["refs"]
print("%d referents, %d references -> %s" % (len(rows), sum(r["refs"] for r in rows), OUTP))
print("by class (referents):", dict(sorted(c.items())))
print("by class (references):", dict(sorted(cr.items())))
