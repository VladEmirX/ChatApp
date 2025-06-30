#include <msxml6.h>
#include <comdef.h>
#include <fstream>
#include <thread>
#include "Finally.h"
#include "Logger.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "msxml6.lib")

constexpr wchar_t log_output[] = L"./chat_server/log.txt";
constexpr wchar_t config_file[] = L"./chat_server/config.xml";
constexpr wchar_t port_path[] = L"/chat_server/port";

extern void SocketLogic(const wchar_t*) noexcept;
extern void PipeLogic() noexcept;


void Logic() noexcept
{
	setlocale(0, "en_US.utf-8");
	Logger::Setup(std::make_unique<std::ofstream>(log_output));

	WSAData wsa_data{};
	if (FAILED(CoInitializeEx(nullptr, 0))) 
	{
		Logger::Error("Unable to initialize COM");
		return;
	};
	WSAStartup(MAKEWORD(2, 2), &wsa_data);

	FINALLY{ WSACleanup(); CoUninitialize(); };

	IXMLDOMDocument* doc;
	if (FAILED(CoCreateInstance(CLSID_DOMDocument60, nullptr,
                                CLSCTX_INPROC_SERVER,
                                IID_IXMLDOMDocument,
                                (void**)&doc)))
	{
		Logger::Error("Error: Unable to create DOMDocument60");
		return;
	};
    FINALLY{doc->Release();};

	doc->put_async(VARIANT_FALSE);
	doc->put_validateOnParse(VARIANT_TRUE);

	if (VARIANT_BOOL loaded;FAILED(doc->load(_variant_t(config_file), &loaded)) || !loaded)
	{
        IXMLDOMParseError* err;
        if (FAILED(doc->get_parseError(&err)) || !err)
        {
            Logger::Error(L"Unable to load {}", config_file);
        }
        else
        {
            BSTR reason{};
            FINALLY{err->Release(); SysFreeString(reason);};
            long line, pos;
            err->get_reason(&reason);
            err->get_line(&line);
            err->get_linepos(&pos);

            Logger::Error(L"Unable to parse {}\n\tReason: {}\n\tLine: {}\n\tPosition: {}",
                          config_file,
                          reason,
                          line,
                          pos);
        }
        return;
    }

	Logger::Ok("The loading of XML file is successful");

    IXMLDOMNode* node{};
	BSTR text{};
    auto port_path_bstr = SysAllocString(port_path);

    FINALLY{if (node) node->Release();SysFreeString(text);SysFreeString(port_path_bstr);};

	if (SUCCEEDED(doc->selectSingleNode(SysAllocString(port_path_bstr), &node)) && node && SUCCEEDED(node->get_text(&text)) && text)
	{
        std::thread(PipeLogic).detach();

		Logger::Ok(L"Using port {}\n", (const wchar_t*)text);
		SocketLogic(text);
        return;
	}
	else
	{
		Logger::Error(L"Unable to find port. Please, write it at {}.", port_path);
		return;
	}
}