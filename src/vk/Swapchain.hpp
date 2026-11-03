//
// Created by Gregstr on 03/11/2026.
//

#pragma once
#include <vector>

#include "VkSilk.hpp"

namespace silk {

	// Owns the VkSwapchainKHR and its image views. Borrows the context.
	class Swapchain {
	public:
		Swapchain() = default;
		explicit Swapchain(const VulkanContext& ctx);
		~Swapchain();

		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;
		Swapchain(Swapchain&& other) noexcept;
		Swapchain& operator=(Swapchain&& other) noexcept;

		void Recreate();

		[[nodiscard]] VkSwapchainKHR Handle() const { return swapchain; }
		[[nodiscard]] VkFormat Format() const { return format; }
		[[nodiscard]] VkExtent2D Extent() const { return extent; }
		[[nodiscard]] uint32_t ImageCount() const
		{
			return static_cast<uint32_t>(images.size());
		}
		[[nodiscard]] VkImageView View(uint32_t i) const { return views[i]; }

	private:
		const VulkanContext* ctx = nullptr; // borrowed
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		std::vector<VkImage> images; // owned by the swapchain itself
		std::vector<VkImageView> views;
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkExtent2D extent{};

		void Destroy();
	};

}
