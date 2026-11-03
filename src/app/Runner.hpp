//
// Created by Gregstr on 18/09/2026.
//

#pragma once
#include <vector>

#include "Application.hpp"
#include "Swapchain.hpp"
#include "VkSilk.hpp"

namespace silk {
class Window;

	/**
	 * Owner of the command pool and per-frame sync objects.
	 * Drives the application given to it.
	 */
	class Runner {
	public:
		static constexpr uint32_t MaxFramesInFlight = 2;

		Runner(VulkanContext& ctx, Swapchain& swapchain, Application& app, Window& window);
		~Runner();

		Runner(const Runner&) = delete;
		Runner& operator=(const Runner&) = delete;
		Runner(Runner&&) = delete;
		Runner& operator=(Runner&&) = delete;

		void Step(float dt);
		void RecreateSwapchain();


	private:
		struct Frame {
			VkCommandBuffer cmd = VK_NULL_HANDLE;
			VkSemaphore imageAvailable = VK_NULL_HANDLE;
			VkFence inFlight = VK_NULL_HANDLE;
		};

		VulkanContext& ctx;
		Swapchain& swapchain;
		Application& app;

		VkCommandPool commandPool = VK_NULL_HANDLE;
		std::vector<Frame> frames;
		std::vector<VkSemaphore> renderFinished;
		uint32_t currentFrame = 0;

		void CreateSemaphores();
		void DestroySemaphores();
		void DestroyResources();
	};

}
