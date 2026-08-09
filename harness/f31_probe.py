# F31 / INTENT §5.74 — gdb-side probe (sourced by f31_search.gdb, NOT importable
# outside gdb: it uses the `gdb` module).
#
# WHAT IT MEASURES. §5.74 owes a discrimination between two causes of "`search`
# returns no (S) and no (C)": the star/constellation NAME catalogues are not
# loaded, or the prefix match never fires. Each cause has its OWN surface, and
# this probe reads both at the place the aggregation itself reads them:
#
#   - the four per-catalogue `listMatchingObjectsI18n` entries. At each one the
#     probe prints THE CATALOGUE'S OWN COUNT (the container the match loop walks)
#     and the `maxNbItem` quota it was handed. The two catalogues that DO answer
#     (planets, nebulae) are read on the SAME channel, at the same instant, in
#     the same call — they are the positive control, not a separate instrument.
#   - the load sites themselves (`ConstellationMgr::loadLinesAndArt`,
#     `HipStarMgr::loadCommonNames`) and the gate above them
#     (`Core::setSkyCultureDir` + its reject line core.cpp:1430), as HIT COUNTS.
#     A load that never ran and a load that ran and found nothing are different
#     facts, and only the hit count separates them.
#
# Output goes to $F31_PROBE, opened/flushed/closed per line, deliberately NOT
# through gdb's stdout: gdb's own stream is block-buffered when redirected, and
# a probe whose evidence is still in a buffer when the run ends is a silent
# no-op probe (the instrument-chain failure class, INTENT §11.47).
import os
import gdb

PROBE = os.environ.get("F31_PROBE", "/tmp/f31_probe.txt")
HITS = {}


def emit(msg):
    with open(PROBE, "a") as f:
        f.write(msg + "\n")
        f.flush()


def hit(name):
    HITS[name] = HITS.get(name, 0) + 1
    return HITS[name]


def _ev(expr):
    return gdb.parse_and_eval(expr)


def vec_count(expr):
    """std::vector element count, read from the container's own pointers (no
    inferior call: `size()` is inline and may have no symbol at -O2)."""
    try:
        v = _ev(expr)
        return int(v['_M_impl']['_M_finish'] - v['_M_impl']['_M_start'])
    except Exception as e:
        return "ERR(%s)" % e


def node_count(expr):
    """std::map/std::set element count (libstdc++ _Rb_tree keeps it)."""
    try:
        v = _ev(expr)
        return int(v['_M_t']['_M_impl']['_M_node_count'])
    except Exception:
        pass
    try:                                   # unordered_map fallback
        v = _ev(expr)
        return int(v['_M_h']['_M_element_count'])
    except Exception as e:
        return "ERR(%s)" % e


def grid_count(expr):
    """SphereGrid<T> element count: sum of the per-zone std::list sizes over
    `allDataCenter` (vector<pair<list<T>,bool>>). O(1) per zone — libstdc++
    keeps _M_size in the list header."""
    try:
        v = _ev(expr + ".allDataCenter")
        start = v['_M_impl']['_M_start']
        n = int(v['_M_impl']['_M_finish'] - start)
        tot, zones = 0, 0
        for i in range(n):
            lst = (start + i).dereference()['first']
            tot += int(lst['_M_impl']['_M_node']['_M_size'])
            zones += 1
        return tot, zones
    except Exception as e:
        return "ERR(%s)" % e, -1


def pstr(expr):
    """A std::string as gdb prints it (one line, without gdb's `$N = ` prefix)."""
    try:
        s = gdb.execute("print " + expr, to_string=True).strip().replace("\n", " ")
        return s.split("= ", 1)[1] if s.startswith("$") and "= " in s else s
    except Exception as e:
        return "ERR(%s)" % e


def sample(expr):
    """First `print elements` entries of a container, via gdb's own printer."""
    try:
        s = gdb.execute("print " + expr, to_string=True)
        return " ".join(s.split())
    except Exception as e:
        return "ERR(%s)" % e


# ------------------------------------------------------------------ the four
def probe_core():
    n = hit("core")
    emit("CORE #%d prefix=%s maxNbItem=%s" % (n, pstr("objPrefix"), pstr("maxNbItem")))


def probe_planet():
    n = hit("planet")
    emit("PLANET #%d catalogue_count=%s maxNbItem=%s"
         % (n, node_count("this->systemBodies"), pstr("maxNbItem")))


def probe_const():
    n = hit("const")
    c = vec_count("this->asterisms")
    emit("CONST #%d catalogue_count=%s maxNbItem=%s" % (n, c, pstr("maxNbItem")))
    if isinstance(c, int) and c:
        names = []
        try:
            start = _ev("this->asterisms")['_M_impl']['_M_start']
            for i in range(min(c, 8)):
                a = (start + i).dereference()
                names.append("%s/%s" % (str(a['abbreviation']), str(a['nameI18'])))
        except Exception as e:
            names = ["ERR(%s)" % e]
        emit("CONST #%d sample=%s" % (n, " | ".join(names)))


def probe_neb():
    n = hit("neb")
    tot, zones = grid_count("this->nebGrid")
    emit("NEB #%d catalogue_count=%s zones=%s maxNbItem=%s"
         % (n, tot, zones, pstr("maxNbItem")))


def probe_star():
    n = hit("star")
    emit("STAR #%d common_names_index_i18n=%s sci_names_index_i18n=%s "
         "common_names_map=%s maxNbItem=%s"
         % (n,
            node_count("HipStarMgr::common_names_index_i18n"),
            node_count("HipStarMgr::sci_names_index_i18n"),
            node_count("HipStarMgr::common_names_map"),
            pstr("maxNbItem")))
    emit("STAR #%d sample=%s" % (n, sample("HipStarMgr::common_names_index_i18n")))


# ------------------------------------------------------------- the load sites
def probe_setculture():
    emit("SETCULTURE #%d dir=%s" % (hit("setculture"), pstr("cultureDir")))


def probe_gate_reject():
    emit("GATE-REJECT #%d (core.cpp:1430 — culture name resolved to \"\")"
         % hit("gate_reject"))


def probe_loadlines():
    emit("LOADLINES #%d path=%s" % (hit("loadlines"), pstr("skyCultureDir")))


def probe_loadlines_done():
    emit("LOADLINES-DONE #%d asterisms=%s"
         % (hit("loadlines_done"), vec_count("this->asterisms")))


def probe_loadcommon():
    emit("LOADCOMMON #%d file=%s" % (hit("loadcommon"), pstr("commonNameFile")))


def probe_loadsky():
    emit("LOADSKY #%d path=%s" % (hit("loadsky"), pstr("culturePath")))


def probe_updatei18n():
    # AT ENTRY: the rebuild has not run yet, so this pair says whether
    # loadCommonNames filled the english map that updateI18n is about to
    # translate into the index the match walks.
    emit("UPDATEI18N #%d at-entry common_names_map=%s index_i18n=%s"
         % (hit("updatei18n"),
            node_count("HipStarMgr::common_names_map"),
            node_count("HipStarMgr::common_names_index_i18n")))


emit("PROBE LOADED (f31_probe.py)")
