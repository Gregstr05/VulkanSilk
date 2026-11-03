//
// Created by Gregstr on 03/11/2026.
//

#pragma once
#include <array>
#include <functional>
#include <vector>

#include <volk.h>

#include "Vector.hpp"
#include "VkSilk.hpp"

struct GLFWwindow;

namespace silk {

enum class Key : int {
	Space = 32,
	D0 = 48, D1, D2, D3, D4, D5, D6, D7, D8, D9,
	A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
	N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	Escape = 256, Enter, Tab, Backspace, Insert, Delete,
	Right = 262, Left, Down, Up,
	PageUp, PageDown, Home, End,
	F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	LeftShift = 340, LeftControl, LeftAlt, LeftSuper,
	RightShift, RightControl, RightAlt, RightSuper,
};

enum class MouseButton : int {
	Left = 0, Right = 1, Middle = 2,
};

namespace detail {
struct WindowState {
	std::array<bool, 349> down{};
	std::array<bool, 349> pressed{};
	std::array<bool, 349> released{};
	std::array<bool, 8> mouseDown{};
	double mouseX = 0.0;
	double mouseY = 0.0;
	double deltaX = 0.0;
	double deltaY = 0.0;
	bool hasPosition = false;
	bool resized = false;
};
}

class Window {
public:
	Window(int width, int height, const char* title);
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator=(Window&&) = delete;

	[[nodiscard]] bool ShouldClose() const;
	void RequestClose() const;

	void PollEvents();

	[[nodiscard]] bool TakeResize(); // true exactly once per resize
	[[nodiscard]] bool IsIconified() const;

	[[nodiscard]] bool IsKeyDown(Key key) const;
	[[nodiscard]] bool WasKeyPressed(Key key) const;
	[[nodiscard]] bool WasKeyReleased(Key key) const;

	[[nodiscard]] bool IsMouseButtonDown(MouseButton button) const;
	[[nodiscard]] math::Vector2 MousePosition() const;
	[[nodiscard]] math::Vector2 MouseDelta() const;
	void SetCursorEnabled(bool enabled);

	[[nodiscard]] std::vector<const char*> RequiredInstanceExtensions() const;
	[[nodiscard]] VulkanContext::SurfaceProvider SurfaceProvider() const;

private:
	GLFWwindow* window = nullptr;
	detail::WindowState state;
};
}