#include "ShadowMap.h"

#include "Log.h"

#include "Renderer.h"

ShadowMap::ShadowMap(GLuint width, GLuint height, bool is_base_class)
	: _fbo(0), _shadow_map(0), _shadow_width(width), _shadow_height(height), _texture_unit(GL_TEXTURE3)
{
	if (!is_base_class)
	{
		Init();
	}
}

bool ShadowMap::Init()
{
	LOG_CORE_TRACE("[SHADOW MAP] Initializing");
	/* Generate GL framebuffer */
	glGenFramebuffers(1, &_fbo);

	/* Create & bind texture */
	glGenTextures(1, &_shadow_map);
	glBindTexture(GL_TEXTURE_2D, _shadow_map);

	/* Mipmaps */
	// GL_DEPTH_COMPONENT [0, 1] how deep pixel is? (based on far plane)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _shadow_width, _shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	// fbo will output image to texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	static float b_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, b_color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Framebuffer will render (write depth values) to texture _shadow_map */
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _shadow_map, 0);

	/* Explicitly state we don't want to draw color, only depth values */
	// Do not read from GL_COLOR_ATTACHMENT
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	/* Errors handling */
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_CORE_ERROR("[SHADOW MAP] Framebuffer error: {0}\n", status);
		return false;
	}

	/* Unbind framebuffer */
	// 0 is default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	LOG_CORE_INFO("[SHADOW MAP] Initialized");
	return true;
}

void ShadowMap::Write()
{
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
}

void ShadowMap::Read(GLenum texture_unit)
{
	glActiveTexture(_texture_unit);
	glBindTexture(GL_TEXTURE_2D, _shadow_map);
}

ShadowMap::~ShadowMap()
{
	if (_fbo)
	{
		glDeleteFramebuffers(1, &_fbo);
	}

	if (_shadow_map)
	{
		glDeleteTextures(1, &_shadow_map);
	}
}
