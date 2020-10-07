#!/bin/bash
cd src
echo "================ VERTEX SHADER ================"
for src in *.vert
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1" -fshader-stage=vert
done
echo "==============================================="
echo "=============== FRAGMENT SHADER ==============="
for src in *.frag
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1" -fshader-stage=frag
done
echo "==============================================="
echo "=============== GEOMETRY SHADER ==============="
for src in *.geom
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1" -fshader-stage=geom
done
echo "==============================================="
echo "========== TESSELATION CONTROL SHADER ========="
for src in *.tesc
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1" -fshader-stage=tesc
done
echo "==============================================="
echo "======== TESSELATION EVALUATION SHADER ========"
for src in *.tese
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1" -fshader-stage=tese
done
echo "==============================================="
