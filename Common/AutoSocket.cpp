#define NOMINMAX
#include <utility>
#include <WinSock2.h>
#include <WS2Tcpip.h>
#include "AutoSocket.h"
#include "Finally.h"


#pragma comment(lib, "Ws2_32.lib")


AutoSocket::AutoSocket() noexcept : _socket(INVALID_SOCKET) 
{
	static_assert(std::is_same_v<decltype(_socket), SOCKET>);
}

AutoSocket::~AutoSocket() noexcept
{
	closesocket(_socket);
}

std::expected<AutoSocket, int> AutoSocket::TryConnect(wchar_t const* address, wchar_t const* port) noexcept
{
	auto hints = addrinfoW{.ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP,};
	addrinfoW* res;
	
	if (GetAddrInfoW(address, port, &hints, &res))
	{
		return std::unexpected(WSAGetLastError());
	}

	FINALLY {FreeAddrInfoW(res); };

	for (auto ptr = res; ptr; ptr = ptr->ai_next)
	{
		auto result = AutoSocket(socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol));
		if (!result) 
			return std::unexpected(WSAGetLastError());
		if (connect(result._socket, ptr->ai_addr, (int)ptr->ai_addrlen)) 
			continue;
		
		return result;
	}

	return std::unexpected(-1);
}

std::expected<void, int> AutoSocket::TrySend(std::string_view s) const
{
	auto len = s.length();
	auto slen = (std::uint64_t)htonll(len);
	if (send(_socket, reinterpret_cast<const char*>(&slen), sizeof slen, 0) == SOCKET_ERROR)
		return std::unexpected(WSAGetLastError());
	
	size_t sent = 0;

	while (sent < len)
	{
		if (auto const got = send(_socket, s.data() + sent, (int)std::min(len - sent, send_max), 0); got == SOCKET_ERROR)
			return std::unexpected(WSAGetLastError());
		else sent += got;
	}

	return {};
}

std::expected<std::string, int> AutoSocket::TryRecv() const
{
	auto result = std::string(sizeof(uint64_t), '\0');


	auto len0 = recv(_socket, result.data(), (int)result.length(), 0);

	if(len0 == SOCKET_ERROR)
		return std::unexpected(WSAGetLastError());

	if (len0 != sizeof(uint64_t))
		return std::unexpected(ERROR_INVALID_MESSAGE);

	auto llen = ntohll(*reinterpret_cast<uint64_t*>(result.data()));
	if (llen > SIZE_MAX)
		return std::unexpected(ERROR_BUFFER_OVERFLOW);
	size_t len = llen;
	result.resize(len);
	size_t received = 0;

	while (received < len)
	{
		if (auto const got = recv(_socket, result.data() + received, (int)std::min(len - received, send_max), 0); got == SOCKET_ERROR)
			return std::unexpected(WSAGetLastError());
		else if (got == 0)
			return std::unexpected(ERROR_CONNECTION_ABORTED);
		else received += got;
	}

	return result;
}

std::expected<std::pair<std::wstring, std::wstring>, int> AutoSocket::Name() const {
    sockaddr_storage addr{};
    int len = sizeof addr;
    if (getpeername(_socket, reinterpret_cast<sockaddr*>(&addr), &len) == SOCKET_ERROR)
        return std::unexpected(WSAGetLastError());

    wchar_t host[NI_MAXHOST];
    wchar_t serv[NI_MAXSERV];

    if (auto err = GetNameInfoW(
            reinterpret_cast<sockaddr*>(&addr),
            len,
            host, NI_MAXHOST,
            serv, NI_MAXSERV,
            0);
    err != 0)
    {
        return std::unexpected(err);
    }

    return {{host, serv}};
}

std::expected<void, int> AutoSocket::TryClose() const
{
    if (auto err = closesocket(_socket))
        return std::unexpected(err);
    return {};
}


