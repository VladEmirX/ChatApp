#include <exception>
#include <chrono>
#include "Logger.h"

#include <windows.h>

static auto Now() noexcept
{
    return std::chrono::sys_time<std::chrono::seconds>::clock::now();
}

static std::shared_ptr<std::ostream> g_out{};

void Logger::Setup(std::shared_ptr<std::ostream> out) noexcept
{
    g_out = std::move(out);
}

void Logger::CustomRaw(std::string_view type, std::string_view msg) noexcept(false)
{
    if (!g_out)
        throw std::runtime_error("Logger is uninitialized");
    std::println(*g_out, "[{}] {}: {}", Now(), type, msg);
    g_out->flush();
}

void Logger::CustomRaw(std::wstring_view type, std::wstring_view msg) noexcept(false)
{
    if (!g_out)
        throw std::runtime_error("Logger is uninitialized");

    auto formatted = std::format(L"[{}] {}: {}\n", Now(), type, msg);

    int len = WideCharToMultiByte(CP_UTF8, 0, formatted.data(), (int)formatted.size(), nullptr, 0, nullptr, nullptr);
    auto converted = std::string(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, formatted.data(), (int)formatted.size(), converted.data(), len, nullptr, nullptr);

    *g_out << converted << std::flush;
}


