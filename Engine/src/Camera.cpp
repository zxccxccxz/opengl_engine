#include "Camera.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Log.h"

Camera::Camera(MWindow& window, glm::vec3 pos, glm::vec3 world_up, GLfloat yaw, GLfloat pitch, GLfloat move_speed, GLfloat turn_speed)
	: _window(window), _position(pos), _front(glm::vec3(0.0f, 0.0f, -1.0f)), _world_up(world_up),
	_yaw(yaw), _pitch(pitch),
	_move_speed(move_speed), _turn_speed(turn_speed),
	_last_mouse_pos(0.0)
{
	RecalculateView();
	LOG_CORE_INFO("[CAMERA] Created");
}

void Camera::OnUpdate()
{
	bool* keys = _window.GetKeys();
	float dt = _window.GetDeltaTime();
	glm::vec2 mouse_pos = _window.GetMousePosition();
	glm::vec2 delta = (mouse_pos - _last_mouse_pos);
	_last_mouse_pos = mouse_pos;

	if (!_window.IsCameraMovementEnabled())
	{
		_window.ReleaseCursor();
		return;
	}

	_window.LockCursor();

	_right = glm::normalize(glm::cross(_front, _world_up));

	// Movement
	float velocity = _move_speed * dt;
	if (keys[GLFW_KEY_W])
	{
		_position += _front * velocity;
		_moved = true;
	}

	if (keys[GLFW_KEY_S])
	{
		_position -= _front * velocity;
		_moved = true;
	}

	if (keys[GLFW_KEY_A])
	{
		_position -= _right * velocity;
		_moved = true;
	}

	if (keys[GLFW_KEY_D])
	{
		_position += _right * velocity;
		_moved = true;
	}

	//LOG_CORE_TRACE("Mouse delta x: {0}, y: {1}", delta.x, delta.y);
	// Rotation
	if (delta.x != 0.0f || delta.y != 0.0f)
	{
		_yaw	+=   delta.x * _turn_speed;
		_pitch	+=	-delta.y * _turn_speed;

		if (abs(_pitch) >= 89.0f) _pitch = _pitch > 0 ? 89.0f : -89.0f;
		if (abs(_yaw) >= 360.0f)  _yaw += _yaw > 0 ? -360.0f : 360.0f;

		_front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
		_front.y = sin(glm::radians(_pitch));
		_front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
		_front = glm::normalize(_front);

		/*glm::quat q = glm::normalize(glm::cross(glm::angleAxis(pitch_delta, -_right),
			glm::angleAxis(yaw_delta, _world_up)));
		_front = glm::rotate(q, _front);*/

		_moved = true;
	}

	if (_moved)
	{
		RecalculateView();
		_moved = false;
	}
}

void Camera::RecalculateView()
{
	_view = glm::lookAt(_position, _position + _front, _world_up);
}

glm::mat4 Camera::GetViewMatrix()
{
	return _view;
}

//void Camera::Update()
//{
//	_front.x = glm::cos(glm::radians(_pitch)) * glm::cos(glm::radians(_yaw));
//	_front.y = glm::sin(glm::radians(_pitch));
//	_front.z = glm::cos(glm::radians(_pitch)) * glm::sin(glm::radians(_yaw));
//	_front   = glm::normalize(_front);
//
//	_right = glm::normalize(glm::cross(_front, _world_up));
//	_up    = glm::normalize(glm::cross(_right, _front));
//}

Camera::~Camera()
{
}