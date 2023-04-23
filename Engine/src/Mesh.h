#pragma once

#include <glad/glad.h>
#include <vector>

struct BufferElement {
	unsigned int size;
	unsigned int count;
	unsigned int offset;
	bool normalized;
};

class BufferLayout {
public:
	BufferLayout()
		: _elements(), _stride(0)
	{};

	BufferLayout(const std::initializer_list<BufferElement>& elems)
		: _elements(elems), _stride(0) {
	}

	inline unsigned int GetStride() const { return _stride; }
	inline const std::vector<BufferElement>& GetElements() const { return _elements; }

	std::vector<BufferElement>::iterator begin() { return _elements.begin(); }
	std::vector<BufferElement>::iterator end() { return _elements.end(); }
	std::vector<BufferElement>::const_iterator cbegin() const { return _elements.cbegin(); }
	std::vector<BufferElement>::const_iterator cend() const { return _elements.cend(); }

	template <class T>
	void Push(unsigned int count) {
		static_assert();
	}
	
	template <>
	void Push<float>(unsigned int count) {
		//printf("Push<float>(%u)\n", count);
		unsigned int offset = 0;
		unsigned int size = sizeof(float) * count;
		if (!_elements.empty())
		{
			offset = _elements.rbegin()->offset + _elements.rbegin()->size;
		}
		_elements.push_back({ size, count, offset, GL_FALSE });
		_stride += size;
	}

private:
	std::vector<BufferElement> _elements;
	unsigned int _stride;
};

class Mesh {
public:
	Mesh();
	~Mesh();

	void CreateMesh(GLfloat* vertices, unsigned int* indices, unsigned int vertices_size, unsigned int indices_size);
	void RenderMesh();
	void ClearMesh();

private:
	GLuint _vao, _vbo, _ibo;
	GLsizei _index_count;
	BufferLayout _layout;
};
