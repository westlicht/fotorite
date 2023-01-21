#include "process/device.h"
#include "shaders/shaders.h"

#include <doctest/doctest.h>

using namespace fr;

TEST_SUITE_BEGIN("process");

TEST_CASE("Device" * doctest::skip(true || FOTORITE_GITHUB_CI))
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
        auto blob = get_shader_blob(ShaderID::test_buffer_cs);
        ShaderHandle shader = device.create_shader({
            .code = blob.data(),
            .code_size = blob.size_bytes(),
            .entry_point_name = "main",
        });
        device.destroy_shader(shader);
    }
}

TEST_CASE("compute buffer" * doctest::skip(FOTORITE_GITHUB_CI))
{
    static const size_t N = 1024;

    std::vector<float> buffer0_data(N);
    std::vector<float> buffer1_data(N);
    std::vector<float> result_data(N);

    for (size_t i = 0; i < N; ++i) {
        buffer0_data[i] = i;
        buffer1_data[i] = i * 10;
    }

    Device device({
        .enable_validation_layers = true,
    });

    BufferHandle buffer0 = device.create_buffer({
        .size = N * sizeof(float),
        .usage = ResourceUsage::ShaderResource | ResourceUsage::TransferSrc | ResourceUsage::TransferDst,
        .memory = MemoryType::Device,
    });

    BufferHandle buffer1 = device.create_buffer({
        .size = N * sizeof(float),
        .usage = ResourceUsage::ShaderResource | ResourceUsage::TransferSrc | ResourceUsage::TransferDst,
        .memory = MemoryType::Device,
    });

    BufferHandle result = device.create_buffer({
        .size = N * sizeof(float),
        .usage = ResourceUsage::UnorderedAccess | ResourceUsage::TransferSrc | ResourceUsage::TransferDst,
        .memory = MemoryType::Device,
    });

    ShaderBlob blob = get_shader_blob(ShaderID::test_buffer_cs);
    ShaderHandle shader = device.create_shader({
        .code = blob.data(),
        .code_size = blob.size_bytes(),
    });

    PipelineHandle pipeline = device.create_pipeline({
        .shader = shader,
        .binding_layout{
            {.binding = 0, .type = DescriptorType::StructuredBuffer},
            {.binding = 1, .type = DescriptorType::StructuredBuffer},
            {.binding = 2, .type = DescriptorType::RWStructuredBuffer},
        },
        .push_constants_size = 4,
    });

    ContextHandle context = device.create_context();
#if 0
    device.begin(context);
    device.write_buffer(context, buffer0, buffer0_data.data(), N * sizeof(float));
    device.copy_buffer(context, buffer0, buffer1, N * sizeof(float));
    device.copy_buffer(context, buffer1, result, N * sizeof(float));
    device.read_buffer(context, buffer1, buffer1_data.data(), N * sizeof(float));
    device.submit(context);
    device.wait(context);
#else
    device.begin(context);
    device.write_buffer(context, buffer0, buffer0_data.data(), N * sizeof(float));
    device.write_buffer(context, buffer1, buffer1_data.data(), N * sizeof(float));

    uint32_t push_constants = N;
    device.dispatch(context, {
                                 .pipeline = pipeline,
                                 .binding_set{
                                     {.binding = 0, .resource = buffer0},
                                     {.binding = 1, .resource = buffer1},
                                     {.binding = 2, .resource = result},
                                 },
                                 .push_constants = &push_constants,
                                 .push_constants_size = sizeof(push_constants),
                                 .group_count{N / 256, 1, 1},
                             });

    device.read_buffer(context, result, result_data.data(), N * sizeof(float));
    device.submit(context);
    device.wait(context);
#endif

    device.destroy_context(context);

    device.destroy_pipeline(pipeline);
    device.destroy_shader(shader);
    device.destroy_buffer(buffer0);
    device.destroy_buffer(buffer1);
    device.destroy_buffer(result);
}

TEST_CASE("compute image" * doctest::skip(FOTORITE_GITHUB_CI))
{
    static const size_t N = 1024;

    std::vector<float> src_image_data(N * N * 4);
    std::vector<float> dst_image_data(N * N * 4);

    {
        float *data = src_image_data.data();
        for (uint32_t y = 0; y < N; ++y) {
            for (uint32_t x = 0; x < N; ++x) {
                *data++ = float(x) / N;
                *data++ = float(y) / N;
                *data++ = 0.f;
                *data++ = 1.f;
            }
        }
    }

    Device device({
        .enable_validation_layers = true,
    });

    ImageHandle src_image = device.create_image({
        .width = N,
        .height = N,
        .format = ImageFormat::RGBA32Float,
        .usage = ResourceUsage::ShaderResource | ResourceUsage::TransferSrc | ResourceUsage::TransferDst,
        .memory = MemoryType::Device,
    });

    ImageHandle dst_image = device.create_image({
        .width = N,
        .height = N,
        .format = ImageFormat::RGBA32Float,
        .usage = ResourceUsage::UnorderedAccess | ResourceUsage::TransferSrc | ResourceUsage::TransferDst,
        .memory = MemoryType::Device,
    });

    ShaderBlob blob = get_shader_blob(ShaderID::test_image_cs);
    ShaderHandle shader = device.create_shader({
        .code = blob.data(),
        .code_size = blob.size_bytes(),
    });

    PipelineHandle pipeline = device.create_pipeline({
        .shader = shader,
        .binding_layout{
            {.binding = 0, .type = DescriptorType::Texture},
            {.binding = 1, .type = DescriptorType::RWTexture},
        },
        .push_constants_size = 8,
    });

    ContextHandle context = device.create_context();
#if 0
    device.begin(context);
    device.write_image(context, src_image, src_image_data.data(), src_image_data.size() * sizeof(float));
    device.copy_image(context, src_image, dst_image, N, N);
    device.read_image(context, dst_image, dst_image_data.data(), dst_image_data.size() * sizeof(float));
    device.submit(context);
    device.wait(context);
#else
    device.begin(context);
    device.write_image(context, src_image, src_image_data.data(), src_image_data.size() * sizeof(float));

    uint32_t push_constants[2] = {N, N};
    device.dispatch(context, {
                                 .pipeline = pipeline,
                                 .binding_set{
                                     {.binding = 0, .resource = src_image},
                                     {.binding = 1, .resource = dst_image},
                                 },
                                 .push_constants = &push_constants,
                                 .push_constants_size = sizeof(push_constants),
                                 .group_count{N / 32, N / 32, 1},
                             });

    device.read_image(context, dst_image, dst_image_data.data(), dst_image_data.size() * sizeof(float));
    device.submit(context);
    device.wait(context);
#endif

    device.destroy_context(context);

    device.destroy_pipeline(pipeline);
    device.destroy_shader(shader);
    device.destroy_image(src_image);
    device.destroy_image(dst_image);
}

TEST_SUITE_END();
