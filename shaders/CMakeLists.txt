
########### install files ###############

file(
	GLOB
	myShaders
	*.spv
	)

INSTALL(FILES ${myShaders} 
	DESTINATION share/${PACKAGE}/shaders
)

