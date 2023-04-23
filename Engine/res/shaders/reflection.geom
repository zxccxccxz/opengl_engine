#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out; // 18 % 2 == 0 must be true

// proj * view, model is set in vertex shader
uniform mat4 u_light_matrices[6]; // Each mat4 for each side of the cube

out vec4 frag_pos;
out vec2 v_tex_coords;

in vec2 s_tex_coords[];

void main()
{
	for (int face = 0; face < 6; ++face)
	{
		gl_Layer = face;
		for (int vertex = 0; vertex < 3; ++vertex)
		{
			frag_pos = gl_in[vertex].gl_Position;
			v_tex_coords = s_tex_coords[vertex];
			gl_Position = u_light_matrices[face] * frag_pos;
			EmitVertex();
		}
		EndPrimitive();
	}
}