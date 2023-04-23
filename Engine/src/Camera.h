#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

#include "MWindow.h"

class Camera {
public:
	Camera(MWindow& window, glm::vec3 pos, glm::vec3 world_up, GLfloat yaw, GLfloat pitch, GLfloat move_speed, GLfloat turn_speed);
	~Camera();

	void OnUpdate();

	glm::mat4 GetViewMatrix();

	glm::vec3 GetPosition() const { return _position; }

	glm::vec3 GetFront() const { return _front; }

private:
	MWindow& _window;

	glm::vec3 _position;
	glm::vec3 _front;
	glm::vec3 _up;
	glm::vec3 _right;
	glm::vec3 _world_up;

	GLfloat _yaw;
	GLfloat _pitch;
	// GLfloat _roll; // might be useful in plane simulators

	GLfloat _move_speed;
	GLfloat _turn_speed;

	bool _moved = false;
	glm::mat4 _view;

	glm::vec2 _last_mouse_pos;

	void RecalculateView();
};