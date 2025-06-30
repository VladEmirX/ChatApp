#include "AutoSocket.h"
#include "Finally.h"
#include <unordered_map>
#include <shared_mutex>
#include <format>

#define TRY(...) ({ \
	decltype(auto) val = (__VA_ARGS__);\
	if (!val) return /*std::unexpected*/(std::forward<decltype(val)>(val).error()); \
	std::forward<decltype(val)>(val).value(); \
})

auto users = std::unordered_map<std::string, std::pair<AutoSocket, std::mutex>>{};
std::shared_mutex mut;

int ClientLogic(AutoSocket sock) noexcept
{
    decltype(users)::iterator it;
    while (true)
    {
        auto const name = TRY(sock.TryRecv());
        auto lock = std::unique_lock(mut);
        auto const [_it, inserted] = users.try_emplace(name);
        if (!inserted)
        {
            lock.release();
            TRY(sock.TrySend("0"));
        }
        else
        {
            it = _it;
            TRY(sock.TrySend("1"));
            it->second.first = std::move(sock);
            break;
        }
    }

    auto& sock_a = it->second.first;

    FINALLY{auto _ = std::unique_lock(mut); users.erase(it);};

    while(true)
    {
        auto msg = TRY(sock_a.TryRecv());
        auto _ = std::shared_lock(mut);
        for (auto& [_, p] : users)
        {
            auto& [s, m] = p;
            auto _ = std::unique_lock(m);
            void(s.TrySend(std::format("{}: {}", sock_a == s ? "me" : it->first, msg)));
        }
    }
}