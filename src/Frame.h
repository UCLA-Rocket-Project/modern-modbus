#pragma once
#include <cstdint>
#include <memory>
#include <string>

namespace modbus {
	// Copy off of wikipedia like a boss: https://en.wikipedia.org/wiki/Modbus
	enum FunctionCode {
		UNDEFINED = 0, // don't use this one in normal code
		READ_DISCRETE_INPUTS = 2,
		READ_COILS = 1,
		WRITE_SINGLE_COIL = 5,
		WRITE_MULTIPLE_COILS = 15,
		READ_INPUT_REGISTERS = 4,
		READ_MULTIPLE_HOLDING_REGISTERS = 3,
		WRITE_SINGLE_HOLDING_REGISTER = 6,
		WRITE_MULTIPLE_HOLDING_REGISTERS = 16
	};
	const char* functionCodeToStr(FunctionCode code);

	inline bool isReadBools(FunctionCode code) {
		return code == READ_DISCRETE_INPUTS || code == READ_COILS;
	}
	inline bool isReadShorts(FunctionCode code) {
		return code == READ_INPUT_REGISTERS || code == READ_MULTIPLE_HOLDING_REGISTERS;
	}
	inline bool isWriteSingle(FunctionCode code) {
		return code == WRITE_SINGLE_COIL || code == WRITE_SINGLE_HOLDING_REGISTER;
	}
	inline bool isWriteMultiple(FunctionCode code) {
		return code == WRITE_MULTIPLE_HOLDING_REGISTERS || code == WRITE_MULTIPLE_COILS;
	}

	struct Frame {
		FunctionCode func = UNDEFINED;
		uint16_t startOffset = 0;
		uint16_t numAddrs = 0;
		uint8_t writeSize;
		std::shared_ptr<uint8_t[]> writeBuf;
		std::string str();
		uint16_t startAddr() const {
			switch(func) {
			case READ_DISCRETE_INPUTS:
				return 10001 + startOffset;
			case READ_INPUT_REGISTERS:
				return 30001 + startOffset;
			case READ_COILS:
			case WRITE_SINGLE_COIL:
			case WRITE_MULTIPLE_COILS:
				return 1 + startOffset;
			case READ_MULTIPLE_HOLDING_REGISTERS:
			case WRITE_SINGLE_HOLDING_REGISTER:
			case WRITE_MULTIPLE_HOLDING_REGISTERS:
				return 40001 + startOffset;
			default:
				return -1;
			};
		}
	};

}

