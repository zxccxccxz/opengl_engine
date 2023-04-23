#version 330 core

out vec4 o_color;
in float depth;

void main()
{
	float color =  ((depth * 0.5) + 0.5) * 3.0;
	o_color = vec4(color, color, color, 1.0);
}