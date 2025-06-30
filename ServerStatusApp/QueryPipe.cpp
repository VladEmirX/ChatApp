#define NOMINMAX
#include <windows.h>
#include <string>
#include "Finally.h"

constexpr wchar_t pipe_name[] = L"\\\\.\\pipe\\ChatServicePipe";
constexpr size_t buffer_size = 1 << 16;
static wchar_t received[buffer_size]{};

using namespace std::string_literals;

std::wstring QueryPipe(std::wstring_view cmd) {
    HANDLE pipe = CreateFileW(
            pipe_name,
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr,
            OPEN_EXISTING,
            0, nullptr
    );

    if (pipe == INVALID_HANDLE_VALUE)
        return L"Unable to connect to server."s;

    FINALLY{ CloseHandle(pipe); };

    WriteFile(pipe, cmd.data(), cmd.size() * sizeof (wchar_t), nullptr, nullptr);

    DWORD read_count;
    if (ReadFile(pipe, received, sizeof received, &read_count, nullptr)) {
        return received;
    } else {
        return L"Failed to read from pipe."s;
    }
}