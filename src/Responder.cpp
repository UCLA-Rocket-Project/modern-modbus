#include "Responder.h"
#include <memory>
#include <endian.h>
using namespace std;
using namespace modbus;

/** Issue with Out of bounds Integers!!!! */
void Responder::respWithAccessors(Accessors &accessors) {
	if(isReadBools(frame.func)) {
		uint8_t dataSize = (frame.numAddrs + 7) / 8;
		unique_ptr<uint8_t[]> resp(new uint8_t[1 + dataSize]());
		resp[0] = dataSize;
		for(uint16_t i = 0; i < frame.numAddrs; i++) {
			uint16_t addr = i + frame.startOffset + (frame.func == READ_COILS ? 1 : 10001);
			uint8_t &theByte = resp[2 + i / 8];
			if(accessors.getBool(addr)) {
				theByte |= 1 << (i % 8);
			}
			else {
				theByte &= ~(1 << (i % 8));
			}
		}
		send(frame.func, resp.get(), 1 + dataSize);
	}
	else if(isReadShorts(frame.func)) {
		uint8_t dataSize = frame.numAddrs * 2;
		unique_ptr<uint8_t[]> resp(new uint8_t[1 + dataSize]());
		resp[0] = dataSize;
		for(uint16_t i = 0; i < frame.numAddrs; i++) {
			uint16_t addr = i + frame.startOffset +
			                (frame.func == READ_MULTIPLE_HOLDING_REGISTERS ? 40001 : 30001);
			uint16_t val = htobe16(accessors.getShort(addr));
			uint16_t* valPtr = (uint16_t*)(resp.get() + 1 + (2 * i));
			*valPtr = val;
		}
		send(frame.func, resp.get(), 1 + dataSize);
	}
	else if(isWriteSingle(frame.func)) {
		uint16_t bigEndianWriteValue = *((uint16_t*)frame.writeBuf.get());
		uint16_t writeValue = be16toh(bigEndianWriteValue);
		uint16_t resp[2] = { htobe16(frame.startOffset), bigEndianWriteValue };

		if(frame.func == WRITE_SINGLE_COIL) {
			accessors.setBool(1 + frame.startOffset, writeValue != 0);
		}
		else {
			accessors.setShort(40001 + frame.startOffset, writeValue);
		}
		send(frame.func, (uint8_t*)resp, 4);
	}
	else if(frame.func == WRITE_MULTIPLE_COILS) {
		uint16_t resp[2] = { htobe16(frame.startOffset), htobe16(frame.numAddrs) };

		for(int i = 0; i < frame.numAddrs; i++) {
			uint16_t addr = 1 + frame.startOffset + i;
			bool val = frame.writeBuf[i / 8] & (1 << (i % 8));
			accessors.setBool(addr, val);
		}
		send(frame.func, (uint8_t*)resp, 4);
	}
	else if(frame.func == WRITE_MULTIPLE_HOLDING_REGISTERS) {
		uint16_t resp[2] = { htobe16(frame.startOffset), htobe16(frame.numAddrs) };
		for(int i = 0; i < frame.numAddrs; i++) {
			uint16_t addr = 40001 + frame.startOffset + i;
			uint16_t *valPtr = (uint16_t *)(frame.writeBuf.get() + 2 * i);
			uint16_t val = be16toh(*valPtr);
			accessors.setShort(addr, val);
		}
		send(frame.func, (uint8_t*)resp, 4);
	}
	else {
		respWithException(ILLEGAL_FUNC);
	}
}

void Responder::respWithException(ModbusException exception) {
	uint8_t resp = (uint8_t)exception;
	send(0x80 + frame.func, &resp, 1);
}

bool modbus::defaultBoolGetter(uint16_t addr) { return false; }
uint16_t modbus::defaultShortGetter(uint16_t addr) { return 0; }
void modbus::defaultBoolSetter(uint16_t a, bool b) {}
void modbus::defaultShortSetter(uint16_t a, uint16_t s) {}