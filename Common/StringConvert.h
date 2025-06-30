#pragma once
#include <string_view>

namespace StringConvert
{
    std::string Narrow(std::wstring_view w);
    std::wstring Widen(std::string_view s);
}