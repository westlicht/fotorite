#pragma once

#include "defs.h"
#include "properties.h"

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

    Properties get(const char *section) const;
    void set(const char *section, const Properties&props);

private:
    using json = nlohmann::ordered_json;

    std::unique_ptr<json> _json;
};

FR_NAMESPACE_END
