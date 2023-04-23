#include "OmniShadowMap.h"

#include "Log.h"

#include "Renderer.h"

size_t OmniShadowMap::_count = 0;

OmniShadowMap::OmniShadowMap(GLuint width, GLuint height)
	: ShadowMap(width, height, true)
{
	/* Assign texture unit */
	_texture_unit = GL_TEXTURE4 + _count;
	++_count;
	LOG_CORE_WARN("_count: {0}", _count);
	Init();
}

OmniShadowMap::~OmniShadowMap()
{
}

bool OmniShadowMap::Init()
{
	LOG_CORE_TRACE("[OMNI SHADOW MAP] Initializing");

	/* Create & bind texture */
	glGenTextures(1, &_shadow_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _shadow_map);

	/* Mipmaps */
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

	/* Cube map is basically 6 2D textures */
	for (size_t i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, _shadow_width, _shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	/* Generate GL framebuffer */
	glGenFramebuffers(1, &_fbo);
	/* Framebuffer will render (write depth values) to texture _shadow_map */
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _shadow_map, 0);

	/* Explicitly state we don't want to draw color, only depth values */
	// Do not read from GL_COLOR_ATTACHMENT
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	/* Errors handling */
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_CORE_ERROR("[OMNI SHADOW MAP] Framebuffer error: {0}\n", status);
		return false;
	}

	/* Unbind framebuffer */
	// 0 is default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	LOG_CORE_INFO("[OMNI SHADOW MAP] Initialized");

	return true;
}

void OmniShadowMap::Write()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
}

void OmniShadowMap::Read(GLenum texture_unit)
{
	//_texture_unit = texture_unit;
	glActiveTexture(_texture_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _shadow_map);
}
