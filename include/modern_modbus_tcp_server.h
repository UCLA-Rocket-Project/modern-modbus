#pragma once
#include <sys/epoll.h>
#include <map>
#include <memory>
extern "C" {
#include <modern_modbus_core.h>
}

namespace modbus { namespace tcp {
	class Socket;
	template <typename T>
	using Getter = T(*)(MB_WORD addr);
	template <typename T>
	using Setter = void(*)(MB_WORD addr, T value);

	struct Accessors {
		Getter<bool> getBool;
		Setter<bool> setBool;
		Getter<MB_WORD> getWord;
		Setter<MB_WORD> setWord;
	};

	class TCPServer {
	public:
		TCPServer(int port, std::shared_ptr<Accessors> accs);
		void tick();
		~TCPServer();
	private:
		int epfd;
		int serverFd;
		struct epoll_event events[16];
		std::map<int, std::unique_ptr<Socket>> sockMap;
		std::shared_ptr<Accessors> accs;
	};
}}