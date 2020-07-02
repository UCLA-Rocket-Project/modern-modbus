#pragma once
#include "Frame.h"
#include "Responder.h"
#include "FrameReader.h"

namespace modbus {
	enum class ModbusTCPState {
		AWAITING_TRANSACTION_ID,
		AWAITING_PROTOCOL_ID,
		AWAITING_LENGTH,
		AWAITING_IDENTIFIER,
		AWAITING_DATA
	};

	class ModbusTCPSocket {
	public:
		ModbusTCPSocket(int sockFd, ModbusCallback cb);
		void drainSocket();
		void readByte(uint8_t c);
		void reset();
		bool shouldDie();
		uint16_t transactionId;
		uint8_t slaveId;
		~ModbusTCPSocket();
	private:
		FrameReader reader;
		ModbusCallback cb;
		ModbusTCPState state = ModbusTCPState::AWAITING_TRANSACTION_ID;
		uint8_t shortBuf[2] = { 0 };
		bool shortFirstByte = true;
		uint16_t numBytes = 0;
		uint16_t bytesRead = 0;
		int sockFd;
		void sendResponse(uint8_t func, const uint8_t *msg, int len);
	};
}

