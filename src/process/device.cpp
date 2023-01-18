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
};

struct ResourceUsageInfo {
    VkBufferUsageFlags buffer_usage;
    VkImageUsageFlags image_usage;
};

const ResourceUsageInfo RESOURCE_USAGE_MAP[] = {
    // ResourceUsage::Unknown
    {0, 0},
    // ResourceUsage::ConstantBuffer
    {VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 0},
    // ResourceUsage::UnorderedAccess
    {VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, VK_IMAGE_USAGE_STORAGE_BIT},
    // ResourceUsage::ShaderResource
    {VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, VK_IMAGE_USAGE_SAMPLED_BIT},
    // ResourceUsage::TransferDst
    {VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT},
    // ResourceUsage::TransferSrc
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

struct ResourceStateInfo {
    VkImageLayout layout;
    VkAccessFlags access;
};

const ResourceStateInfo RESOURCE_STATE_MAP[] = {
    // ResourceState::Undefined
    {VK_IMAGE_LAYOUT_UNDEFINED, 0},
    // ResourceState::ConstantBuffer
    {VK_IMAGE_LAYOUT_UNDEFINED, 0},
    // ResourceState::UnorderedAccess
    {VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT},
    // ResourceState::ShaderResource
    {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT},
    // ResourceState::TransferDst
    {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT},
    // ResourceState::TransferSrc
    {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT},
};

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
    if (!(type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT))
        return VK_FALSE;

    spdlog::info("VK: {}", callback_data->pMessage);

    // VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT = 0x00000001,
    // VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT = 0x00000002,
    // VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT = 0x00000004,
    // VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT = 0x00000008,

    return VK_FALSE;
}

struct ShaderImpl {
    ShaderDesc desc;
    VkShaderModule vk_shader_module;
};

struct BufferImpl {
    BufferDesc desc;
    ResourceState state;
    VkDeviceMemory vk_device_memory;
    VkBuffer vk_buffer;
};

struct ImageImpl {
    ImageDesc desc;
    ResourceState state;
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

struct DescriptorSet {
    VkDescriptorSet vk_descriptor_set;

    std::vector<VkImageView> vk_image_views;
    std::vector<VkBufferView> vk_buffer_views;
};

struct ContextImpl {
    VkCommandBuffer vk_command_buffer;
    VkFence vk_fence;

    bool is_recording;

    std::vector<ResourceHandle> transient_resources;

    std::vector<DescriptorSet> descriptor_sets;
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

    VkDescriptorPool vk_descriptor_pool;
    VkCommandPool vk_command_pool;

    Pool<ShaderImpl, ShaderHandle> shaders;
    Pool<BufferImpl, BufferHandle> buffers;
    Pool<ImageImpl, ImageHandle> images;
    Pool<SamplerImpl, SamplerHandle> samplers;
    Pool<PipelineImpl, PipelineHandle> pipelines;
    Pool<ContextImpl, ContextHandle> contexts;
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
            // if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_synchronization2") == 0)
            //     validation_layers.emplace_back("VK_LAYER_KHRONOS_synchronization2");
            // if (std::strcmp(layer.layerName, "VK_LAYER_LUNARG_api_dump") == 0)
            //     validation_layers.emplace_back("VK_LAYER_LUNARG_api_dump");
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

    // create descriptor pool
    {
        uint32_t set_count = 100;
        uint32_t descriptor_count = 1000;

        const VkDescriptorPoolSize sizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, descriptor_count},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptor_count},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descriptor_count},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptor_count},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descriptor_count},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descriptor_count},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptor_count},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptor_count},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptor_count},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, descriptor_count},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, descriptor_count},
            {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, descriptor_count},
        };

        VkDescriptorPoolCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        create_info.flags =
            VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        create_info.maxSets = set_count;
        create_info.poolSizeCount = static_cast<uint32_t>(std::size(sizes));
        create_info.pPoolSizes = sizes;

        VK_CHECK(vkCreateDescriptorPool(vk_device, &create_info, nullptr, &vk_descriptor_pool));
    }

    // create command pool
    {
        VkCommandPoolCreateInfo create_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        create_info.queueFamilyIndex = graphics_queue_family;

        VK_CHECK(vkCreateCommandPool(vk_device, &create_info, nullptr, &vk_command_pool));
    }
}

DeviceImpl::~DeviceImpl()
{
    vkDeviceWaitIdle(vk_device);

    vkDestroyCommandPool(vk_device, vk_command_pool, nullptr);
    vkDestroyDescriptorPool(vk_device, vk_descriptor_pool, nullptr);

    vkDestroyDevice(vk_device, nullptr);

    if (desc.enable_validation_layers) {
        vkDestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_messenger, nullptr);
    }

    vkDestroyInstance(vk_instance, nullptr);
}

inline void transition_state(DeviceImpl &device, ContextImpl &context, BufferImpl &buffer, ResourceState new_state)
{
    ResourceState old_state = buffer.state;
    buffer.state = new_state;

    const ResourceStateInfo &old_info = RESOURCE_STATE_MAP[static_cast<size_t>(old_state)];
    const ResourceStateInfo &new_info = RESOURCE_STATE_MAP[static_cast<size_t>(new_state)];

    VkBufferMemoryBarrier buffer_memory_barrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    buffer_memory_barrier.srcAccessMask = old_info.access;
    buffer_memory_barrier.dstAccessMask = new_info.access;
    buffer_memory_barrier.srcQueueFamilyIndex = device.graphics_queue_family;
    buffer_memory_barrier.dstQueueFamilyIndex = device.graphics_queue_family;
    buffer_memory_barrier.buffer = buffer.vk_buffer;
    buffer_memory_barrier.offset = 0;
    buffer_memory_barrier.size = VK_WHOLE_SIZE;

    vkCmdPipelineBarrier(context.vk_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkDependencyFlags(0), 0, nullptr, 1,
                         &buffer_memory_barrier, 0, nullptr);
}

inline VkDescriptorSet create_descriptor_set(DeviceImpl &device, ContextImpl &context, PipelineImpl &pipeline,
                                             const BindingSet &binding_set)
{
    DescriptorSet descriptor_set;

    FR_ASSERT(binding_set.size() == pipeline.desc.binding_layout.size());

    VkDescriptorSetAllocateInfo allocate_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocate_info.descriptorPool = device.vk_descriptor_pool;
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &pipeline.vk_descriptor_set_layout;

    VK_CHECK(vkAllocateDescriptorSets(device.vk_device, &allocate_info, &descriptor_set.vk_descriptor_set));

    for (size_t i = 0; i < binding_set.size(); ++i) {
        const auto &set_item = binding_set[i];
        const auto &layout_item = pipeline.desc.binding_layout[i];

        FR_ASSERT(set_item.binding == layout_item.binding);

        VkDescriptorBufferInfo buffer_info{};
        VkDescriptorImageInfo image_info{};

        VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        write.dstSet = descriptor_set.vk_descriptor_set;
        write.dstBinding = set_item.binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;

        if (auto buffer = std::get<BufferHandle>(set_item.resource)) {
            BufferImpl *buffer_impl = device.buffers[buffer];
            FR_ASSERT(buffer_impl);

            write.descriptorType = DESCRIPTOR_TYPE_MAP[static_cast<size_t>(layout_item.type)].type;

            switch (layout_item.type) {
                case DescriptorType::ConstantBuffer:
                case DescriptorType::StructuredBuffer:
                case DescriptorType::RWStructuredBuffer:
                    buffer_info.buffer = buffer_impl->vk_buffer;
                    buffer_info.offset = 0;
                    buffer_info.range = VK_WHOLE_SIZE;
                    write.pBufferInfo = &buffer_info;
                    break;
                case DescriptorType::Buffer:
                case DescriptorType::RWBuffer: {
                    FR_ASSERT(false);
                    // auto view = new BufferView(buffer, format, offset, range);
                    // _buffer_views.push_back(view);
                    // buffer_view = view->vk_buffer_view();
                    // write.pTexelBufferView = &buffer_view;
                    break;
                }
                default:
                    FR_ASSERT(false);
            }
        } else if (auto image = std::get<ImageHandle>(set_item.resource)) {
            // TODO
        } else if (auto sampler = std::get<SamplerHandle>(set_item.resource)) {
            SamplerImpl *sampler_impl = device.samplers[sampler];
            FR_ASSERT(sampler);
            image_info.sampler = sampler_impl->vk_sampler;
            write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            write.pImageInfo = &image_info;
        }

        vkUpdateDescriptorSets(device.vk_device, 1, &write, 0, nullptr);
    }

    context.descriptor_sets.push_back(descriptor_set);

    return descriptor_set.vk_descriptor_set;
}

inline void destroy_descriptor_set(DeviceImpl &device, DescriptorSet &descriptor_set)
{
    VK_CHECK(vkFreeDescriptorSets(device.vk_device, device.vk_descriptor_pool, 1, &descriptor_set.vk_descriptor_set));
}

Device::Device(const DeviceDesc &desc) { m_impl = std::make_unique<DeviceImpl>(desc); }

Device::~Device() {}

ShaderHandle Device::create_shader(const ShaderDesc &desc)
{
    ShaderHandle shader = m_impl->shaders.alloc();
    ShaderImpl *shader_impl = m_impl->shaders[shader];

    shader_impl->desc = desc;

    VkShaderModuleCreateInfo create_info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    create_info.pCode = static_cast<const uint32_t *>(desc.code);
    create_info.codeSize = desc.code_size;

    VK_CHECK(vkCreateShaderModule(m_impl->vk_device, &create_info, nullptr, &shader_impl->vk_shader_module));

    return shader;
}

void Device::destroy_shader(ShaderHandle shader)
{
    ShaderImpl *shader_impl = m_impl->shaders[shader];
    if (!shader_impl)
        return;

    vkDestroyShaderModule(m_impl->vk_device, shader_impl->vk_shader_module, nullptr);
    m_impl->shaders.free(shader);
}

BufferHandle Device::create_buffer(const BufferDesc &desc)
{
    BufferHandle buffer = m_impl->buffers.alloc();
    BufferImpl *buffer_impl = m_impl->buffers[buffer];

    buffer_impl->desc = desc;
    buffer_impl->state = ResourceState::Undefined;

    ResourceUsageInfo usage_info = get_resource_usage_info(desc.usage);

    VkBufferCreateInfo create_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    create_info.size = desc.size;
    create_info.usage = usage_info.buffer_usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 1;
    create_info.pQueueFamilyIndices = &m_impl->graphics_queue_family;
    VK_CHECK(vkCreateBuffer(m_impl->vk_device, &create_info, nullptr, &buffer_impl->vk_buffer));

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(m_impl->vk_device, buffer_impl->vk_buffer, &memory_requirements);

    // VkMemoryAllocateFlagsInfo flags_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
    // flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    VkMemoryAllocateInfo allocate_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    // allocate_info.pNext = &flags_info;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = m_impl->find_memory_type(memory_requirements.memoryTypeBits, desc.memory);
    VK_CHECK(vkAllocateMemory(m_impl->vk_device, &allocate_info, nullptr, &buffer_impl->vk_device_memory));
    VK_CHECK(vkBindBufferMemory(m_impl->vk_device, buffer_impl->vk_buffer, buffer_impl->vk_device_memory, 0));

    return buffer;
}

void Device::destroy_buffer(BufferHandle buffer)
{
    BufferImpl *buffer_impl = m_impl->buffers[buffer];
    if (!buffer_impl)
        return;

    vkDestroyBuffer(m_impl->vk_device, buffer_impl->vk_buffer, nullptr);
    vkFreeMemory(m_impl->vk_device, buffer_impl->vk_device_memory, nullptr);
    m_impl->buffers.free(buffer);
}

ImageHandle Device::create_image(const ImageDesc &desc)
{
    ImageHandle image = m_impl->images.alloc();
    ImageImpl *image_impl = m_impl->images[image];

    image_impl->desc = desc;
    image_impl->state = ResourceState::Undefined;

    ResourceUsageInfo usage_info = get_resource_usage_info(desc.usage);

    return image;
}

void Device::destroy_image(ImageHandle image)
{
    ImageImpl *image_impl = m_impl->images[image];
    if (!image_impl)
        return;

    m_impl->images.free(image);
}

SamplerHandle Device::create_sampler(const SamplerDesc &desc)
{
    SamplerHandle sampler = m_impl->samplers.alloc();
    SamplerImpl *sampler_impl = m_impl->samplers[sampler];

    VkSamplerCreateInfo create_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    create_info.magFilter = SAMPLER_FILTER_MAP[static_cast<uint32_t>(desc.mag_filter)];
    create_info.minFilter = SAMPLER_FILTER_MAP[static_cast<uint32_t>(desc.min_filter)];
    create_info.mipmapMode = SAMPLER_MIP_MAP_MODE_MAP[static_cast<uint32_t>(desc.mip_map_mode)];
    create_info.addressModeU = SAMPLER_ADDRESS_MODE_MAP[static_cast<uint32_t>(desc.address_mode_u)];
    create_info.addressModeV = SAMPLER_ADDRESS_MODE_MAP[static_cast<uint32_t>(desc.address_mode_v)];
    create_info.addressModeW = SAMPLER_ADDRESS_MODE_MAP[static_cast<uint32_t>(desc.address_mode_w)];

    VK_CHECK(vkCreateSampler(m_impl->vk_device, &create_info, nullptr, &sampler_impl->vk_sampler));

    return sampler;
}

void Device::destroy_sampler(SamplerHandle sampler)
{
    SamplerImpl *sampler_impl = m_impl->samplers[sampler];
    if (!sampler_impl)
        return;

    vkDestroySampler(m_impl->vk_device, sampler_impl->vk_sampler, nullptr);
    m_impl->samplers.free(sampler);
}

PipelineHandle Device::create_pipeline(const PipelineDesc &desc)
{
    PipelineHandle pipeline = m_impl->pipelines.alloc();
    PipelineImpl *pipeline_impl = m_impl->pipelines[pipeline];

    pipeline_impl->desc = desc;

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (const auto &item : desc.binding_layout) {
        FR_ASSERT(item.type != DescriptorType::Unknown);
        FR_ASSERT(item.count >= 1);
        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = item.binding;
        binding.descriptorType = DESCRIPTOR_TYPE_MAP[static_cast<size_t>(item.type)].type;
        binding.descriptorCount = item.count;
        binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptor_set_layout_create_info.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_impl->vk_device, &descriptor_set_layout_create_info, nullptr,
                                         &pipeline_impl->vk_descriptor_set_layout));

    VkPushConstantRange push_constant_range{};
    push_constant_range.offset = 0;
    push_constant_range.size = desc.push_constants_size;
    push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts = &pipeline_impl->vk_descriptor_set_layout;
    pipeline_layout_create_info.pushConstantRangeCount = desc.push_constants_size > 0 ? 1 : 0;
    pipeline_layout_create_info.pPushConstantRanges = desc.push_constants_size > 0 ? &push_constant_range : nullptr;

    VK_CHECK(vkCreatePipelineLayout(m_impl->vk_device, &pipeline_layout_create_info, nullptr,
                                    &pipeline_impl->vk_pipeline_layout));

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
    pipeline_create_info.layout = pipeline_impl->vk_pipeline_layout;

    VK_CHECK(vkCreateComputePipelines(m_impl->vk_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr,
                                      &pipeline_impl->vk_pipeline));

    return pipeline;
}

void Device::destroy_pipeline(PipelineHandle pipeline)
{
    PipelineImpl *pipeline_impl = m_impl->pipelines[pipeline];
    if (!pipeline_impl)
        return;

    vkDestroyPipeline(m_impl->vk_device, pipeline_impl->vk_pipeline, nullptr);
    vkDestroyPipelineLayout(m_impl->vk_device, pipeline_impl->vk_pipeline_layout, nullptr);
    vkDestroyDescriptorSetLayout(m_impl->vk_device, pipeline_impl->vk_descriptor_set_layout, nullptr);

    m_impl->pipelines.free(pipeline);
}

ContextHandle Device::create_context()
{
    ContextHandle context = m_impl->contexts.alloc();
    ContextImpl *context_impl = m_impl->contexts[context];

    context_impl->is_recording = false;

    VkFenceCreateInfo fence_create_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    VK_CHECK(vkCreateFence(m_impl->vk_device, &fence_create_info, nullptr, &context_impl->vk_fence));

    VkCommandBufferAllocateInfo alloc_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    alloc_info.commandPool = m_impl->vk_command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    VK_CHECK(vkAllocateCommandBuffers(m_impl->vk_device, &alloc_info, &context_impl->vk_command_buffer));

    return context;
}

void Device::destroy_context(ContextHandle context)
{
    ContextImpl *context_impl = m_impl->contexts[context];
    if (!context_impl)
        return;

    vkDestroyFence(m_impl->vk_device, context_impl->vk_fence, nullptr);
    vkFreeCommandBuffers(m_impl->vk_device, m_impl->vk_command_pool, 1, &context_impl->vk_command_buffer);

    m_impl->contexts.free(context);
}

void Device::begin(ContextHandle context)
{
    ContextImpl *context_impl = m_impl->contexts[context];
    if (!context_impl)
        return;

    FR_ASSERT(!context_impl->is_recording);

    VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(context_impl->vk_command_buffer, &begin_info));

    VK_CHECK(vkResetFences(m_impl->vk_device, 1, &context_impl->vk_fence));

    context_impl->is_recording = true;
}
void Device::submit(ContextHandle context)
{
    ContextImpl *context_impl = m_impl->contexts[context];
    if (!context_impl)
        return;

    FR_ASSERT(context_impl->is_recording);

    VK_CHECK(vkEndCommandBuffer(context_impl->vk_command_buffer));

    VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &context_impl->vk_command_buffer;

    VK_CHECK(vkQueueSubmit(m_impl->vk_queue, 1, &submit_info, context_impl->vk_fence));

    context_impl->is_recording = false;
}

void Device::wait(ContextHandle context)
{
    ContextImpl *context_impl = m_impl->contexts[context];
    if (!context_impl)
        return;

    VK_CHECK(vkWaitForFences(m_impl->vk_device, 1, &context_impl->vk_fence, VK_TRUE, 1000000000));

    // release transient resources
    for (const ResourceHandle &resource : context_impl->transient_resources) {
        // TODO use visitor
        if (auto buffer = std::get<BufferHandle>(resource))
            destroy_buffer(buffer);
        else if (auto image = std::get<ImageHandle>(resource))
            destroy_image(image);
        else if (auto sampler = std::get<SamplerHandle>(resource))
            destroy_sampler(sampler);
    }

    // release descriptor sets
    for (DescriptorSet &descriptor_set : context_impl->descriptor_sets)
        destroy_descriptor_set(*m_impl, descriptor_set);
    context_impl->descriptor_sets.clear();
}

void Device::write_buffer(ContextHandle context, BufferHandle buffer, const void *data, size_t size, size_t offset)
{
    ContextImpl *context_impl = m_impl->contexts[context];
    BufferImpl *buffer_impl = m_impl->buffers[buffer];
    if (!context_impl || !buffer_impl || size == 0)
        return;

    FR_ASSERT(data);

    if (buffer_impl->desc.memory == MemoryType::Host) {
        void *buffer_data;
        VK_CHECK(vkMapMemory(m_impl->vk_device, buffer_impl->vk_device_memory, offset, size, 0, &buffer_data));
        std::memcpy(buffer_data, data, size);
        vkUnmapMemory(m_impl->vk_device, buffer_impl->vk_device_memory);
    } else if (buffer_impl->desc.memory == MemoryType::Device) {
        // create staging buffer
        BufferHandle staging_buffer = create_buffer({
            .size = size,
            .usage = ResourceUsage::TransferSrc,
            .memory = MemoryType::Host,
        });
        write_buffer(context, staging_buffer, data, size);

        submit(context);
        wait(context);
        begin(context);

        copy_buffer(context, staging_buffer, buffer, size, 0, offset);

        context_impl->transient_resources.push_back(staging_buffer);
    }
}

void Device::read_buffer(ContextHandle context, BufferHandle buffer, void *data, size_t size, size_t offset)
{
    ContextImpl *context_impl = m_impl->contexts[context];
    BufferImpl *buffer_impl = m_impl->buffers[buffer];
    if (!context_impl || !buffer_impl || size == 0)
        return;

    FR_ASSERT(data);

    if (buffer_impl->desc.memory == MemoryType::Host) {
        void *buffer_data;
        VK_CHECK(vkMapMemory(m_impl->vk_device, buffer_impl->vk_device_memory, offset, size, 0, &buffer_data));
        std::memcpy(data, buffer_data, size);
        vkUnmapMemory(m_impl->vk_device, buffer_impl->vk_device_memory);
    } else if (buffer_impl->desc.memory == MemoryType::Device) {
        // create staging buffer
        BufferHandle staging_buffer = create_buffer({
            .size = size,
            .usage = ResourceUsage::TransferDst,
            .memory = MemoryType::Host,
        });
        copy_buffer(context, buffer, staging_buffer, size, offset, 0);

        submit(context);
        wait(context);
        begin(context);

        read_buffer(context, staging_buffer, data, size);

        context_impl->transient_resources.push_back(staging_buffer);
    }
}

void Device::copy_buffer(ContextHandle context, BufferHandle src, BufferHandle dst, size_t size, size_t src_offset,
                         size_t dst_offset)
{
    ContextImpl *context_impl = m_impl->contexts[context];
    BufferImpl *src_impl = m_impl->buffers[src];
    BufferImpl *dst_impl = m_impl->buffers[dst];

    if (!context_impl || !src_impl || !dst_impl || size == 0)
        return;

    transition_state(*m_impl, *context_impl, *src_impl, ResourceState::TransferSrc);
    transition_state(*m_impl, *context_impl, *dst_impl, ResourceState::TransferDst);

    VkBufferCopy buffer_copy{};
    buffer_copy.srcOffset = src_offset;
    buffer_copy.dstOffset = dst_offset;
    buffer_copy.size = size;

    vkCmdCopyBuffer(context_impl->vk_command_buffer, src_impl->vk_buffer, dst_impl->vk_buffer, 1, &buffer_copy);
}

// copy_image

void Device::dispatch(ContextHandle context, DispatchDesc desc)
{
    ContextImpl *context_impl = m_impl->contexts[context];
    PipelineImpl *pipeline_impl = m_impl->pipelines[desc.pipeline];
    if (!context_impl || !pipeline_impl)
        return;

    FR_ASSERT(desc.group_count[0] > 0 && desc.group_count[1] > 0 && desc.group_count[2] > 0);

    // transition resource state
    FR_ASSERT(desc.binding_set.size() == pipeline_impl->desc.binding_layout.size());
    for (size_t i = 0; i < desc.binding_set.size(); ++i) {
        const auto &set_item = desc.binding_set[i];
        const auto &layout_item = pipeline_impl->desc.binding_layout[i];

        FR_ASSERT(set_item.binding == layout_item.binding);

        if (auto buffer = std::get<BufferHandle>(set_item.resource)) {
            BufferImpl *buffer_impl = m_impl->buffers[buffer];
            FR_ASSERT(buffer);

            switch (layout_item.type) {
                case DescriptorType::ConstantBuffer:
                    transition_state(*m_impl, *context_impl, *buffer_impl, ResourceState::ConstantBuffer);
                    break;
                case DescriptorType::StructuredBuffer:
                case DescriptorType::Buffer:
                case DescriptorType::Texture:
                    transition_state(*m_impl, *context_impl, *buffer_impl, ResourceState::ShaderResource);
                    break;
                case DescriptorType::RWStructuredBuffer:
                case DescriptorType::RWBuffer:
                case DescriptorType::RWTexture:
                    transition_state(*m_impl, *context_impl, *buffer_impl, ResourceState::UnorderedAccess);
                    break;
                default:
                    FR_ASSERT(false);
            }
        }
    }

    VkDescriptorSet vk_descriptor_set = create_descriptor_set(*m_impl, *context_impl, *pipeline_impl, desc.binding_set);

    vkCmdBindPipeline(context_impl->vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_impl->vk_pipeline);
    vkCmdBindDescriptorSets(context_impl->vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipeline_impl->vk_pipeline_layout, 0, 1, &vk_descriptor_set, 0, nullptr);

    if (desc.push_constants_size > 0) {
        FR_ASSERT(desc.push_constants);
        vkCmdPushConstants(context_impl->vk_command_buffer, pipeline_impl->vk_pipeline_layout,
                           VK_SHADER_STAGE_COMPUTE_BIT, 0, desc.push_constants_size, desc.push_constants);
    }

    vkCmdDispatch(context_impl->vk_command_buffer, desc.group_count[0], desc.group_count[1], desc.group_count[2]);
}

FR_NAMESPACE_END
