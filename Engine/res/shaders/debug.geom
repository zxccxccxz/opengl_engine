#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vec3 v_pos[];

uniform mat4 projection;

vec3 GetTriangleMiddle() // (centroid?)
{
	vec3 mid2 = (v_pos[0] + v_pos[1]) / 2.0;
	vec3 mid0 = (v_pos[1] + v_pos[2]) / 2.0;
	vec3 mid1 = (v_pos[0] + v_pos[2]) / 2.0;

	float s = (((v_pos[2].y - v_pos[0].y) * (mid2.x - v_pos[2].x)) + ((mid2.y - v_pos[2].y) * (v_pos[0].x - v_pos[2].x))) 
	/ (((mid0.y - v_pos[0].y) * (mid2.x - v_pos[2].x)) - ((mid0.x - v_pos[0].x) * (mid2.y - v_pos[2].y)));

	return v_pos[2] + (s * (mid2 - v_pos[2]));
}

void main()
{
	// output triangle
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	EndPrimitive();

	// output normal
	vec3 m = GetTriangleMiddle();
	gl_Position = projection * vec4(m, 1.0);
	EmitVertex();
	vec3 a = v_pos[0] - m;
	vec3 b = v_pos[1] - m;
	vec3 n = cross(b, a);
	gl_Position = projection* vec4(m + normalize(n), 1.0);
	EmitVertex();
	EndPrimitive();
}