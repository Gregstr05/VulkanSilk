//
// Created by Gregstr on 18/09/2026.
//

#pragma once
#include <cstdint>

namespace silk {

struct FrameInfo {
	VkCommandBuffer cmd;
	VkExtent2D extent;
	uint32_t frameIndex;
	uint32_t swapchainIndex;
};

class Application {
public:
	virtual ~Application() = default;

	virtual void OnInit(const VulkanContext& ctx, const Swapchain& swapchain) = 0;

	virtual void OnUpdate(float dt) = 0;

	virtual void OnRender(const FrameInfo& info) = 0;

	virtual void OnResize(const Swapchain& swapchain) {}
};

}
