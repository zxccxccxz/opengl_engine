#pragma once
#include <glad/glad.h>
#include "Shader.h"
// describes surface properties

class Material {
public:
	Material(GLfloat specular_intensity, GLfloat shininess);

	void UseMaterial(Shader& shader);

	~Material();

private:
	GLfloat _specular_intensity;
	GLfloat _shininess; // specular_power

};