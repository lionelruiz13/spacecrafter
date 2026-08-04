# One comparator for every `scedit --check` gate (I2: the corpus gate and the
# rules fixture must not drift apart by having two of these).
#
# Inputs: SCEDIT (binary), SRC (util/scedit dir), FILES (';'-list, missing files
# are skipped), EXPECTED (recorded findings, paths relative to the repo root).
#
# EXPECTED is a RECORD, not a silencer: every line in it is dispositioned in the
# dispatch report, and a finding that appears without being recorded there fails
# the gate — which is the point of C3.

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
	COMMAND ${SCEDIT} --grammar ${SRC}/grammar/sc-grammar.json --check ${PRESENT}
	OUTPUT_VARIABLE OUT
	ERROR_VARIABLE ERR
	RESULT_VARIABLE RC)

if(NOT ERR STREQUAL "")
	message(FATAL_ERROR "scedit --check reported an error: ${ERR}")
endif()
if(RC GREATER 1)
	message(FATAL_ERROR "scedit --check exited ${RC} (usage or I/O failure)")
endif()

file(READ ${EXPECTED} WANT)
string(REPLACE "${SRC}/../../" "" GOT "${OUT}")
string(REPLACE "${SRC}/" "" GOT "${GOT}")
string(STRIP "${GOT}" GOT)
string(STRIP "${WANT}" WANT)
if(NOT GOT STREQUAL WANT)
	message(FATAL_ERROR
		"findings changed vs ${EXPECTED}.\n--- recorded ---\n${WANT}\n--- got ---\n${GOT}\n")
endif()
message(STATUS "check gate: findings match ${EXPECTED}")
