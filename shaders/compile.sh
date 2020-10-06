#!/bin/bash
cd shaders
cd src
for src in *.vert
do
    dst="../$src.spv"
    glslc $src -o $dst -I . --target-env="vulkan1.1" -fshader-stage=vert
done
for src in *.frag
do
    dst="../$src.spv"
    glslc $src -o $dst -I . --target-env="vulkan1.1" -fshader-stage=frag
done
for src in *.geom
do
    dst="../$src.spv"
    glslc $src -o $dst -I . --target-env="vulkan1.1" -fshader-stage=geom
done
for src in *.tesc
do
    dst="../$src.spv"
    glslc $src -o $dst -I . --target-env="vulkan1.1" -fshader-stage=tesc
done
for src in *.tese
do
    dst="../$src.spv"
    glslc $src -o $dst -I . --target-env="vulkan1.1" -fshader-stage=tese
done
cd ..
./compare.sh
