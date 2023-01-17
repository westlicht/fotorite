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

    SUBCASE("sampler")
    {
        SamplerHandle sampler = device.create_sampler({
            .mag_filter = SamplerFilter::Linear,
            .min_filter = SamplerFilter::Nearest,
        });
        device.destroy_sampler(sampler);
    }

    SUBCASE("buffer")
    {
        BufferHandle buffer = device.create_buffer({
            .size = 1024,
            .usage = ResourceUsage::ShaderResource | ResourceUsage::UnorderedAccess,
            .memory = MemoryType::Device,
        });
        device.destroy_buffer(buffer);
    }

    SUBCASE("shader")
    {
        auto blob = get_shader_blob(ShaderID::test_cs);
        ShaderHandle shader = device.create_shader({
            .code = blob.data(),
            .code_size = blob.size_bytes(),
            .entry_point_name = "main",
        });
        device.destroy_shader(shader);
    }
}

TEST_CASE("compute" * doctest::skip(FOTORITE_GITHUB_CI))
{
    static const size_t N = 1024;
    std::vector<float> buffer0_data(N);
    std::vector<float> buffer1_data(N);
    std::vector<float> result_data(N);

    Device device({
        .enable_validation_layers = true,
    });

    BufferHandle buffer0 = device.create_buffer({
        .size = N * sizeof(float),
        .usage = ResourceUsage::ShaderResource,
        .memory = MemoryType::Device,
    });

    BufferHandle buffer1 = device.create_buffer({
        .size = N * sizeof(float),
        .usage = ResourceUsage::ShaderResource,
        .memory = MemoryType::Device,
    });

    BufferHandle result = device.create_buffer({
        .size = N * sizeof(float),
        .usage = ResourceUsage::UnorderedAccess,
        .memory = MemoryType::Device,
    });

    ShaderBlob blob = get_shader_blob(ShaderID::test_cs);
    ShaderHandle shader = device.create_shader({
        .code = blob.data(),
        .code_size = blob.size_bytes(),
    });

    PipelineHandle pipeline = device.create_pipeline({
        .shader = shader,
        .bindings{
            {.binding = 0, .type = DescriptorType::Buffer},
            {.binding = 1, .type = DescriptorType::Buffer},
            {.binding = 2, .type = DescriptorType::RWBuffer},
        },
        .push_constants_size = 4,
    });

#if 0
    SequenceHandle sequence = device.start_sequence();
    device.write_buffer(sequence, buffer0, buffer0_data.data(), N * sizeof(float));
    device.write_buffer(sequence, buffer1, buffer1_data.data(), N * sizeof(float));

    uint32_t push_constants = N;
    device.dispatch({
        .pipeline = pipeline,
        .bindings{
            {.binding = 0, .resource = buffer0},
            {.binding = 1, .resource = buffer1},
            {.binding = 2, .resource = result},
        },
        .push_constants = &push_constants,
        .push_constants_size = sizeof(push_constants),
    });

    device.read_buffer(sequence, result, result_data.data(), N * sizeof(float));
    device.end_sequence(sequence);
#endif

    device.destroy_pipeline(pipeline);
    device.destroy_shader(shader);
    device.destroy_buffer(buffer0);
    device.destroy_buffer(buffer1);
    device.destroy_buffer(result);
}

TEST_SUITE_END();
