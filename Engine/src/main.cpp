#include <stdio.h>
#include <string.h>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Log.h"
#include "MWindow.h"
#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "Timestep.h"
#include "Texture.h"
#include "Light.h"
#include "Material.h"

#include "Model.h"

#include "Renderer.h"

#include "OmniShadowMap.h"

#include "Skybox.h"

#include "ReflectionMap.h"

#include "Line.h"

/* OpenGL stuff */
glm::mat4 projection(0.0f);

std::vector<std::unique_ptr<Mesh>> mesh_v;
//std::vector<std::unique_ptr<Shader>> shaders_v;
std::unique_ptr<Shader> main_shader, lightsource_shader, directional_shadow_map_shader, omni_shadow_map_shader, reflection_shader, debug_shader;
// Remember about MAX_POINT_LIGHTS
std::vector<std::unique_ptr<Light>> lights_v;
std::vector<std::unique_ptr<Material>> materials_v;
//std::vector<std::unique_ptr<PointLight>> point_lights_v;

/* Camera */
std::unique_ptr<Camera> camera;

/* Skybox */
std::unique_ptr<Skybox> skybox;

/* Textures */
Texture pudge_texture("res/textures/pudge.png");
Texture brick_texture("res/textures/brick.jpg");
Texture metal_texture("res/textures/metal1.jpg");
Texture plain_texture("res/textures/plain.png");
Texture container_texture("res/textures/container2.png");
Texture container_texture_specular("res/textures/container2_specular.png");

//Texture pudge_texture("res/textures/plain.png");
//Texture brick_texture("res/textures/plain.png");
//Texture metal_texture("res/textures/plain.png");
//Texture plain_texture("res/textures/plain.png");
//Texture container_texture("res/textures/plain.png");
//Texture container_texture_specular("res/textures/plain.png");

/* Models */
Model car_model;
Model portalgun_model;

/* Other vars */
int direction = 1;
float triangleOffset = 0.0f;
const float triangleMaxOffset = 0.7f;
const float triangleIncrement = 0.005f;

/* Main shader filepaths */
const char* vs_fp = "res/shaders/shader.vert";
const char* fs_fp = "res/shaders/shader.frag";

void CreateShaders()
{
	main_shader = std::make_unique<Shader>(vs_fp, fs_fp);

	lightsource_shader = std::make_unique<Shader>
		("res/shaders/lightsource.vert", "res/shaders/lightsource.frag");
	
	directional_shadow_map_shader = std::make_unique<Shader>
		("res/shaders/directional_shadow_map.vert", "res/shaders/directional_shadow_map.frag");

	omni_shadow_map_shader = std::make_unique<Shader>
		("res/shaders/omni_shadow_map.vert", "res/shaders/omni_shadow_map.frag", "res/shaders/omni_shadow_map.geom");

	reflection_shader = std::make_unique<Shader>
		("res/shaders/reflection.vert", "res/shaders/reflection.frag", "res/shaders/reflection.geom");

	debug_shader = std::make_unique<Shader>
		("res/shaders/debug.vert", "res/shaders/debug.frag", "res/shaders/debug.geom");

}

void CalcAvgNormals(unsigned int* indices, unsigned int indices_count,
					GLfloat* vertices, unsigned int vertices_count,
					unsigned int vertex_data_count, unsigned int normal_offset)
{
	// assume indices_count % 3 == 0
	for (size_t i = 0; i < indices_count; i += 3)
	{
		unsigned int idx0 = indices[i] * vertex_data_count;
		unsigned int idx1 = indices[i + 1] * vertex_data_count;
		unsigned int idx2 = indices[i + 2] * vertex_data_count;
		glm::vec3 first_edge
		(
			vertices[idx1]		- vertices[idx0], 
			vertices[idx1 + 1]	- vertices[idx0 + 1], 
			vertices[idx1 + 2]	- vertices[idx0 + 2]
		);
		glm::vec3 second_edge
		(
			vertices[idx2]		- vertices[idx0], 
			vertices[idx2 + 1]	- vertices[idx0 + 1], 
			vertices[idx2 + 2]	- vertices[idx0 + 2]
		);

		glm::vec3 normal = glm::normalize(glm::cross(second_edge, first_edge));

		// shift indices to set normal values
		idx0 += normal_offset; idx1 += normal_offset; idx2 += normal_offset;
		vertices[idx0] += normal.x; vertices[idx0 + 1] += normal.y; vertices[idx0 + 2] += normal.z;
		vertices[idx1] += normal.x; vertices[idx1 + 1] += normal.y; vertices[idx1 + 2] += normal.z;
		vertices[idx2] += normal.x; vertices[idx2 + 1] += normal.y; vertices[idx2 + 2] += normal.z;
	}

	/* normalize found average normals */
	for (size_t i = 0; i < vertices_count / vertex_data_count; ++i)
	{
		unsigned int idx = i * vertex_data_count + normal_offset;
		glm::vec3 normalized = glm::normalize(glm::vec3(vertices[idx], vertices[idx + 1], vertices[idx + 2]));
		vertices[idx]		= normalized.x;
		vertices[idx + 1]	= normalized.y;
		vertices[idx + 2]	= normalized.z;
	}
}

void CreateSphere()
{
	constexpr float pi = glm::pi<float>();
	const int segments_count = 72;
	const int stacks_count = 24;
	const float radius = 1.0f;
	float length_inv = 1.0f / radius;

	float y_angle_inc = pi / (float)stacks_count;
	float xz_angle_inc = (2.0f * pi) / (float)segments_count;

	/*std::cout << "[CREATING SPHERE]\nstacks: " << stacks_count << " (" << y_angle_inc << ")\nsegments: " <<
		segments_count << " (" << xz_angle_inc << ")" << std::endl;*/
	LOG_CORE_TRACE("[SPHERE]: Creating sphere: {0} stacks, {1} segments", stacks_count, segments_count);

	static std::vector<GLfloat> vertices;

	float y_angle = 0.0f, xz_angle = 0.0f;
	GLfloat y, xz;
	GLfloat x, z;
	for (int i = 0; i <= stacks_count; ++i)
	{
		y_angle = pi / 2 - i * y_angle_inc;
		if (y_angle <= -pi * 2)
			y_angle += pi * 2;
		y = radius * sin(y_angle);
		xz = radius * cos(y_angle);
		if (abs(y_angle) == pi / 2) xz = 0;

		for (int j = 0; j <= segments_count; ++j)
		{
			/*std::cout << "\n\t y_anlge: " << y_angle << " xz_angle: " << xz_angle;
			std::cout << "\n\t (" <<
				(cos(glm::radians(y_angle)) * cos(glm::radians(xz_angle))) << ", " <<
				(sin(glm::radians(y_angle))) << ", " <<
				(cos(glm::radians(y_angle)) * sin(glm::radians(xz_angle))) << ")\n";*/
				/*GLfloat x = cos(y_angle) * cos(xz_angle);
				GLfloat y = sin(y_angle);
				GLfloat z = cos(y_angle) * sin(xz_angle);*/
			xz_angle = j * xz_angle_inc;
			if (xz_angle >= 2 * pi) xz_angle -= 2 * pi;
			x = xz * cos(xz_angle);
			z = xz * sin(xz_angle);
			vertices.emplace_back(x);
			//if (y == -1)
				//vertices.emplace_back(-2);
			//else
			vertices.emplace_back(y);
			vertices.emplace_back(z);
			// Push tex coords
			float s = (float)j / segments_count;
			float t = (float)i / stacks_count;
			vertices.emplace_back(s);
			vertices.emplace_back(t);
			// Push normals
			vertices.emplace_back(x * length_inv);
			vertices.emplace_back(y * length_inv);
			vertices.emplace_back(z * length_inv);
		}
	}

	/* Create indices
		k1--k1+1
		|  / |
		| /  |
		k2--k2+1
	*/
	static std::vector<unsigned int> indices;
	unsigned int k1, k2;

	for (int i = 0; i < stacks_count; ++i)
	{
		k1 = i * (segments_count + 1);
		k2 = k1 + segments_count + 1;

		for (int j = 0; j < segments_count; ++j, ++k1, ++k2)
		{
			if (i != 0)
			{
				indices.emplace_back(k1);
				indices.emplace_back(k1 + 1);
				indices.emplace_back(k2);
			}

			if (i != stacks_count - 1)
			{
				indices.emplace_back(k1 + 1);
				indices.emplace_back(k2);
				indices.emplace_back(k2 + 1);
			}
		}
	}

	//for (int i = 0; i < indices.size(); i += 3)
	//{
		//std::cout << indices[i] << " " << indices[i + 1] << " " << indices[i + 2] << "\n";
	//}
	
	mesh_v.emplace_back(std::make_unique<Mesh>())->CreateMesh
	(vertices.data(), indices.data(), vertices.size(), indices.size());

	LOG_CORE_INFO("[SPHERE] created");
}

void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2 
	};

	static GLfloat vertices[] = {
		// x      y      z        u      v        nx     ny     nz
		-1.0f,  -1.0f,  -0.6f,	  0.0f,  0.0f,    0.0f,  0.0f,  0.0f,
		 0.0f,  -1.0f,   1.0f,    0.5f,  0.0f,	  0.0f,  0.0f,  0.0f,
		 1.0f,  -1.0f,  -0.6f,    1.0f,  0.0f,    0.0f,  0.0f,  0.0f,
		 0.0f,   1.0f,   0.0f,    0.5f,  1.0f,    0.0f,  0.0f,  0.0f
	};

	unsigned int floor_indices[] = {
		0, 1, 2,
		2, 1, 3
	};

	static GLfloat floor_vertices[] = { // top left, top right, bot left, bot right
		-10.0f,  0.0f, -10.0f,		0.00f,  0.00f,		0.0f,  1.0f, 0.0f,
		 10.0f,  0.0f, -10.0f,		10.0f,  0.00f,		0.0f,  1.0f, 0.0f,
		-10.0f,  0.0f,  10.0f,		0.00f,  10.0f,		0.0f,  1.0f, 0.0f,
		 10.0f,  0.0f,  10.0f,		10.0f,  10.0f,		0.0f,  1.0f, 0.0f
	};
	
	unsigned int cube_indices[] = {
		//// front
		//1, 0, 2,
		//1, 2, 3,
		//// back
		//4, 5, 7,
		//4, 7, 6,
		//// right
		//5, 1, 3,
		//5, 3, 7,
		//// left
		//0, 4, 6,
		//0, 6, 2,
		//// up
		//5, 4, 0,
		//5, 0, 1,
		//// down
		//3, 2, 6,
		//3, 6, 7
		// front
		1, 0, 2,
		1, 2, 3,
		// back
		4, 5, 7,
		4, 7, 6,
		// right
		5, 1, 3,
		5, 3, 7,
		// left
		0, 4, 6,
		0, 6, 2,
		// up
		11, 10, 8,
		11, 8, 9,
		// down
		13, 12, 14,
		13, 14, 15
	};

	static GLfloat cube_vertices[] = { 
		// near top left 0, near top right 1, near bot left 2, near bot right 3
		// near
		-1.0f,  1.0f,  1.0f,		0.0f,   1.0f,		0.0f,  0.0f,  0.0f,
		 1.0f,  1.0f,  1.0f,		1.0f,   1.0f,		0.0f,  0.0f,  0.0f,
		-1.0f, -1.0f,  1.0f,		0.0f,   0.0f,		0.0f,  0.0f,  0.0f,
		 1.0f, -1.0f,  1.0f,		1.0f,   0.0f,		0.0f,  0.0f,  0.0f,
		// far top left 4, far top right 5, far bot left 6, far bot right 7
		-1.0f,  1.0f, -1.0f,		1.0f,   1.0f,		0.0f,  0.0f,  0.0f,
		 1.0f,  1.0f, -1.0f,		0.0f,   1.0f,		0.0f,  0.0f,  0.0f,
		-1.0f, -1.0f, -1.0f,		1.0f,   0.0f,		0.0f,  0.0f,  0.0f,
		 1.0f, -1.0f, -1.0f,		0.0f,   0.0f,		0.0f,  0.0f,  0.0f,
		 // top (8 - 11)
		-1.0f,  1.0f,  1.0f,		0.0f,   1.0f,		0.0f,  0.0f,  0.0f,
		 1.0f,  1.0f,  1.0f,		1.0f,   1.0f,		0.0f,  0.0f,  0.0f,
		-1.0f,  1.0f, -1.0f,		0.0f,   0.0f,		0.0f,  0.0f,  0.0f,
		 1.0f,  1.0f, -1.0f,		1.0f,   0.0f,		0.0f,  0.0f,  0.0f,
		 // down (12 - 15)
		-1.0f, -1.0f,  1.0f,		0.0f,   1.0f,		0.0f,  0.0f,  0.0f,
		 1.0f, -1.0f,  1.0f,		1.0f,   1.0f,		0.0f,  0.0f,  0.0f,
		-1.0f, -1.0f, -1.0f,		0.0f,   0.0f,		0.0f,  0.0f,  0.0f,
		 1.0f, -1.0f, -1.0f,		1.0f,   0.0f,		0.0f,  0.0f,  0.0f
	};
	for (int i = 0; i < std::size(cube_vertices); i += 8)
	{
		glm::vec3 coords(cube_vertices[i], cube_vertices[i + 1], cube_vertices[i + 2]);
		coords = glm::normalize(coords);
		int idx = i + 5;
		cube_vertices[idx] = coords.x;
		cube_vertices[idx + 1] = coords.y;
		cube_vertices[idx + 2] = coords.z;
	}


	/* calculate normals */
	CalcAvgNormals(indices, 12, vertices, 32, 8, 5);
	
	const int obj_count = 3;

	for (int i = 0; i < obj_count - 1; ++i)
	{
		mesh_v.emplace_back(std::make_unique<Mesh>())->CreateMesh(vertices, indices, 32, 12);
	}

	mesh_v.emplace_back(std::make_unique<Mesh>())->CreateMesh
	(floor_vertices, floor_indices, std::size(floor_vertices), std::size(floor_indices));

	mesh_v.emplace_back(std::make_unique<Mesh>())->CreateMesh
	(cube_vertices, cube_indices, std::size(cube_vertices), std::size(cube_indices));

	/*for (int i = 0; i < obj_count; ++i)
	{
		m->CreateMesh(vertices, indices, 32, 12);
	}*/
}

void CreateLights()
{
	/* Directional light */
	/*lights_v.emplace_back(std::make_unique<DirectionalLight>
		(glm::vec3(1.0f, 1.0f, 1.0f), 0.2f, 1.0f, glm::vec3(5.0f, -2.0f, -3.0f))
	);*/
	glm::vec3 plight_color = glm::vec3(51.0f, 51.0f, 153.0f) / 255.0f;
	DirectionalLightCreateInfo dlight_info;
	// (1024, 1024, glm::vec3(1.0f, 1.0f, 1.0f), 0.1f, 0.2f, 
	dlight_info.shadow_width = 1024;
	dlight_info.shadow_height = 1024;
	dlight_info.color = glm::vec3(1.0f);
	dlight_info.a_intensity = 0.1f;
	dlight_info.d_intensity = 0.2f;
	dlight_info.direction = glm::vec3(0.0f, -10.0f, -6.0f);
	lights_v.emplace_back(std::make_unique<DirectionalLight>(dlight_info));

	GLint point_lights_count = 0;
	GLint spot_lights_count = 0;

	/* Point lights */
	// MAKE SURE NOT TO ADD MORE THAN MAX_POINT_LIGHTS
	lights_v.emplace_back(std::make_unique<PointLight>
		(256, 256, 0.01f, 100.0f,
		 plight_color, 0.0f, 0.8f, 
		 glm::vec3(2.0f, 2.0f, 1.0f), 0.032f, 0.09f, 1.0f)
	);
	lights_v.emplace_back(std::make_unique<PointLight>
		(256, 256, 0.01f, 100.0f,
		 glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, 0.7f, 
		 glm::vec3(-4.f, 2.0f, 0.0f), 0.1f, 0.2f, 0.3f)
	);
	point_lights_count = 2;

	/* Spot lights*/
	// MAKE SURE NOT TO ADD MORE THAN MAX_SPOT_LIGHTS
	// flashlight
	lights_v.emplace_back(std::make_unique<SpotLight>
		(256, 256, 0.01f, 100.0f,	
		 glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, 1.0f,
		 glm::vec3(0.0f, 0.0f, 5.0f), 0.032f, 0.09f, 1.0f,
		 glm::vec3(0.0f, -1.0f, 0.0f), 12.0f));
	// rotating light
	lights_v.emplace_back(std::make_unique<SpotLight>
		(256, 256, 0.01f, 100.0f, 
		 glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, 2.0f,
		 glm::vec3(0.0f, -1.5f, 0.0f), 0.0f, 0.0f, 1.0f,
		 glm::vec3(-100.0f, -1.0f, 0.0f), 30.0f));
	spot_lights_count = 2;

	/* Set lights count uniforms */
	main_shader->Bind();
	main_shader->SetUniform1i("point_lights_count", point_lights_count);
	main_shader->SetUniform1i("spot_lights_count", spot_lights_count);

	main_shader->SetUniform1i("material.diffuse_map", 1);
	main_shader->SetUniform1i("material.specular_map", 2);
	main_shader->SetUniform1i("u_directional_shadow_map", 3);

	for (int i = 1; i < lights_v.size(); ++i)
	{
		PointLight& l = dynamic_cast<PointLight&>(*lights_v[i]);
		//auto& m = (OmniShadowMap&)l.GetShadowMap();
		//m.Read();
		//main_shader->SetUniform1i("u_omni_shadow_maps[" + std::to_string(i - 1) + "].shadow_map", (i - 1 + 4));
		//LOG_CORE_WARN("USEPOINTLIGHT uniform: {0} txunit: {1}", "u_omni_shadow_maps[" + std::to_string(i - 1) + "].shadow_map", 4+i-1);
		l.UsePointLight(*main_shader);
	}
	// TODO: REMOVE

	LOG_CORE_TRACE("\nVALIDATE");
	main_shader->Validate();

	main_shader->Unbind();

	/*SpotLight(glm::vec3(0.0f, 1.0f, 0.0f), 0.2f, 1.0f,
		glm::vec3(0.0f, 2.0f, 0.0f), 0.1f, 0.2f, 0.3f,
		glm::vec3(0.0f, 0.0f, 0.0f), 45);*/
}

void LoadTextures()
{
	pudge_texture.LoadTexture(true, true);
	brick_texture.LoadTexture(false, false);
	plain_texture.LoadTexture(false, false);
	container_texture.LoadTexture(true, false);
	container_texture_specular.LoadTexture(true, false);
}

void CreateMaterials()
{
	materials_v.emplace_back(std::make_unique<Material>(1.0f, 32.0f)); // 0.3f, 4.0f
	materials_v.emplace_back(std::make_unique<Material>(0.3f, 4.00f));
	materials_v.emplace_back(std::make_unique<Material>(4.0f, 255.00f));
}

void LoadModels()
{
	car_model.LoadModel("res/models/car.obj");
	portalgun_model.LoadModel("res/models/PortalGun.obj");
}

float angle = 0.0f;

/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */

void RenderScene(bool is_shadow, Shader& shader);

std::vector<glm::mat4> CalcPointTransforms(const glm::vec3& pos)
{
	static glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
	std::vector<glm::mat4> dir_matrices;
	// +X, -X
	dir_matrices.emplace_back(proj * glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	dir_matrices.emplace_back(proj * glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	// +Y, -Y
	dir_matrices.emplace_back(proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	dir_matrices.emplace_back(proj * glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	// +Z, -Z
	dir_matrices.emplace_back(proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	dir_matrices.emplace_back(proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

	return dir_matrices;
}

bool draw_sphere = true;
void ReflectionPass(const glm::vec3& pos)
{
	static ReflectionMap rmap(512);
	glViewport(0, 0, 512, 512);

	/*main_shader->Bind();
	main_shader->SetUniform1i("material.reflection_map", 9);
	main_shader->Unbind();*/

	reflection_shader->Validate();
	reflection_shader->Bind();
	reflection_shader->SetUniform1i("u_texture", 1);
	glCheckError();

	rmap.BindFramebuffer();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	reflection_shader->SetUniformMatrix4fv("model", glm::mat4(1.0f));
	std::vector<glm::mat4> matrices = CalcPointTransforms(pos);
	for (int i = 0; i < 6; ++i)
	{
		std::string uniform_name = "u_light_matrices[" + std::to_string(i) + "]";
		reflection_shader->SetUniformMatrix4fv(uniform_name, matrices[i]);
	}
	glCheckError();

	rmap.BindTexture();
	glCheckError();

	reflection_shader->Unbind();

	draw_sphere = false;
	RenderScene(true, *reflection_shader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DirectionalShadowMapPass()
{
	// todo: loop over directional lights
	directional_shadow_map_shader->Bind();
	directional_shadow_map_shader->Validate();

	DirectionalLight& light = *(DirectionalLight*)(&(*lights_v[0]));

	glViewport(0, 0, light.GetShadowMap().GetShadowWidth(), light.GetShadowMap().GetShadowHeight());

	// Write mode
	light.GetShadowMap().Write();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Clear any depth buffer if present
	glClear(GL_DEPTH_BUFFER_BIT);

	directional_shadow_map_shader->SetUniformMatrix4fv
	("u_directional_light_transform", light.CalcLightTransform());

	directional_shadow_map_shader->Unbind();

	RenderScene(true, *directional_shadow_map_shader);
	// Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OmniShadowMapPass()
{
	for (auto& l : lights_v)
	{
		PointLight* plight = dynamic_cast<PointLight*>(&(*(l)));
		if (plight)
		{
			//LOG_CORE_TRACE("OmniSHadowMapPass");
			PointLight& light = *plight;
			// ??
			
			glViewport(0, 0, light.GetShadowMap().GetShadowWidth(), light.GetShadowMap().GetShadowHeight());

			((OmniShadowMap&)light.GetShadowMap()).Write();
			glClear(GL_DEPTH_BUFFER_BIT);

			omni_shadow_map_shader->Bind();
			// Uniforms
			omni_shadow_map_shader->SetUniformVec3("u_light_pos", light.GetPos());
			omni_shadow_map_shader->SetUniform1f("u_far_plane", light.GetFarPlane());
			std::vector<glm::mat4> light_matrices = light.CalcLightTransform();
			for (int i = 0; i < 6; ++i)
			{
				std::string uniform_name = "u_light_matrices[" + std::to_string(i) + "]";
				//LOG_CORE_TRACE("un: {0}", uniform_name);
				omni_shadow_map_shader->SetUniformMatrix4fv(uniform_name, light_matrices[i]);
			}
			
			((OmniShadowMap&)light.GetShadowMap()).Read();

			//omni_shadow_map_shader->Validate();
			omni_shadow_map_shader->Unbind();

			RenderScene(true, *omni_shadow_map_shader);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
}

glm::vec3 clear_color = glm::vec3(25.0f, 25.0f, 25.0f) / 255.0f;

void RenderPass(MWindow& window)
{
	/* Lightsource shader uniforms */
	lightsource_shader->Bind();
	lightsource_shader->SetUniformMatrix4fv("view", camera->GetViewMatrix());
	lightsource_shader->SetUniformMatrix4fv("projection", projection);
	lightsource_shader->Unbind();


	/* Light rotation & flashlight */
	glm::vec3& dir = (*(SpotLight*)&(*lights_v[3])).GetDiretion();
	glm::vec3& pos = (*(SpotLight*)&(*lights_v[3])).GetPos();
	//dir.z = -5.0f;
	dir = camera->GetFront();
	dir *= 1.1f;
	dir.y -= 0.2f;
	dir = glm::normalize(dir);
	pos = camera->GetPosition();
	pos.y -= 0.5f;
	// static float angle = 0.0f;
	angle += 1.0f;
	if (angle > 360.0f)
		angle -= 360.0f;
	glm::vec3& dir2 = (*(SpotLight*)&(*lights_v[4])).GetDiretion();
	glm::vec3& pos2 = (*(SpotLight*)&(*lights_v[4])).GetPos();
	static glm::vec3 s_dir = dir2;
	glm::mat4 m(1.0f);
	m = glm::rotate(m, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec4 t = m * glm::vec4(s_dir, 1.0f);
	dir2 = glm::normalize(glm::vec3(t.x, t.y, t.z));
	/* some logic */
	if (abs(triangleOffset) > triangleMaxOffset)
	{
		direction *= -1;
	}

	triangleOffset += triangleIncrement * direction;

	/* Bind main shader & set uniforms */
	main_shader->Bind();

	/* Use lights */
	for (auto& light : lights_v)
	{
		light->UseLight(*main_shader);
	}

	/* Set uniform cam_pos(eye_pos) */
	main_shader->SetUniformVec3("eye_pos", camera->GetPosition());

	/* View & Projection matrices */
	main_shader->SetUniformMatrix4fv("view", camera->GetViewMatrix());
	main_shader->SetUniformMatrix4fv("projection", projection);

	/* Set texture uniforms */
	main_shader->SetUniform1i("material.diffuse_map", 1); // by defaul is zero
	main_shader->SetUniform1i("material.specular_map", 2);
	main_shader->SetUniform1i("u_directional_shadow_map", 3);

	/* viewport */
	glViewport(0, 0, 1366, 768);
	window.ClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);

	/* Set dlight transform */
	DirectionalLight& dlight = *(DirectionalLight*)&(*lights_v[0]);
	main_shader->SetUniformMatrix4fv("u_directional_light_transform", dlight.CalcLightTransform());

	// Read mode
	dlight.GetShadowMap().Read(GL_TEXTURE3);

	main_shader->Unbind();

	debug_shader->Bind();

	debug_shader->SetUniformMatrix4fv("view", camera->GetViewMatrix());
	debug_shader->SetUniformMatrix4fv("projection", projection);

	debug_shader->Unbind();

	/* Render Scene */
	RenderScene(false, *main_shader);
}

int fps_counter = 0;

std::unique_ptr<Lines> lines;

int main(int argc, char* argv[])
{
	/* Init logger */
	Log::Init();
	LOG_CORE_TRACE("[STARTING SETUP]");

	/* Create glfw window */
	MWindow window(1366, 768); // 1280, 1024 // 1024, 768

	/* Camera */
	camera = std::make_unique<Camera>
		(window, glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 0.5f);

	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);*/
	// glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	/* OpenGl stuff */
	CreateObjects();
	CreateSphere();
	
	/*projection = glm::perspective(
		45.0f, (GLfloat)window.GetBufferWidth() / (GLfloat)window.GetBufferHeight(), 0.1f, 100.0f
	);*/
	{
		float a = (GLfloat)window.GetBufferWidth() / (GLfloat)window.GetBufferHeight();
		constexpr float n = 0.1f;
		constexpr float f = 100.0f;
		constexpr float fovy = glm::radians(60.0f);

#if 0
		float fov_tan = tan(fovy / 2.0f);
		float t = n * fov_tan;
		float r = a * t;

		projection[0][0] = n / r;
		projection[1][1] = n / t;
		projection[2][2] = (f + n) / (n - f);
		projection[3][2] = (2.0f * f * n) / (n - f);
		projection[2][3] = -1.0f;
#else

		float far = f;
		float near = n;
		float halftan = tan(fovy / 2.0f);

		projection[0][0] = 1.0f / (halftan * a);
		projection[1][1] = 1.0f / halftan;
		//projection[2][2] = 2.0f / (far - near);
		//projection[3][2] = (far + near) / (far - near) * -1.0f;
		projection[2][2] = (far + near) / (near - far);
		projection[3][2] = (2.0f * far * near) / (near - far);
		projection[2][3] = -1.0f;
#endif
		
	}

	/* timestep */
	Timestep time;

	/* Shaders */
	CreateShaders();

	/* Lighting */
	CreateLights();

	/* Textures */
	LoadTextures();

	/* Materials */
	CreateMaterials();

	/* Models */
	LoadModels();

	/* Skybox */
	std::vector<std::string> faces = {
		"res/textures/skybox/east.jpg",  "res/textures/skybox/west.jpg", 
		"res/textures/skybox/top.jpg",   "res/textures/skybox/bottom.jpg",
		"res/textures/skybox/north.jpg", "res/textures/skybox/south.jpg"
	};
	std::vector<std::string> nightfaces = {
		"res/textures/skybox/eastnightsky.jpg",  "res/textures/skybox/westnightsky.jpg", 
		"res/textures/skybox/topnightsky.jpg",   "res/textures/skybox/bottomnightsky.jpg",
		"res/textures/skybox/northnightsky.jpg", "res/textures/skybox/southnightsky.jpg"
	};
	skybox = std::make_unique<Skybox>(nightfaces);

	lines = std::make_unique<Lines>();

	LOG_CORE_INFO("[SETUP COMPLETED]");

	while (!window.GetWindowShouldClose())
	{
		window.UpdateTime();
		window.PollEvents();
		//window.ClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		//printf("dt: %.3f\n", window.GetDeltaTime());

		/* Handle user input */
		/*camera->KeyControl(window.GetKeys(), window.GetDeltaTime());
		camera->MouseControl(window.GetDeltaX(), window.GetDeltaY());*/
		camera->OnUpdate();

		/* Render scene to directional shadow framebuffer */
		if (fps_counter == 0)
		{
			DirectionalShadowMapPass();

			/* Render scene to omni shadow framebuffer */
			OmniShadowMapPass();

			glm::mat4 model(1.0f);
			model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(0.0f, 4.0f, 7.0f));
			glm::vec3 pos(model * glm::vec4(1.0f));
			ReflectionPass(pos);
		}
		/* Render */
		RenderPass(window);

		/* glfw stuff */
		window.SwapBuffers();

		/*++fps_counter;
		if (fps_counter > 5)
			fps_counter = 0;*/

		glCheckError();
	}

	/* cleanup */
}

void RenderScene(bool is_shadow, Shader& shader)
{
	/* Render objects ithout shadows */
	/* Draw ligt sources */
	if (!is_shadow)
	{
		skybox->Render(camera->GetViewMatrix(), projection);

		lightsource_shader->Validate();
		lightsource_shader->Bind();

		glm::mat4 m = glm::mat4(1.0f);
		lightsource_shader->SetUniformMatrix4fv("model", m);
		lines->Render(*lightsource_shader);
		int i = 0;
		for (auto& light : lights_v)
		{
			if (i != lights_v.size() - 2 && i != 0)
			{
				/* Render sphere correponding to this light source */
				glm::mat4 mm = glm::mat4(1.0f);
				lightsource_shader->SetUniformMatrix4fv("model", mm);
				mm = glm::translate(mm, (*(PointLight*)&(*light)).GetPos());
				
				//model = glm::rotate(model, glm::radians(190.0f * triangleOffset), glm::vec3(0.0f, 1.0f, 0.0f));
				mm = glm::scale(mm, glm::vec3(0.1f));
				lightsource_shader->SetUniformMatrix4fv("model", mm);
				//lightsource_shader->SetUniformMatrix4fv("model", mm);
				lightsource_shader->SetUniformVec3("u_color", light->GetColor());
				mesh_v[4]->RenderMesh();
				
			}
			++i;
		}
		lightsource_shader->Unbind();

	}

	/* Rendering */
		/*
		*	To render object:
		*	1. UseTexture(), UseMaterial()
		*
		*	2. update model IN ORDER:
		*		translate, rotate, scale
		*
		*	3. set uniform model
		*
		*	4. Call RenderMesh() on mesh of this object
	*/
	shader.Bind();
	shader.Validate();

	//main_shader->SetUniform1i("material.diffuse_map", 1);
	//main_shader->SetUniform1i("material.specular_map", 2);

	/* Render first object */
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(-3.0f, -1.0, -2.0f));
	model = glm::rotate(model, glm::radians(190.0f * triangleOffset), glm::vec3(0.0f, 1.0f, 0.0f));
	shader.SetUniformMatrix4fv("model", model);
	brick_texture.UseTexture();
	materials_v[0]->UseMaterial(*main_shader);
	mesh_v[0]->RenderMesh();

	/* Render second object */
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(5.0f, 3.0f, -2.0f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	//model = glm::translate(model, glm::vec3(0.0f, 3.0f, -2.0f));
	shader.SetUniformMatrix4fv("model", model);
	brick_texture.UseTexture();
	materials_v[1]->UseMaterial(*main_shader);
	mesh_v[1]->RenderMesh();

	/* Render third object (floor) */
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(190.0f * triangleOffset), glm::vec3(0.0f, 1.0f, 0.0f));
	//model = glm::scale(model, glm::vec3(10.0f, 0.0f, 10.0f));
	shader.SetUniformMatrix4fv("model", model);
	plain_texture.UseTexture();
	materials_v[2]->UseMaterial(*main_shader);
	mesh_v[2]->RenderMesh();

	/* Render fourth object (sphere) */
	if (draw_sphere)
	{
		main_shader->SetUniform1i("material.reflection_map", 9);
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.0f, 4.0f, 7.0f));
		//model = glm::rotate(model, glm::radians(190.0f * triangleOffset), glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(10.0f, 0.0f, 10.0f));
		shader.SetUniformMatrix4fv("model", model);
		plain_texture.UseTexture();
		materials_v[2]->UseMaterial(*main_shader);
		mesh_v[4]->RenderMesh();
		main_shader->SetUniform1i("material.reflection_map", 10);
	}
	else
	{
		draw_sphere = true;

	}

	/* Car */
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, -1.98f, -7.5f));
	model = glm::scale(model, glm::vec3(0.2f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	shader.SetUniformMatrix4fv("model", model);
	materials_v[2]->UseMaterial(*main_shader);
	car_model.RenderModel();
	
	/* PortalGun */
	model = glm::mat4(1.0f);
	//model = glm::translate(model, camera->GetPosition() + camera->GetFront() + glm::vec3(0.0f, -0.2f, 0.0f));
	model = glm::translate(model, glm::vec3(3.0f, 2.0f, 3.0f));
	//model = glm::scale(model, glm::vec3(2.0f));
	
	//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	shader.SetUniformMatrix4fv("model", model);
	materials_v[2]->UseMaterial(*main_shader);
	portalgun_model.RenderModel();

	/* Render fifth object (cube) */
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-5.0f, 3.0f, 9.0f));
	//model = glm::rotate(model, glm::radians(190.0f * triangleOffset), glm::vec3(0.0f, 1.0f, 0.0f));
	//model = glm::rotate(model, glm::dot(-camera->GetFront(), glm::normalize(glm::vec3(-5.0f, 3.0f, 9.0f))), glm::vec3(1.0f, 1.0f, 1.0f));
	//model = glm::scale(model, glm::vec3(10.0f, 0.0f, 10.0f));
	shader.SetUniformMatrix4fv("model", model);
	container_texture_specular.UseTexture(GL_TEXTURE2);
	container_texture.UseTexture(GL_TEXTURE1);
	materials_v[2]->UseMaterial(*main_shader);
	mesh_v[3]->RenderMesh();

	// Specular map
	plain_texture.UseTexture(GL_TEXTURE2);
	
	shader.Unbind();
}