#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 normal;

uniform mat4 model;
out vec2 s_tex_coords;

void main()
{
	s_tex_coords = tex;
	gl_Position = model * vec4(pos, 1.0);
}