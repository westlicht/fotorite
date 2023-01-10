#pragma once

#include "defs.h"

#include <nlohmann/json_fwd.hpp>

#include <memory>
#include <filesystem>

FR_NAMESPACE_BEGIN

class Settings {
public:
    Settings();
    ~Settings();

    bool load(const std::filesystem::path &path);
    bool save(const std::filesystem::path &path);

private:
    std::unique_ptr<nlohmann::ordered_json> _json;
};

FR_NAMESPACE_END
