#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 Color[];
layout(location = 0) out vec3 Coloring;

void main()
{
	vec4 pos1, pos2, pos3;

	pos1 = gl_in[0].gl_Position;
	pos2 = gl_in[1].gl_Position;
	pos3 = gl_in[2].gl_Position;

	if ( pos1.w * pos2.w * pos3.w == 1.0) {

        Coloring = Color[0];
        gl_Position = pos1;
        EmitVertex();

		Coloring = Color[1];
        gl_Position = pos2;
        EmitVertex();

		Coloring = Color[2];
        gl_Position = pos3;
        EmitVertex();

        EndPrimitive();
    }
}
