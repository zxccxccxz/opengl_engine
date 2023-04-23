#version 330 core

out vec4 output_color;

in vec3 frag_pos;

uniform samplerCube u_texture;

void main()
{
	output_color = texture(u_texture, frag_pos);
}