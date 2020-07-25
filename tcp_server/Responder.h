#pragma once
extern "C" {
	#include <modern_modbus_core.h>
}
#include <modern_modbus_tcp_server.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <array>

namespace modbus { namespace tcp {
	std::unique_ptr<uint8_t[]> fillResponse(modbus_command_t *cmd, uint16_t *readLen, const std::shared_ptr<Accessors> accs);
}};

