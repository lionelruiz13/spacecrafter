#!/bin/bash
cd shaders
cd src
for src in *.vert *.frag *.geom *.tesc *.tese
do
    dst="../$src.spv"
    glslc $src -o $dst -I . -fauto-bind-uniforms --target-env="vulkan1.1"
done
