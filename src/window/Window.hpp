//
// Created by Gregstr on 03/11/2026.
//

#pragma once
#include <functional>
#include <vector>

#include <volk.h>

#include "VkSilk.hpp"

struct GLFWwindow;

namespace silk {
class Window {
public:
	Window(int width, int height, const char* title);
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator=(Window&&) = delete;

	[[nodiscard]] bool ShouldClose() const;
	void PollEvents();
	[[nodiscard]] bool TakeResize(); // true exactly once per resize
	[[nodiscard]] bool IsIconified() const;
	[[nodiscard]] std::vector<const char*> RequiredInstanceExtensions() const;
	[[nodiscard]] VulkanContext::SurfaceProvider SurfaceProvider() const;

private:
	GLFWwindow* window = nullptr;
	bool resized = false;
};
}