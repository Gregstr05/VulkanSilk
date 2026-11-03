//
// Created by Gregstr on 18/09/2026.
//

#define GLFW_INCLUDE_VULKAN
#define VK_NO_PROTOTYPES
#include <GLFW/glfw3.h>

#include <vector>

#include "app/Application.hpp"
#include "app/Runner.hpp"
#include "vk/Swapchain.hpp"
#include "vk/VkSilk.hpp"

using namespace silk;

// The world's smallest renderer: clear the swapchain to a drifting color.
class ClearApp : public Application {
public:
    void OnInit(const VulkanContext& ctx, const Swapchain& swapchain) override
    {
        device = ctx.Device();
        CreateRenderPass(swapchain);
        CreateFramebuffers(swapchain);
    }

    void OnUpdate(float dt) override
    {
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
        vkCreateRenderPass(device, &ci, nullptr, &renderPass);
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
            vkCreateFramebuffer(device, &ci, nullptr, &framebuffers[i]);
        }
    }

    void DestroyFramebuffers()
    {
        for (VkFramebuffer fb : framebuffers)
            vkDestroyFramebuffer(device, fb, nullptr);
        framebuffers.clear();
    }

    VkDevice device = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers;
    float color[3] = {0.10f, 0.20f, 0.30f};
};

int main()
{
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Silk Sandbox", nullptr, nullptr);

    bool resized = false;
    glfwSetWindowUserPointer(window, &resized);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int, int) {
        *static_cast<bool*>(glfwGetWindowUserPointer(w)) = true;
    });

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    ContextDesc desc;
    desc.applicationName = "Silk Sandbox";
    desc.instanceExtensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // Lifetime order: context -> swapchain -> app -> runner.
    // Destruction runs in reverse, which is exactly the order Vulkan wants.
    VulkanContext ctx{desc, [window](VkInstance instance) {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        glfwCreateWindowSurface(instance, window, nullptr, &surface);
        return surface;
    }};
    Swapchain swapchain{ctx};
    ClearApp app;
    Runner runner{ctx, swapchain, app};

    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
            continue;

        double now = glfwGetTime();
        float dt = static_cast<float>(now - lastTime);
        lastTime = now;

        if (resized)
        {
            resized = false;
            runner.RecreateSwapchain();
        }

        runner.Step(dt);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
