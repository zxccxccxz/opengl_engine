#pragma once

#include "ShadowMap.h"

class OmniShadowMap : public ShadowMap
{
public:
	OmniShadowMap(GLuint width, GLuint height);
	~OmniShadowMap();

	virtual bool Init() override;

	// Write to shadow map, binds fbo
	virtual void Write() override;

	// Read texture, texture_unit GL_TEXTURE2
	virtual void Read(GLenum texture_unit = GL_TEXTURE3) override;

private:

	static size_t _count;

};