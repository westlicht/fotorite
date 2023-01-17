#pragma once

#include "core/defs.h"
#include "core/pool.h"

#include <memory>
#include <variant>

FR_NAMESPACE_BEGIN

enum class ImageFormat : uint32_t {
    Unknown,

    R8Unorm,
    R8Snorm,
    R8UInt,
    R8Int,
    R16Uint,
    R16Int,
    R16Float,
    R32Uint,
    R32Int,
    R32Float,

    RG16Uint,
    RG16Int,
    RG16Float,
    RG32Uint,
    RG32Int,
    RG32Float,

    RGB16Uint,
    RGB16Int,
    RGB16Float,
    RGB32Uint,
    RGB32Int,
    RGB32Float,

    RGBA8Unorm,
    RGBA16Uint,
    RGBA16Int,
    RGBA16Float,
    RGBA32Uint,
    RGBA32Int,
    RGBA32Float,
};

enum class ResourceUsage : uint32_t {
    Unknown = (1 << 0),
    ConstantBuffer = (1 << 1),
    UnorderedAccess = (1 << 4),
    ShaderResource = (1 << 5),
    TransferDst = (1 << 7),
    TransferSrc = (1 << 8),
};
FR_ENUM_FLAG_OPERATORS(ResourceUsage)

enum class ResourceState : uint32_t {
    Undefined,
    ConstantBuffer,
    UnorderedAccess,
    ShaderResource,
    TransferDst,
    TransferSrc,
};

enum class MemoryType : uint32_t {
    Host,
    Device,
};

enum class DescriptorType : uint32_t {
    Unknown,
    Sampler,
    ConstantBuffer,
    StructuredBuffer,
    RWStructuredBuffer,
    ByteAddressBuffer = StructuredBuffer,
    RWByteAddressBuffer = RWStructuredBuffer,
    Buffer,
    RWBuffer,
    Texture,
    RWTexture,
};

enum class SamplerFilter : uint32_t {
    Nearest,
    Linear,
};

enum class SamplerMipMapMode : uint32_t {
    Nearest,
    Linear,
};

enum class SamplerAddressMode : uint32_t {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
    MirrorClampToEdge,
};

struct DeviceImpl;

using ShaderHandle = Handle<struct ShaderTag>;
using BufferHandle = Handle<struct BufferTag>;
using ImageHandle = Handle<struct ImageTag>;
using SamplerHandle = Handle<struct SamplerTag>;
using PipelineHandle = Handle<struct PipelineTag>;
using ContextHandle = Handle<struct ContextTag>;

using ResourceHandle = std::variant<BufferHandle, ImageHandle, SamplerHandle>;

struct DeviceDesc {
    bool enable_validation_layers{false};
};

struct ShaderDesc {
    const void *code{nullptr};
    size_t code_size{0};
    const char *entry_point_name{"main"};
};

struct BufferDesc {
    size_t size = 0;
    ResourceUsage usage = ResourceUsage::Unknown;
    MemoryType memory = MemoryType::Device;
};

struct ImageDesc {
    uint32_t width = 0;
    uint32_t height = 0;
    ImageFormat format = ImageFormat::Unknown;
    ResourceUsage usage = ResourceUsage::Unknown;
    MemoryType memory = MemoryType::Device;
};

struct SamplerDesc {
    SamplerFilter mag_filter = SamplerFilter::Linear;
    SamplerFilter min_filter = SamplerFilter::Linear;
    SamplerMipMapMode mip_map_mode = SamplerMipMapMode::Linear;
    SamplerAddressMode address_mode_u = SamplerAddressMode::Repeat;
    SamplerAddressMode address_mode_v = SamplerAddressMode::Repeat;
    SamplerAddressMode address_mode_w = SamplerAddressMode::Repeat;
};

struct PipelineDesc {
    static const uint32_t INVALID_BINDING = static_cast<uint32_t>(-1);

    ShaderHandle shader{ShaderHandle::null()};
    struct {
        uint32_t binding{INVALID_BINDING};
        DescriptorType type{DescriptorType::Unknown};
        uint32_t count{1};
    } bindings[4];
    uint32_t push_constants_size = 0;
};

struct DispatchDesc {
    PipelineHandle pipeline;
    struct {
        uint32_t binding{0};
        ResourceHandle resource;
    } bindings[4];
    const void *push_constants;
    uint32_t push_constants_size;
};

class Device {
public:
    Device(const DeviceDesc &desc = {});
    ~Device();

    ShaderHandle create_shader(const ShaderDesc &desc);
    void destroy_shader(ShaderHandle shader);

    BufferHandle create_buffer(const BufferDesc &desc);
    void destroy_buffer(BufferHandle buffer);

    ImageHandle create_image(const ImageDesc &desc);
    void destroy_image(ImageHandle image);

    SamplerHandle create_sampler(const SamplerDesc &desc);
    void destroy_sampler(SamplerHandle sampler);

    PipelineHandle create_pipeline(const PipelineDesc &desc);
    void destroy_pipeline(PipelineHandle pipeline);

    ContextHandle create_context();
    void destroy_context(ContextHandle context);

    void begin(ContextHandle context);
    void submit(ContextHandle context);
    void wait(ContextHandle context);

    void write_buffer(ContextHandle context, BufferHandle buffer, const void *data, size_t size, size_t offset = 0);
    void read_buffer(ContextHandle context, BufferHandle buffer, void *data, size_t size, size_t offset = 0);

    void copy_buffer(ContextHandle context, BufferHandle src, BufferHandle dst, size_t size, size_t src_offset = 0,
                     size_t dst_offset = 0);
    void copy_image(ContextHandle context);

    void dispatch(ContextHandle context, DispatchDesc desc);

private:
    std::unique_ptr<DeviceImpl> m_impl;
};

FR_NAMESPACE_END
