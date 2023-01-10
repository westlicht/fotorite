#pragma once

#include "defs.h"

#include <nanogui/vector.h>
#include <nlohmann/json_fwd.hpp>

#include <memory>

FR_NAMESPACE_BEGIN

class Properties {
public:
    Properties();
    ~Properties();

    template <typename T>
    T get(const char *name, const T &default_value) const;

    template <typename T>
    void set(const char *name, const T &value);

private:
    std::unique_ptr<nlohmann::ordered_json> _json;
};

#define ACCESSOR_DECL(T)                                 \
    template <>                                          \
    T Properties::get<T>(const char *, const T &) const; \
    template <>                                          \
    void Properties::set<T>(const char *, const T &);

ACCESSOR_DECL(int)
ACCESSOR_DECL(bool)
ACCESSOR_DECL(nanogui::Vector2i)
ACCESSOR_DECL(nanogui::Vector3i)
ACCESSOR_DECL(float)
ACCESSOR_DECL(nanogui::Vector2f)
ACCESSOR_DECL(nanogui::Vector3f)

#undef ACCESSOR_DECL

FR_NAMESPACE_END
