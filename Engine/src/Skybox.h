#pragma once

#include <vector>
#include <memory>
#include <string>
#include <glad/glad.h>
#include <stb_image.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Mesh.h"
#include "Shader.h"

class Skybox
{
public:
	Skybox(const std::vector<std::string>& face_locations);
	//Skybox();
	~Skybox();

	void BindTexture();
	void Render(const glm::mat4& view_matrix, const glm::mat4& projection_matrix);

private:
	std::unique_ptr<Mesh> _mesh;
	std::unique_ptr<Shader> _skybox_shader;

	GLuint _texture;

};