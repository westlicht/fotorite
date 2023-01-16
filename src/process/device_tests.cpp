#include "process/device.h"

#include <doctest/doctest.h>

using namespace fr;

TEST_SUITE_BEGIN("process");

TEST_CASE("Device" * doctest::skip(FOTORITE_GITHUB_CI))
{
    DeviceDesc desc;
    desc.enable_validation_layers = true;
    Device device(desc);

    SamplerHandle s = device.create_sampler({
        .mag_filter = SamplerFilter::Linear,
        .min_filter = SamplerFilter::Nearest,
    });

    BufferHandle b = device.create_buffer({
        .size = 1024,
        .usage = ResourceUsage::ShaderResource | ResourceUsage::UnorderedAccess,
        .memory = MemoryType::Device,
    });

    device.destroy_sampler(s);
    device.destroy_buffer(b);
}

TEST_CASE("compute")
{
    // auto pipeline = device.create_pipeline({
    //     .shader = {},
    //     .bindings =
    //         {
    //             {
    //                 .binding = 0,
    //                 .type = DescriptorType::Texture,
    //                 .count = 1,
    //             },
    //             {
    //                 .binding = 1,
    //                 .type = DescriptorType::Image,
    //                 .count = 1,
    //             },
    //         },
    //     .push_constants_size = 128,
    // });

    // auto sequence = device.begin();

    // sequence.bind
    // sequence.dispatch(pipeline);
}

TEST_SUITE_END();
