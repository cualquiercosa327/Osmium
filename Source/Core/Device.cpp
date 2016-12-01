#include <Core/Device.h>
#include <iostream>
#include <Graphics/DebugRenderer.h>

using namespace Osm;

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

GraphicsDevice::GraphicsDevice(CEngine& engine) : Component(engine)
{
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);		 // yes, 3 and 2!!!
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // But also 4 if present
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	_window = glfwCreateWindow(mode->width, mode->height, "Blastastic", glfwGetPrimaryMonitor(), nullptr);
	//_window = glfwCreateWindow(1920, 1080, "Blastastic", nullptr, nullptr);
	// _window = glfwCreateWindow(640, 360, "Blastastic", nullptr, nullptr);

	//_window = glfwCreateWindow(1280, 720, "Blastastic", nullptr, nullptr);

	int major = glfwGetWindowAttrib(_window, GLFW_CONTEXT_VERSION_MAJOR);
	int minor = glfwGetWindowAttrib(_window, GLFW_CONTEXT_VERSION_MINOR);
	int revision = glfwGetWindowAttrib(_window, GLFW_CONTEXT_REVISION);
	std::cout << "OpenGL Version " << major << "." << minor << "." << revision << std::endl;

	if (!_window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(_window);
	glfwSwapInterval(1);
	glfwSetKeyCallback(_window, key_callback);

	if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
	{
		cout << "Failed to initialize OpenGL context" << endl;
		ASSERT(false);
	}
	InitDebugMessages();

	glfwGetFramebufferSize(_window, &_width, &_height);
	_ratio = _width / float(_height);
}

GraphicsDevice::~GraphicsDevice()
{
	glfwDestroyWindow(_window);
	glfwTerminate();
}

int GraphicsDevice::GetScreenWidth() const
{
	return _width;
}

int GraphicsDevice::GetScreenHeight() const
{
	return _height;
}

bool GraphicsDevice::GetKeyOnce(char key)
{
	return (glfwGetKey(_window, key) ?
		(_keyOnce[key] ? false : (_keyOnce[key] = true)) : \
		(_keyOnce[key] = false));
}
