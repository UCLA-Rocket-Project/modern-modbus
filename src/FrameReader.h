#pragma once

#include <cstdint>
#include <memory>
#include "Frame.h"

namespace modbus {
	enum class FrameReaderState {
		AWAITING_FUNCTION,
		AWAITING_START_ADDR,
		AWAITING_NUM_ADDRS,
		AWAITING_WRITE_SIZE,
		AWAITING_WRITE_VALUE
	};

	class FrameReader {
	public:
		modbus::Frame frame;
		FrameReaderState state = FrameReaderState::AWAITING_FUNCTION;

		bool readByte(uint8_t c, modbus::Frame &result);

		void reset();

	private:
		uint8_t shortBuf[2] = {0};
		bool shortFirstByte = true; // true if writing the 1st byte of shortBuf, false if writing the 2nd
		uint16_t writeIdx = 0;
	};
}