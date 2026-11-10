//
// Created by Gregstr on 07/10/2026.
//

//#define VOLK_IMPLEMENTATION

#include "VkSilk.hpp"

#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace silk {

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT types,
        const VkDebugUtilsMessengerCallbackDataEXT* data,
        void* userData)
{
    std::fprintf(stderr, "[validation] %s\n", data->pMessage);
    return VK_FALSE;
}

bool SupportsLayer(const char* name)
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> available(count);
    vkEnumerateInstanceLayerProperties(&count, available.data());

    for (const auto& layer : available)
        if (std::strcmp(layer.layerName, name) == 0)
            return true;
    return false;
}

// Discrete beats integrated, everything else loses
int ScoreDevice(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(device, &props);
    switch (props.deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return 2;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return 1;
        default:
            return 0;
    }
}

}

VulkanContext::VulkanContext(const ContextDesc& desc, const SurfaceProvider& createSurface)
{
    try
    {
        if (volkInitialize() != VK_SUCCESS)
            throw std::runtime_error("volkInitialize failed - is a Vulkan loader installed?");

        // instance
        VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        appInfo.pApplicationName = desc.applicationName;
        appInfo.pEngineName = "Silk";
        appInfo.apiVersion = VK_API_VERSION_1_3;

        std::vector<const char*> extensions = desc.instanceExtensions;
        std::vector<const char*> layers;
        if (desc.enableValidation && SupportsLayer("VK_LAYER_KHRONOS_validation"))
        {
            validation = true;
            layers.push_back("VK_LAYER_KHRONOS_validation");
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        VkInstanceCreateInfo instanceCI{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        instanceCI.pApplicationInfo = &appInfo;
        instanceCI.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        instanceCI.ppEnabledExtensionNames = extensions.data();
        instanceCI.enabledLayerCount = static_cast<uint32_t>(layers.size());
        instanceCI.ppEnabledLayerNames = layers.data();

        if (vkCreateInstance(&instanceCI, nullptr, &instance) != VK_SUCCESS)
            throw std::runtime_error("failed to create VkInstance");

        volkLoadInstance(instance);

        if (validation)
        {
            VkDebugUtilsMessengerCreateInfoEXT debugCI{
                    VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
            debugCI.messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCI.messageType =
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCI.pfnUserCallback = DebugCallback;
            vkCreateDebugUtilsMessengerEXT(instance, &debugCI, nullptr, &debugMessenger);
        }

        // surface needs the instance, so this is the only moment we can ask
        surface = createSurface(instance);
        if (surface == VK_NULL_HANDLE)
            throw std::runtime_error("surface provider returned VK_NULL_HANDLE");

        // physical device + queue families
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            throw std::runtime_error("no Vulkan-capable device found");
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        int bestScore = -1;
        for (VkPhysicalDevice candidate : devices)
        {
            uint32_t familyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(candidate, &familyCount, nullptr);
            std::vector<VkQueueFamilyProperties> families(familyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(
                    candidate, &familyCount, families.data());

            uint32_t graphics = UINT32_MAX;
            uint32_t present = UINT32_MAX;
            for (uint32_t i = 0; i < familyCount; ++i)
            {
                if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    graphics = i;
                VkBool32 presentSupported = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(
                        candidate, i, surface, &presentSupported);
                if (presentSupported)
                    present = i;
            }

            if (graphics == UINT32_MAX || present == UINT32_MAX)
                continue;

            int score = ScoreDevice(candidate);
            if (score > bestScore)
            {
                bestScore = score;
                physicalDevice = candidate;
                graphicsFamily = graphics;
                presentFamily = present;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("no device with graphics + present support");

        // --- logical device ---
        float priority = 1.f;
        std::vector<VkDeviceQueueCreateInfo> queueCIs;
        VkDeviceQueueCreateInfo graphicsCI{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        graphicsCI.queueFamilyIndex = graphicsFamily;
        graphicsCI.queueCount = 1;
        graphicsCI.pQueuePriorities = &priority;
        queueCIs.push_back(graphicsCI);
        if (presentFamily != graphicsFamily)
        {
            VkDeviceQueueCreateInfo presentCI{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
            presentCI.queueFamilyIndex = presentFamily;
            presentCI.queueCount = 1;
            presentCI.pQueuePriorities = &priority;
            queueCIs.push_back(presentCI);
        }

        std::vector<const char*> deviceExtensions = desc.deviceExtensions;
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VkPhysicalDeviceFeatures features{};

        VkDeviceCreateInfo deviceCI{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        deviceCI.queueCreateInfoCount = static_cast<uint32_t>(queueCIs.size());
        deviceCI.pQueueCreateInfos = queueCIs.data();
        deviceCI.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCI.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCI.pEnabledFeatures = &features;

        if (vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device) != VK_SUCCESS)
            throw std::runtime_error("failed to create logical device");

        vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
        vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);
        volkLoadDevice(device);
    }
    catch (...)
    {
        // the destructor does not run for a throwing constructor:
        // clean up the half-built state ourselves, then rethrow
        Destroy();
        throw;
    }
}

VulkanContext::~VulkanContext()
{
    Destroy();
}

VulkanContext::VulkanContext(VulkanContext&& other) noexcept
{
    *this = std::move(other);
}

VulkanContext& VulkanContext::operator=(VulkanContext&& other) noexcept
{
    if (this != &other)
    {
        Destroy();
        instance = other.instance;
        debugMessenger = other.debugMessenger;
        surface = other.surface;
        physicalDevice = other.physicalDevice;
        device = other.device;
        graphicsQueue = other.graphicsQueue;
        presentQueue = other.presentQueue;
        graphicsFamily = other.graphicsFamily;
        presentFamily = other.presentFamily;
        validation = other.validation;

        other.instance = VK_NULL_HANDLE;
        other.debugMessenger = VK_NULL_HANDLE;
        other.surface = VK_NULL_HANDLE;
        other.physicalDevice = VK_NULL_HANDLE;
        other.device = VK_NULL_HANDLE;
        other.graphicsQueue = VK_NULL_HANDLE;
        other.presentQueue = VK_NULL_HANDLE;
        other.validation = false;
    }
    return *this;
}

void VulkanContext::Destroy()
{
    // children before parents, in strict order
    if (device != VK_NULL_HANDLE)
        vkDestroyDevice(device, nullptr);
    if (surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance, surface, nullptr);
    if (debugMessenger != VK_NULL_HANDLE)
        vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    if (instance != VK_NULL_HANDLE)
        vkDestroyInstance(instance, nullptr);

    device = VK_NULL_HANDLE;
    surface = VK_NULL_HANDLE;
    debugMessenger = VK_NULL_HANDLE;
    instance = VK_NULL_HANDLE;
    physicalDevice = VK_NULL_HANDLE;
    validation = false;
}

}
