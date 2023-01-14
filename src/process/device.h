#pragma once

#include "core/defs.h"
#include "core/pool.h"

#include <memory>

FR_NAMESPACE_BEGIN

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
struct BufferDesc {};
struct ImageDesc {};
struct SamplerDesc {
    SamplerFilter mag_filter = SamplerFilter::Linear;
    SamplerFilter min_filter = SamplerFilter::Linear;
    SamplerMipMapMode mip_map_mode = SamplerMipMapMode::Linear;
    SamplerAddressMode address_mode_u = SamplerAddressMode::Repeat;
    SamplerAddressMode address_mode_v = SamplerAddressMode::Repeat;
    SamplerAddressMode address_mode_w = SamplerAddressMode::Repeat;
};
struct PipelineDesc {};

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
