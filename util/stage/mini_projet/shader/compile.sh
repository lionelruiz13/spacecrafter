cd shader
glslc new.vert -o vert.spv
glslc new.frag -o frag.spv
glslc experiment.vert -o experiment_vert.spv -O
glslc experiment.frag -o experiment_frag.spv -O
glslc experiment.geom -o experiment_geom.spv -O
