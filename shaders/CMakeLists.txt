
########### install files ###############

INSTALL(CODE "execute_process(COMMAND \"./compile.sh\" WORKING_DIRECTORY ./)")

INSTALL(DIRECTORY compiled/
	DESTINATION share/${PACKAGE}/shaders
)

