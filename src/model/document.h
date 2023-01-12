#pragma once

#include "core/defs.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

FR_NAMESPACE_BEGIN

class Document {
public:
    Document(const std::filesystem::path &path) : m_path(path) {}
    ~Document() {}

private:
    std::filesystem::path m_path;
};

FR_NAMESPACE_END
