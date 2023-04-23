#pragma once

#include <glad/glad.h>

/* Directional Shadow Map */
class ShadowMap {
public:
	ShadowMap(GLuint width, GLuint height, bool is_base_class = false);
	virtual ~ShadowMap();

	virtual bool Init();

	// Write to shadow map, binds fbo
	virtual void Write();

	// Read texture, texture_unit GL_TEXTURE2
	virtual void Read(GLenum texture_unit = GL_TEXTURE3);

	GLuint GetShadowWidth() const { return _shadow_width; }
	GLuint GetShadowHeight() const { return _shadow_height; }

	GLuint GetTextureUnit() const
	{
		return _texture_unit - GL_TEXTURE0;
	}

	GLuint _shadow_map; // texture id
protected:
	GLuint _fbo;
	GLuint _shadow_width, _shadow_height;
	GLuint _texture_unit;

};