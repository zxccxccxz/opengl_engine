#include "Mesh.h"

#include "Log.h"

Mesh::Mesh() : _vao(0), _vbo(0), _ibo(0), _index_count(0)
{
	
}

Mesh::~Mesh()
{
	ClearMesh();
}

void Mesh::CreateMesh(GLfloat* vertices, unsigned int* indices, unsigned int vertices_num, unsigned int indices_num)
{
	_index_count = indices_num;

	// create vao
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// create ibo
	glGenBuffers(1, &_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices_num, indices, GL_STATIC_DRAW);

	// create vbo
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices_num, vertices, GL_STATIC_DRAW);

	/* vertex attrib array */
	// vertex position
	_layout.Push<float>(3);
	// texture coords
	_layout.Push<float>(2);
	// normals
	_layout.Push<float>(3);

	/*for (auto& e : _layout)
	{
		printf("size: %u, offset: %u, stride: %u\n", e.size, e.offset, _layout.GetStride());
	}*/

	int i = 0;
	for (auto& e : _layout)
	{
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i++, e.count, GL_FLOAT, e.normalized, _layout.GetStride(), reinterpret_cast<const void*>(e.offset)); // 3 * sizeof(GLfloat)
	}

	LOG_CORE_INFO("[MESH] Created");

	// cleanup
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::RenderMesh()
{
	glBindVertexArray(_vao);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
	glDrawElements(GL_TRIANGLES, _index_count, GL_UNSIGNED_INT, 0); // indices already bound
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Mesh::ClearMesh()
{
	if (_ibo)
	{
		glDeleteBuffers(1, &_ibo);
		_ibo = 0;
	}

	if (_vbo)
	{
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
	}
	
	if (_vao)
	{
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}

	_index_count = 0;
}