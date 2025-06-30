#include "ServerSocket.h"
#include "Logger.h"
#include <thread>

static std::atomic_bool g_stop = false;

extern int ClientLogic(AutoSocket sock) noexcept;

static void WaitForSocket(ServerSocket ssock) noexcept
{
	if (auto mb_sock = ssock.Wait())
	{
        using namespace std::string_literals;
		auto sock = *std::move(mb_sock);
		auto mb_name = sock.Name();

        auto name = mb_name ? *mb_name : std::pair{L"<unknown>"s, L"<unknown>"s};

        Logger::Ok(L"Opened connection {}", name, name);

		std::thread(WaitForSocket, std::move(ssock)).detach();

		auto res = ClientLogic(std::move(sock));
        Logger::Msg(L"Closed connection {} with code {}", name, res);

	}
	else
	{
		Logger::Error("Listening failed (error code: {}) ", mb_sock.error());
		g_stop.store(true);
		g_stop.notify_all();
	}
}

void SocketLogic(const wchar_t* port) noexcept
{
	if (auto mb_ssock = ServerSocket::TryCreate(port))
	{
		auto ssock = std::move(mb_ssock).value();
		Logger::Ok("Created server socket");

		std::thread(WaitForSocket, std::move(ssock)).detach();
		g_stop.wait(false);

		return;
	}
	else
	{
		Logger::Error("Unable to create socket (winsock error {})", mb_ssock.error());
		return;
	}
}
