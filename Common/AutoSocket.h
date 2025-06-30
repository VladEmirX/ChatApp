#pragma once
#include "required.h"
#include <expected>
#include <string_view>
#include <string>

class AutoSocket
{
	uintptr_t _socket = required;


public:
	
	void swap(AutoSocket& other) noexcept
	{
		std::swap(_socket, other._socket);
	}

	[[nodiscard]]
	explicit AutoSocket(decltype(_socket) _socket) noexcept : _socket(_socket) {}

	[[nodiscard]]
	AutoSocket() noexcept;

	[[nodiscard]]
	AutoSocket(AutoSocket&& other) noexcept : AutoSocket()
	{
		swap(other);
	}

	AutoSocket& operator=(AutoSocket&& other) noexcept
	{
		AutoSocket().swap(*this);
		swap(other);
		return *this;
	}

	~AutoSocket() noexcept;



	[[nodiscard]]
	friend bool operator==(AutoSocket const&, AutoSocket const&) noexcept = default;

	[[nodiscard]]
	explicit operator bool() const noexcept
	{
		return *this != AutoSocket();
	}

	[[nodiscard]]
	static std::expected<AutoSocket, int> TryConnect(wchar_t const* address, wchar_t const* port) noexcept;

	[[nodiscard]]
	std::expected<void, int> TrySend(std::string_view) const;

	[[nodiscard]]
	std::expected<std::string, int> TryRecv() const;

	static constexpr size_t send_max = INT_MAX;

    [[nodiscard]]
    std::expected<std::pair<std::wstring, std::wstring>, int> Name() const;

    [[nodiscard]]
    std::expected<void, int> TryClose() const;
};

