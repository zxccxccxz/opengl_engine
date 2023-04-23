#version 330 core

out vec4 output_color;

in vec4 v_vertex_color;
in vec2 v_tex_coords;
in vec3 v_normal;
in vec3 v_fragment_pos;
in vec4 v_directional_light_space_pos;

const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 3;

/* Lights */
struct Light 
{
	vec3	color;
	float	ambient_intensity;
	float	diffuse_intensity;
};

struct DirectionalLight 
{
	Light	base;
	vec3	direction;
};

struct PointLight 
{
	Light base;
	vec3 position;
	float exponent;
	float linear;
	float constant;
};

struct SpotLight 
{
	PointLight base;
	vec3 direction;
	float cos_edge;
};

struct OmniShadowMap 
{
	samplerCube shadow_map;
	float far_plane;
};

/* Material */
struct Material 
{
	samplerCube reflection_map;
	sampler2D diffuse_map;
	sampler2D specular_map;
	float specular_intensity;
	float shininess;
};

uniform samplerCube skybox;

/* Uniforms */
uniform int	 point_lights_count;
uniform int	 spot_lights_count;
uniform vec3 eye_pos; // camera position
//uniform sampler2D	texture_sampler; // default is zero
uniform Material	material;
uniform DirectionalLight directional_light;
uniform PointLight	point_lights[MAX_POINT_LIGHTS];
uniform SpotLight	spot_lights[MAX_SPOT_LIGHTS];

uniform sampler2D u_directional_shadow_map;
uniform OmniShadowMap u_omni_shadow_maps[MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS];

vec3 omni_shadow_offset_directions[20] = vec3[]
(
	vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
	vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

/* Functions */
float CalcOmniShadowFactor(PointLight light, int shadow_index)
{
	vec3 light_to_frag = v_fragment_pos - light.position;
	// current depth
	float current = length(light_to_frag);

	float shadow = 0.0;
	float bias = 0.15;
	int samples = 20;
	//float disk_radius = 0.05; // distance in each direction
	float view_distance = length(eye_pos - v_fragment_pos); // dist between frag and camera
	float disk_radius = (1.0 + (view_distance/u_omni_shadow_maps[shadow_index].far_plane)) / 25.0;

	for (int i = 0; i < samples; ++i)
	{
		float closest = texture(u_omni_shadow_maps[shadow_index].shadow_map, light_to_frag + omni_shadow_offset_directions[i] * disk_radius).r;
		closest *= u_omni_shadow_maps[shadow_index].far_plane;

		if (current - bias > closest)
		{
			shadow += 1.0;
		}
	} 

	shadow /= float(samples);

	// closest depth
	//float closest = texture(u_omni_shadow_maps[shadow_index].shadow_map, light_to_frag).r;
	//closest *= u_omni_shadow_maps[shadow_index].far_plane;

	//float bias = 0.15;
	//float shadow = current - bias > closest ? 1.0 : 0.0; // 1.0 here means full shadow

	return shadow;
}

float CalcDirectionalShadowFactor(DirectionalLight light)
{
	// Convert NDC coords to [0, 1], as it is exatly the same coords used by shadow_map
	vec3 directional_light_ndc_pos = v_directional_light_space_pos.xyz / v_directional_light_space_pos.w;
	vec3 proj_coords = (directional_light_ndc_pos * 0.5) + 0.5;
	//vec3 proj_coords = (v_directional_light_space_pos * 0.5) + 0.5;

	// Works with ortho view, does not work with perspective
	// float closest = texture(u_directional_shadow_map, proj_coords.xy).r;
	float current = proj_coords.z;

	// resolve shadow acne
	vec3 normal = normalize(v_normal);
	vec3 light_dir = normalize(light.direction);
	float bias = max(0.005 * (1.0 - dot(normal, light_dir)), 0.0005);

	// PCF
	float shadow = 0.0;
	vec2 texel_size = 1.0 / textureSize(u_directional_shadow_map, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcf_depth = texture(u_directional_shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r;
			shadow += current - bias > pcf_depth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	// current_depth > closest_depth // if current is more far than closest
	// 1 means full shadow
	//float shadow = (current - bias) > closest ? 1.0 : 0.0;

	// far plane of ortho frustum, 
	if(proj_coords.z > 1.0)
	{
		shadow = 0.0;
	}	

	return shadow;
}

/* Lights */
vec4 CalcLightByDirection(Light light, vec3 direction, float shadow_factor)
{
	/* Ambient light  */
	vec4 ambient_color = vec4(light.color, 1.0) * light.ambient_intensity;
	
	/* Diffuse light */
	float diffuse_factor = max(dot(normalize(v_normal), -normalize(direction)), 0.0);
	vec4 diffuse_color = vec4(light.color, 1.0) * light.diffuse_intensity * diffuse_factor;

	/* Specular light */
	vec4 specular_color = vec4(0.0);
	if (diffuse_factor > 0.0)
	{
		vec3 frag_to_eye = normalize(eye_pos - v_fragment_pos);

		// Phong
		vec3 reflected_vertex = normalize(reflect(normalize(direction), normalize(v_normal)));
		float specular_factor = dot(frag_to_eye, reflected_vertex);

		// Blinn
		//vec3 h = normalize(-normalize(direction) + frag_to_eye);
		//float specular_factor = dot(normalize(v_normal), h);


		if (specular_factor > 0.0)
		{
			specular_factor = pow(specular_factor, material.shininess);
			specular_color = vec4(light.color * material.specular_intensity * specular_factor * (texture(material.specular_map, v_tex_coords).rgb), 1.0);
		}
	}

	return (ambient_color + ((1.0 - shadow_factor) * (diffuse_color + specular_color)));
}

vec4 CalcDirectionalLight()
{
	float shadow_factor = CalcDirectionalShadowFactor(directional_light);
	return CalcLightByDirection(directional_light.base, directional_light.direction, shadow_factor);
}

vec4 CalcPointLight(PointLight p_light, int shadow_index)
{
	/* Calculate the direction our light is shining in */
	vec3 direction = v_fragment_pos - p_light.position;
	float dist = length(direction);
	direction = normalize(direction);

	/* Shadows */
	float shadow_factor = CalcOmniShadowFactor(p_light, shadow_index);

	vec4 color = CalcLightByDirection(p_light.base, direction, shadow_factor);
	/* Based on how far away the light is from the fragment we need to change how much it is lit */
	float attenuation = p_light.exponent * dist * dist +
						p_light.linear	 * dist +
						p_light.constant;
	return (color / attenuation);
}

vec4 CalcPointLights()
{
	vec4 result_color = vec4(0.0);
	for (int i = 0; i < point_lights_count; i++)
	{
		result_color += CalcPointLight(point_lights[i], i);
	}
	return result_color;
}

vec4 CalcSpotLight(SpotLight s_light, int shadow_index)
{
	vec3 light_to_frag_ray = normalize(v_fragment_pos - s_light.base.position);
	float sl_factor = dot(light_to_frag_ray, s_light.direction);
	// Use dot product as factor to soften edges
	// But first, scale it to fit range [0, 1]
	float fade = 1 + ((sl_factor - 1) / (1 - s_light.cos_edge));
	//if (light_dir_dot > spot_lights[i].cos_edge)
	return CalcPointLight(s_light.base, shadow_index) * max(fade, 0.0);
}

vec4 CalcSpotLights()
{
	vec4 result_color = vec4(0.0);
	for (int i = 0; i < spot_lights_count; i++)
	{
		result_color += CalcSpotLight(spot_lights[i], point_lights_count + i);
	}
	return result_color;
}

void main()
{
	vec4 final_color = CalcDirectionalLight();
	final_color += CalcPointLights();
	final_color += CalcSpotLights();

	//vec4 final_color = CalcPointLights();

	//vec3 directional_light_ndc_pos = v_directional_light_space_pos.xyz / v_directional_light_space_pos.w;
	//vec3 proj_coords = (directional_light_ndc_pos * 0.5) + 0.5;

	// reflection
	vec3 reflected_dir = reflect(normalize(v_fragment_pos - eye_pos), normalize(v_normal));
//	vec4 reflection_skybox = texture(skybox, reflected_dir);
	vec4 reflection_scene = texture(material.reflection_map, reflected_dir);
	vec4 reflection = reflection_scene;


	output_color = texture(material.diffuse_map, v_tex_coords) * (final_color) + reflection * 0.2;// + vec4(proj_coords, 1.0);

}