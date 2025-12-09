#!/bin/bash
failed_compilations=0
failed_compilations_list=()
force_compile=false
if [ "$1" == "-f" ] || [ "$1" == "--force" ] || [ "$1" == "-force" ] # Short / Long / Windows style
then
    force_compile=true
    shift
fi

cd src
if [ -z "$1" ]
then
echo "================ VERTEX SHADER ================"
for src in *.vert
do
    dst="../compiled/$src.spv"
    if [ $(date -r $src +%s) != "$(date -r $dst +%s)" ] || [ "$force_compile" = true ]
    then
        echo "compile $src"
        glslc $src -o $dst -I . --target-env="vulkan1.1" && touch -cmt "$(date -r $dst +%Y%m%d%H%M.%S)" $src
        if [ $? -ne 0 ];
        then
            failed_compilations=$((failed_compilations + 1))
            failed_compilations_list+=("$src")
        fi
    fi
done
echo "==============================================="
# echo "=============== FRAGMENT SHADER ==============="
# for src in *.frag
# do
#     dst="../compiled/$src.spv"
#     if [ $(date -r $src +%s) != "$(date -r $dst +%s)" ] || [ "$force_compile" = true ]
#     then
#         echo "compile $src"
#         glslc $src -o $dst -I . --target-env="vulkan1.1" && touch -cmt "$(date -r $dst +%Y%m%d%H%M.%S)" $src
#         if [ $? -ne 0 ];
#         then
#             failed_compilations=$((failed_compilations + 1))
#             failed_compilations_list+=("$src")
#         fi
#     fi
# done
# echo "==============================================="
echo "=============== GEOMETRY SHADER ==============="
for src in *.geom
do
    dst="../compiled/$src.spv"
    if [ $(date -r $src +%s) != "$(date -r $dst +%s)" ] || [ "$force_compile" = true ]
    then
        echo "compile $src"
        glslc $src -o $dst -I . --target-env="vulkan1.1" && touch -cmt "$(date -r $dst +%Y%m%d%H%M.%S)" $src
        if [ $? -ne 0 ];
        then
            failed_compilations=$((failed_compilations + 1))
            failed_compilations_list+=("$src")
        fi
    fi
done
echo "==============================================="
# echo "========== TESSELATION CONTROL SHADER ========="
# for src in *.tesc
# do
#     dst="../compiled/$src.spv"
#     if [ $(date -r $src +%s) != "$(date -r $dst +%s)" ] || [ "$force_compile" = true ]
#     then
#         echo "compile $src"
#         glslc $src -o $dst -I . --target-env="vulkan1.1" && touch -cmt "$(date -r $dst +%Y%m%d%H%M.%S)" $src
#         if [ $? -ne 0 ];
#         then
#             failed_compilations=$((failed_compilations + 1))
#             failed_compilations_list+=("$src")
#         fi
#     fi
# done
# echo "==============================================="
echo "======== TESSELATION EVALUATION SHADER ========"
for src in *.tese
do
    dst="../compiled/$src.spv"
    if [ $(date -r $src +%s) != "$(date -r $dst +%s)" ] || [ "$force_compile" = true ]
    then
        echo "compile $src"
        glslc $src -o $dst -I . --target-env="vulkan1.1" && touch -cmt "$(date -r $dst +%Y%m%d%H%M.%S)" $src
        if [ $? -ne 0 ];
        then
            failed_compilations=$((failed_compilations + 1))
            failed_compilations_list+=("$src")
        fi
    fi
done
echo "==============================================="
# echo "======== COMPUTE SHADER ========"
# for src in *.comp
# do
#     dst="../compiled/$src.spv"
#     if [ $(date -r $src +%s) != "$(date -r $dst +%s)" ] || [ "$force_compile" = true ]
#     then
#         echo "compile $src ($(date -r $src +%s) != $(date -r $dst +%s)"
#         glslc $src -o $dst -I . --target-env="vulkan1.1" && touch -cmt "$(date -r $dst +%Y%m%d%H%M.%S)" $src
#         if [ $? -ne 0 ];
#         then
#             failed_compilations=$((failed_compilations + 1))
#             failed_compilations_list+=("$src")
#         fi
#     fi
# done
# echo "==============================================="
if [ $failed_compilations -ne 0 ]
then
    echo "$failed_compilations shader compilations failed:"
    for failed in "${failed_compilations_list[@]}";
    do
        echo " - $failed"
    done
    exit 1
else
    echo "All shader compilations succeeded."
    exit 0
fi
fi

# One by one compilation
for src in $@
do
    dst="../compiled/$src.spv"
    echo "compile $src"
    glslc $src -o $dst -I . --target-env="vulkan1.1"
done
