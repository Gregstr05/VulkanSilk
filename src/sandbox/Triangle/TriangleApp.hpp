//
// Created by gregstr on 10.11.26.
//

#pragma once
#include <vector>

#include <volk.h>

#include "Application.hpp"
#include "Commands.hpp"
#include "Create.hpp"
#include "MathUtils.hpp"

using namespace silk;
using namespace silk::math;

class TriangleApp : public Application {
public:
    void OnInit(const VulkanContext& ctx, const Swapchain& swapchain, Window& window) override
    {
        device = ctx.Device();

        renderPass = vk::CreateRenderPass(device, swapchain.Format());

        m_window = &window;

        framebuffers.reserve(swapchain.ImageCount());
        for (uint32_t i = 0; i < swapchain.ImageCount(); ++i)
            framebuffers.push_back(vk::CreateFramebuffer(device, renderPass,
                    swapchain.View(i), swapchain.Extent()));

        VkShaderModule vert =
                vk::CreateShaderModule(device, "shaders/triangle.vert.spv");
        VkShaderModule frag =
                vk::CreateShaderModule(device, "shaders/triangle.frag.spv");

        layout = vk::CreatePipelineLayout(device,
                {{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(math::Matrix4)}});

        pipeline = vk::CreateGraphicsPipeline(device, renderPass, layout,
                {{VK_SHADER_STAGE_VERTEX_BIT, vert},
                 {VK_SHADER_STAGE_FRAGMENT_BIT, frag}});

        // the pipeline owns the compiled code, the modules were just a means
        vkDestroyShaderModule(device, vert, nullptr);
        vkDestroyShaderModule(device, frag, nullptr);
    }

    void OnUpdate(float dt) override
    {
        angle += dt * 0.5f;
        if (m_window->IsKeyDown(Key::Escape))
            m_window->RequestClose();
    }

    void OnRender(const FrameInfo& info) override
    {
        float aspect = static_cast<float>(info.extent.width) / info.extent.height;
        math::Matrix4 mvp = Perspective(1.0472f, aspect, 0.1f, 100.f) *
                            LookAt(Vector3(2.f, -3.f, 1.5f), Vector3(0.f, 0.f, 0.f)) *
                            TRS(Vector3(0.f, 0.f, 0.f),
                                Quat::FromAxisAngle(Vector3(0.f, 0.f, 1.f), angle),
                                Vector3(1.f, 1.f, 1.f));

        vk::BeginRenderPass(info.cmd, renderPass,
                framebuffers[info.swapchainIndex], info.extent,
                {0.02f, 0.03f, 0.05f, 1.f});
        vk::BindGraphicsPipeline(info.cmd, pipeline);
        vk::SetViewportScissor(info.cmd, info.extent);
        vk::PushConstants(info.cmd, layout, VK_SHADER_STAGE_VERTEX_BIT, Transpose(mvp));
        vk::Draw(info.cmd, 3);
        vk::EndRenderPass(info.cmd);
    }

    void OnResize(const Swapchain& swapchain) override
    {
        DestroyFramebuffers();
        framebuffers.reserve(swapchain.ImageCount());
        for (uint32_t i = 0; i < swapchain.ImageCount(); ++i)
            framebuffers.push_back(vk::CreateFramebuffer(device, renderPass,
                    swapchain.View(i), swapchain.Extent()));
        // note: no pipeline rebuild - dynamic viewport/scissor covers it
    }

    void OnShutdown(const VulkanContext& ctx) override
    {
        DestroyFramebuffers();
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, layout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
    }

private:
    void DestroyFramebuffers()
    {
        for (VkFramebuffer framebuffer : framebuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        framebuffers.clear();
    }

    VkDevice device = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers;
    float angle = 0.f;

    Window* m_window = nullptr;
};
