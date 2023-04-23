#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

#include "Mesh.h"
#include "Shader.h"
#include "Renderer.h"
#include "Log.h"

class Line {
public:
    unsigned int _vbo, _vao, _ibo;
    std::vector<GLfloat> _vertices;
    std::vector<unsigned int> _indices;
    BufferLayout _layout;
    glm::vec3 _start;
    glm::vec3 _end;
    glm::vec3 _color;
public:
    Line() {}
    Line(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color = glm::vec3(-1.0f))
        : _vao(0), _vbo(0), _ibo(0), _start(start), _end(end), _color(color)
    {
        if (color.r < 0)
            _color = _end;

        GLfloat vertices[] = {
             start.x,   start.y,     start.z, 0.0, 0.0, 0.0, 0.0, 0.0,
             end.x,     end.y,       end.z,   0.0, 0.0, 0.0, 0.0, 0.0
        };

        unsigned int indices[] = {
            0, 1
        };

        // vao
        //glCheckError();
        glGenVertexArrays(1, &_vao);
        LOG_CORE_WARN("VAO: {0}", _vao);
        glBindVertexArray(_vao);

        // ibo
        glGenBuffers(1, &_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * 3, indices, GL_STATIC_DRAW);

        // vbo
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * 16, vertices, GL_STATIC_DRAW);

        // vertex attrib array
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

        glCheckError();
        // cleanup
        //glDisableVertexAttribArray(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    glm::vec3 GetPos() const {
        return _end;
    }

    glm::vec3 GetColor() const {
        return _color;
    }

    void Render() 
    {
        glBindVertexArray(_vao);
        glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    ~Line()
    {
        LOG_CORE_TRACE("LINE DTOR");
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
    }
};


class Lines {
public:
    Lines();

    void Render(Shader& shader);

    void Show();

    void Hide();

    std::array<Line, 3> _lines;
private:
    bool _is_visible;
};
