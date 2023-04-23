#version 330 core

out vec4 output_color;

in vec4 frag_pos;
in vec2 v_tex_coords;

uniform sampler2D u_texture;

void main()
{
	frag_pos;
	output_color = texture(u_texture, v_tex_coords);
	//output_color = vec4(1.0, 0.0, 0.0, 1.0);
}