#define NOMINMAX
#include <windows.h>
#include <string>
#include <clocale>


constexpr wchar_t class_name[] = L"Window";
constexpr wchar_t window_name[] = L"Server Status App";

HWND hRefreshBtn, hUsersEdit;

std::wstring QueryPipe(std::wstring_view cmd) ;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        case WM_CREATE: {
            hRefreshBtn = CreateWindowW(L"BUTTON", L"Refresh", WS_CHILD | WS_VISIBLE,
                                   0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);
            hUsersEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                                          0, 0, 0, 0, hwnd, nullptr, nullptr, nullptr);
            break;
        }
        case WM_SIZE: {
            const int w = LOWORD(lParam);
            const int h = HIWORD(lParam);

            constexpr int padding = 10;
            constexpr int btn_h = 40;

            MoveWindow(hUsersEdit, padding, padding, w - padding * 2, h - padding * 3 - btn_h, TRUE);
            MoveWindow(hRefreshBtn, padding, h - padding - btn_h, w - padding * 2, btn_h, TRUE);


            return 0;
        }
        case WM_COMMAND:
            if ((HWND)lParam == hRefreshBtn) {
                std::wstring content = QueryPipe(L"all");
                SetWindowTextW(hUsersEdit, content.c_str());
            }
            break;
        default:
    }


    return DefWindowProcW(hwnd, msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    setlocale(0, "en_US.UTF-8");
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    auto wc = WNDCLASSEXW{
            .cbSize = sizeof(WNDCLASSEXW),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = WindowProc,
            .hInstance = hInstance,
            .hCursor = LoadCursor(nullptr, IDC_ARROW),
            .hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1),
            .lpszClassName = class_name,
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
