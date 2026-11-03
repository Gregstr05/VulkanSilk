//
// Created by Gregstr on 03/11/2026.
//

#pragma once
#include <vector>
#include <stdexcept>

#include <volk.h>

#include "Application.hpp"

using namespace silk;

class ClearApp : public Application {
public:
    void OnInit(const VulkanContext& ctx, const Swapchain& swapchain, Window& window) override
    {
        device = ctx.Device();
        CreateRenderPass(swapchain);
        CreateFramebuffers(swapchain);
        m_window = &window;
    }

    void OnUpdate(float dt) override
    {
        if (m_window->IsKeyDown(Key::Escape))
            m_window->RequestClose();

        color[0] += 0.05f * dt; // proof of life: hue slowly drifts
        color[1] += 0.03f * dt;
        color[2] += 0.07f * dt;
    }

    void OnRender(const FrameInfo& info) override
    {
        VkClearValue clear{};
        clear.color = {{color[0], color[1], color[2], 1.f}};

        VkRenderPassBeginInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        rp.renderPass = renderPass;
        rp.framebuffer = framebuffers[info.swapchainIndex];
        rp.renderArea.extent = info.extent;
        rp.clearValueCount = 1;
        rp.pClearValues = &clear;
        vkCmdBeginRenderPass(info.cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdEndRenderPass(info.cmd);
    }

    void OnResize(const Swapchain& swapchain) override
    {
        DestroyFramebuffers();
        CreateFramebuffers(swapchain);
    }

    void OnShutdown(const VulkanContext& ctx) override
    {
        DestroyFramebuffers();
        vkDestroyRenderPass(device, renderPass, nullptr);
        renderPass = VK_NULL_HANDLE;
    }

private:
    void CreateRenderPass(const Swapchain& swapchain)
    {
        VkAttachmentDescription color{};
        color.format = swapchain.Format();
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

        if (vkCreateRenderPass(device, &ci, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("failed to create render pass");
    }

    void CreateFramebuffers(const Swapchain& swapchain)
    {
        framebuffers.resize(swapchain.ImageCount());
        for (uint32_t i = 0; i < swapchain.ImageCount(); ++i)
        {
            VkImageView view = swapchain.View(i);
            VkFramebufferCreateInfo ci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            ci.renderPass = renderPass;
            ci.attachmentCount = 1;
            ci.pAttachments = &view;
            ci.width = swapchain.Extent().width;
            ci.height = swapchain.Extent().height;
            ci.layers = 1;

            if (vkCreateFramebuffer(device, &ci, nullptr, &framebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create framebuffer");
        }
    }

    void DestroyFramebuffers()
    {
        for (VkFramebuffer framebuffer : framebuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        framebuffers.clear();
    }

    VkDevice device = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers;
    float color[3] = {0.10f, 0.20f, 0.30f};
    Window* m_window = nullptr;
};
