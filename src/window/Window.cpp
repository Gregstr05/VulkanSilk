//
// Created by Gregstr on 03/11/2026.
//

#define GLFW_INCLUDE_NONE
#include <volk.h>
#include <GLFW/glfw3.h>

#include "Window.hpp"

#include <stdexcept>

namespace silk {

	Window::Window(int width, int height, const char* title)
	{
		if (!glfwInit())
			throw std::runtime_error("glfwInit failed");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (window == nullptr)
		{
			glfwTerminate();
			throw std::runtime_error("glfwCreateWindow failed");
		}

		glfwSetWindowUserPointer(window, &resized);
		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int, int) {
			*static_cast<bool*>(glfwGetWindowUserPointer(w)) = true;
		});
	}

	Window::~Window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(window);
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	bool Window::TakeResize()
	{
		bool was = resized;
		resized = false;
		return was;
	}

	bool Window::IsIconified() const
	{
		return glfwGetWindowAttrib(window, GLFW_ICONIFIED);
	}

	std::vector<const char*> Window::RequiredInstanceExtensions() const
	{
		uint32_t count = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&count);
		return {extensions, extensions + count};
	}

	VulkanContext::SurfaceProvider Window::SurfaceProvider() const
	{
		GLFWwindow* handle = window;
		return [handle](VkInstance instance) {
			VkSurfaceKHR surface = VK_NULL_HANDLE;
			glfwCreateWindowSurface(instance, handle, nullptr, &surface);
			return surface;
		};
	}

}