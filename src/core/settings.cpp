#include "settings.h"

#include <nlohmann/json.hpp>

#include <fstream>

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

Properties Settings::get(const char *section) const {
    auto it = _json->find(section);
    auto j = it != _json->end() ? *it : nlohmann::ordered_json{};
    return Properties(j);
}

void Settings::set(const char *section, const Properties &props) {
    _json->operator[](section) = props.json_value();
}

FR_NAMESPACE_END
