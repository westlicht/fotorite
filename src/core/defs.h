#pragma once

#include <stdexcept>  // TODO remove

// OS defines
#if defined(__WIN32__) || defined(_MSC_VER)
#define FR_WINDOWS 1
#else
#define FR_WINDOWS 0
#endif

#if defined(__linux__)
#define FR_LINUX 1
#else
#define FR_LINUX 0
#endif

#if defined(__APPLE__)
#define FR_MACOS 1
#else
#define FR_MACOS 0
#endif

#define FR_NAMESPACE_BEGIN namespace fr {
#define FR_NAMESPACE_END }

#define FR_ASSERT(c)                            \
    {                                           \
        if (!(c))                               \
            throw std::runtime_error("assert"); \
    }

#define FR_ENUM_FLAG_OPERATORS(T)                                                                                \
    static_assert(sizeof(T) == sizeof(uint32_t));                                                                \
    inline T operator~(T a) { return static_cast<T>(~static_cast<uint32_t>(a)); }                                \
    inline T operator|(T a, T b) { return static_cast<T>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); } \
    inline T operator&(T a, T b) { return static_cast<T>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); } \
    inline T operator^(T a, T b) { return static_cast<T>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b)); } \
    inline T &operator|=(T &a, T b)                                                                              \
    {                                                                                                            \
        return reinterpret_cast<T &>(reinterpret_cast<uint32_t &>(a) |= static_cast<uint32_t>(b));               \
    }                                                                                                            \
    inline T &operator&=(T &a, T b)                                                                              \
    {                                                                                                            \
        return reinterpret_cast<T &>(reinterpret_cast<uint32_t &>(a) &= static_cast<uint32_t>(b));               \
    }                                                                                                            \
    inline T &operator^=(T &a, T b)                                                                              \
    {                                                                                                            \
        return reinterpret_cast<T &>(reinterpret_cast<uint32_t &>(a) ^= static_cast<uint32_t>(b));               \
    }                                                                                                            \
    inline bool is_set(T a, T flags) { return (static_cast<uint32_t>(a) & static_cast<uint32_t>(flags)) != 0; }

FR_NAMESPACE_BEGIN

//! Ignore unused parameters.
template <typename... Args>
inline constexpr void ignore_unused(Args &&...)
{
}

FR_NAMESPACE_END
