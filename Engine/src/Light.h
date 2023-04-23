#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

#include "Shader.h"

#include "ShadowMap.h"

/* Create Info */
struct DirectionalLightCreateInfo
{
	GLfloat shadow_width;
	GLfloat shadow_height;
	glm::vec3 color;
	GLfloat a_intensity;
	GLfloat d_intensity;
	glm::vec3 direction;
};

/* Light classes */
class Light {
public:
	virtual ~Light() {}
	virtual void UseLight(Shader& shader) = 0;// , std::string ambient_intensity, std::string ambient_color);
	const glm::vec3& GetColor() const { return _color; }

	ShadowMap& GetShadowMap() { return *_shadow_map; }

protected:
	Light(GLfloat shadow_width, GLfloat shadow_height, glm::vec3 color, GLfloat a_intensity, GLfloat d_intensity);
	glm::vec3 _color;
	GLfloat _a_intensity;
	GLfloat _d_intensity;

	/* Shadows */
	glm::mat4 _light_proj;
	std::unique_ptr<ShadowMap> _shadow_map;

};

/* Directional light : ambient & diffuse */
class DirectionalLight : public Light {
public:
	DirectionalLight(const DirectionalLightCreateInfo& info);
	virtual void UseLight(Shader& shader) override;

	glm::mat4 CalcLightTransform();

private:
	glm::vec3 _direction;
	glm::mat4 _view_proj;
};

#undef near
#undef far
/* Point light */
class PointLight : public Light {
public:
	PointLight(GLfloat shadow_width, GLfloat shadow_height, 
		GLfloat near, GLfloat far, 
		glm::vec3 color, GLfloat a_intensity, GLfloat d_intensity,
		glm::vec3 position, GLfloat exp = 0.0f, GLfloat lin = 0.0f, GLfloat con = 1.0f,
		bool is_base_of_spot_light = false);
	virtual ~PointLight() {}

	virtual void UseLight(Shader& shader) override;
	void UsePointLight(Shader& shader);

	std::vector<glm::mat4> CalcLightTransform();

	const glm::vec3& GetPos() const 
	{
		return _position;
	}

	GLfloat GetFarPlane()
	{
		return _far_plane;
	}

protected:
	virtual void Init();

	glm::vec3 _position;

	/* Attenuation */
	GLfloat _exponent, _linear, _constant;

	/* Near, Far plane */
	GLfloat _near_plane, _far_plane;

	/* Uniforms */
	std::vector<std::string> _uniform_names;
	static GLuint _count;
	GLuint _id;
};

/* Spot light */
class SpotLight : public PointLight
{
public:
	SpotLight(GLfloat shadow_width, GLfloat shadow_height,
		GLfloat near, GLfloat far, 
		glm::vec3 color, GLfloat a_intensity, GLfloat d_intensity,
		glm::vec3 position, GLfloat exp, GLfloat lin, GLfloat con, 
		glm::vec3 direction, GLfloat edge);
	~SpotLight() {}

	virtual void UseLight(Shader& shader) override;

	// TODO:
	glm::vec3& GetDiretion() {
		return _direction;
	}
	glm::vec3& GetPos() {
		return _position;
	}

private:
	virtual void Init() override;

	//glm::vec3 _position;
	/* Attenuation */
	//GLfloat _exponent, _linear, _constant;

	glm::vec3 _direction;
	
	/* Angle */
	GLfloat _edge, _cos_edge;

	/* Uniforms */
	//std::string _uniform_names[9];
	static GLuint _count;
	const GLuint _id;
};