#pragma once
#include <ostream>
#include <format>

#define FORWARD(...) (::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__))

namespace Logger
{
    void Setup(std::shared_ptr<std::ostream> out) noexcept;
    void CustomRaw(std::string_view type, std::string_view msg) noexcept(false);
    void CustomRaw(std::wstring_view type, std::wstring_view msg) noexcept(false);

    template<class... ArgsT>
    void Custom(std::string_view type, std::format_string<ArgsT...> fmt, ArgsT&&... args)
    {
        CustomRaw(type, std::format(fmt, FORWARD(args)...));
    }

    template<class... ArgsT>
    void Custom(std::wstring_view type, std::wformat_string<ArgsT...> fmt, ArgsT&&... args)
    {
        CustomRaw(type, std::format(fmt, FORWARD(args)...));
    }


    template<class... ArgsT>
    void Error(std::format_string<ArgsT...> fmt, ArgsT&&... args)
    {
        Custom("Error", fmt, FORWARD(args)...);
    }

    template<class... ArgsT>
    void Msg(std::format_string<ArgsT...> fmt, ArgsT&&... args)
    {
        Custom("Msg", fmt, FORWARD(args)...);
    }

    template<class... ArgsT>
    void Ok(std::format_string<ArgsT...> fmt, ArgsT&&... args)
    {
        Custom("Ok", fmt, FORWARD(args)...);
    }

    template<class... ArgsT>
    void Error(std::wformat_string<ArgsT...> fmt, ArgsT&&... args)
    {
        Custom(L"Error", fmt, FORWARD(args)...);
    }

    template<class... ArgsT>
    void Msg(std::wformat_string<ArgsT...> fmt, ArgsT&&... args)
    {
        Custom(L"Msg", fmt, FORWARD(args)...);
    }

    template<class... ArgsT>
    void Ok(std::wformat_string<ArgsT...> fmt, ArgsT&&... args)
    {
        Custom(L"Ok", fmt, FORWARD(args)...);
    }
}