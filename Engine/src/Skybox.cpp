#include "Skybox.h"

#include "Log.h"
#include "Renderer.h"

Skybox::Skybox(const std::vector<std::string>& faces_fp)
	: _skybox_shader(std::make_unique<Shader>("res/shaders/skybox.vert", "res/shaders/skybox.frag")),
	_mesh(std::make_unique<Mesh>())
{
	LOG_CORE_TRACE("[SKYBOX] Initializing");
	/* Texture */
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	int width, height, bit_depth;
	for (size_t i = 0; i < 6; ++i)
	{
		unsigned char* texture_data = stbi_load(faces_fp[i].c_str(), &width, &height, &bit_depth, 0);
		if (!texture_data)
		{
			LOG_CORE_ERROR("[stbi_image] Failed to load file\n");
			return;
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
		stbi_image_free(texture_data);
		LOG_CORE_TRACE("[Skybox] Loaded texture {0}, {1}", i, faces_fp[i]);
	}

	glCheckError();
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	/* Mesh */
	//GLfloat skybox_vertices[] =
	//{
	//	-1.0f,  1.0f,  1.0f, 0.f,0.f,0.f,0.f,0.f,
	//	 1.0f,  1.0f,  1.0f, 0.f,0.f,0.f,0.f,0.f,
	//	-1.0f, -1.0f,  1.0f, 0.f,0.f,0.f,0.f,0.f,
	//	 1.0f, -1.0f,  1.0f, 0.f,0.f,0.f,0.f,0.f,
	//
	//	-1.0f,  1.0f, -1.0f, 0.f,0.f,0.f,0.f,0.f,
	//	-1.0f, -1.0f, -1.0f, 0.f,0.f,0.f,0.f,0.f,
	//	 1.0f,  1.0f, -1.0f, 0.f,0.f,0.f,0.f,0.f,
	//	 1.0f, -1.0f, -1.0f, 0.f,0.f,0.f,0.f,0.f
	//};
	//

	//GLuint skybox_indices[] =
	//{
	//	// front
	//	0, 1, 2,
	//	2, 1, 3,
	//	// right
	//	2, 3, 5,
	//	5, 3, 7,
	//	// back
	//	5, 7, 4,
	//	4, 7, 6,
	//	// left
	//	4, 6, 0,
	//	0, 6, 1,
	//	// up
	//	4, 0, 5,
	//	5, 0, 2,
	//	// down
	//	1, 6, 3,
	//	3, 6, 7
	//};

	// Use triangle almost at far plane instead
	GLuint skybox_indices[] =
	{
		0, 1, 2
	};
	static GLfloat skybox_vertices[] =
	{
		-1.0f, -1.0f,  0.999f, 0.f,0.f,0.f,0.f,0.f,
		-1.0f,  3.0f,  0.999f, 0.f,0.f,0.f,0.f,0.f,
		 3.0f, -1.0f,  0.999f, 0.f,0.f,0.f,0.f,0.f
	};

	_mesh->CreateMesh(skybox_vertices, skybox_indices, std::size(skybox_vertices), std::size(skybox_indices));

	LOG_CORE_INFO("[SKYBOX] Initialized");
}

void Skybox::BindTexture()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);
}

void Skybox::Render(const glm::mat4& view_matrix, const glm::mat4& projection_matrix)
{

	_skybox_shader->Validate();
	_skybox_shader->Bind();
	glm::mat4 view_proj = glm::inverse(projection_matrix * glm::mat4(glm::mat3(view_matrix)));

	_skybox_shader->SetUniformMatrix4fv("view_proj", view_proj);
	//_skybox_shader->SetUniform1i("u_texture", 4);

	glDepthMask(GL_FALSE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);

	_mesh->RenderMesh();

	glDepthMask(GL_TRUE);
	_skybox_shader->Unbind();
}

Skybox::~Skybox()
{
}