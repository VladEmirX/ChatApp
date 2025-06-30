#pragma once

constexpr struct
{
	template <class T>
	operator T() const noexcept
	{
		static_assert(false);
	}
} required;

