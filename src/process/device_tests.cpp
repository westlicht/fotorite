#include "process/device.h"

#include <doctest/doctest.h>

using namespace fr;

TEST_SUITE_BEGIN("process");

TEST_CASE("Device")
{
    DeviceDesc desc;
    desc.enable_validation_layers = true;
    Device device(desc);

    SamplerHandle s = device.create_sampler({
        .mag_filter = SamplerFilter::Linear,
        .min_filter = SamplerFilter::Nearest,
    });

    device.destroy_sampler(s);
}

TEST_SUITE_END();
