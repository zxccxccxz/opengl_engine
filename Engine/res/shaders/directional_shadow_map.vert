#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 u_directional_light_transform; // point of view of light: projection * view

out float depth;

void main()
{
	vec4 pos = u_directional_light_transform * model * vec4(pos, 1.0);
	depth = (pos.xyz / pos.w).z;
	gl_Position = pos;
}