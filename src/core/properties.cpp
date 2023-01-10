#include "properties.h"

#include <nlohmann/json.hpp>

#include <fstream>

using json = fr::Properties::json;

namespace nanogui {
template <typename T, size_t N>
void to_json(json &j, const Array<T, N> &a) {
    j = json::array();
    for (size_t i = 0; i < N; ++i) {
        j[i] = a[i];
    }
}
template <typename T, size_t N>
void from_json(const json &j, Array<T, N> &a) {
    if (j.is_array() && j.size() == N) {
        for (size_t i = 0; i < N; ++i) {
            j[i].get_to(a[i]);
        }
    }
}

}  // namespace nanogui

FR_NAMESPACE_BEGIN

Properties::Properties() { _json = std::make_unique<json>(); }
Properties::Properties(const nlohmann::ordered_json &j) { _json = std::make_unique<json>(j); }
Properties::~Properties() {}

const Properties::json &Properties::json_value() const { return *_json; }

bool Properties::has(const char *name) const { return _json->find(name) != _json->end(); }

#define ACCESSOR_IMPL(T)                                               \
    template <>                                                        \
    T Properties::get<T>(const char *name, const T &def_value) const { \
        T result = def_value;                                          \
        if (auto it = _json->find(name); it != _json->end()) {         \
            it->get_to(result);                                        \
        }                                                              \
        return result;                                                 \
    }                                                                  \
    template <>                                                        \
    void Properties::set<T>(const char *name, const T &value) {        \
        (*_json)[name] = value;                                        \
    }

ACCESSOR_IMPL(bool)
ACCESSOR_IMPL(int)
ACCESSOR_IMPL(nanogui::Vector2i)
ACCESSOR_IMPL(nanogui::Vector3i)
ACCESSOR_IMPL(float)
ACCESSOR_IMPL(nanogui::Vector2f)
ACCESSOR_IMPL(nanogui::Vector3f)
ACCESSOR_IMPL(std::string)

FR_NAMESPACE_END
