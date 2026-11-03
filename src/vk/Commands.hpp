//
// Created by Gregstr on 03/11/2026.
//

#pragma once

#include <cstddef>
#include <initializer_list>

#include <volk.h>

namespace silk::vk {

inline void BeginRenderPass(VkCommandBuffer cmd, VkRenderPass renderPass,
        VkFramebuffer framebuffer, VkExtent2D extent,
        std::initializer_list<float> clearColor = {0.f, 0.f, 0.f, 1.f})
{
    float color[4]{};
    std::size_t i = 0;
    for (float c : clearColor)
        color[i++] = c;

    VkClearValue clear{};
    clear.color = {{color[0], color[1], color[2], color[3]}};

    VkRenderPassBeginInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    rp.renderPass = renderPass;
    rp.framebuffer = framebuffer;
    rp.renderArea.extent = extent;
    rp.clearValueCount = 1;
    rp.pClearValues = &clear;
    vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);
}

inline void EndRenderPass(VkCommandBuffer cmd)
{
    vkCmdEndRenderPass(cmd);
}

inline void BindGraphicsPipeline(VkCommandBuffer cmd, VkPipeline pipeline)
{
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

inline void SetViewportScissor(VkCommandBuffer cmd, VkExtent2D extent)
{
    VkViewport viewport{0.f, 0.f,
            static_cast<float>(extent.width),
            static_cast<float>(extent.height), 0.f, 1.f};
    VkRect2D scissor{{0, 0}, extent};
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

template <typename T>
inline void PushConstants(VkCommandBuffer cmd, VkPipelineLayout layout,
        VkShaderStageFlags stages, const T& value)
{
    vkCmdPushConstants(cmd, layout, stages, 0, sizeof(T), &value);
}

inline void Draw(VkCommandBuffer cmd, uint32_t vertexCount,
        uint32_t instanceCount = 1, uint32_t firstVertex = 0,
        uint32_t firstInstance = 0)
{
    vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
}

inline void DrawIndexed(VkCommandBuffer cmd, uint32_t indexCount,
        uint32_t instanceCount = 1, uint32_t firstIndex = 0,
        int32_t vertexOffset = 0, uint32_t firstInstance = 0)
{
    vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstIndex,
            vertexOffset, firstInstance);
}

inline void BindVertexBuffer(VkCommandBuffer cmd, VkBuffer buffer,
        VkDeviceSize offset = 0)
{
    vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, &offset);
}

inline void BindIndexBuffer(VkCommandBuffer cmd, VkBuffer buffer,
        VkIndexType type, VkDeviceSize offset = 0)
{
    vkCmdBindIndexBuffer(cmd, buffer, offset, type);
}

inline void BindDescriptorSets(VkCommandBuffer cmd, VkPipelineLayout layout,
        std::initializer_list<VkDescriptorSet> sets)
{
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
            0, static_cast<uint32_t>(sets.size()), sets.begin(), 0, nullptr);
}

}
