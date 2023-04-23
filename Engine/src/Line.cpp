#include "Line.h"


Lines::Lines()
    : _is_visible(true)
{
    new (&_lines[0]) Line(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    new (&_lines[1]) Line(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    new (&_lines[2]) Line(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
}

void Lines::Render(Shader& shader)
{
    if (_is_visible)
    {
        for (auto& l : _lines)
        //auto& l = _lines[0];
        {
            shader.SetUniformVec3("u_color", l.GetColor());
            l.Render();
        }
    }
}

void Lines::Show()
{
    _is_visible = true;
}

void Lines::Hide()
{
    _is_visible = false;
}