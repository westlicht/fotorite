#pragma once

#include "core/defs.h"
#include "core/pool.h"

#include <memory>

FR_NAMESPACE_BEGIN

struct DeviceImpl;

using BufferHandle = Handle<struct BufferTag>;
using ImageHandle = Handle<struct ImageTag>;
using PipelineHandle = Handle<struct PipelineTag>;

struct DeviceDesc {
    bool enable_validation_layers{false};
};
struct BufferDesc {};
struct ImageDesc {};
struct PipelineDesc {};

class Device {
public:
    Device(const DeviceDesc &desc = {});
    ~Device();

    BufferHandle create_buffer(const BufferDesc &desc);
    void destroy_buffer(BufferHandle handle);

    ImageHandle create_image(const ImageDesc &desc);
    void destroy_image(ImageHandle handle);

    PipelineHandle create_pipeline(const PipelineDesc &desc);
    void destroy_pipeline(PipelineHandle handle);

private:
    std::unique_ptr<DeviceImpl> m_impl;
};

FR_NAMESPACE_END
