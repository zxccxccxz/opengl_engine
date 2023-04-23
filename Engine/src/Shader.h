#pragma once
#include <string>
#include <fstream>

#include <iostream>
#include <unordered_map>

#include <glad/glad.h>
#include <glm/glm.hpp>

struct ShaderPrograms
{
	std::string vertex_shader, fragment_shader, geometry_shader;
};

std::string GetFileAsString(const std::string& fp);

class Shader
{
public:
	Shader(const std::string& vertex_shader_fp, const std::string& fragment_shader_fp);
	Shader(const std::string& vertex_shader_fp, const std::string& fragment_shader_fp, const std::string& geometry_shader_fp);

	~Shader();

	void Bind() const;
	void Unbind() const;

	void SetUniformMatrix4fv(const std::string& name, const glm::mat4& mat);
	void SetUniformVec3(const std::string& name, const glm::vec3& vec);
	void SetUniform1f(const std::string& name, const GLfloat value);
	void Shader::SetUniform1i(const std::string& name, const GLint value);

	const ShaderPrograms& GetShaders() const;

	GLuint GetProgram() const { return _shader_program_id; }
	bool Validate();

private:

	ShaderPrograms ParseShaders
	(
		const std::string& vertex_shader_fp,
		const std::string& fragment_shader_fp
	);

	ShaderPrograms ParseShaders
	(
		const std::string& vertex_shader_fp, const std::string& fragment_shader_fp, const std::string& geometry_shader_fp
	);

	GLuint CompileShader(GLuint type, const std::string& src);

	GLuint CreateShaderProgram(GLuint vs, GLuint fs);
	GLuint CreateShaderProgram(GLuint vs, GLuint fs, GLuint gs);

	ShaderPrograms _shader_programs;
	GLuint _shader_program_id;

	mutable std::unordered_map<std::string, GLuint> _map;
	GLuint GetUniformLocation(const std::string& name) const;

	mutable bool _is_bound;
};