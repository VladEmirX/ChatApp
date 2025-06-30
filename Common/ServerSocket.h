#pragma once
#include <string_view>
#include <expected>
#include "AutoSocket.h"

class ServerSocket
{
	std::uintptr_t _socket;

public:

	void swap(ServerSocket& other) noexcept
	{
		std::swap(_socket, other._socket);
	}

	[[nodiscard]]
	explicit ServerSocket(decltype(_socket) _socket) noexcept : _socket(_socket) {}

	[[nodiscard]]
	ServerSocket() noexcept;

	[[nodiscard]]
	ServerSocket(ServerSocket&& other) noexcept
	{
		swap(other);
	}

	[[nodiscard]]
	static std::expected<ServerSocket, int> TryCreate(wchar_t const* port) noexcept;

	ServerSocket& operator=(ServerSocket&& other) noexcept
	{
		ServerSocket().swap(*this);
		swap(other);
		return *this;
	}

	~ServerSocket() noexcept;

	[[nodiscard]]
	friend bool operator==(ServerSocket const&, ServerSocket const&) noexcept = default;

	[[nodiscard]]
	explicit operator bool() noexcept
	{
		return ServerSocket() != *this;
	}

	std::expected<AutoSocket, int> Wait() noexcept;
};

