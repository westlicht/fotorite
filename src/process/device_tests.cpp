#include "process/device.h"

#include <doctest/doctest.h>

using namespace fr;

TEST_SUITE_BEGIN("process");

TEST_CASE("Device")
{
    DeviceDesc desc;
    desc.enable_validation_layers = false;
    Device device(desc);
}

TEST_SUITE_END();
