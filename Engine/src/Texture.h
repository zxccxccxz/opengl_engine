#pragma once

#include <glad/glad.h>
#include <string>

class Texture {
public:
	//Texture();
	//Texture(std::string&& filepath);
	Texture(const std::string& filepath);
	~Texture();

	bool LoadTexture(bool use_alpha_channel, bool flip_vertically);
	void UseTexture(GLenum texture_slot = GL_TEXTURE1);
	void UnbindTexture(GLenum texture_slot);
	void Clear();

private:
	GLuint _id;
	int _width, _height, _bit_depth;
	std::string _filepath;

};