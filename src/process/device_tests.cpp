#include "process/device.h"
#include "shaders/shaders.h"

#include <doctest/doctest.h>

using namespace fr;

TEST_SUITE_BEGIN("process");

TEST_CASE("Device" * doctest::skip(FOTORITE_GITHUB_CI))
{
    Device device({
        .enable_validation_layers = true,
    });

    auto blob = get_shader_blob(ShaderID::test_cs);
    ShaderHandle shader = device.create_shader({
        .code = blob.data(),
        .code_size = blob.size_bytes(),
    });

    PipelineHandle pipeline = device.create_pipeline({
        .shader = shader,
        .bindings{
            {.binding = 0, .type = DescriptorType::Buffer, .count = 1},
            {.binding = 1, .type = DescriptorType::Buffer, .count = 1},
            {.binding = 2, .type = DescriptorType::RWBuffer, .count = 1},
        },
        .push_constants_size = 4,
    });

    SamplerHandle sampler = device.create_sampler({
        .mag_filter = SamplerFilter::Linear,
        .min_filter = SamplerFilter::Nearest,
    });

    BufferHandle buffer = device.create_buffer({
        .size = 1024,
        .usage = ResourceUsage::ShaderResource | ResourceUsage::UnorderedAccess,
        .memory = MemoryType::Device,
    });

    DispatchDesc desc{
        .pipeline = pipeline,
        .bindings{
            {.binding = 0, .resource = buffer},
            // {.binding = 0, .resource = image},
            {.binding = 1, .resource = sampler},

        },
    };

    device.destroy_pipeline(pipeline);
    device.destroy_shader(shader);
    device.destroy_sampler(sampler);
    device.destroy_buffer(buffer);
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
