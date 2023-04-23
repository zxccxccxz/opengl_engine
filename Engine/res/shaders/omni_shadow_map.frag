#version 330 core

in vec4 frag_pos;

uniform vec3 u_light_pos;
uniform float u_far_plane;

void main()
{
	float dist = length(frag_pos.xyz - u_light_pos);
	dist = dist / u_far_plane; // dist [0; 1], assumes near plane is roughly == zero
	gl_FragDepth = dist;
}