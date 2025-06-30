/*
Creating:
    sc create ChatServer binPath= "_:\Path\To\ChatServerApp.exe"
For me:
    sc create ChatServer binPath= "D:\spaceless\practice\Common\cmake-build-debug-mingw-clang\ChatServerApp.exe"
Start:
    sc start ChatServer
Delete:
    sc delete ChatServer

Files path:
    C:\Windows\System32\chat_server
 */

#define NOMINMAX
#include <Windows.h>
#include <algorithm>
#include <print>
#include <chrono>
#include <thread>
#include "Finally.h"



SERVICE_STATUS g_ServiceStatus = {};
SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
HANDLE g_ServiceStopEvent = nullptr;

extern void Logic() noexcept;

void WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	if (CtrlCode == SERVICE_CONTROL_STOP && g_ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

		SetEvent(g_ServiceStopEvent);
	}
}

void WINAPI ServiceMain(DWORD argc, LPWSTR argv[])
{
	g_StatusHandle = RegisterServiceCtrlHandlerW(L"MyService", ServiceCtrlHandler);
	if (!g_StatusHandle)
		return;


	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	g_ServiceStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
	if (!g_ServiceStopEvent)
	{
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
		return;
	}

	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	FINALLY 
	{
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        ExitProcess(0); // Don't want to make normal stop handle
	};

	std::thread(Logic).detach();
    WaitForSingleObject(g_ServiceStopEvent, INFINITE);
}

int main()
{
    wchar_t name[] = L"ChatServer";

	SERVICE_TABLE_ENTRYW table[]
	{
		{const_cast<LPWSTR>(name), ServiceMain},
		{}
	};

	if (!StartServiceCtrlDispatcherW(table))
	{
		constexpr wchar_t error_message[] = L"Error: the app isn't opened as service\n";
		WriteConsoleW(GetStdHandle(STD_ERROR_HANDLE), error_message, std::size(error_message), nullptr, nullptr);
		return -1;
	}
}
