#pragma once

#define FR_NAMESPACE_BEGIN namespace fr {
#define FR_NAMESPACE_END }

FR_NAMESPACE_BEGIN

//! Ignore unused parameters.
template<typename... Args>
inline constexpr void ignore_unused(Args &&...)
{
}

FR_NAMESPACE_END
