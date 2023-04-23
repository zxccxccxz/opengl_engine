#version 330 core

layout (location = 0) in vec3 pos;

out vec3 frag_pos;

//uniform mat4 view;
//uniform mat4 projection;
uniform mat4 view_proj;

void main()
{
	//frag_pos = pos;
	//gl_Position = projection * view * vec4(pos, 1.0);
	frag_pos = vec3(view_proj * vec4(pos, 1.0));
	gl_Position = vec4(pos, 1.0);
}