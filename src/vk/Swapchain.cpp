//
// Created by Gregstr on 03/11/2026.
//

#include "Swapchain.hpp"

#include <algorithm>
#include <stdexcept>

namespace silk {

Swapchain::Swapchain(const VulkanContext& context)
    : ctx(&context)
{
    try
    {
        Recreate();
    }
    catch (...)
    {
        Destroy();
        throw;
    }
}

Swapchain::~Swapchain()
{
    Destroy();
}

Swapchain::Swapchain(Swapchain&& other) noexcept
{
    *this = std::move(other);
}

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept
{
    if (this != &other)
    {
        Destroy();
        ctx = other.ctx;
        swapchain = other.swapchain;
        images = std::move(other.images);
        views = std::move(other.views);
        format = other.format;
        extent = other.extent;

        other.ctx = nullptr;
        other.swapchain = VK_NULL_HANDLE;
        other.format = VK_FORMAT_UNDEFINED;
    }
    return *this;
}

void Swapchain::Destroy()
{
    if (ctx == nullptr)
        return;
    for (VkImageView view : views)
        vkDestroyImageView(ctx->Device(), view, nullptr);
    views.clear();
    images.clear();
    if (swapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(ctx->Device(), swapchain, nullptr);
    swapchain = VK_NULL_HANDLE;
}

void Swapchain::Recreate()
{
    VkDevice device = ctx->Device();
    VkPhysicalDevice physicalDevice = ctx->PhysicalDevice();
    VkSurfaceKHR surface = ctx->Surface();

    // views belong to the current swapchain's images: retire them first
    for (VkImageView view : views)
        vkDestroyImageView(device, view, nullptr);
    views.clear();
    images.clear();

    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount == 0)
        throw std::runtime_error("surface exposes no formats");
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice, surface, &formatCount, formats.data());

    VkSurfaceFormatKHR chosen = formats[0];
    for (const auto& candidate : formats)
    {
        if (candidate.format == VK_FORMAT_B8G8R8A8_SRGB &&
            candidate.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            chosen = candidate;
            break;
        }
    }
    format = chosen.format;

    if (caps.currentExtent.width != UINT32_MAX)
    {
        extent = caps.currentExtent;
    }
    else
    {
        // rare platforms: keep the last extent, clamped into bounds
        extent.width = std::clamp(
                extent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = std::clamp(
                extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
    }

    // FIFO is guaranteed
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
        imageCount = caps.maxImageCount;

    VkSwapchainKHR old = swapchain;
    swapchain = VK_NULL_HANDLE;

    try
    {
        VkSwapchainCreateInfoKHR ci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        ci.surface = surface;
        ci.minImageCount = imageCount;
        ci.imageFormat = chosen.format;
        ci.imageColorSpace = chosen.colorSpace;
        ci.imageExtent = extent;
        ci.imageArrayLayers = 1;
        ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ci.preTransform = caps.currentTransform;
        ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        ci.presentMode = presentMode;
        ci.clipped = VK_TRUE;
        ci.oldSwapchain = old;

        if (vkCreateSwapchainKHR(device, &ci, nullptr, &swapchain) != VK_SUCCESS)
            throw std::runtime_error("failed to create swapchain");

        uint32_t count = 0;
        vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
        images.resize(count);
        vkGetSwapchainImagesKHR(device, swapchain, &count, images.data());

        views.resize(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            VkImageViewCreateInfo viewCI{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            viewCI.image = images[i];
            viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewCI.format = format;
            viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewCI.subresourceRange.levelCount = 1;
            viewCI.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &viewCI, nullptr, &views[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create swapchain image view");
        }
    }
    catch (...)
    {
        if (old != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(device, old, nullptr);
        throw;
    }

    if (old != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(device, old, nullptr);
}

}
