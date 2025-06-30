#pragma once
#include <string_view>
#include <string>

namespace StringConvert
{
    std::string Narrow(std::wstring_view w);
    std::wstring Widen(std::string_view s);
}