#!/usr/bin/env bash

for i in *Vertex*.glsl; do
    echo "Compiling fragment shader $i as vertex"
    shader="${i%.glsl}"
    glslc -fshader-stage=vertex --target-env=vulkan1.2 $shader.glsl -O -o $shader.spv
    python3 generate_code.py $shader.spv $shader.spv.h ${shader}_spv timeline_vlk
    rm $shader.spv
    echo "Created shader code as ${shader}_spv[] and ${shader}_spv_len for length"
done

for i in *Fragment*.glsl; do
    echo "Compiling fragment shader $i as fragment"
    shader="${i%.glsl}"
    glslc -fshader-stage=fragment --target-env=vulkan1.2 $shader.glsl -O -o $shader.spv
    python3 generate_code.py $shader.spv $shader.spv.h ${shader}_spv timeline_vlk
    rm $shader.spv
    echo "Created shader code as ${shader}_spv[] and ${shader}_spv_len for length"
done

for i in *Compute*.glsl; do
    echo "Compiling compute shader $i as compute"
    shader="${i%.glsl}"
    glslc -fshader-stage=compute --target-env=vulkan1.2 $shader.glsl -O -o $shader.spv
    python3 generate_code.py $shader.spv $shader.spv.h ${shader}_spv timeline_vlk
    rm $shader.spv
    echo "Created shader code as ${shader}_spv[] and ${shader}_spv_len for length"
done
