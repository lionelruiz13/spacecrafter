# The UI gate: what the EDITOR PUTS ON A SCREEN, not what its core computes.
#
# `scedit --ui-selftest` renders a fixed set of (buffer, cursor) cases into an
# off-screen 100x14 terminal and prints, per case, the frame and four masks of
# the caret's row: which cells are DIM (the greyed ghost text — D31's whole
# point is that it is grey, and a ghost drawn in normal ink is a different
# promise), which carry the look-alike-space marker (whose COLUMN must be the
# one the `invisible-separator` message names), which are UNDERLINED (a
# finding's span, at exactly its bytes) and which is INVERTED (the caret: the
# standard SGR inversion, one cell — scedit/INTENT.md §5 item 15(c)).
#
# tests/ui-selftest-expected.txt is a RECORD, like the lint and corpus files:
# every line of it was read and is intended. Changing the layout means editing
# it deliberately, never regenerating it blind.
#
# Inputs: SCEDIT, SRC, EXPECTED.

execute_process(
	COMMAND ${SCEDIT} --grammar ${SRC}/grammar/sc-grammar.json --ui-selftest
	OUTPUT_VARIABLE GOT
	ERROR_VARIABLE ERR
	RESULT_VARIABLE RC)

if(NOT RC EQUAL 0)
	message(FATAL_ERROR "scedit --ui-selftest exited ${RC}: ${ERR}")
endif()
if(NOT ERR STREQUAL "")
	message(FATAL_ERROR "scedit --ui-selftest wrote to stderr: ${ERR}")
endif()

file(READ ${EXPECTED} WANT)
string(STRIP "${GOT}" GOT)
string(STRIP "${WANT}" WANT)
if(NOT GOT STREQUAL WANT)
	message(FATAL_ERROR
		"the rendered frames changed vs ${EXPECTED}.\n--- recorded ---\n${WANT}\n--- got ---\n${GOT}\n")
endif()
message(STATUS "ui gate: rendered frames match ${EXPECTED}")
