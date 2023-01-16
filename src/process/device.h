#pragma once

#include "core/defs.h"
#include "core/pool.h"

#include <memory>

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

enum class MemoryType : uint32_t {
    Host,
    Device,
    DeviceOnly,
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

using BufferHandle = Handle<struct BufferTag>;
using ImageHandle = Handle<struct ImageTag>;
using SamplerHandle = Handle<struct SamplerTag>;
using PipelineHandle = Handle<struct PipelineTag>;

struct DeviceDesc {
    bool enable_validation_layers{false};
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
    void *shader = nullptr;
    struct {
    } bindings[4];
    size_t push_constants_size = 0;
};

class Device {
public:
    Device(const DeviceDesc &desc = {});
    ~Device();

    BufferHandle create_buffer(const BufferDesc &desc);
    void destroy_buffer(BufferHandle handle);

    ImageHandle create_image(const ImageDesc &desc);
    void destroy_image(ImageHandle handle);

    SamplerHandle create_sampler(const SamplerDesc &desc);
    void destroy_sampler(SamplerHandle handle);

    PipelineHandle create_pipeline(const PipelineDesc &desc);
    void destroy_pipeline(PipelineHandle handle);

private:
    std::unique_ptr<DeviceImpl> m_impl;
};

FR_NAMESPACE_END
