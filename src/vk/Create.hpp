//
// Created by Gregstr on 03/11/2026.
//

#pragma once
#include <cstdint>
#include <initializer_list>

#include <volk.h>

#include "VkSilk.hpp"

namespace silk::vk {

// Data tuples for the calls that genuinely need more than one dimension.
struct ShaderStage {
	VkShaderStageFlagBits stage;
	VkShaderModule module;
	const char* entry = "main";
};

struct VertexBinding {
	uint32_t binding;
	uint32_t stride;
	VkVertexInputRate rate = VK_VERTEX_INPUT_RATE_VERTEX;
};

struct VertexAttribute {
	uint32_t location;
	uint32_t binding;
	VkFormat format;
	uint32_t offset;
};

// pass + attachments
[[nodiscard]] VkRenderPass CreateRenderPass(VkDevice device, VkFormat colorFormat);
[[nodiscard]] VkFramebuffer CreateFramebuffer(VkDevice device,
        VkRenderPass renderPass, VkImageView view, VkExtent2D extent);
[[nodiscard]] VkImageView CreateImageView(VkDevice device, VkImage image,
        VkFormat format);

// shaders + pipelines
[[nodiscard]] VkShaderModule CreateShaderModule(VkDevice device, const char* path);
[[nodiscard]] VkPipelineLayout CreatePipelineLayout(VkDevice device,
        std::initializer_list<VkPushConstantRange> pushConstants = {},
        std::initializer_list<VkDescriptorSetLayout> layouts = {});
[[nodiscard]] VkPipeline CreateGraphicsPipeline(VkDevice device,
        VkRenderPass renderPass, VkPipelineLayout layout,
        std::initializer_list<ShaderStage> stages,
        std::initializer_list<VertexBinding> bindings = {},
        std::initializer_list<VertexAttribute> attributes = {});

// buffers + memory
[[nodiscard]] VkBuffer CreateBuffer(VkDevice device, VkDeviceSize size,
        VkBufferUsageFlags usage);
[[nodiscard]] uint32_t FindMemoryType(const VulkanContext& ctx,
        uint32_t typeFilter, VkMemoryPropertyFlags properties);
[[nodiscard]] VkDeviceMemory AllocateMemory(const VulkanContext& ctx,
        VkBuffer buffer, VkMemoryPropertyFlags properties);
[[nodiscard]] VkDeviceMemory AllocateMemory(const VulkanContext& ctx,
        VkDeviceSize size, uint32_t memoryTypeIndex);
void BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory);
void CopyToMemory(VkDevice device, VkDeviceMemory memory,
        const void* data, VkDeviceSize size);

// sync
[[nodiscard]] VkSemaphore CreateSemaphore(VkDevice device);
[[nodiscard]] VkFence CreateFence(VkDevice device, bool signaled = false);
void WaitForFence(VkDevice device, VkFence fence);
void ResetFence(VkDevice device, VkFence fence);

}
