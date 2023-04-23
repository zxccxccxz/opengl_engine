#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

#include "Renderer.h"

#include "Log.h"

//Texture::Texture()
//	: _id(0), _width(0), _height(0), _bit_depth(0), _filepath("")
//{
//
//}

Texture::Texture(const std::string& filepath)
	: _id(0), _width(0), _height(0), _bit_depth(0), _filepath(filepath)
{
}

//Texture::Texture(std::string&& filepath)
//	: _id(0), _width(0), _height(0), _bit_depth(0), _filepath(std::move(filepath))
//{
//}

bool Texture::LoadTexture(bool use_alpha_channel, bool flip_vertically)
{
	stbi_set_flip_vertically_on_load(flip_vertically);
	unsigned char* texture_data = stbi_load(_filepath.c_str(), &_width, &_height, &_bit_depth, 0);
	if (!texture_data)
	{
		LOG_CORE_ERROR("[stbi_image] Failed to load file\n");
		return false;
	}
	else
	{
		LOG_CORE_TRACE("[LoadTexture] Loaded texture, {0}", _filepath);
	}

	/* opengl */
	glGenTextures(1, &_id);
	glBindTexture(GL_TEXTURE_2D, _id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* generate mipmaps automatically */
	glTexImage2D(GL_TEXTURE_2D, 0, (use_alpha_channel ? GL_RGBA : GL_RGB), _width, _height, 0, (use_alpha_channel ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, texture_data);

	// !!!
	glCheckError();

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(texture_data);

	return true;
}

void Texture::UseTexture(GLenum texture_slot)
{
	/* set texture unit to x */
	glActiveTexture(texture_slot);
	/* bind texture to texture unit x */
	glBindTexture(GL_TEXTURE_2D, _id);
}

void Texture::UnbindTexture(GLenum texture_slot)
{
	glActiveTexture(texture_slot);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Clear()
{
	glDeleteTextures(1, &_id);
	_id = 0; _width = 0; _height = 0; _filepath = "";
}

Texture::~Texture()
{
	Clear();
}