#include "ReflectionMap.h"

#include "Log.h"
#include "Renderer.h"

size_t ReflectionMap::_count = 0;

ReflectionMap::ReflectionMap(GLuint size)
	: _size(size), _texture_unit(GL_TEXTURE9 + _count)
{
	++_count;

	glGenTextures(1, &_reflection_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _reflection_map);
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (size_t i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, _size, _size, 0, GL_RGBA, GL_FLOAT, nullptr);
	}

	glGenFramebuffers(1, &_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _reflection_map, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_CORE_ERROR("[REFLECTION MAP] Framebuffer error: {0}\n", status);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCheckError();
}

ReflectionMap::~ReflectionMap()
{
}

void ReflectionMap::BindFramebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
}

void ReflectionMap::BindTexture()
{
	glActiveTexture(_texture_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _reflection_map);
}
