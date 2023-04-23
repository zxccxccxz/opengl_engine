#include <iostream>
#include "Light.h"
#include "OmniShadowMap.h"

#include "Log.h"

/* common constructor */
Light::Light(GLfloat shadow_width, GLfloat shadow_height,
			glm::vec3 color, GLfloat a_intensity, GLfloat d_intensity)
	: 
	_color(color), _a_intensity(a_intensity), _d_intensity(d_intensity), 
	_light_proj(0.0f)
{
}

/* directional light */
DirectionalLight::DirectionalLight(const DirectionalLightCreateInfo& info)
	: 
	Light(info.shadow_width, info.shadow_height, 
		info.color, info.a_intensity, info.d_intensity), 
	_direction(info.direction)
{
	_shadow_map = std::make_unique<ShadowMap>(info.shadow_width, info.shadow_height);
	_light_proj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.0f);
	_view_proj = _light_proj * glm::lookAt(-_direction, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	LOG_CORE_INFO("[DIRECTIONAL LIGHT] Created");
}

void DirectionalLight::UseLight(Shader& shader)
{
	/* ambient light */
	shader.SetUniformVec3("directional_light.base.color", _color);
	shader.SetUniform1f("directional_light.base.ambient_intensity", _a_intensity);

	/* diffuse light */
	shader.SetUniform1f("directional_light.base.diffuse_intensity", _d_intensity);
	shader.SetUniformVec3("directional_light.direction", _direction);
}

glm::mat4 DirectionalLight::CalcLightTransform()
{
	//return _view_proj;
	return _light_proj * glm::lookAt(-_direction, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

#undef near
#undef far

/* Point light */
GLuint PointLight::_count = 0;
PointLight::PointLight(GLfloat shadow_width, GLfloat shadow_height,
	GLfloat near, GLfloat far,
	glm::vec3 color, GLfloat a_intensity, GLfloat d_intensity,
	glm::vec3 position, GLfloat exp, GLfloat lin, GLfloat con,
	bool is_base_of_spot_light)
	: 
	Light(shadow_width, shadow_height, color, a_intensity, d_intensity),
	_position(position), _exponent(exp), _linear(lin), _constant(con),
	_near_plane(near), _far_plane(far)
{
	if (!is_base_of_spot_light)
	{
		Init();
	}

	GLfloat aspect = shadow_width / shadow_height;
	_light_proj = glm::perspective(glm::radians(90.0f), 1.0f, near, far);
	_shadow_map = std::make_unique<OmniShadowMap>(shadow_width, shadow_height);
	// Texture unit assignment is in Init() of shadowmap 
}
	

void PointLight::Init()
{
	LOG_CORE_TRACE("[POINT LIGHT] Creating");
	_id = _count++;
	_uniform_names.resize(7 + 2);
	std::string id_str = std::to_string(_id);
	_uniform_names[0] = "point_lights[" + id_str + "].base.color";
	_uniform_names[1] = "point_lights[" + id_str + "].base.ambient_intensity";
	_uniform_names[2] = "point_lights[" + id_str + "].base.diffuse_intensity";
	_uniform_names[3] = "point_lights[" + id_str + "].position";
	_uniform_names[4] = "point_lights[" + id_str + "].exponent";
	_uniform_names[5] = "point_lights[" + id_str + "].linear";
	_uniform_names[6] = "point_lights[" + id_str + "].constant";

	_uniform_names[7] = "u_omni_shadow_maps[" + id_str + "].shadow_map";
	_uniform_names[8] = "u_omni_shadow_maps[" + id_str + "].far_plane";

	for (int i = 0; i < 9; ++i)
	{
		LOG_CORE_TRACE(_uniform_names[i].c_str());
	}
	LOG_CORE_INFO("[POINT LIGHT] Created, id: {0}", _id);
}

std::vector<glm::mat4> PointLight::CalcLightTransform()
{
	std::vector<glm::mat4> light_matrices;
	// +X, -X, +Y, -Y, +Z, -Z
	light_matrices.emplace_back(_light_proj * glm::lookAt(_position, _position + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
	light_matrices.emplace_back(_light_proj * glm::lookAt(_position, _position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

	light_matrices.emplace_back(_light_proj * glm::lookAt(_position, _position + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
	light_matrices.emplace_back(_light_proj * glm::lookAt(_position, _position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));

	light_matrices.emplace_back(_light_proj * glm::lookAt(_position, _position + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
	light_matrices.emplace_back(_light_proj * glm::lookAt(_position, _position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

	return light_matrices;
}

void PointLight::UseLight(Shader& shader)
{
	shader.SetUniformVec3(_uniform_names[0], _color);
	shader.SetUniform1f(_uniform_names[1], _a_intensity);
	shader.SetUniform1f(_uniform_names[2], _d_intensity);
	shader.SetUniformVec3(_uniform_names[3], _position);
	shader.SetUniform1f(_uniform_names[4], _exponent);
	shader.SetUniform1f(_uniform_names[5], _linear);
	shader.SetUniform1f(_uniform_names[6], _constant);
	//shader.SetUniform1i(_uniform_names[7], _shadow_map->GetTextureUnit());
	//shader.SetUniform1f(_uniform_names[8], _far_plane);
}

void PointLight::UsePointLight(Shader& shader)
{
	shader.SetUniform1i(_uniform_names[_uniform_names.size() - 2], _shadow_map->GetTextureUnit());
	//LOG_CORE_WARN("USEPOINTLIGHT uniform: {0} txunit: {1}", _uniform_names[_uniform_names.size() - 2], _shadow_map->GetTextureUnit());
	shader.SetUniform1f(_uniform_names[_uniform_names.size() - 1], _far_plane);
}

/* Spot light */
GLuint SpotLight::_count = 0;
SpotLight::SpotLight(GLfloat shadow_width, GLfloat shadow_height,
	GLfloat near, GLfloat far, 
	glm::vec3 color, GLfloat a_intensity, GLfloat d_intensity,
	glm::vec3 position, 
	GLfloat exp, GLfloat lin, GLfloat con,
	glm::vec3 direction, GLfloat edge)
	: PointLight(shadow_width, shadow_height, near, far, color, a_intensity, d_intensity, position, exp, lin, con, true),
	//_position(), _exponent(), _linear(lin), _constant(con),
	_direction(glm::normalize(direction)), _edge(edge), _cos_edge(cos(glm::radians(_edge))),
	_id(_count++)
{
	Init();
}

void SpotLight::Init()
{
	LOG_CORE_TRACE("[SPOT LIGHT] Creating");
	_uniform_names.resize(9 + 2);
	std::string id_str = std::to_string(SpotLight::_id);
	_uniform_names[0] = "spot_lights[" + id_str + "].base.base.color";
	_uniform_names[1] = "spot_lights[" + id_str + "].base.base.ambient_intensity";
	_uniform_names[2] = "spot_lights[" + id_str + "].base.base.diffuse_intensity";
	_uniform_names[3] = "spot_lights[" + id_str + "].base.position";
	_uniform_names[4] = "spot_lights[" + id_str + "].base.exponent";
	_uniform_names[5] = "spot_lights[" + id_str + "].base.linear";
	_uniform_names[6] = "spot_lights[" + id_str + "].base.constant";
	_uniform_names[7] = "spot_lights[" + id_str + "].direction";
	_uniform_names[8] = "spot_lights[" + id_str + "].cos_edge";

	id_str = std::to_string(PointLight::_count + SpotLight::_id);
	_uniform_names[9] =  "u_omni_shadow_maps[" + id_str + "].shadow_map";
	_uniform_names[10] = "u_omni_shadow_maps[" + id_str + "].far_plane";

	for (int i = 0; i < 11; ++i)
	{
		LOG_CORE_TRACE(_uniform_names[i]);
	}
	LOG_CORE_TRACE("cos edge = {0}", _cos_edge);
	LOG_CORE_INFO("[SPOT LIGHT] Created, id: {0}", _id);
}

void SpotLight::UseLight(Shader& shader)
{
	shader.SetUniformVec3(_uniform_names[0], _color);
	shader.SetUniform1f(_uniform_names[1], _a_intensity);
	shader.SetUniform1f(_uniform_names[2], _d_intensity);
	shader.SetUniformVec3(_uniform_names[3], _position);
	shader.SetUniform1f(_uniform_names[4], _exponent);
	shader.SetUniform1f(_uniform_names[5], _linear);
	shader.SetUniform1f(_uniform_names[6], _constant);
	shader.SetUniformVec3(_uniform_names[7], _direction);
	shader.SetUniform1f(_uniform_names[8], _cos_edge);
	// shader.SetUniform1i(_uniform_names[9], _shadow_map->GetTextureUnit());
	// shader.SetUniform1f(_uniform_names[10], _far_plane);
}
