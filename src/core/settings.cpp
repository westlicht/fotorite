#include "settings.h"

#include <nlohmann/json.hpp>

#include <fstream>

using json = nlohmann::ordered_json;

FR_NAMESPACE_BEGIN

Settings::Settings() { _json = std::make_unique<json>(); }
Settings::~Settings() {}

bool Settings::load(const std::filesystem::path &path) {
    std::ifstream ifs(path);
    if (!ifs.good())
        return false;

    *_json = json::parse(ifs);
    return true;
}

bool Settings::save(const std::filesystem::path &path) {
    std::ofstream ofs(path);
    if (!ofs.good())
        return false;

    ofs << _json->dump(4);
    return true;
}

FR_NAMESPACE_END
