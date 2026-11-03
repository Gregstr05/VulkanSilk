//
// Created by Gregstr on 07/10/2026.
//

#pragma once
#include <functional>
#include <vector>

#include <volk.h>

namespace silk {

struct ContextDesc {
    const char* applicationName = "Silk";
    std::vector<const char*> instanceExtensions;
    std::vector<const char*> deviceExtensions;
    bool enableValidation = true;
};

// Owns everything it creates: instance, debug messenger, surface,
// physical + logical device.
class VulkanContext {
public:

    /**
     * Context cannot create the surface without an instance
     * and the instance cannot be completely created without a surface.
     * The constructor takes the SurfaceProvider function so that it is window provider agnostic
     */
    using SurfaceProvider = std::function<VkSurfaceKHR(VkInstance)>;

    VulkanContext() = default;
    VulkanContext(const ContextDesc& desc, const SurfaceProvider& createSurface);
    ~VulkanContext();

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&& other) noexcept;
    VulkanContext& operator=(VulkanContext&& other) noexcept;

    [[nodiscard]] VkInstance Instance() const { return instance; }
    [[nodiscard]] VkPhysicalDevice PhysicalDevice() const { return physicalDevice; }
    [[nodiscard]] VkDevice Device() const { return device; }
    [[nodiscard]] VkSurfaceKHR Surface() const { return surface; }
    [[nodiscard]] VkQueue GraphicsQueue() const { return graphicsQueue; }
    [[nodiscard]] VkQueue PresentQueue() const { return presentQueue; }
    [[nodiscard]] uint32_t GraphicsFamily() const { return graphicsFamily; }
    [[nodiscard]] uint32_t PresentFamily() const { return presentFamily; }

private:
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    uint32_t graphicsFamily = 0;
    uint32_t presentFamily = 0;
    bool validation = false;

    void Destroy();
};

}
