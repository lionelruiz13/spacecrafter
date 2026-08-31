cmake_minimum_required(VERSION 3.16)
project(scedit LANGUAGES CXX)

# Standalone tool -- deliberately NOT wired into the spacecrafter build
# (util/ siblings pattern). Build:
#   cmake -B build && cmake --build build
# Test:
#   cd build && ctest --output-on-failure
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# WARNINGS, ON SCEDIT'S OWN TARGETS ONLY (2026-08-31, F66 scope 7).
# Until now the project set no warning flags at all, so every "clean build, 0
# warnings" claim in a delivery was a criterion that COULD NOT FAIL. It can now.
# Target-scoped rather than global on purpose: third_party/ftxui and
# third_party/nlohmann are vendored upstream trees (README S Vendoring) compiled
# exactly as their authors ship them -- warnings there are neither ours to fix
# nor evidence about our code, and a global add_compile_options() would bury
# ours in theirs. Every target below is one we write; the list is the whole of
# scedit's own code.
set(SCEDIT_WARNINGS -Wall -Wextra)

# Headless core, library first: the TUI slice links this and reads the headers,
# never the .cpp (scedit/INTENT.md C1 + engineering invariant I1).
add_library(scedit_core STATIC
	src/sc_tokenizer.cpp
	src/sc_grammar.cpp
	src/sc_check.cpp
	src/sc_document.cpp
	src/sc_docindex.cpp
	src/sc_docjson.cpp
	src/sc_editcore.cpp
	src/sc_tcpclient.cpp
	src/sc_mcp.cpp)
target_compile_options(scedit_core PRIVATE ${SCEDIT_WARNINGS})
target_include_directories(scedit_core PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# The terminal layer, kept in its own target so the headless core never links
# a UI library and cannot start depending on one (decision D1: FTXUI; vendoring
# record in README.md S Vendoring).
add_subdirectory(third_party/ftxui)
add_library(scedit_tui STATIC src/sc_tui.cpp)
target_compile_options(scedit_tui PRIVATE ${SCEDIT_WARNINGS})
target_link_libraries(scedit_tui PUBLIC scedit_core PRIVATE ftxui::component)

add_executable(scedit src/main.cpp)
target_compile_options(scedit PRIVATE ${SCEDIT_WARNINGS})
target_link_libraries(scedit PRIVATE scedit_core scedit_tui)

enable_testing()

# The MCP gate's client is Python (stdlib only): it has to be a real second
# implementation of the protocol, not this code checking itself.
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Differential self-test: constructed lines with expected tokenizations, one
# per sharp edge of the parse model. No grammar file needed.
add_executable(scedit_tokenizer_test tests/tokenizer_test.cpp)
target_compile_options(scedit_tokenizer_test PRIVATE ${SCEDIT_WARNINGS})
target_link_libraries(scedit_tokenizer_test PRIVATE scedit_core)
add_test(NAME tokenizer COMMAND scedit_tokenizer_test)

# Differential test against a VERBATIM copy of the engine's parseCommand:
# exhaustive short strings over the branch-driving alphabet, plus every line of
# the real corpus. This is what makes C1 a measurement instead of a claim.
add_executable(scedit_parse_oracle_test tests/parse_oracle_test.cpp)
target_compile_options(scedit_parse_oracle_test PRIVATE ${SCEDIT_WARNINGS})
target_link_libraries(scedit_parse_oracle_test PRIVATE scedit_core)
add_test(NAME parse_oracle COMMAND scedit_parse_oracle_test
	${CMAKE_CURRENT_SOURCE_DIR}/../../doc/superscript.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/dual-dump.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/model-generalization.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/fix-validation.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/b10_cmd_script.sts
	${CMAKE_CURRENT_SOURCE_DIR}/../../claude/harness/b16_reload_check.sts)

# The headless interaction core: buffer, cursor->token, completion, doc bar,
# live findings -- every editor behaviour, reached without a terminal. The
# fixture contract exists for the one path the real one cannot arm yet (D31's
# machine-readable default); tests/fixture-grammar.json says so itself.
add_executable(scedit_editcore_test tests/editcore_test.cpp)
target_compile_options(scedit_editcore_test PRIVATE ${SCEDIT_WARNINGS})
target_link_libraries(scedit_editcore_test PRIVATE scedit_core)
add_test(NAME editcore COMMAND scedit_editcore_test
	${CMAKE_CURRENT_SOURCE_DIR}/grammar/sc-grammar.json
	${CMAKE_CURRENT_SOURCE_DIR}/tests/fixture-grammar.json)

# Round-trip gate: open a real ISO-8859, CRLF, 1407-line script in the editor,
# save it untouched, and compare MD5s -- plus one edit that must change exactly
# the line it was made on and nothing else.
add_executable(scedit_roundtrip_test tests/roundtrip_test.cpp)
target_compile_options(scedit_roundtrip_test PRIVATE ${SCEDIT_WARNINGS})
target_link_libraries(scedit_roundtrip_test PRIVATE scedit_core)
add_test(NAME roundtrip
	COMMAND ${CMAKE_COMMAND}
		-DWORKER=$<TARGET_FILE:scedit_roundtrip_test>
		-DGRAMMAR=${CMAKE_CURRENT_SOURCE_DIR}/grammar/sc-grammar.json
		-DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/../../doc/superscript.sts
		-DTMPDIR=${CMAKE_CURRENT_BINARY_DIR}/roundtrip
		-P ${CMAKE_CURRENT_SOURCE_DIR}/tests/roundtrip_gate.cmake)

# UI gate: the frames the editor actually draws, rendered off-screen.
add_test(NAME ui_selftest
	COMMAND ${CMAKE_COMMAND}
		-DSCEDIT=$<TARGET_FILE:scedit>
		-DSRC=${CMAKE_CURRENT_SOURCE_DIR}
		-DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/tests/ui-selftest-expected.txt
		-P ${CMAKE_CURRENT_SOURCE_DIR}/tests/ui_gate.cmake)

# Seed gate: the contract file still validates (unchanged behaviour).
add_test(NAME seed_gate
	COMMAND scedit --grammar ${CMAKE_CURRENT_SOURCE_DIR}/grammar/sc-grammar.json)

# One construct per armed lint id: proves each rule fires, with the right id,
# severity and D6 shape -- and that the "must stay silent" section stays silent.
# NOTE: tests/lint_cases.sts is NOT pure ASCII -- it carries raw 0xA0 bytes for
# the `invisible-separator` cases (one that must fire, one inside a quoted value
# that must not). Anything that "cleans up" that file byte-wise destroys the
# test. Same discipline as the corpus: scripts are read as bytes, never decoded.
add_test(NAME lint_rules
	COMMAND ${CMAKE_COMMAND}
		-DSCEDIT=$<TARGET_FILE:scedit>
		-DSRC=${CMAKE_CURRENT_SOURCE_DIR}
		"-DFILES=${CMAKE_CURRENT_SOURCE_DIR}/tests/lint_cases.sts"
		-DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/tests/lint-expected.txt
		-P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_gate.cmake)

# History gate: `--history` -- the error pane's list, printed. Two inputs, on
# purpose: the lint fixture, whose scedit rows must stay in step with the
# `--check` record above (one reader, two printers -- if they ever disagree, one
# of these two gates goes red), and tests/history_cases.sts, an F63-SHAPED file
# carrying real `#!` tails the ENGINE wrote (scedit composes none). Every row of
# the record has its disposition in a comment beside the bytes that produce it,
# in history_cases.sts itself.
add_test(NAME history_list
	COMMAND ${CMAKE_COMMAND}
		-DSCEDIT=$<TARGET_FILE:scedit>
		-DSRC=${CMAKE_CURRENT_SOURCE_DIR}
		-DMODE=--history
		"-DFILES=${CMAKE_CURRENT_SOURCE_DIR}/tests/lint_cases.sts;${CMAKE_CURRENT_SOURCE_DIR}/tests/history_cases.sts"
		-DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/tests/history-expected.txt
		-P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_gate.cmake)

# JSON check gate: the SAME findings as objects. Runs the same comparator as the
# text record above with `--json` added to the mode, which is the point -- one
# finding set, two printers, and if they ever disagree about what was found, one
# of these two gates goes red.
add_test(NAME check_json
	COMMAND ${CMAKE_COMMAND}
		-DSCEDIT=$<TARGET_FILE:scedit>
		-DSRC=${CMAKE_CURRENT_SOURCE_DIR}
		"-DMODE=--check;--json"
		"-DFILES=${CMAKE_CURRENT_SOURCE_DIR}/tests/lint_cases.sts"
		-DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/tests/check-json-expected.txt
		-P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_gate.cmake)

# Query gate: `--doc` and `--search`, one recorded (arguments -> stdout + exit)
# per line of tests/doc-queries.txt, each line saying in a comment what it pins.
add_test(NAME doc_queries
	COMMAND ${CMAKE_COMMAND}
		-DSCEDIT=$<TARGET_FILE:scedit>
		-DSRC=${CMAKE_CURRENT_SOURCE_DIR}
		-DQUERIES=${CMAKE_CURRENT_SOURCE_DIR}/tests/doc-queries.txt
		-DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/tests/doc-expected.txt
		-P ${CMAKE_CURRENT_SOURCE_DIR}/tests/query_gate.cmake)

# MCP gate: a stdlib-only client spawns `scedit --mcp`, speaks both protocol
# eras at it, calls every registered tool with good and bad arguments, and
# asserts the shapes the fetched specification defines -- including that a `doc`
# the contract file does not have arrives as JSON null through the protocol.
add_test(NAME mcp_protocol
	COMMAND ${CMAKE_COMMAND} -E env
		${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/mcp_gate.py
		$<TARGET_FILE:scedit> ${CMAKE_CURRENT_SOURCE_DIR}/grammar/sc-grammar.json)

# TCP gate: the client that talks to a live engine, checked from BOTH ends
# without one. tests/fake_engine.py is a stand-in whose framing rules are read
# from src/tools/io.cpp line by line; the gate runs one leg per process and then
# asserts what the stand-in RECEIVED, so a leg cannot pass by agreeing with
# itself. The same facts are asserted against the real binary, on a fresh
# launch, by claude/harness/f67_tcp_live.py.
add_executable(scedit_tcpclient_test tests/tcpclient_test.cpp)
target_compile_options(scedit_tcpclient_test PRIVATE ${SCEDIT_WARNINGS})
target_link_libraries(scedit_tcpclient_test PRIVATE scedit_core)
add_test(NAME tcp_client
	COMMAND ${CMAKE_COMMAND} -E env
		${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/tcp_gate.py
		$<TARGET_FILE:scedit_tcpclient_test>)

# PTY gate: the editor's live KEYS, pressed on a pseudo-terminal, with the
# stand-in engine asserting what arrived. This is the seam between the two gates
# on either side of it -- ui_selftest pins what is DRAWN, the live legs pin what
# the core and client DO -- and without it "F8 plays the file" was read from the
# source rather than measured. Control codes rather than function keys: the
# twins exist for terminals that eat F-keys, and they are the same branch.
add_test(NAME pty_keys
	COMMAND ${CMAKE_COMMAND} -E env
		${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/pty_gate.py
		$<TARGET_FILE:scedit> ${CMAKE_CURRENT_SOURCE_DIR}/grammar/sc-grammar.json)

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
