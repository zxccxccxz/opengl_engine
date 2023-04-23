#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 v_pos;

void main()
{
	v_pos = vec3(view * model * vec4(pos, 1.0));
	gl_Position = projection * view * model * vec4(pos, 1.0);
}