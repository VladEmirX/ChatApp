#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <thread>
#include "Logger.h"
#include "AutoSocket.h"
#include "StringConvert.h"
#include "Finally.h"

constexpr wchar_t class_name[] = L"Window";
constexpr wchar_t window_name[] = L"Chat App";

static HWND hIpEdit, hPortEdit, hNameEdit,
     hIpLbl,  hPortLbl,  hNameLbl,
    hLoadBtn, hMessagesEdit, hInputEdit, hSendBtn;

static AutoSocket client_socket;
static std::thread receive_thread;
static std::atomic_bool connected = false;

#define TRY(...) ({ \
	decltype(auto) val = (__VA_ARGS__);\
	if (!val) { AppendMessage(std::format(L"Error: {}", val.error())); break; } \
	std::forward<decltype(val)>(val).value(); \
})

#define TRY_CONNECT(...) ({ \
	decltype(auto) val = (__VA_ARGS__);\
	if (!val) { AppendMessage(std::format(L"Can't connect to the server, winsock error: {}", val.error())); break; } \
	std::forward<decltype(val)>(val).value(); \
})

static void AppendMessage(const std::wstring& msg) {
    int len = GetWindowTextLengthW(hMessagesEdit);
    SendMessageW(hMessagesEdit, EM_SETSEL, len, len);
    SendMessageW(hMessagesEdit, EM_REPLACESEL, FALSE, (LPARAM)(msg + L"\r\n").c_str());
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    using namespace StringConvert;
    using namespace std::string_view_literals;

    switch (msg) {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        case WM_CREATE:
        {
            //Сверху
            hIpLbl = CreateWindowW(L"STATIC", L"IP:", WS_CHILD | WS_VISIBLE,
                          0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);
            hIpEdit = CreateWindowW(L"EDIT", L"127.0.0.1", WS_CHILD | WS_VISIBLE | WS_BORDER,
                                    0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);

            hPortLbl = CreateWindowW(L"STATIC", L"Port:", WS_CHILD | WS_VISIBLE,
                          0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);
            hPortEdit = CreateWindowW(L"EDIT", L"1998", WS_CHILD | WS_VISIBLE | WS_BORDER,
                                      0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);

            hNameLbl = CreateWindowW(L"STATIC", L"Name:", WS_CHILD | WS_VISIBLE,
                          0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);
            hNameEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
                                      0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);

            hLoadBtn = CreateWindowW(L"BUTTON", L"Connect", WS_CHILD | WS_VISIBLE,
                                     0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);

            // Основное

            hMessagesEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                                          0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);

            hInputEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
                                       0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);

            hSendBtn = CreateWindowW(L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE,
                                     0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);


            SendMessageW(hIpEdit, EM_LIMITTEXT, 15, 0);
            SendMessageW(hPortEdit, EM_LIMITTEXT, 5, 0);
            SendMessageW(hNameEdit, EM_LIMITTEXT, 32, 0);
            SendMessageW(hInputEdit, EM_LIMITTEXT, 256, 0);
            break;
        }

        case WM_SIZE:
        {
            const int w = LOWORD(lParam);
            const int h = HIWORD(lParam);

            constexpr int padding = 10;
            constexpr int edit_h = 20;
            constexpr int row_h = 40;

            MoveWindow(hIpLbl, 10, padding, 20, edit_h, TRUE);
            MoveWindow(hIpEdit, 40, padding, 120, edit_h, TRUE);
            MoveWindow(hPortLbl, 170, padding, 40, edit_h, TRUE);
            MoveWindow(hPortEdit, 210, padding, 80, edit_h, TRUE);
            MoveWindow(hNameLbl, 295, padding, 45, edit_h, TRUE);
            MoveWindow(hNameEdit, 345, padding, 100, edit_h, TRUE);
            MoveWindow(hLoadBtn, w - 80, padding, 70, edit_h, TRUE);

            MoveWindow(hMessagesEdit, padding, row_h, w - 2 * padding, h - 2 * row_h - padding, TRUE);

            MoveWindow(hInputEdit, padding, h - row_h, w - 100 - 3 * padding, edit_h, TRUE);
            MoveWindow(hSendBtn, w - 85 - padding, h - row_h, 85, edit_h, TRUE);
            return 0;
        }
        case WM_CTLCOLOREDIT:
        {
            SetBkColor((HDC)wParam, RGB(255, 255, 255));
            SetTextColor((HDC)wParam, RGB(0, 0, 0));
            return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
        }
        case WM_COMMAND:
            // Connect button
            if ((HWND)lParam == hLoadBtn) {
                if (connected) {
                    connected = false;
                    if (auto ok = client_socket.TryClose(); !ok)
                    {
                        std::wclog << std::format(L"Can't close socket ({})\n", ok.error());
                    }
                    if (receive_thread.joinable())
                        receive_thread.join();
                    std::wclog << L"closed\n";
                    client_socket = {};
                }

                SetWindowTextW(hMessagesEdit, L"");

                wchar_t ip[32], port[8], name[64]{};
                GetWindowTextW(hIpEdit, ip, 32);
                GetWindowTextW(hPortEdit, port, 8);
                GetWindowTextW(hNameEdit, name, 64);

                if (name == L""sv)
                {
                    AppendMessage(std::format(L"Can't use empty name", name));
                    break;
                }
                if (name == L"me"sv)
                {
                    AppendMessage(std::format(L"Please don't", name));
                    break;
                }

                std::string name8 = Narrow(name);

                std::wclog << L"pressed\n";
                auto sock = TRY_CONNECT(AutoSocket::TryConnect(ip, port));
                std::wclog << L"connected\n";
                TRY_CONNECT(sock.TrySend(name8));
                std::wclog << L"sent\n";
                auto resp = TRY_CONNECT(sock.TryRecv());


                if (resp == "0") {
                    AppendMessage(std::format(L"Can't connect to the server, name {} already in use", name));
                    std::wclog << std::format(L"Can't connect to the server, name {} already in use", name);
                    break;
                }
                std::clog << "ok\n";


                client_socket = std::move(sock);
                connected = true;

                receive_thread = std::thread([hwnd]() {
                    while (connected) {
                        auto msg = client_socket.TryRecv();
                        if (!msg) break;
                        AppendMessage(Widen(*msg));
                    }
                    connected = false;
                });
                AppendMessage(L"Connection success");
            }
            // Send button
            else if ((HWND)lParam == hSendBtn && connected) {
                wchar_t input[256];
                GetWindowTextW(hInputEdit, input, 256);
                SetWindowTextW(hInputEdit, L"");

                std::string msg8 = Narrow(input);
                TRY(client_socket.TrySend(msg8));
            }
            break;
        default:
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    WSAData wsa_data{};
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
    FINALLY{ WSACleanup(); };

    setlocale(0, "en_US.UTF-8");
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    auto wc = WNDCLASSEXW{
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WindowProc,
        .hInstance = hInstance,
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1),        .lpszClassName = class_name,
    };
    RegisterClassExW(&wc);

    auto win = CreateWindowExW(
            0,
            class_name,
            window_name,
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            CW_USEDEFAULT, CW_USEDEFAULT, 550, 450,
            nullptr,
            nullptr,
            hInstance,
            nullptr
            );

    if (!win) return -1;

    ShowWindow(win, SW_SHOWDEFAULT);

    auto msg = MSG{};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}
