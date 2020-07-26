
########### install files ###############

file(
	GLOB
	myShaders
	*.vert
	*.tesc
	*.tese
	*.geom
	*.frag
	*.glsl
	)

INSTALL(FILES ${myShaders} 
	DESTINATION share/${PACKAGE}/shaders
)

