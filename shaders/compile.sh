#!/bin/bash
cd src
if [ -z "$1" ]
then
echo "================ VERTEX SHADER ================"
for src in *.vert
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1"
done
echo "==============================================="
echo "=============== FRAGMENT SHADER ==============="
for src in *.frag
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1"
done
echo "==============================================="
echo "=============== GEOMETRY SHADER ==============="
for src in *.geom
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1"
done
echo "==============================================="
echo "========== TESSELATION CONTROL SHADER ========="
for src in *.tesc
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1"
done
echo "==============================================="
echo "======== TESSELATION EVALUATION SHADER ========"
for src in *.tese
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1"
done
echo "==============================================="
echo "======== COMPUTE SHADER ========"
for src in *.comp
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1"
done
echo "==============================================="
  exit 0
fi
for src in $@
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1"
done
