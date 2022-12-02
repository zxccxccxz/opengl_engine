#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// window width and height
const GLuint WIDTH = 800, HEIGHT = 800;

/* callback to close window */
void m_key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == 256 && action == GLFW_RELEASE)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

int main(int argc, char* argv[])
{
	/* glfw init */
	if (!glfwInit())
	{
		printf("[GLFW] Failed to init glfw\n");
		glfwTerminate();
		return 1;
	}

	// glfw window props (v. 3.3; core_profile means no deprecated opengl)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", NULL, NULL);
	if (!window)
	{
		printf("[GLFW] Failed to create window\n");
		glfwTerminate();
		return 1;
	}

	// get buffer size info
	int buffer_width, buffer_height;
	glfwGetFramebufferSize(window, &buffer_width, &buffer_height);

	/* set glfw context */
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	/* set glfw key callback */
	glfwSetKeyCallback(window, m_key_callback);

	/* glad init */
	if (!gladLoadGL())
	{
		printf("[GLAD] Failed to init glad\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	/* set viewport */
	glViewport(0, 0, WIDTH, HEIGHT);


	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* glfw stuff */
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	/* cleanup */
	glfwDestroyWindow(window);
	glfwTerminate();
}