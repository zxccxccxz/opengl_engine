#include "MWindow.h"
#include "Log.h"
#include <cassert>
#include <memory>


MWindow::MWindow() 
	: MWindow(800, 600)
{
}

MWindow::MWindow(int width, int height)
	: _width(width), _height(height), _prev_x(0.f), _prev_y(0.f), _delta_x(0.f), _delta_y(0.f),
	_delta_time(0.0f), _prev_time(0.0f)
{
	if (Init())
		assert(0);
}

int MWindow::Init()
{
	LOG_CORE_TRACE("[GLFW WINDOW] Initializing");
	memset(_keys, 0, sizeof(_keys));

	/* glfw init */
	if (!glfwInit())
	{
		LOG_CORE_ERROR("[GLFW] Failed to init glfw\n");
		glfwTerminate();
		return 1;
	}

	// glfw window props (v. 3.3; core_profile means no deprecated opengl)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	_window = glfwCreateWindow(_width, _height, "OpenGL", NULL, NULL);
	if (!_window)
	{
		LOG_CORE_ERROR("[GLFW] Failed to create window\n");
		glfwTerminate();
		return -1;
	}

	// get buffer size info
	glfwGetFramebufferSize(_window, &_buffer_width, &_buffer_height);

	/* set glfw context */
	glfwMakeContextCurrent(_window);
	glfwSwapInterval(1);

	/* set glfw callbacks */
	SetCallbacks();

	/* glad init */
	if (!gladLoadGL())
	{
		LOG_CORE_ERROR("[GLAD] Failed to init glad\n");
		glfwDestroyWindow(_window);
		glfwTerminate();
		return 1;
	}

	/* depth */
	glEnable(GL_DEPTH_TEST);

	/* set viewport */
	glViewport(0, 0, _width, _height);

	/* set user pointer to this MWindow class instance */
	glfwSetWindowUserPointer(_window, this);

	LOG_CORE_INFO("[GLFW WINDOW] Initialized");

	return 0;
}

void MWindow::HandleKeys(GLFWwindow* window, int key, int code, int action, int mode)
{
	/* get MWindow class isntance asigned to glfwwindow */
	MWindow* w = static_cast<MWindow*>(glfwGetWindowUserPointer(window));

	/* callback to close window */
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	/* callback to close window */
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			w->_keys[key] = true;
			// printf("[KEY PRESSED] key: %d\n", key);
		}
		else if (action == GLFW_RELEASE)
		{
			w->_keys[key] = false;
			// printf("[KEY RELEASED] key: %d\n", key);
		}
	}
}

void MWindow::HandleMouse(GLFWwindow* window, double x_pos, double y_pos)
{
	/* get MWindow class isntance asigned to glfwwindow */
	MWindow* w = static_cast<MWindow*>(glfwGetWindowUserPointer(window));
	w->_mouse_pos = { x_pos, y_pos };

	//if (w->_mouse_first_moved)
	//{
	//	w->_prev_x = x_pos;
	//	w->_prev_x = y_pos;
	//	w->_mouse_first_moved = false;
	//}

	//w->_delta_x = x_pos - w->_prev_x;
	//w->_delta_y = w->_prev_y - y_pos; // to prevent inverted controls

	//w->_prev_x = x_pos;
	//w->_prev_y = y_pos;

	// printf("x: %.6f y: %.6f\n", w->_delta_x, w->_delta_y);
}

void MWindow::SetCallbacks()
{
	glfwSetKeyCallback(_window, HandleKeys);
	glfwSetCursorPosCallback(_window, HandleMouse);
	glfwSetMouseButtonCallback(_window, MouseButtonCallback);
	
}

void MWindow::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	MWindow* w = static_cast<MWindow*>(glfwGetWindowUserPointer(window));

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		w->_camera_movement_enabled = true;
	}
	else
	{
		w->_camera_movement_enabled = false;
	}
}

MWindow::~MWindow()
{
	glfwDestroyWindow(_window);
	glfwTerminate();
}