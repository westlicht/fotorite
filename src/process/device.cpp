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

static const VkFormat IMAGE_FORMAT_MAP[] = {
    // ImageFormat::Unknown
    VK_FORMAT_UNDEFINED,

    // ImageFormat::R8Unorm
    VK_FORMAT_R8_UNORM,
    // ImageFormat::R8Snorm
    VK_FORMAT_R8_SNORM,
    // ImageFormat::R8UInt
    VK_FORMAT_R8_UINT,
    // ImageFormat::R8Int
    VK_FORMAT_R8_SINT,
    // ImageFormat::R16Uint
    VK_FORMAT_R16_UINT,
    // ImageFormat::R16Int
    VK_FORMAT_R16_SINT,
    // ImageFormat::R16Float
    VK_FORMAT_R16_SFLOAT,
    // ImageFormat::R32Uint
    VK_FORMAT_R32_UINT,
    // ImageFormat::R32Int
    VK_FORMAT_R32_SINT,
    // ImageFormat::R32Float
    VK_FORMAT_R32_SFLOAT,

    // ImageFormat::RG16Uint
    VK_FORMAT_R16G16_UINT,
    // ImageFormat::RG16Int
    VK_FORMAT_R16G16_SINT,
    // ImageFormat::RG16Float
    VK_FORMAT_R16G16_SFLOAT,
    // ImageFormat::RG32Uint
    VK_FORMAT_R32G32_UINT,
    // ImageFormat::RG32Int
    VK_FORMAT_R32G32_SINT,
    // ImageFormat::RG32Float
    VK_FORMAT_R32G32_SFLOAT,

    // ImageFormat::RGB16Uint
    VK_FORMAT_R16G16B16_UINT,
    // ImageFormat::RGB16Int
    VK_FORMAT_R16G16B16_SINT,
    // ImageFormat::RGB16Float
    VK_FORMAT_R16G16B16_SFLOAT,
    // ImageFormat::RGB32Uint
    VK_FORMAT_R32G32B32_UINT,
    // ImageFormat::RGB32Int
    VK_FORMAT_R32G32B32_SINT,
    // ImageFormat::RGB32Float
    VK_FORMAT_R32G32B32_SFLOAT,

    // ImageFormat::RGBA8Unorm
    VK_FORMAT_R8G8B8A8_UNORM,
    // ImageFormat::RGBA16Uint
    VK_FORMAT_R16G16B16A16_UINT,
    // ImageFormat::RGBA16Int
    VK_FORMAT_R16G16B16A16_SINT,
    // ImageFormat::RGBA16Float
    VK_FORMAT_R16G16B16A16_SFLOAT,
    // ImageFormat::RGBA32Uint
    VK_FORMAT_R32G32B32A32_UINT,
    // ImageFormat::RGBA32Int
    VK_FORMAT_R32G32B32A32_SINT,
    // ImageFormat::RGBA32Float
    VK_FORMAT_R32G32B32A32_SFLOAT,
};

const VkMemoryPropertyFlags MEMORY_TYPE_MAP[] = {
    // MemoryType::Host
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    // MemoryType::Device
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    // MemoryType::DeviceOnly
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
};

struct ResourceUsageInfo {
    VkBufferUsageFlags buffer_usage;
    VkImageUsageFlags image_usage;
};

const ResourceUsageInfo RESOURCE_USAGE_MAP[] = {
    // Unknown
    {0, 0},
    // ConstantBuffer
    {VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 0},
    // UnorderedAccess
    {VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, VK_IMAGE_USAGE_STORAGE_BIT},
    // ShaderResource
    {VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, VK_IMAGE_USAGE_SAMPLED_BIT},
    // TransferDst
    {VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT},
    // TransferSrc
    {VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
};

inline ResourceUsageInfo get_resource_usage_info(ResourceUsage usage)
{
    ResourceUsageInfo info{};
    for (size_t i = 0; i < std::size(RESOURCE_USAGE_MAP); ++i) {
        if (uint32_t(usage) & (1 << i)) {
            info.buffer_usage |= RESOURCE_USAGE_MAP[i].buffer_usage;
            info.image_usage |= RESOURCE_USAGE_MAP[i].image_usage;
        }
    }
    return info;
}

struct DescriptorTypeInfo {
    VkDescriptorType type;
    VkImageLayout layout;
};

const DescriptorTypeInfo DESCRIPTOR_TYPE_MAP[] = {
    // DescriptorType::Unknown
    {VkDescriptorType(0), VkImageLayout(0)},
    // DescriptorType::Sampler
    {VK_DESCRIPTOR_TYPE_SAMPLER, VK_IMAGE_LAYOUT_UNDEFINED},
    // DescriptorType::ConstantBuffer
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_IMAGE_LAYOUT_UNDEFINED},
    // DescriptorType::StructuredBuffer, ByteAddressBuffer
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_IMAGE_LAYOUT_UNDEFINED},
    // DescriptorType::RWStructuredBuffer, RWByteAddressBuffer
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_IMAGE_LAYOUT_UNDEFINED},
    // DescriptorType::Buffer
    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, VK_IMAGE_LAYOUT_UNDEFINED},
    // DescriptorType::RWBuffer
    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, VK_IMAGE_LAYOUT_UNDEFINED},
    // DescriptorType::Texture
    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
    // DescriptorType::RWTexture
    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL},
};

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

struct ShaderImpl {
    ShaderDesc desc;
    VkShaderModule vk_shader_module;
};

struct BufferImpl {
    BufferDesc desc;
    VkDeviceMemory vk_device_memory;
    VkBuffer vk_buffer;
};

struct ImageImpl {
    ImageDesc desc;
    VkImage vk_image;
};

struct SamplerImpl {
    SamplerDesc desc;
    VkSampler vk_sampler;
};

struct PipelineImpl {
    PipelineDesc desc;
    VkDescriptorSetLayout vk_descriptor_set_layout;
    VkPipelineLayout vk_pipeline_layout;
    VkPipeline vk_pipeline;
};

struct SequenceImpl {
    VkCommandBuffer vk_command_buffer;
    VkSemaphore vk_semaphore;
};

struct TransientHeap {
    // VkDescriptorPool vk_descriptor_pool;
    // VkCommandPool vk_command_pool;

    void init() {}

    void destroy() {}
};

struct DeviceImpl {
    DeviceImpl(const DeviceDesc &desc);
    ~DeviceImpl();

    uint32_t find_memory_type(uint32_t type_filter, MemoryType memory_type) const
    {
        uint32_t properties = MEMORY_TYPE_MAP[static_cast<uint32_t>(memory_type)];
        for (uint32_t i = 0; i < vk_memory_properties.memoryTypeCount; i++) {
            if ((type_filter & (1 << i)) &&
                (vk_memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("Failed to find suitable memory type!");
    }

    DeviceDesc desc;
    VkInstance vk_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT vk_debug_messenger{VK_NULL_HANDLE};
    VkPhysicalDevice vk_physical_device{VK_NULL_HANDLE};
    VkPhysicalDeviceMemoryProperties vk_memory_properties;
    VkDevice vk_device{VK_NULL_HANDLE};
    uint32_t graphics_queue_family;
    VkQueue vk_queue;

    TransientHeap transient_heaps[4];

    Pool<ShaderImpl, ShaderHandle> shaders;
    Pool<BufferImpl, BufferHandle> buffers;
    Pool<ImageImpl, ImageHandle> images;
    Pool<SamplerImpl, SamplerHandle> samplers;
    Pool<PipelineImpl, PipelineHandle> pipelines;
    Pool<SequenceImpl, SequenceHandle> sequences;
};

DeviceImpl::DeviceImpl(const DeviceDesc &desc_) : desc(desc_)
{
    VK_CHECK(volkInitialize());

    std::vector<const char *> validation_layers;

    if (desc.enable_validation_layers) {
        // enumerate validation layers
        uint32_t layer_count;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));
        std::vector<VkLayerProperties> layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

        spdlog::info("available vulkan layers:");
        for (const auto &layer : layers) {
            spdlog::info("{}", layer.layerName);
            if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
                validation_layers.emplace_back("VK_LAYER_KHRONOS_validation");
        }
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
    app_info.apiVersion = VK_API_VERSION_1_2;

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

    VK_CHECK(vkCreateInstance(&instance_info, nullptr, &vk_instance));
    volkLoadInstance(vk_instance);

    if (desc.enable_validation_layers) {
        VK_CHECK(vkCreateDebugUtilsMessengerEXT(vk_instance, &debug_info, nullptr, &vk_debug_messenger));
    }

    // find physical device
    {
        uint32_t device_count = 0;
        VK_CHECK(vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr));
        std::vector<VkPhysicalDevice> devices(device_count);
        VK_CHECK(vkEnumeratePhysicalDevices(vk_instance, &device_count, devices.data()));

        vk_physical_device = devices.empty() ? VK_NULL_HANDLE : devices[0];

        if (!vk_physical_device)
            throw std::runtime_error("No suitable Vulkan device available!");
    }

    // check memory types
    vkGetPhysicalDeviceMemoryProperties(vk_physical_device, &vk_memory_properties);

    // check queues
    graphics_queue_family = static_cast<uint32_t>(-1);
    {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_family_count, queue_families.data());

        for (uint32_t i = 0; i < queue_family_count; ++i) {
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphics_queue_family = i;
        }

        if (graphics_queue_family == static_cast<uint32_t>(-1))
            throw std::runtime_error("No graphics queue!");
    }

    // create device
    {
        VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queue_info.queueFamilyIndex = graphics_queue_family;
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

        VK_CHECK(vkCreateDevice(vk_physical_device, &device_info, nullptr, &vk_device));
        vkGetDeviceQueue(vk_device, graphics_queue_family, 0, &vk_queue);
    }
}

DeviceImpl::~DeviceImpl()
{
    vkDestroyDevice(vk_device, nullptr);

    if (desc.enable_validation_layers) {
        vkDestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_messenger, nullptr);
    }

    vkDestroyInstance(vk_instance, nullptr);
}

Device::Device(const DeviceDesc &desc) { m_impl = std::make_unique<DeviceImpl>(desc); }

Device::~Device() {}

ShaderHandle Device::create_shader(const ShaderDesc &desc)
{
    ShaderHandle handle = m_impl->shaders.alloc();
    ShaderImpl *shader = m_impl->shaders[handle];

    shader->desc = desc;

    VkShaderModuleCreateInfo create_info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    create_info.pCode = static_cast<const uint32_t *>(desc.code);
    create_info.codeSize = desc.code_size;

    VK_CHECK(vkCreateShaderModule(m_impl->vk_device, &create_info, nullptr, &shader->vk_shader_module));

    return handle;
}

void Device::destroy_shader(ShaderHandle handle)
{
    ShaderImpl *shader = m_impl->shaders[handle];
    if (!shader)
        return;

    vkDestroyShaderModule(m_impl->vk_device, shader->vk_shader_module, nullptr);
    m_impl->shaders.free(handle);
}

BufferHandle Device::create_buffer(const BufferDesc &desc)
{
    BufferHandle handle = m_impl->buffers.alloc();
    BufferImpl *buffer = m_impl->buffers[handle];

    buffer->desc = desc;

    ResourceUsageInfo usage_info = get_resource_usage_info(desc.usage);

    VkBufferCreateInfo create_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    create_info.size = desc.size;
    create_info.usage = usage_info.buffer_usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 1;
    create_info.pQueueFamilyIndices = &m_impl->graphics_queue_family;
    VK_CHECK(vkCreateBuffer(m_impl->vk_device, &create_info, nullptr, &buffer->vk_buffer));

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(m_impl->vk_device, buffer->vk_buffer, &memory_requirements);

    // VkMemoryAllocateFlagsInfo flags_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
    // flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    VkMemoryAllocateInfo allocate_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    // allocate_info.pNext = &flags_info;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = m_impl->find_memory_type(memory_requirements.memoryTypeBits, desc.memory);
    VK_CHECK(vkAllocateMemory(m_impl->vk_device, &allocate_info, nullptr, &buffer->vk_device_memory));
    VK_CHECK(vkBindBufferMemory(m_impl->vk_device, buffer->vk_buffer, buffer->vk_device_memory, 0));

    return handle;
}

void Device::destroy_buffer(BufferHandle handle)
{
    BufferImpl *buffer = m_impl->buffers[handle];
    if (!buffer)
        return;

    vkDestroyBuffer(m_impl->vk_device, buffer->vk_buffer, nullptr);
    vkFreeMemory(m_impl->vk_device, buffer->vk_device_memory, nullptr);
    m_impl->buffers.free(handle);
}

ImageHandle Device::create_image(const ImageDesc &desc)
{
    ImageHandle handle = m_impl->images.alloc();
    ImageImpl *image = m_impl->images[handle];

    image->desc = desc;

    ResourceUsageInfo usage_info = get_resource_usage_info(desc.usage);

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
    create_info.magFilter = SAMPLER_FILTER_MAP[static_cast<uint32_t>(desc.mag_filter)];
    create_info.minFilter = SAMPLER_FILTER_MAP[static_cast<uint32_t>(desc.min_filter)];
    create_info.mipmapMode = SAMPLER_MIP_MAP_MODE_MAP[static_cast<uint32_t>(desc.mip_map_mode)];
    create_info.addressModeU = SAMPLER_ADDRESS_MODE_MAP[static_cast<uint32_t>(desc.address_mode_u)];
    create_info.addressModeV = SAMPLER_ADDRESS_MODE_MAP[static_cast<uint32_t>(desc.address_mode_v)];
    create_info.addressModeW = SAMPLER_ADDRESS_MODE_MAP[static_cast<uint32_t>(desc.address_mode_w)];

    VK_CHECK(vkCreateSampler(m_impl->vk_device, &create_info, nullptr, &sampler->vk_sampler));

    return handle;
}

void Device::destroy_sampler(SamplerHandle handle)
{
    SamplerImpl *sampler = m_impl->samplers[handle];
    if (!sampler)
        return;

    vkDestroySampler(m_impl->vk_device, sampler->vk_sampler, nullptr);
    m_impl->samplers.free(handle);
}

PipelineHandle Device::create_pipeline(const PipelineDesc &desc)
{
    PipelineHandle handle = m_impl->pipelines.alloc();
    PipelineImpl *pipeline = m_impl->pipelines[handle];

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto &b : desc.bindings) {
        if (b.binding == PipelineDesc::INVALID_BINDING)
            continue;
        FR_ASSERT(b.type != DescriptorType::Unknown);
        FR_ASSERT(b.count >= 1);
        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = b.binding;
        binding.descriptorType = DESCRIPTOR_TYPE_MAP[static_cast<size_t>(b.type)].type;
        binding.descriptorCount = b.count;
        binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptor_set_layout_create_info.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_impl->vk_device, &descriptor_set_layout_create_info, nullptr,
                                         &pipeline->vk_descriptor_set_layout));

    VkPushConstantRange push_constant_range{};
    push_constant_range.offset = 0;
    push_constant_range.size = desc.push_constants_size;
    push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts = &pipeline->vk_descriptor_set_layout;
    pipeline_layout_create_info.pushConstantRangeCount = desc.push_constants_size > 0 ? 1 : 0;
    pipeline_layout_create_info.pPushConstantRanges = desc.push_constants_size > 0 ? &push_constant_range : nullptr;

    VK_CHECK(vkCreatePipelineLayout(m_impl->vk_device, &pipeline_layout_create_info, nullptr,
                                    &pipeline->vk_pipeline_layout));

    ShaderImpl *shader = m_impl->shaders[desc.shader];
    FR_ASSERT(shader);
    FR_ASSERT(shader->desc.entry_point_name);

    VkPipelineShaderStageCreateInfo stage_create_info{};
    stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage_create_info.module = shader->vk_shader_module;
    stage_create_info.pName = shader->desc.entry_point_name;

    VkComputePipelineCreateInfo pipeline_create_info{};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_create_info.stage = stage_create_info;
    pipeline_create_info.layout = pipeline->vk_pipeline_layout;

    VK_CHECK(vkCreateComputePipelines(m_impl->vk_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr,
                                      &pipeline->vk_pipeline));

    return handle;
}

void Device::destroy_pipeline(PipelineHandle handle)
{
    PipelineImpl *pipeline = m_impl->pipelines[handle];
    if (!pipeline)
        return;

    vkDestroyPipeline(m_impl->vk_device, pipeline->vk_pipeline, nullptr);
    vkDestroyPipelineLayout(m_impl->vk_device, pipeline->vk_pipeline_layout, nullptr);
    vkDestroyDescriptorSetLayout(m_impl->vk_device, pipeline->vk_descriptor_set_layout, nullptr);
}

SequenceHandle Device::start_sequence()
{
    SequenceHandle handle = m_impl->sequences.alloc();
    SequenceImpl *sequence = m_impl->sequences[handle];

    VkSemaphoreCreateInfo semaphore_create_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VK_CHECK(vkCreateSemaphore(m_impl->vk_device, &semaphore_create_info, nullptr, &sequence->vk_semaphore));

    return handle;
}

void Device::end_sequence(SequenceHandle sequence_handle)
{
    SequenceImpl *sequence = m_impl->sequences[sequence_handle];
    if (!sequence)
        return;
}

void Device::write_buffer(SequenceHandle sequence_handle, BufferHandle buffer_handle, const void *data, size_t size,
                          size_t offset)
{
    SequenceImpl *sequence = m_impl->sequences[sequence_handle];
    BufferImpl *buffer = m_impl->buffers[buffer_handle];
    if (!sequence || !buffer)
        return;

    FR_ASSERT(buffer->desc.memory == MemoryType::Host);

    void *buffer_data;
    VK_CHECK(vkMapMemory(m_impl->vk_device, buffer->vk_device_memory, offset, size, 0, &buffer_data));
    std::memcpy(buffer_data, data, size);
    vkUnmapMemory(m_impl->vk_device, buffer->vk_device_memory);
}

void Device::read_buffer(SequenceHandle sequence_handle, BufferHandle buffer_handle, void *data, size_t size,
                         size_t offset)
{
    SequenceImpl *sequence = m_impl->sequences[sequence_handle];
    BufferImpl *buffer = m_impl->buffers[buffer_handle];
    if (!sequence || !buffer)
        return;

    FR_ASSERT(buffer->desc.memory == MemoryType::Host);

    void *buffer_data;
    VK_CHECK(vkMapMemory(m_impl->vk_device, buffer->vk_device_memory, offset, size, 0, &buffer_data));
    std::memcpy(data, buffer_data, size);
    vkUnmapMemory(m_impl->vk_device, buffer->vk_device_memory);
}

FR_NAMESPACE_END
