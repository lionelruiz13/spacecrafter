//
//	body_Hints - batched variant (new-path HINT service family)
//
// Per-vertex color replaces bodyHints' per-draw push constant so circles of
// DIFFERENT colors batch into one LINE_LIST draw (Renderer batching service,
// PipelineFamily.hpp BatchDesc - the dissolution of the DrawHelper hint seam).
// Position stays render-space pixels against the shared cam block, exactly
// like bodyHints.vert.

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0) in vec2 position;
layout (location=1) in vec4 color;
#include <cam_block_only.glsl>

layout (location=0) out vec4 vColor;

void main()
{
	gl_Position = MVP2D * vec4(position,0.0,1.0);
	vColor = color;
}
