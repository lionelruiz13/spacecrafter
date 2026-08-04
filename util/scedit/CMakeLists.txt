cmake_minimum_required(VERSION 3.16)
project(scedit LANGUAGES CXX)

# Standalone tool — deliberately NOT wired into the spacecrafter build
# (util/ siblings pattern). Build:
#   cmake -B build && cmake --build build
# Test:
#   cd build && ctest --output-on-failure
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Headless core, library first: the TUI slice links this and reads the headers,
# never the .cpp (scedit/INTENT.md C1 + engineering invariant I1).
add_library(scedit_core STATIC
	src/sc_tokenizer.cpp
	src/sc_grammar.cpp
	src/sc_check.cpp)
target_include_directories(scedit_core PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/third_party)

add_executable(scedit src/main.cpp)
target_link_libraries(scedit PRIVATE scedit_core)

enable_testing()

# Differential self-test: constructed lines with expected tokenizations, one
# per sharp edge of the parse model. No grammar file needed.
add_executable(scedit_tokenizer_test tests/tokenizer_test.cpp)
target_link_libraries(scedit_tokenizer_test PRIVATE scedit_core)
add_test(NAME tokenizer COMMAND scedit_tokenizer_test)

# Differential test against a VERBATIM copy of the engine's parseCommand:
# exhaustive short strings over the branch-driving alphabet, plus every line of
# the real corpus. This is what makes C1 a measurement instead of a claim.
add_executable(scedit_parse_oracle_test tests/parse_oracle_test.cpp)
target_link_libraries(scedit_parse_oracle_test PRIVATE scedit_core)
add_test(NAME parse_oracle COMMAND scedit_parse_oracle_test
	${CMAKE_CURRENT_SOURCE_DIR}/../../doc/superscript.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/dual-dump.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/model-generalization.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/fix-validation.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/b10_cmd_script.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/b16_reload_check.sts)

# Seed gate: the contract file still validates (unchanged behaviour).
add_test(NAME seed_gate
	COMMAND scedit --grammar ${CMAKE_CURRENT_SOURCE_DIR}/grammar/sc-grammar.json)

# One construct per armed lint id: proves each rule fires, with the right id,
# severity and D6 shape — and that the "must stay silent" section stays silent.
add_test(NAME lint_rules
	COMMAND ${CMAKE_COMMAND}
		-DSCEDIT=$<TARGET_FILE:scedit>
		-DSRC=${CMAKE_CURRENT_SOURCE_DIR}
		"-DFILES=${CMAKE_CURRENT_SOURCE_DIR}/tests/lint_cases.sts"
		-DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/tests/lint-expected.txt
		-P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_gate.cmake)

# C3 corpus gate: the engine-exercised scripts must stay clean of NEW findings.
# Boundary [measured 2026-08-04]: ~/.spacecrafter/scripts/ is EMPTY on this
# machine, so the shipped-scripts half of C3 cannot run here; it re-arms by
# adding those files to this list once the data package is installed.
set(SCEDIT_CORPUS
	${CMAKE_CURRENT_SOURCE_DIR}/../../doc/superscript.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/b10_cmd_script.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/b16_reload_check.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/dual-dump.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/fix-validation.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/model-generalization.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/artifacts/f27/pre/f27_script_get.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/artifacts/f27/pre_final/f27_script_get.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/artifacts/f27/post/f27_script_get.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/artifacts/f27/post_final/f27_script_get.sts)
add_test(NAME corpus_gate
	COMMAND ${CMAKE_COMMAND}
		-DSCEDIT=$<TARGET_FILE:scedit>
		-DSRC=${CMAKE_CURRENT_SOURCE_DIR}
		"-DFILES=${SCEDIT_CORPUS}"
		-DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/tests/corpus-expected.txt
		-P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_gate.cmake)
