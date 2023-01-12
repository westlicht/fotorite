#include "stringutils.h"

FR_NAMESPACE_BEGIN

std::string to_lower(std::string str) {
    for (auto &c : str)
        c = ::tolower(c);
    return str;
}

std::string to_upper(std::string str) {
    for (auto &c : str)
        c = ::toupper(c);
    return str;
}

FR_NAMESPACE_END
