# The comparator for the QUERY surfaces (`--doc`, `--search`): a recorded
# (arguments -> exit code + stdout) table.
#
# WHY NOT check_gate.cmake. That one runs ONE scedit invocation over a list of
# FILES and treats any exit above 1 as a failure -- the right contract for
# `--check`/`--history`, and the wrong one here: these gates need MANY
# invocations with different arguments, none of which are file names, and the
# EXIT CODE is part of what is recorded (a `--doc` of a name that does not exist
# must answer with a JSON error object on stdout AND exit 2; a gate that treated
# that as a failure could not record it).
#
# Inputs: SCEDIT (binary), SRC (util/scedit dir), QUERIES (a file, one argument
# line per query, `#` comments and blank lines skipped), EXPECTED (the record).
#
# Contract asserted for every query, beyond the recorded bytes: the product goes
# to stdout and stderr stays EMPTY. A machine consumer reads one stream.
#
# EXPECTED is a RECORD, not a silencer: every byte of it is a deliberate answer,
# and a change to any of them fails the gate until someone re-records it on
# purpose.

file(READ ${QUERIES} RAW)
string(REPLACE "\n" ";" LINES "${RAW}")
set(GOT "")
foreach(line ${LINES})
	string(STRIP "${line}" line)
	if(line STREQUAL "" OR line MATCHES "^#")
		continue()
	endif()
	separate_arguments(ARGV UNIX_COMMAND "${line}")
	execute_process(
		COMMAND ${SCEDIT} --grammar ${SRC}/grammar/sc-grammar.json ${ARGV}
		OUTPUT_VARIABLE OUT
		ERROR_VARIABLE ERR
		RESULT_VARIABLE RC)
	if(NOT ERR STREQUAL "")
		message(FATAL_ERROR "scedit ${line} wrote to stderr: ${ERR}")
	endif()
	string(APPEND GOT "$ scedit ${line}\n${OUT}exit ${RC}\n")
endforeach()

file(READ ${EXPECTED} WANT)
string(STRIP "${GOT}" GOT)
string(STRIP "${WANT}" WANT)
if(NOT GOT STREQUAL WANT)
	message(FATAL_ERROR
		"scedit query output changed vs ${EXPECTED}.\n--- recorded ---\n${WANT}\n--- got ---\n${GOT}\n")
endif()
message(STATUS "query gate: output matches ${EXPECTED}")
