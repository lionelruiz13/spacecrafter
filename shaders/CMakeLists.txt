
########### install files ###############

INSTALL(CODE "execute_process(COMMAND \"./compile.sh\" WORKING_DIRECTORY ./)")

if (UNIX)
INSTALL(DIRECTORY compiled/
	DESTINATION share/${PACKAGE}/shaders
)
else()
INSTALL(DIRECTORY compiled/
	DESTINATION shaders
)
endif()
