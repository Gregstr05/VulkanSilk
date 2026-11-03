//
// Created by Gregstr on 18/09/2026.
//

#include "Runner.hpp"

#include <stdexcept>

namespace silk {

Runner::Runner(VulkanContext& context, Swapchain& swap, Application& application, Window& window)
    : ctx(context), swapchain(swap), app(application)
{
    try
    {
        VkCommandPoolCreateInfo poolCI{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        poolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolCI.queueFamilyIndex = ctx.GraphicsFamily();
        if (vkCreateCommandPool(ctx.Device(), &poolCI, nullptr, &commandPool) != VK_SUCCESS)
            throw std::runtime_error("failed to create command pool");

        frames.resize(MaxFramesInFlight);
        VkCommandBufferAllocateInfo allocCI{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocCI.commandPool = commandPool;
        allocCI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocCI.commandBufferCount = 1;

        for (Frame& frame : frames)
        {
            vkAllocateCommandBuffers(ctx.Device(), &allocCI, &frame.cmd);

            VkFenceCreateInfo fenceCI{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT; // first Step skips the wait
            vkCreateFence(ctx.Device(), &fenceCI, nullptr, &frame.inFlight);
        }

        CreateSemaphores();

        app.OnInit(ctx, swapchain, window);
    }
    catch (...)
    {
        // no OnShutdown here: OnInit never finished
        DestroyResources();
        throw;
    }
}

Runner::~Runner()
{
    vkDeviceWaitIdle(ctx.Device());
    app.OnShutdown(ctx);
    DestroyResources();
}

void Runner::CreateSemaphores()
{
    VkSemaphoreCreateInfo semCI{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    for (Frame& frame : frames)
    {
        if (vkCreateSemaphore(ctx.Device(), &semCI, nullptr, &frame.imageAvailable) !=
            VK_SUCCESS)
            throw std::runtime_error("failed to create semaphore");
    }

    renderFinished.resize(swapchain.ImageCount());
    for (VkSemaphore& sem : renderFinished)
    {
        if (vkCreateSemaphore(ctx.Device(), &semCI, nullptr, &sem) != VK_SUCCESS)
            throw std::runtime_error("failed to create semaphore");
    }
}

void Runner::DestroySemaphores()
{
    for (Frame& frame : frames)
    {
        vkDestroySemaphore(ctx.Device(), frame.imageAvailable, nullptr);
        frame.imageAvailable = VK_NULL_HANDLE;
    }
    for (VkSemaphore sem : renderFinished)
        vkDestroySemaphore(ctx.Device(), sem, nullptr);
    renderFinished.clear();
}

void Runner::DestroyResources()
{
    DestroySemaphores();
    for (Frame& frame : frames)
    {
        vkDestroyFence(ctx.Device(), frame.inFlight, nullptr);
        frame.inFlight = VK_NULL_HANDLE;
    }
    frames.clear();
    if (commandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(ctx.Device(), commandPool, nullptr);
    commandPool = VK_NULL_HANDLE;
}

void Runner::Step(float dt)
{
    Frame& frame = frames[currentFrame];
    VkDevice device = ctx.Device();

    vkWaitForFences(device, 1, &frame.inFlight, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(
            device, swapchain.Handle(), UINT64_MAX, frame.imageAvailable,
            VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
        return;
    }

    vkResetFences(device, 1, &frame.inFlight);
    vkResetCommandBuffer(frame.cmd, 0);

    VkCommandBufferBeginInfo beginCI{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(frame.cmd, &beginCI);

    app.OnUpdate(dt);
    app.OnRender(FrameInfo{frame.cmd, swapchain.Extent(), currentFrame, imageIndex});

    vkEndCommandBuffer(frame.cmd);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &frame.imageAvailable;
    submit.pWaitDstStageMask = &waitStage;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &frame.cmd;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &renderFinished[imageIndex];

    if (vkQueueSubmit(ctx.GraphicsQueue(), 1, &submit, frame.inFlight) != VK_SUCCESS)
        throw std::runtime_error("failed to submit command buffer");

    VkSwapchainKHR handle = swapchain.Handle();
    VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &renderFinished[imageIndex];
    present.swapchainCount = 1;
    present.pSwapchains = &handle;
    present.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(ctx.PresentQueue(), &present);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        RecreateSwapchain();

    currentFrame = (currentFrame + 1) % MaxFramesInFlight;
}

void Runner::RecreateSwapchain()
{
    vkDeviceWaitIdle(ctx.Device());
    swapchain.Recreate();
    DestroySemaphores();
    CreateSemaphores();
    app.OnResize(swapchain);
}

}
