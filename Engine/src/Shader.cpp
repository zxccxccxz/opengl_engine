#include "Shader.h"

#include "Renderer.h"

#include "Log.h"

#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const std::string& vertex_shader_fp, const std::string& fragment_shader_fp)
{
	LOG_CORE_TRACE("[SHADER] Creating shader:");
	LOG_CORE_TRACE("vert: {0}, frag: {1}", vertex_shader_fp, fragment_shader_fp);
	
	_shader_programs = ParseShaders(vertex_shader_fp, fragment_shader_fp);

	GLuint vs = CompileShader(GL_VERTEX_SHADER, _shader_programs.vertex_shader);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, _shader_programs.fragment_shader);

	_shader_program_id = CreateShaderProgram(vs, fs);
	LOG_CORE_INFO("[SHADER] Created");
}

Shader::Shader(const std::string& vertex_shader_fp, const std::string& fragment_shader_fp, const std::string& geometry_shader_fp)
{
	LOG_CORE_TRACE("[SHADER] Creating shader:");
	LOG_CORE_TRACE("vert: {0}, frag: {1}, geom: {2}", vertex_shader_fp, fragment_shader_fp, geometry_shader_fp);
	
	_shader_programs = ParseShaders(vertex_shader_fp, fragment_shader_fp, geometry_shader_fp);

	GLuint vs = CompileShader(GL_VERTEX_SHADER, _shader_programs.vertex_shader);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, _shader_programs.fragment_shader);
	GLuint gs = CompileShader(GL_GEOMETRY_SHADER, _shader_programs.geometry_shader);

	_shader_program_id = CreateShaderProgram(vs, fs, gs);
	LOG_CORE_INFO("[SHADER] Created");
}

Shader::~Shader()
{
	glDeleteProgram(_shader_program_id);
}

void Shader::Bind() const
{
	_is_bound = true;
	glUseProgram(_shader_program_id);
}

void Shader::Unbind() const
{
	_is_bound = false;
	glUseProgram(0);
}

const ShaderPrograms& Shader::GetShaders() const
{
	return _shader_programs;
}

std::string GetFileAsString(const std::string& fp)
{
	std::ifstream fs;
	std::string r;

	fs.open(fp);
	if (fs.is_open())
	{
		r.append
		(
			(std::istreambuf_iterator<char>(fs)),
			(std::istreambuf_iterator<char>())
		);
	}
	else
	{
		LOG_CORE_ERROR("[SHADER] failed to open file\n");
	}
	fs.close();

	return r;
}

ShaderPrograms Shader::ParseShaders(const std::string& vertex_shader_fp, const std::string& fragment_shader_fp)
{
	/* Vertex shader */
	std::string vertex_shader = GetFileAsString(vertex_shader_fp);

	/* Fragment shader */
	std::string fragment_shader = GetFileAsString(fragment_shader_fp);

	return { vertex_shader, fragment_shader, "no geometry shader" };
}

ShaderPrograms Shader::ParseShaders(const std::string& vertex_shader_fp, const std::string& fragment_shader_fp, const std::string& geometry_shader_fp)
{
	/* Vertex shader */
	std::string vertex_shader = GetFileAsString(vertex_shader_fp);

	/* Fragment shader */
	std::string fragment_shader = GetFileAsString(fragment_shader_fp);

	/* Geometry shader */
	std::string geometry_shader = GetFileAsString(geometry_shader_fp);

	return { vertex_shader, fragment_shader, geometry_shader };
}

GLuint Shader::CompileShader(GLuint type, const std::string& src)
{
	const char* s = src.c_str();

	GLuint shader_id = glCreateShader(type);
	glShaderSource(shader_id, 1, &s, NULL);
	glCompileShader(shader_id);

	// Error handling
	int r;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &r);
	if (r == GL_FALSE)
	{
		int length;
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
		char* msg = (char*)_malloca(length * sizeof(char)); // alloca is unsafe
		glGetShaderInfoLog(shader_id, length, &length, msg);
		LOG_CORE_ERROR("Failed to compile {0} shader!", (type == GL_VERTEX_SHADER ? "vertex" : (type == GL_FRAGMENT_SHADER ? "fragment" : "geometry")));
		LOG_CORE_ERROR("MSG({0}): {1}", length, msg);
		glDeleteShader(shader_id);
		return 0;
	}

	return shader_id;
}

bool Shader::Validate()
{
	GLint r;
	glValidateProgram(_shader_program_id);
	glGetProgramiv(_shader_program_id, GL_VALIDATE_STATUS, &r);
	if (!r)
	{
		int length;
		glGetProgramiv(_shader_program_id, GL_INFO_LOG_LENGTH, &length);
		char* msg = (char*)_malloca(length);
		glGetProgramInfoLog(_shader_program_id, length, &length, msg);
		LOG_CORE_ERROR("Failed to validate program!");
		LOG_CORE_ERROR("MSG({0}): {1}", length, msg);
		return false;
	}
	return true;
}

GLuint Shader::CreateShaderProgram(GLuint vs, GLuint fs)
{
	GLuint program = glCreateProgram();

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

// TEMP
GLuint Shader::CreateShaderProgram(GLuint vs, GLuint fs, GLuint gs)
{
	GLuint program = glCreateProgram();

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glAttachShader(program, gs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteShader(gs);

	return program;
}

GLuint Shader::GetUniformLocation(const std::string& name) const
{
	if (_map.find(name) == _map.end())
	{
		GLuint r = _map[name] = glGetUniformLocation(_shader_program_id, name.c_str());
		LOG_CORE_TRACE("[SHADER::MAP]: {0} -> {1}", name.c_str(), r);
		if (r == -1)
		{
			LOG_CORE_ERROR("[SHADER] Uniform is unreferenced or does not exist, name: {0}", name.c_str());
		}
		return r;
	}
	return _map.at(name);
}

void Shader::SetUniformMatrix4fv(const std::string& name, const glm::mat4& mat)
{
	if (!_is_bound) return;
	GLuint location = GetUniformLocation(name);
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::SetUniformVec3(const std::string& name, const glm::vec3& vec)
{
	if (!_is_bound) return;
	GLuint location = GetUniformLocation(name);
	static float arr[3];
	arr[0] = vec.x; arr[1] = vec.y; arr[2] = vec.z;
	glUniform3fv(location, 1, arr);
};

void Shader::SetUniform1f(const std::string& name, const GLfloat value)
{
	if (!_is_bound) return;
	GLuint location = GetUniformLocation(name);
	glUniform1f(location, value);
};

void Shader::SetUniform1i(const std::string& name, const GLint value)
{
	if (!_is_bound) return;
	GLuint location = GetUniformLocation(name);
	glUniform1i(location, value);
};

//void Shader::SetUniformMatrix4fv(const std::string& name, const glm::mat4& mat)
//{
//	GLuint location = glGetUniformLocation(_shader_program_id, name.c_str());
//	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
//}
//
//void Shader::SetUniformVec3(const std::string& name, const glm::vec3& vec)
//{
//	GLuint location = glGetUniformLocation(_shader_program_id, name.c_str());
//	static float arr[3];
//	arr[0] = vec.x; arr[1] = vec.y; arr[2] = vec.z;
//	glUniform3fv(location, 1, arr);
//};
//
//void Shader::SetUniform1f(const std::string& name, const GLfloat value)
//{
//	GLuint location = glGetUniformLocation(_shader_program_id, name.c_str());
//	glUniform1f(location, value);
//};
//
//void Shader::SetUniform1i(const std::string& name, const GLint value)
//{
//	GLuint location = glGetUniformLocation(_shader_program_id, name.c_str());
//	glUniform1i(location, value);
//};
