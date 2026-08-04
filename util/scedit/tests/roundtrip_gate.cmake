# The round-trip gate: opening a script in the editor and saving it must give
# back the same file, BYTE FOR BYTE. Measured as an MD5 comparison, because that
# is the claim — not "looks the same", not "parses the same".
#
# Inputs: WORKER (the roundtrip_test binary), GRAMMAR, INPUT, TMPDIR.

file(MAKE_DIRECTORY ${TMPDIR})
set(OUT1 ${TMPDIR}/roundtrip-unedited.sts)
set(OUT2 ${TMPDIR}/roundtrip-edited.sts)
file(REMOVE ${OUT1} ${OUT2})

execute_process(
	COMMAND ${WORKER} ${GRAMMAR} ${INPUT} ${OUT1} ${OUT2}
	OUTPUT_VARIABLE OUT
	ERROR_VARIABLE ERR
	RESULT_VARIABLE RC)
message(STATUS "${OUT}")
if(NOT RC EQUAL 0)
	message(FATAL_ERROR "roundtrip worker exited ${RC}: ${ERR}")
endif()

file(MD5 ${INPUT} MD5_IN)
file(MD5 ${OUT1} MD5_OUT)
if(NOT MD5_IN STREQUAL MD5_OUT)
	message(FATAL_ERROR
		"open+save changed the file.\n  ${INPUT}: ${MD5_IN}\n  ${OUT1}: ${MD5_OUT}")
endif()
message(STATUS "round trip: md5 ${MD5_IN} in == out")

# The edited copy must NOT be identical — a gate that would pass on a buffer
# that ignores edits is not a gate.
file(MD5 ${OUT2} MD5_EDITED)
if(MD5_IN STREQUAL MD5_EDITED)
	message(FATAL_ERROR "the edited copy is identical to the input: the edit did not happen")
endif()
