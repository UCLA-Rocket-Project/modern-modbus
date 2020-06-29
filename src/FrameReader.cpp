#include "FrameReader.h"
#include "ModbusError.h"
using namespace modbus;

// remember to deal with all endian stuff properly
bool FrameReader::readByte(uint8_t c, modbus::Frame& result) {
	if(state == FrameReaderState::AWAITING_FUNCTION) {
		frame.func = static_cast<FunctionCode>(c);
		state = FrameReaderState::AWAITING_START_ADDR;
		shortFirstByte = true;
	}
	else if(state == FrameReaderState::AWAITING_START_ADDR) {
		if(shortFirstByte) {
			shortBuf[0] = c;
			state = FrameReaderState::AWAITING_START_ADDR;
		}
		else {
			shortBuf[1] = c;
			if(modbus::isWriteSingle(frame.func)) {
				// no num addr will be specified, so pre-allocate for a single address.
				frame.numAddrs = 1;
				frame.writeSize = 2;
				frame.writeBuf.reset(new uint8_t [2]()); // 2 bytes to hold a short
				writeIdx = 0;
				state = FrameReaderState::AWAITING_WRITE_VALUE;
			}
			else {
				state = FrameReaderState::AWAITING_NUM_ADDRS;
			}
			frame.startOffset = be16toh(*(uint16_t*)shortBuf);
		}
		shortFirstByte = !shortFirstByte;
	}
	else if(state == FrameReaderState::AWAITING_NUM_ADDRS) {
		if(shortFirstByte) {
			shortBuf[0] = c;
			state = FrameReaderState::AWAITING_NUM_ADDRS;
		}
		else {
			shortBuf[1] = c;
			frame.numAddrs = be16toh(*(uint16_t*)shortBuf);
			// TODO: its late and I didn't want to do the math, just don't request crazy amounts of registers. will check in with the spec later
			if(isReadBools(frame.func) && frame.numAddrs > 2008) {
				throw ModbusError("Assertion failed: more than the max possible bits requested");
			}
			else if(isReadShorts(frame.func) && frame.numAddrs > 123) {
				throw ModbusError("Assertion failed: more than the max possible shorts requested");
			}
			else if(frame.func == WRITE_MULTIPLE_COILS && frame.numAddrs > 2000) {
				throw ModbusError("Assertion failed: more than the max possible bits requested");
			}
			else if(frame.func == WRITE_MULTIPLE_HOLDING_REGISTERS && frame.numAddrs > 120) {
				throw ModbusError("Assertion failed: more than the max possible shorts requested");
			}

			if(isReadShorts(frame.func) || isReadBools(frame.func)) {
				result = frame;
				return true;
			}
			else {
				state = FrameReaderState::AWAITING_WRITE_VALUE;
			}
			writeIdx = 0;
		}
		shortFirstByte = !shortFirstByte;
	}
	else if(state == FrameReaderState::AWAITING_WRITE_SIZE) {
		// TODO: check if write size matches the expected number of bytes from "numAddrs"
		frame.writeSize = c;

		if(frame.func == modbus::WRITE_MULTIPLE_HOLDING_REGISTERS && frame.writeSize != 2 * frame.numAddrs) {
			throw ModbusError("Assertion failed: writing multiple registers the number of bytes to write != 2 * the number of addresses");
		}
		else if(frame.func == modbus::WRITE_MULTIPLE_COILS && frame.writeSize != (frame.numAddrs + 7) / 8) {
			throw ModbusError("Assertion failed: writing multiple coils the number of bytes tow write != ceil(number of addresses / 8)");
		}

		frame.writeBuf.reset(new uint8_t[frame.writeSize]());
		writeIdx = 0;
		state = FrameReaderState::AWAITING_WRITE_VALUE;
	}
	else if(state == FrameReaderState::AWAITING_WRITE_VALUE) {
		frame.writeBuf[writeIdx] = c;
		writeIdx++;
		if(writeIdx == frame.writeSize) {
			result = frame; // copy out before we destroy it
			reset();
			return true; // yey we reached the end
		}
	}
	return false;
}

void FrameReader::reset() {
	frame = modbus::Frame();
	state = FrameReaderState::AWAITING_FUNCTION;
}