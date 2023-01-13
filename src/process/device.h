#pragma once

#include "core/defs.h"

#include <memory>

FR_NAMESPACE_BEGIN

struct DeviceImpl;

class Device {
public:
    Device();
    ~Device();

private:
    std::unique_ptr<DeviceImpl> m_impl;
};

FR_NAMESPACE_END
