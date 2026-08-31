# One comparator for every gate that runs scedit over files and compares the
# printed lines against a record (I2: the corpus gate, the rules fixture and the
# history gate must not drift apart by having three of these).
#
# Inputs: SCEDIT (binary), SRC (util/scedit dir), FILES (';'-list, missing files
# are skipped), EXPECTED (recorded output, paths relative to the repo root),
# MODE (the subcommand: `--check` by default, `--history` for the error pane's
# list). Every mode this comparator serves must print its product on stdout,
# leave stderr empty, and exit 0 clean / 1 with content / >1 on failure.
#
# EXPECTED is a RECORD, not a silencer: every line in it is dispositioned in the
# dispatch report, and a finding that appears without being recorded there fails
# the gate — which is the point of C3.

if(NOT DEFINED MODE)
	set(MODE "--check")
endif()

set(PRESENT "")
foreach(f ${FILES})
	if(EXISTS ${f})
		list(APPEND PRESENT ${f})
	endif()
endforeach()
if(PRESENT STREQUAL "")
	message(FATAL_ERROR "no input file present out of: ${FILES}")
endif()

execute_process(
	COMMAND ${SCEDIT} --grammar ${SRC}/grammar/sc-grammar.json ${MODE} ${PRESENT}
	OUTPUT_VARIABLE OUT
	ERROR_VARIABLE ERR
	RESULT_VARIABLE RC)

if(NOT ERR STREQUAL "")
	message(FATAL_ERROR "scedit ${MODE} reported an error: ${ERR}")
endif()
if(RC GREATER 1)
	message(FATAL_ERROR "scedit ${MODE} exited ${RC} (usage or I/O failure)")
endif()

file(READ ${EXPECTED} WANT)
string(REPLACE "${SRC}/../../" "" GOT "${OUT}")
string(REPLACE "${SRC}/" "" GOT "${GOT}")
string(STRIP "${GOT}" GOT)
string(STRIP "${WANT}" WANT)
if(NOT GOT STREQUAL WANT)
	message(FATAL_ERROR
		"scedit ${MODE} output changed vs ${EXPECTED}.\n--- recorded ---\n${WANT}\n--- got ---\n${GOT}\n")
endif()
message(STATUS "check gate: ${MODE} output matches ${EXPECTED}")
