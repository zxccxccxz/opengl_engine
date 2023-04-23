#include "Material.h"

Material::Material(GLfloat specular_intensity, GLfloat shininess)
	: _specular_intensity(specular_intensity), _shininess(shininess)
{
}

Material::~Material()
{
}

void Material::UseMaterial(Shader& shader)
{
	shader.SetUniform1f("material.specular_intensity", _specular_intensity);
	shader.SetUniform1f("material.shininess", _shininess);
}


