//
// Created by Gregstr on 03/11/2026.
//

#pragma once
#include <chrono>
#include <cstdio>
#include <stdexcept>

#include "../app/Application.hpp"
#include "../app/Runner.hpp"
#include "Swapchain.hpp"
#include "VkSilk.hpp"
#include "Window.hpp"

namespace silk {
template <typename AppT>
int RunApp(const char* title)
{
	try
	{
		Window window(1280, 720, title);

		ContextDesc desc;
		desc.applicationName = title;
		desc.instanceExtensions = window.RequiredInstanceExtensions();

		VulkanContext ctx{desc, window.SurfaceProvider()};
		Swapchain swapchain{ctx};
		AppT app;
		Runner runner{ctx, swapchain, app};

		auto last = std::chrono::steady_clock::now();
		while (!window.ShouldClose())
		{
			window.PollEvents();
			if (window.TakeResize())
				runner.RecreateSwapchain();
			if (window.IsIconified())
				continue;

			auto now = std::chrono::steady_clock::now();
			float dt = std::chrono::duration<float>(now - last).count();
			last = now;

			runner.Step(dt);
		}
		return 0;
	} catch (const std::exception& e)
	{
		std::fprintf(stderr, "fatal: %s\n", e.what());
		return 1;
	}
}
}
