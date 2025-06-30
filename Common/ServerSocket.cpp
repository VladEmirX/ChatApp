#include <WinSock2.h>
#include <WS2Tcpip.h>
#include "ServerSocket.h"
#include "Finally.h"


#pragma comment(lib, "Ws2_32.lib")

ServerSocket::ServerSocket() noexcept : _socket(INVALID_SOCKET)
{
	static_assert(std::is_same_v<decltype(_socket), SOCKET>);
}

std::expected<ServerSocket, int> ServerSocket::TryCreate(wchar_t const* port) noexcept
{
	addrinfoW info{.ai_flags = AI_PASSIVE, .ai_family = AF_INET, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP,}
		, *result;
	
	if (auto err = GetAddrInfoW(nullptr, port, &info, &result))
	{
		return std::unexpected(err);
	}

	FINALLY { FreeAddrInfoW(result);};

	auto res = ServerSocket(socket(result->ai_family, result->ai_socktype, result->ai_protocol));
	if (!res)
	{
		return std::unexpected(WSAGetLastError());
	}

	if (auto err = bind(res._socket, result->ai_addr, (int)result->ai_addrlen); err == SOCKET_ERROR)
	{
		return std::unexpected(WSAGetLastError());
	}

	return res;
}

ServerSocket::~ServerSocket() noexcept
{
	closesocket(_socket);
}

std::expected<AutoSocket, int> ServerSocket::Wait() noexcept
{
	if (listen(_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		return std::unexpected(WSAGetLastError());
	}

	if (auto res = AutoSocket(accept(_socket, nullptr, nullptr)))
	{
		return res;
	}
	else
	{
		return std::unexpected(WSAGetLastError());
	}
}