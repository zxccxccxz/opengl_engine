#pragma once

#include <glad/glad.h>

class ReflectionMap
{
public:
	ReflectionMap(GLuint size);
	~ReflectionMap();

	void BindFramebuffer();
	void BindTexture();

private:
	GLuint _size, _texture_unit, _fbo, _reflection_map;

	static size_t _count;
};