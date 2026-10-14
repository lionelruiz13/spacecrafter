#!/bin/bash
# F157 (INTENT 11.330) - the oracle's replica checked against the COMPILED functions, no engine.
#
#   bash cachekey_cpp.sh [<code repo>]      (default /home/claude/spacecrafter; run cachekey_oracle.py first)
#
# Extracts, by pattern and without editing a byte, s_texture::getCacheName(string) + getCacheEntryName from the code
# repo's HEAD (TODAY) and from its worktree (AFTER: the file-local cacheSlice + the two functions), and
# CallSystem::isAbsolute from the worktree's call_system.cpp; compiles them into one standalone program (the class
# prefix stripped, each set in a namespace); runs both over the oracle's inputs (REL, ABS, PROBES) and compares the
# output byte-exact with the oracle's today_<form>.tsv / proposed_G2_<form>.tsv. Exit 0 = all six identical.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
CODE="${1:-/home/claude/spacecrafter}"
W="$HERE/cpp"
mkdir -p "$W"
fn() {  # the function bodies from "$1" (a file), first line matching "$2" to the first "}" at column 0 after the last match of "$3"
    awk -v a="$2" -v b="$3" '$0 ~ a {on=1} on {print} on && $0 ~ b {last=1} last && /^}/ {exit}' "$1"
}
git -C "$CODE" show HEAD:src/tools/s_texture.cpp > "$W/head_s_texture.cpp"
fn "$W/head_s_texture.cpp" '^std::string s_texture::getCacheName\(const std::string &name\)' '^std::string s_texture::getCacheEntryName' \
    | sed 's/s_texture:://' > "$W/today.inc"
fn "$CODE/src/tools/s_texture.cpp" '^// Return the name the two cache names are cut from' '^std::string s_texture::getCacheEntryName' \
    | sed 's/s_texture:://' > "$W/after.inc"
awk '/^bool CallSystem::isAbsolute/ {on=1} on {print} on && /^}/ {exit}' "$CODE/src/tools/call_system.cpp" > "$W/isabsolute.inc"
cat > "$W/cachekey_cpp.cpp" <<'EOF'
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
struct CallSystem { static bool isAbsolute(const std::string path); };
#include "isabsolute.inc"
namespace today {
#include "today.inc"
}
namespace after {
#include "after.inc"
}
template <class F> static void run(F f, const std::string &n, std::string &st, std::string &v)
{
    try { v = f(n); st = "OK"; } catch (const std::out_of_range &e) { v = e.what(); st = "THROW"; }
}
int main(int argc, char **argv)
{
    const bool t = std::string(argv[1]) == "today";
    std::ifstream in(argv[2], std::ios::binary);
    std::string n, se, e, sc, c;
    while (std::getline(in, n)) {
        if (t) { run(today::getCacheEntryName, n, se, e); run(today::getCacheName, n, sc, c); }
        else { run(after::getCacheEntryName, n, se, e); run(after::getCacheName, n, sc, c); }
        std::cout << n << '\t' << se << '\t' << e << '\t' << sc << '\t' << c << '\n';
    }
}
EOF
g++ -std=c++20 -O2 -Wall -I"$W" -o "$W/cachekey_cpp" "$W/cachekey_cpp.cpp" || { echo "COMPILE FAIL"; exit 2; }
echo "extracted: today.inc $(wc -l < "$W/today.inc") lines, after.inc $(wc -l < "$W/after.inc") lines, isabsolute.inc $(wc -l < "$W/isabsolute.inc") lines"
echo "sources: HEAD $(git -C "$CODE" rev-parse --short=8 HEAD), worktree s_texture.cpp md5 $(md5sum "$CODE/src/tools/s_texture.cpp" | cut -c1-8)"
RC=0
for form in REL ABS PROBES; do
    for pair in "today:today" "after:proposed_G2"; do
        set -- ${pair/:/ }
        "$W/cachekey_cpp" "$1" "$HERE/inputs_$form.txt" > "$W/cpp_$1_$form.tsv"
        if cmp -s "$W/cpp_$1_$form.tsv" "$HERE/$2_$form.tsv"; then v=IDENTICAL; else v=DIFFERENT; RC=1; fi
        echo "$form compiled $1 vs oracle $2: $v ($(wc -l < "$W/cpp_$1_$form.tsv") rows, md5 $(md5sum < "$W/cpp_$1_$form.tsv" | cut -c1-8))"
    done
done
echo "=== COMPILED CROSS-CHECK $([ $RC -eq 0 ] && echo PASS || echo FAIL) ==="
exit $RC
