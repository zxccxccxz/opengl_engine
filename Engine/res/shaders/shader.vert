#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 u_directional_light_transform;

out vec4 v_vertex_color;
out vec2 v_tex_coords;
out vec3 v_normal;
out vec3 v_fragment_pos;

out vec4 v_directional_light_space_pos; // position of fragment relative to the light

void main()
{
	gl_Position = projection * view * model * vec4(pos, 1.0);
	/* Convert this to NDC */
	v_directional_light_space_pos = u_directional_light_transform * model * vec4(pos, 1.0);
	//v_directional_light_space_pos = directional_light_space_pos.xyz / directional_light_space_pos.w;

	v_vertex_color = vec4(clamp(pos, 0.0, 1.0), 1.0);

	v_tex_coords = tex;

	// interpolate normals
	// mat 3 because we dont want to account in any translations because normal is just a direction
	// to invert scale: transpose(inverse(model)) 
	// it is needed for accounting in non-uniform scale f.e. scale_by(0.7, 0.2)
	v_normal = mat3(transpose(inverse(model))) * normal;

	v_fragment_pos = (model * vec4(pos, 1.0)).xyz;
}