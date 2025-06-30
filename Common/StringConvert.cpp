#include "StringConvert.h"
#define NOMINMAX
#include <string>
#include <windows.h>


std::string StringConvert::Narrow(std::wstring_view w)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    auto converted = std::string(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), converted.data(), len, nullptr, nullptr);

    return converted;
}

std::wstring StringConvert::Widen(std::string_view s) {

    int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    auto converted = std::wstring(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), converted.data(), len);

    return converted;
}
