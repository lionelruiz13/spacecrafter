#!/bin/bash
# F84 -- the scratch install recipe: take a clean clone of the code repo through
# INSTALL / install_src.sh as written, with NO sudo and NO write outside $ROOT.
#
#     ./f84_install.sh [ROOT]          # default ROOT=/home/claude/sc-f84
#
# It is a RECIPE, not a gate: every step prints what it did and the script
# continues, because the interesting output is where the documented path
# BREAKS.  Exit code is the number of documented steps that failed.
#
# WHAT IT REPRODUCES (INTENT Sec.11.204, 2026-09-05):
#   1. `git clone --recurse-submodules` fails -- the submodule pin 7ce58350 is a
#      local unpushed commit and is on no remote branch (Sec.5.131).  The clone
#      here therefore uses --reference against THIS host's copy, which is
#      exactly what a newcomer does not have.
#   2. install_src.sh's own `git submodule update --init || git clone ...` fails
#      in BOTH branches, and the line has no `|| exit`.
#   3. The configure line the script builds is shown with BUILD unset both as
#      the script writes it and with the -z fix, so CMAKE_BUILD_TYPE is seen
#      EMPTY (-> CMakeLists.txt forces Debug) and then Release.
#   4. `cmake --install` into a scratch prefix, with NO sudo, and the manifest.
#   5. The install prefix does NOT reach the binary: CONFIG_DATA_DIR is a
#      hardcoded #define (src/spacecrafter.hpp:44), so the installed program
#      still reads /usr/local/share/spacecrafter/.
#
# WHAT IT DOES NOT DO: no sudo, no apt, no write to ~/.spacecrafter or
# /usr/local, no launch.  Launches are harness/f84_coldhome.py's job.
set -u
ROOT="${1:-/home/claude/sc-f84}"
SRC="${F84_SRC:-/home/claude/spacecrafter}"
PREFIX="$ROOT/prefix"
CLONE="$ROOT/spacecrafter"
JOBS="${F84_JOBS:-$(nproc)}"
RC=0
step() { printf '\n=== %s ===\n' "$*"; }
note() { printf '    %s\n' "$*"; }
bad()  { printf '    FAILED (documented step): %s\n' "$*"; RC=$((RC+1)); }

mkdir -p "$ROOT"

step "0. baselines (asserted again at the end)"
md5sum "$HOME/.spacecrafter/config.ini" "$HOME/.spacecrafter/ssystem.ini" > "$ROOT/.f84_home_before"
{ find /usr/local/share/spacecrafter -type f -print0 | sort -z | xargs -0 md5sum
  md5sum /usr/local/bin/spacecrafter; } > "$ROOT/.f84_usrlocal_before" 2>/dev/null
note "home:      $(md5sum "$ROOT/.f84_home_before" | cut -d' ' -f1)"
note "/usr/local: $(md5sum "$ROOT/.f84_usrlocal_before" | cut -d' ' -f1) over $(wc -l < "$ROOT/.f84_usrlocal_before") entries"

step "1. the DOCUMENTED clone (expected to fail on the submodule)"
rm -rf "$CLONE"
# NOT `git ... | sed`: an `if` on a pipeline reads the LAST command's status, so
# the sed would swallow git's 128 and the headline failure would score 0.
# (Measured on this script's own first self-test -- the same class as the
# grep/pipefail hazard in harness/README.md.)
git clone --recurse-submodules "$SRC" "$CLONE" > "$ROOT/clone.log" 2>&1
CLONE_RC=$?
sed 's/^/    /' "$ROOT/clone.log"
if [ "$CLONE_RC" = 0 ]; then
    note "recursive clone returned 0"
else
    bad "git clone --recurse-submodules (rc $CLONE_RC)"
fi
if [ -f "$CLONE/src/EntityCore/Core/VulkanMgr.hpp" ]; then
    note "submodule content IS present -- the pin has been pushed; delete the"
    note "KNOWN ISSUE paragraph in INSTALL section 3."
else
    note "submodule content ABSENT: Sec.5.131 still holds."
    note "pin      : $(git -C "$SRC" ls-tree HEAD src/EntityCore | awk '{print $3}')"
    note "remote   : $(git ls-remote https://github.com/Calvin-Ruiz/EntityCore.git 2>/dev/null | awk '$2=="refs/heads/main"{print $1}')"
    step "1b. the workaround a newcomer does NOT have: --reference this host"
    rm -rf "$CLONE"
    git clone -q "$SRC" "$CLONE" || bad "plain clone"
    git -C "$CLONE" submodule update --init --reference "$SRC/src/EntityCore" 2>&1 | sed 's/^/    /'
fi
note "superproject: $(git -C "$CLONE" rev-parse --short HEAD)   submodule: $(git -C "$CLONE" submodule status | awk '{print $1}')"

step "2. the configure line install_src.sh builds, BOTH forms"
cd "$CLONE" || exit 9
for form in as-written fixed; do
    rm -fr build; mkdir build; cd build || exit 9
    unset BUILD
    if [ "$form" = as-written ]; then
        [ -n "${BUILD-}" ] && BUILD=Release      # install_src.sh:25 as shipped
    else
        [ -z "${BUILD-}" ] && BUILD=Release      # the one-character fix
    fi
    note "$form: BUILD='${BUILD-<unset>}'  ->  cmake .. -DCMAKE_BUILD_TYPE=${BUILD-}"
    cmake .. -DCMAKE_BUILD_TYPE=${BUILD-} -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        > "$ROOT/configure-$form.log" 2>&1 || bad "configure ($form)"
    grep -h 'Build type' "$ROOT/configure-$form.log" | sed 's/^/        /'
    cd "$CLONE" || exit 9
done

step "3. build (Release, the fixed form) -- $JOBS jobs"
free -g | sed -n '2p' | sed 's/^/    /'
cmake --build build -j"$JOBS" > "$ROOT/build.log" 2>&1 || bad "cmake --build"
note "binary: $(md5sum build/src/spacecrafter 2>/dev/null | cut -d' ' -f1)"
note "pending after: $(cmake --build build -- -n 2>&1 | grep -c 'Building CXX') compile steps"
note "VideoSurfaceTexture symbols: $(nm -C build/src/spacecrafter 2>/dev/null | grep -c VideoSurfaceTexture)"

step "4. install into the scratch prefix -- NO sudo"
rm -rf "$PREFIX"
cmake --install build --config Release > "$ROOT/install.log" 2>&1 || bad "cmake --install"
note "$(find "$PREFIX" -type f | wc -l) files installed"
{ printf 'path\tbytes\tmd5\n'
  find "$PREFIX" -type f -print0 | sort -z | while IFS= read -r -d '' f; do
      printf '%s\t%s\t%s\n' "${f#$PREFIX/}" "$(stat -c %s "$f")" "$(md5sum "$f" | cut -d' ' -f1)"
  done; } > "$ROOT/install-manifest.tsv"
awk -F'\t' 'NR>1{n=$1; sub(/\/[^\/]*$/,"",n); c[n]++} END{for(k in c) printf "    %-34s %5d files\n", k, c[k]}' \
    "$ROOT/install-manifest.tsv" | sort

step "5. what the tree install does NOT carry (against the field data root)"
for d in stars textures; do
    [ -d "$PREFIX/share/spacecrafter/$d" ] && note "$d: installed" \
        || note "$d: ABSENT from the install, present in the field root with $(find /usr/local/share/spacecrafter/$d -type f 2>/dev/null | wc -l) files"
done
for d in audio fonts ftp landscapes language model3D scripts sky_cultures videos; do
    [ -d "$PREFIX/share/spacecrafter/data/$d" ] && note "data/$d: installed" \
        || note "data/$d: ABSENT from the install, present in the field root with $(find /usr/local/share/spacecrafter/data/$d -type f 2>/dev/null | wc -l) files"
done
note "shaders aggregate md5, installed vs field:"
note "  $( (cd "$PREFIX/share/spacecrafter/shaders" && find . -type f -print0 | sort -z | xargs -0 md5sum) | md5sum | cut -d' ' -f1)"
note "  $( (cd /usr/local/share/spacecrafter/shaders  && find . -type f -print0 | sort -z | xargs -0 md5sum) | md5sum | cut -d' ' -f1)"

step "6. the prefix does not reach the binary"
grep -n 'define CONFIG_DATA_DIR' "$CLONE/src/spacecrafter.hpp" | sed 's/^/    /'
note "-> the installed binary reads that path, not \$PREFIX; harness/f84_coldhome.py"
note "   prints the applog's 'ROOT   DIR:' line, which is the proof."

step "7. baselines re-asserted"
md5sum "$HOME/.spacecrafter/config.ini" "$HOME/.spacecrafter/ssystem.ini" > "$ROOT/.f84_home_after"
{ find /usr/local/share/spacecrafter -type f -print0 | sort -z | xargs -0 md5sum
  md5sum /usr/local/bin/spacecrafter; } > "$ROOT/.f84_usrlocal_after" 2>/dev/null
cmp -s "$ROOT/.f84_home_before" "$ROOT/.f84_home_after" \
    && note "~/.spacecrafter unchanged" || bad "~/.spacecrafter MOVED"
cmp -s "$ROOT/.f84_usrlocal_before" "$ROOT/.f84_usrlocal_after" \
    && note "/usr/local unchanged" || bad "/usr/local MOVED"

printf '\n=== %d documented step(s) failed ===\n' "$RC"
exit "$RC"
