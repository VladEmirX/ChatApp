#define NOMINMAX
#include <unordered_map>
#include <string>
#include <shared_mutex>
#include <windows.h>
#include <thread>
#include "AutoSocket.h"
#include "Finally.h"
#include "StringConvert.h"
#include "Logger.h"


extern std::unordered_map<std::string, std::pair<AutoSocket, std::mutex>> users;
extern std::shared_mutex mut;

using namespace std::string_view_literals;
using namespace std::string_literals;


constexpr wchar_t pipe_name[] = L"\\\\.\\pipe\\ChatServicePipe";
constexpr int buffer_size = 256;


static void Work(HANDLE pipe) noexcept
{
    FINALLY {
        FlushFileBuffers(pipe);
        DisconnectNamedPipe(pipe);
        CloseHandle(pipe);
    };

    wchar_t request[buffer_size] = {};
    DWORD read = 0;

    if (!ReadFile(pipe, request, sizeof request, &read, nullptr) || read == 0)
        return;

    std::wstring_view command(request, read / sizeof(wchar_t));
    if (command == L"all"sv)
    {
        std::wstring response;

        {
            auto _ = std::shared_lock(mut);
            if (users.empty())
            {
                response = L"There are no users on the server"s;
            }
            for (auto& [name, sock] : users)
            {
                //auto _ = std::unique_lock(sock.second);
                if (auto addr_name = sock.first.Name())
                    response += std::format(L"{:m}: {:?}\r\n", *addr_name, StringConvert::Widen(name));
                else
                    continue;
            }
        }

        DWORD written = 0;
        WriteFile(pipe, response.c_str(), (DWORD)(response.size() * sizeof(wchar_t)), &written, nullptr);
    }
    else
    {
        constexpr wchar_t msg[] = L"Unsupported operation\n";
        WriteFile(pipe, msg, sizeof msg, nullptr, nullptr);
    }
}

[[noreturn]]
void PipeLogic() noexcept
{
    while (true)
    {
        HANDLE pipe = CreateNamedPipeW(
                pipe_name,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                4096, 4096, 0, nullptr
        );

        if (pipe == INVALID_HANDLE_VALUE)
        {
            std::this_thread::yield();
            continue;
        }

        Logger::Msg(L"Pipe created");
        if (!ConnectNamedPipe(pipe, nullptr) && GetLastError() != ERROR_PIPE_CONNECTED) {
            CloseHandle(pipe);
            continue;
        }
        Logger::Ok(L"Pipe connected");

        std::thread(Work, pipe).detach();
    }
}