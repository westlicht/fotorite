#pragma once

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

FR_NAMESPACE_BEGIN

//! Ignore unused parameters.
template<typename... Args>
inline constexpr void ignore_unused(Args &&...)
{
}

FR_NAMESPACE_END
