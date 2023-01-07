#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

#include "defs.h"

FR_NAMESPACE_BEGIN

enum class ComponentType {
    Unknown,
    U8,
};

struct ImageSpec {
    uint32_t width{0};
    uint32_t height{0};
    ComponentType component_type{ComponentType::Unknown};
    uint32_t component_count{0};
};

class ImageReader;

class ImageInput {
public:
    static std::unique_ptr<ImageInput> open(const std::filesystem::path &path);

    ~ImageInput();

    const ImageSpec &get_spec() const { return _spec; }

    bool read_image(void *buffer, size_t len);

private:
    ImageSpec _spec;
    std::unique_ptr<ImageReader> _reader;
};

FR_NAMESPACE_END
