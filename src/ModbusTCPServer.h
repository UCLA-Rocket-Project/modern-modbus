#pragma once
#include <sys/epoll.h>
#include <map>
#include <memory>
#include "ModbusTCPSocket.h"

namespace modbus {
	class ModbusTCPServer {
	public:
		ModbusTCPServer(int port, ModbusCallback cb);
		void tick();
		~ModbusTCPServer();
	private:
		int epfd;
		int serverFd;
		struct epoll_event events[16];
		ModbusCallback cb;
		std::map<int, std::unique_ptr<ModbusTCPSocket>> sockMap;
	};
}

