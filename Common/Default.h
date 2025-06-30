#pragma once

constexpr struct
{
    template <class T>
    operator T() const noexcept
    {
        return T{};
    }
} default_;

