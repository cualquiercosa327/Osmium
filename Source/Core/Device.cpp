#include "stdafx.h"
#include "Device.h"

using namespace igad;

Device* igad::pDevice;

Device::Device(GLFWwindow* window) : _window(window)
{
	glfwGetFramebufferSize(window, &_width, &_height);
}

int Device::GetScreenWidth() const
{
	return _width;
}

int Device::GetScreenHeight() const
{
	return _height;
}

bool Device::GetKeyOnce(char key)
{
	return (glfwGetKey(_window, key) ?
		(keyOnce[key] ? false : (keyOnce[key] = true)) : \
		(keyOnce[key] = false));
}
