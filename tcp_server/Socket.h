#pragma once
extern "C" {
	#include <modern_modbus_tcp.h>
}
#include "Responder.h"

namespace modbus { namespace tcp {
	class Socket {
	public:
		Socket(int sockFd, std::shared_ptr<Accessors> accs);
		void drainSocket();
		void readByte(uint8_t c);
		bool shouldDie();
		~Socket();
	private:
		int sockFd;
		bool zombie = false;
		modbus_tcp_parser_t parser;
		std::shared_ptr<Accessors> accs;
	};
}};