#!/usr/bin/env python3
"""F116 -- the two scratch mutations, applied by exact string replacement so
they are reproducible and revertible (`git -C .. checkout src/tools/log.cpp`).

    python3 mutate.py budget [bytes]   # M1: the budget the COMPARISON uses
    python3 mutate.py instr            # M1: the counter/timing dump at close
    python3 mutate.py rule_written     # M2: rotate the channel LAST WRITTEN

NEITHER touches src/tools/log.hpp, so the SHIPPED constant is never mutated and
the delivered binary rebuilds from an unmodified header.  `budget` works by
defining a macro AFTER the header is included, so every later use in log.cpp --
the comparison and the two figures the D12 line prints -- moves together and the
line tells the truth about the build that wrote it.
"""
import re
import subprocess
import sys
from pathlib import Path

SRC = Path(__file__).resolve().parents[4] / "src/tools/log.cpp"

BUDGET_ANCHOR = 'const std::string LOG_EXTENSION=".log";'

INSTR_HEAD = """
// ---- F116 SCRATCH INSTRUMENTATION (never committed to master-beta) --------
#include <chrono>
static std::uintmax_t g_nWrite = 0, g_nsWrite = 0, g_nsCount = 0, g_nsCheck = 0;
static std::uintmax_t g_nRot = 0, g_nsRot = 0;
static inline std::uintmax_t f116_now()
{
\treturn std::chrono::duration_cast<std::chrono::nanoseconds>(
\t\tstd::chrono::steady_clock::now().time_since_epoch()).count();
}
// --------------------------------------------------------------------------
"""


def read():
    return SRC.read_text(encoding="latin-1")


def write(t):
    SRC.write_text(t, encoding="latin-1")


def must(t, old, new, n=1):
    if t.count(old) != n:
        raise SystemExit("anchor %r appears %d times, expected %d"
                         % (old[:60], t.count(old), n))
    return t.replace(old, new)


def budget(nbytes):
    t = read()
    t = must(t, BUDGET_ANCHOR, BUDGET_ANCHOR + """

// ---- F116 SCRATCH MUTATION: the budget the comparison and the D12 line use.
// Defined AFTER log.hpp, so the shipped constexpr in the header is untouched
// and every use below moves together.  A 1 GiB log is not a test.
#undef LOG_RETENTION_BYTES
#define LOG_RETENTION_BYTES ((std::uintmax_t)%dULL)
// --------------------------------------------------------------------------
""" % nbytes)
    write(t)
    print("M1 budget = %d B" % nbytes)


def instr():
    t = read()
    t = must(t, BUDGET_ANCHOR, BUDGET_ANCHOR + INSTR_HEAD)
    # the whole writeLocked body, the compare alone, and the rotation
    t = must(t, """\twriteMutex.lock();
\twriteLocked(texte, type, fichier);""", """\twriteMutex.lock();
\tconst std::uintmax_t f116_w0 = f116_now();
\twriteLocked(texte, type, fichier);
\tconst std::uintmax_t f116_w1 = f116_now();
\tconst bool f116_over = budgetUsed > LOG_RETENTION_BYTES;
\tconst std::uintmax_t f116_w2 = f116_now();
\t++g_nWrite; g_nsWrite += f116_w1 - f116_w0; g_nsCheck += f116_w2 - f116_w1;
\tif (f116_over) {
\t\tconst std::uintmax_t f116_r0 = f116_now();
\t\trotateForBudget();
\t\t++g_nRot; g_nsRot += f116_now() - f116_r0;
\t}""")
    t = must(t, """\tif (budgetUsed > LOG_RETENTION_BYTES)
\t\trotateForBudget();
\twriteMutex.unlock();""", "\twriteMutex.unlock();")
    # the counter block alone
    t = must(t, """\tconst std::uintmax_t written = ligne.size() + texte.size() + 1;
\ttarget->second.live += written;
\tbudgetUsed += written;""", """\tconst std::uintmax_t f116_c0 = f116_now();
\tconst std::uintmax_t written = ligne.size() + texte.size() + 1;
\ttarget->second.live += written;
\tbudgetUsed += written;
\tg_nsCount += f116_now() - f116_c0;""")
    # the dump, BEFORE close() writes its own EOF lines
    t = must(t, """\tif (singleton != nullptr) {
\t\tfor (auto &channel: singleton->logFile) {""", """\tif (singleton != nullptr) {
\t\t{
\t\t\tstd::uintmax_t cal = 0;
\t\t\tfor (int i = 0; i < 100000; ++i) { const std::uintmax_t a = f116_now(); cal += f116_now() - a; }
\t\t\tstd::cerr << "[f116-instr] total=" << singleton->budgetUsed;
\t\t\tfor (const auto &c : singleton->logFile)
\t\t\t\tstd::cerr << " " << c.second.path << "=" << c.second.live << "/" << c.second.archived;
\t\t\tstd::cerr << " writes=" << g_nWrite << " ns_write=" << g_nsWrite
\t\t\t\t<< " ns_count=" << g_nsCount << " ns_check=" << g_nsCheck
\t\t\t\t<< " rot=" << g_nRot << " ns_rot=" << g_nsRot
\t\t\t\t<< " clock_pair_ns=" << (cal / 100000.0) << std::endl;
\t\t}
\t\tfor (auto &channel: singleton->logFile) {""")
    write(t)
    print("M1 instrumentation in")


def rule_written():
    t = read()
    t = must(t, BUDGET_ANCHOR, BUDGET_ANCHOR + """

// ---- F116 SCRATCH MUTATION M2: the section's option (a) -------------------
static LOG_FILE g_lastWritten = LOG_FILE::INTERNAL;
// --------------------------------------------------------------------------
""")
    t = must(t, """\ttarget->second.live += written;""",
             """\ttarget->second.live += written;
\tg_lastWritten = target->first;""")
    t = must(t, """\tauto biggest = logFile.end();
\tstd::uintmax_t most = 0;
\tfor (auto it = logFile.begin(); it != logFile.end(); ++it) {
\t\tconst std::uintmax_t held = it->second.live + it->second.archived;
\t\tif (biggest == logFile.end() || held > most) {
\t\t\tbiggest = it;
\t\t\tmost = held;
\t\t}
\t}""", """\tauto biggest = logFile.find(g_lastWritten);   // M2: the channel being written""")
    write(t)
    print("M2 rule = the channel last written")


if __name__ == "__main__":
    what = sys.argv[1]
    if what == "budget":
        budget(int(sys.argv[2]) if len(sys.argv) > 2 else 4 * 1024 * 1024)
    elif what == "instr":
        instr()
    elif what == "rule_written":
        rule_written()
    else:
        raise SystemExit(__doc__)
    print(subprocess.run(["git", "-C", str(SRC.parents[2]), "diff", "--stat"],
                         capture_output=True, text=True).stdout.strip())
