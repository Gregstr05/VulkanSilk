//
// Created by Gregstr on 03/11/2026.
//

#include "Create.hpp"

#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace silk::vk {

namespace {

void Check(VkResult result, const char* what)
{
    if (result != VK_SUCCESS)
        throw std::runtime_error(what);
}

}

VkRenderPass CreateRenderPass(VkDevice device, VkFormat colorFormat)
{
    VkAttachmentDescription color{};
    color.format = colorFormat;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    VkSubpassDependency dep{};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo ci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    ci.attachmentCount = 1;
    ci.pAttachments = &color;
    ci.subpassCount = 1;
    ci.pSubpasses = &subpass;
    ci.dependencyCount = 1;
    ci.pDependencies = &dep;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    Check(vkCreateRenderPass(device, &ci, nullptr, &renderPass),
          "failed to create render pass");
    return renderPass;
}

VkFramebuffer CreateFramebuffer(VkDevice device, VkRenderPass renderPass,
        VkImageView view, VkExtent2D extent)
{
    VkFramebufferCreateInfo ci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    ci.renderPass = renderPass;
    ci.attachmentCount = 1;
    ci.pAttachments = &view;
    ci.width = extent.width;
    ci.height = extent.height;
    ci.layers = 1;

    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    Check(vkCreateFramebuffer(device, &ci, nullptr, &framebuffer),
          "failed to create framebuffer");
    return framebuffer;
}

VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format)
{
    VkImageViewCreateInfo ci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    ci.image = image;
    ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ci.format = format;
    ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ci.subresourceRange.levelCount = 1;
    ci.subresourceRange.layerCount = 1;

    VkImageView view = VK_NULL_HANDLE;
    Check(vkCreateImageView(device, &ci, nullptr, &view),
          "failed to create image view");
    return view;
}

VkShaderModule CreateShaderModule(VkDevice device, const char* path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        throw std::runtime_error(std::string("failed to open shader: ") + path);

    std::size_t size = static_cast<std::size_t>(file.tellg());
    std::vector<char> code(size);
    file.seekg(0);
    file.read(code.data(), static_cast<std::streamsize>(size));

    VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    ci.codeSize = size;
    ci.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule module = VK_NULL_HANDLE;
    Check(vkCreateShaderModule(device, &ci, nullptr, &module),
          "failed to create shader module");
    return module;
}

VkPipelineLayout CreatePipelineLayout(VkDevice device,
        std::initializer_list<VkPushConstantRange> pushConstants,
        std::initializer_list<VkDescriptorSetLayout> layouts)
{
    std::vector<VkPushConstantRange> ranges(pushConstants);
    std::vector<VkDescriptorSetLayout> setLayouts(layouts);

    VkPipelineLayoutCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    ci.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    ci.pSetLayouts = setLayouts.data();
    ci.pushConstantRangeCount = static_cast<uint32_t>(ranges.size());
    ci.pPushConstantRanges = ranges.data();

    VkPipelineLayout layout = VK_NULL_HANDLE;
    Check(vkCreatePipelineLayout(device, &ci, nullptr, &layout),
          "failed to create pipeline layout");
    return layout;
}

VkPipeline CreateGraphicsPipeline(VkDevice device, VkRenderPass renderPass,
        VkPipelineLayout layout, std::initializer_list<ShaderStage> stages,
        std::initializer_list<VertexBinding> bindings,
        std::initializer_list<VertexAttribute> attributes)
{
    std::vector<VkPipelineShaderStageCreateInfo> stageInfos;
    for (const ShaderStage& stage : stages)
    {
        VkPipelineShaderStageCreateInfo info{
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        info.stage = stage.stage;
        info.module = stage.module;
        info.pName = stage.entry;
        stageInfos.push_back(info);
    }

    std::vector<VkVertexInputBindingDescription> bindingInfos;
    for (const VertexBinding& binding : bindings)
        bindingInfos.push_back({binding.binding, binding.stride, binding.rate});

    std::vector<VkVertexInputAttributeDescription> attributeInfos;
    for (const VertexAttribute& attribute : attributes)
    {
        attributeInfos.push_back({attribute.location, attribute.binding,
                attribute.format, attribute.offset});
    }

    VkPipelineVertexInputStateCreateInfo vertexInput{
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInput.vertexBindingDescriptionCount =
            static_cast<uint32_t>(bindingInfos.size());
    vertexInput.pVertexBindingDescriptions = bindingInfos.data();
    vertexInput.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(attributeInfos.size());
    vertexInput.pVertexAttributeDescriptions = attributeInfos.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // viewport/scissor are always dynamic: pipelines survive resizes for free
    VkPipelineViewportStateCreateInfo viewportState{
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo raster{
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.cullMode = VK_CULL_MODE_BACK_BIT;
    // matches the math lib's y-flipped Vulkan projection (see MathUtils)
    raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster.lineWidth = 1.f;

    VkPipelineMultisampleStateCreateInfo multisample{
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample.minSampleShading = 1.f;

    // no depth attachment in this render pass: depth test stays off
    VkPipelineDepthStencilStateCreateInfo depthStencil{
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

    VkPipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo blend{
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    blend.attachmentCount = 1;
    blend.pAttachments = &blendAttachment;

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic{
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamic.dynamicStateCount = 2;
    dynamic.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo ci{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    ci.stageCount = static_cast<uint32_t>(stageInfos.size());
    ci.pStages = stageInfos.data();
    ci.pVertexInputState = &vertexInput;
    ci.pInputAssemblyState = &inputAssembly;
    ci.pViewportState = &viewportState;
    ci.pRasterizationState = &raster;
    ci.pMultisampleState = &multisample;
    ci.pDepthStencilState = &depthStencil;
    ci.pColorBlendState = &blend;
    ci.pDynamicState = &dynamic;
    ci.layout = layout;
    ci.renderPass = renderPass;
    ci.subpass = 0;

    VkPipeline pipeline = VK_NULL_HANDLE;
    Check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &ci,
                  nullptr, &pipeline),
          "failed to create graphics pipeline");
    return pipeline;
}

VkBuffer CreateBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo ci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    ci.size = size;
    ci.usage = usage;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer buffer = VK_NULL_HANDLE;
    Check(vkCreateBuffer(device, &ci, nullptr, &buffer),
          "failed to create buffer");
    return buffer;
}

uint32_t FindMemoryType(const VulkanContext& ctx, uint32_t typeFilter,
        VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memory{};
    vkGetPhysicalDeviceMemoryProperties(ctx.PhysicalDevice(), &memory);

    for (uint32_t i = 0; i < memory.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1u << i)) &&
            (memory.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    throw std::runtime_error("no memory type matches the requested properties");
}

VkDeviceMemory AllocateMemory(const VulkanContext& ctx, VkDeviceSize size,
        uint32_t memoryTypeIndex)
{
    VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    ai.allocationSize = size;
    ai.memoryTypeIndex = memoryTypeIndex;

    VkDeviceMemory memory = VK_NULL_HANDLE;
    Check(vkAllocateMemory(ctx.Device(), &ai, nullptr, &memory),
          "failed to allocate memory");
    return memory;
}

VkDeviceMemory AllocateMemory(const VulkanContext& ctx, VkBuffer buffer,
        VkMemoryPropertyFlags properties)
{
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(ctx.Device(), buffer, &requirements);
    return AllocateMemory(ctx, requirements.size,
            FindMemoryType(ctx, requirements.memoryTypeBits, properties));
}

void BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory)
{
    Check(vkBindBufferMemory(device, buffer, memory, 0),
          "failed to bind buffer memory");
}

void CopyToMemory(VkDevice device, VkDeviceMemory memory,
        const void* data, VkDeviceSize size)
{
    void* mapped = nullptr;
    Check(vkMapMemory(device, memory, 0, size, 0, &mapped),
          "failed to map memory");
    std::memcpy(mapped, data, static_cast<std::size_t>(size));
    vkUnmapMemory(device, memory);
}

VkSemaphore CreateSemaphore(VkDevice device)
{
    VkSemaphoreCreateInfo ci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkSemaphore semaphore = VK_NULL_HANDLE;
    Check(vkCreateSemaphore(device, &ci, nullptr, &semaphore),
          "failed to create semaphore");
    return semaphore;
}

VkFence CreateFence(VkDevice device, bool signaled)
{
    VkFenceCreateInfo ci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (signaled)
        ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VkFence fence = VK_NULL_HANDLE;
    Check(vkCreateFence(device, &ci, nullptr, &fence),
          "failed to create fence");
    return fence;
}

void WaitForFence(VkDevice device, VkFence fence)
{
    vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
}

void ResetFence(VkDevice device, VkFence fence)
{
    vkResetFences(device, 1, &fence);
}

}
