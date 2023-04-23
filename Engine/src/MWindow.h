#pragma once

#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

class MWindow {
public:
	MWindow();
	MWindow(int width, int height);
	~MWindow();

	int GetBufferWidth() const { return _buffer_width; }
	int GetBufferHeight() const { return _buffer_height; }

	void PollEvents() { glfwPollEvents(); }
	void ClearColor(GLfloat r = 0.0f, GLfloat g = 0.0f, GLfloat b = 0.0f, GLfloat a = 1.0f)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	bool GetWindowShouldClose() const
	{
		return glfwWindowShouldClose(_window);
	}

	void SwapBuffers()
	{
		glfwSwapBuffers(_window);
	}

	/* TODO: refactor? */
	bool* GetKeys() { return _keys; }
	GLfloat GetDeltaX() { auto r = _delta_x; _delta_x = 0.f; return r; }
	GLfloat GetDeltaY() { auto r = _delta_y; _delta_y = 0.f; return r; }

	void UpdateTime()
	{
		float curr_time = static_cast<float>(glfwGetTime());
		_delta_time = curr_time - _prev_time;
		_prev_time = curr_time;
	}

	float GetDeltaTime() const { return _delta_time; }

	glm::vec2 GetMousePosition() const { return _mouse_pos; }

	void LockCursor()
	{
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void ReleaseCursor()
	{
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	bool IsCameraMovementEnabled() const
	{
		return _camera_movement_enabled;
	}


private:
	int Init();

	GLFWwindow* _window;
	int _width, _height;
	int _buffer_width, _buffer_height;

	bool _keys[1024] = { 0 };

	// for camera
	GLfloat _prev_x;
	GLfloat _prev_y;
	GLfloat _delta_x;
	GLfloat _delta_y;
	bool _mouse_first_moved;
	// bool _mouse_moved;
	glm::vec2 _mouse_pos;

	// timestep
	float _delta_time;
	float _prev_time;

	// camera
	bool _camera_movement_enabled = false;

	void SetCallbacks();
	static void HandleKeys(GLFWwindow* window, int key, int code, int action, int mode);
	static void HandleMouse(GLFWwindow* window, double x_pos, double y_pos);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};