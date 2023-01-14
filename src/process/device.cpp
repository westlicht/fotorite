#include "device.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <volk.h>

#include <spdlog/spdlog.h>
#include <fmt/core.h>

#include <optional>
#include <stdexcept>
#include <vector>

#define VK_CHECK(call)                                                                                            \
    if (VkResult result = call; result != VK_SUCCESS) {                                                           \
        throw std::runtime_error(                                                                                 \
            fmt::format("{}:{} VK error:\n{} failed with result {}", __FILE__, __LINE__, #call, result).c_str()); \
    }

FR_NAMESPACE_BEGIN

static const VkFilter SAMPLER_FILTER_MAP[] = {
    // SamplerFilter::Nearest
    VK_FILTER_NEAREST,
    // SamplerFilter::Linear
    VK_FILTER_LINEAR,
};

static const VkSamplerMipmapMode SAMPLER_MIP_MAP_MODE_MAP[] = {
    // SamplerMipMapMode::Nearest
    VK_SAMPLER_MIPMAP_MODE_NEAREST,
    // SamplerMipMapMode::Linear
    VK_SAMPLER_MIPMAP_MODE_LINEAR,
};

static const VkSamplerAddressMode SAMPLER_ADDRESS_MODE_MAP[] = {
    // SamplerAddressMode::Repeat
    VK_SAMPLER_ADDRESS_MODE_REPEAT,
    // SamplerAddressMode::MirroredRepeat
    VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
    // SamplerAddressMode::ClampToEdge
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    // SamplerAddressMode::ClampToBorder
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    // SamplerAddressMode::MirrorClampToEdge
    VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT type,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                     void *user_data)
{
    spdlog::info("VK: {}", callback_data->pMessage);
    return VK_FALSE;
}

struct BufferImpl {
    BufferDesc desc;
    VkBuffer buffer;
};

struct ImageImpl {
    ImageDesc desc;
    VkImage image;
};

struct SamplerImpl {
    SamplerDesc desc;
    VkSampler sampler;
};

struct PipelineImpl {
    PipelineDesc desc;
    VkPipeline pipeline;
};

struct DeviceImpl {
    DeviceImpl(const DeviceDesc &desc);
    ~DeviceImpl();

    DeviceDesc desc;
    VkInstance instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debug_messenger{VK_NULL_HANDLE};
    VkPhysicalDevice physical_device{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue queue;

    Pool<BufferImpl, BufferHandle> buffers;
    Pool<ImageImpl, ImageHandle> images;
    Pool<SamplerImpl, SamplerHandle> samplers;
    Pool<PipelineImpl, PipelineHandle> pipelines;
};

DeviceImpl::DeviceImpl(const DeviceDesc &desc_) : desc(desc_)
{
    VK_CHECK(volkInitialize());

    // enumerate validation layers
    uint32_t layer_count;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));
    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    std::vector<const char *> validation_layers;
    if (desc.enable_validation_layers) {
        validation_layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    // enumerate instance extensions
    uint32_t extension_count = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));
    std::vector<VkExtensionProperties> extensions(extension_count);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data()));

    VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.pApplicationName = "fotorite";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "no engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    std::vector<const char *> required_extensions;
    // required_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    if (desc.enable_validation_layers) {
        required_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkInstanceCreateInfo instance_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instance_info.pApplicationInfo = &app_info;
    // instance_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instance_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    instance_info.ppEnabledExtensionNames = required_extensions.data();
    instance_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    instance_info.ppEnabledLayerNames = validation_layers.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_info{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    if (desc.enable_validation_layers) {
        debug_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_info.pfnUserCallback = debug_callback;
        instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_info;
    }

    VK_CHECK(vkCreateInstance(&instance_info, nullptr, &instance));
    volkLoadInstance(instance);

    if (desc.enable_validation_layers) {
        VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &debug_info, nullptr, &debug_messenger));
    }

    // find physical device
    {
        uint32_t device_count = 0;
        VK_CHECK(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));
        std::vector<VkPhysicalDevice> devices(device_count);
        VK_CHECK(vkEnumeratePhysicalDevices(instance, &device_count, devices.data()));

        physical_device = devices.empty() ? VK_NULL_HANDLE : devices[0];

        if (!physical_device)
            throw std::runtime_error("No suitable Vulkan device available!");
    }

    // check queues
    std::optional<uint32_t> graphics_queue_family;
    {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

        for (uint32_t i = 0; i < queue_family_count; ++i) {
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphics_queue_family = i;
        }

        if (!graphics_queue_family.has_value())
            throw std::runtime_error("No graphics queue!");
    }

    // create device
    {
        VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queue_info.queueFamilyIndex = graphics_queue_family.value();
        queue_info.queueCount = 1;
        float queue_priority = 1.0f;
        queue_info.pQueuePriorities = &queue_priority;

        VkPhysicalDeviceFeatures device_features{};

        VkDeviceCreateInfo device_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        device_info.pQueueCreateInfos = &queue_info;
        device_info.queueCreateInfoCount = 1;
        device_info.pEnabledFeatures = &device_features;
        if (desc.enable_validation_layers) {
            device_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            device_info.ppEnabledLayerNames = validation_layers.data();
        }

        VK_CHECK(vkCreateDevice(physical_device, &device_info, nullptr, &device));
        vkGetDeviceQueue(device, graphics_queue_family.value(), 0, &queue);
    }
}

DeviceImpl::~DeviceImpl()
{
    vkDestroyDevice(device, nullptr);

    if (desc.enable_validation_layers) {
        vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);
}

Device::Device(const DeviceDesc &desc) { m_impl = std::make_unique<DeviceImpl>(desc); }

Device::~Device() {}

BufferHandle Device::create_buffer(const BufferDesc &desc)
{
    BufferHandle handle = m_impl->buffers.alloc();
    BufferImpl *buffer = m_impl->buffers[handle];
    return handle;
}

void Device::destroy_buffer(BufferHandle handle)
{
    BufferImpl *buffer = m_impl->buffers[handle];
    if (!buffer)
        return;
}

ImageHandle Device::create_image(const ImageDesc &desc)
{
    ImageHandle handle = m_impl->images.alloc();
    ImageImpl *image = m_impl->images[handle];
    return handle;
}

void Device::destroy_image(ImageHandle handle)
{
    ImageImpl *image = m_impl->images[handle];
    if (!image)
        return;
}

SamplerHandle Device::create_sampler(const SamplerDesc &desc)
{
    SamplerHandle handle = m_impl->samplers.alloc();
    SamplerImpl *sampler = m_impl->samplers[handle];

    VkSamplerCreateInfo create_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    create_info.magFilter = SAMPLER_FILTER_MAP[static_cast<size_t>(desc.mag_filter)];
    create_info.minFilter = SAMPLER_FILTER_MAP[static_cast<size_t>(desc.min_filter)];
    create_info.mipmapMode = SAMPLER_MIP_MAP_MODE_MAP[static_cast<size_t>(desc.mip_map_mode)];
    create_info.addressModeU = SAMPLER_ADDRESS_MODE_MAP[static_cast<size_t>(desc.address_mode_u)];
    create_info.addressModeV = SAMPLER_ADDRESS_MODE_MAP[static_cast<size_t>(desc.address_mode_v)];
    create_info.addressModeW = SAMPLER_ADDRESS_MODE_MAP[static_cast<size_t>(desc.address_mode_w)];

    VK_CHECK(vkCreateSampler(m_impl->device, &create_info, nullptr, &sampler->sampler));

    return handle;
}

void Device::destroy_sampler(SamplerHandle handle)
{
    SamplerImpl *sampler = m_impl->samplers[handle];
    if (!sampler)
        return;

    vkDestroySampler(m_impl->device, sampler->sampler, nullptr);
    m_impl->samplers.free(handle);
}

PipelineHandle Device::create_pipeline(const PipelineDesc &desc)
{
    PipelineHandle handle = m_impl->pipelines.alloc();
    PipelineImpl *pipeline = m_impl->pipelines[handle];
    return handle;
}

void Device::destroy_pipeline(PipelineHandle handle)
{
    PipelineImpl *pipeline = m_impl->pipelines[handle];
    if (!pipeline)
        return;
}

FR_NAMESPACE_END
