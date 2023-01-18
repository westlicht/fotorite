#pragma once

#include "core/defs.h"

#include <span>

FR_NAMESPACE_BEGIN

using ShaderBlob = std::span<const unsigned char>;

enum class ShaderID {
    test_buffer_cs,
    test_image_cs,
};

ShaderBlob get_shader_blob(ShaderID shader_id);

FR_NAMESPACE_END
