//
// Created by Gregstr on 03/11/2026.
//

#define GLFW_INCLUDE_NONE
#include <volk.h>
#include <GLFW/glfw3.h>

#include "Window.hpp"

#include <stdexcept>

namespace silk {

	namespace {

		void OnKey(GLFWwindow* handle, int key, int scancode, int action, int mods)
		{
			auto& state = *static_cast<detail::WindowState*>(glfwGetWindowUserPointer(handle));
			if (key < 0 || key >= 349)
				return;
			if (action == GLFW_PRESS)
			{
				state.down[key] = true;
				state.pressed[key] = true;
			}
			else if (action == GLFW_RELEASE)
			{
				state.down[key] = false;
				state.released[key] = true;
			}
		}

		void OnMouseButton(GLFWwindow* handle, int button, int action, int mods)
		{
			auto& state = *static_cast<detail::WindowState*>(glfwGetWindowUserPointer(handle));
			if (button >= 0 && button < 8)
				state.mouseDown[button] = (action == GLFW_PRESS);
		}

		void OnCursorPos(GLFWwindow* handle, double x, double y)
		{
			auto& state = *static_cast<detail::WindowState*>(glfwGetWindowUserPointer(handle));
			if (!state.hasPosition)
			{
				state.mouseX = x;
				state.mouseY = y;
				state.hasPosition = true;
				return;
			}
			state.deltaX += x - state.mouseX;
			state.deltaY += y - state.mouseY;
			state.mouseX = x;
			state.mouseY = y;
		}

		void OnFramebufferSize(GLFWwindow* handle, int width, int height)
		{
			static_cast<detail::WindowState*>(glfwGetWindowUserPointer(handle))->resized = true;
		}

	}

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

		glfwSetWindowUserPointer(window, &state);
		glfwSetFramebufferSizeCallback(window, OnFramebufferSize);
		glfwSetKeyCallback(window, OnKey);
		glfwSetMouseButtonCallback(window, OnMouseButton);
		glfwSetCursorPosCallback(window, OnCursorPos);
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

	void Window::RequestClose() const
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	bool Window::IsKeyDown(Key key) const
	{
		return state.down[static_cast<int>(key)];
	}

	bool Window::WasKeyPressed(Key key) const
	{
		return state.pressed[static_cast<int>(key)];
	}

	bool Window::WasKeyReleased(Key key) const
	{
		return state.released[static_cast<int>(key)];
	}

	bool Window::IsMouseButtonDown(MouseButton button) const
	{
		return state.mouseDown[static_cast<int>(button)];
	}

	math::Vector2 Window::MousePosition() const
	{
		double x = 0.0;
		double y = 0.0;
		glfwGetCursorPos(window, &x, &y);
		return math::Vector2(static_cast<float>(x), static_cast<float>(y));
	}

	math::Vector2 Window::MouseDelta() const
	{
		return math::Vector2(
			static_cast<float>(state.deltaX), static_cast<float>(state.deltaY));
	}

	void Window::SetCursorEnabled(bool enabled)
	{
		glfwSetInputMode(window, GLFW_CURSOR,
			enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		state.hasPosition = false;
	}

	void Window::PollEvents()
	{
		state.pressed.fill(false);
		state.released.fill(false);
		state.deltaX = state.deltaY = 0.0;
		glfwPollEvents();
	}

	bool Window::TakeResize()
	{
		bool was = state.resized;
		state.resized = false;
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