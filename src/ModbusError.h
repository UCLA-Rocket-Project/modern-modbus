#pragma once
#include <stdexcept>
#include <string>
#include <functional>

namespace modbus {
	class ModbusError: public std::runtime_error {
	public:
		explicit ModbusError(const char* msg): std::runtime_error(msg) {};
		explicit ModbusError(const std::string& msg): std::runtime_error(msg) {};
	};

	using ProtocolErrReporter = std::function<void(ModbusError&)>;
	using OSErrReporter = std::function<void(std::runtime_error&)>;

	extern ProtocolErrReporter protocolErrReporter;
	extern OSErrReporter  osErrReporter;

	void setProtocolErrReporter(ProtocolErrReporter rep);
	void setOSErrReporter(OSErrReporter rep);
}

