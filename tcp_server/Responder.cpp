#include "Responder.h"
#include <memory>
#include <endian.h>
#include "mb_exceptions.h"
#include <iostream>

using namespace std;
using namespace modbus::tcp;

unique_ptr<uint8_t[]> modbus::tcp::fillResponse(modbus_command_t *cmd, uint16_t *readLen, const std::shared_ptr<Accessors> accs) {
	uint8_t bytesNeeded;
	checkModbusCmdErr(modbus_util_read_bytes_needed(cmd, &bytesNeeded), "Checking bytes needed for read buffer: ");
	*readLen = bytesNeeded;

	if(cmd->func_code == MB_FUNC_READ_COILS || cmd->func_code == MB_FUNC_READ_DISCRETE_INPUTS) {
		MB_WORD offset = cmd->func_code == MB_FUNC_READ_COILS ? MODBUS_OFFSET_COIL : MODBUS_OFFSET_DISCRETE_INPUT;
		unique_ptr<uint8_t[]> resp(new uint8_t[*readLen]());
		for(uint16_t i = 0; i < cmd->num_addr; i++) {
			uint16_t addr = i + cmd->start_addr + offset;

			uint8_t &theByte = resp[i / 8];
			if(accs->getBool(addr)) {
				theByte |= 1 << (i % 8);
			}
			else {
				theByte &= ~(1 << (i % 8));
			}
		}
		return resp;
	}
	else if(cmd->func_code == MB_FUNC_READ_MULTIPLE_HOLDING_REGISTERS || cmd->func_code == MB_FUNC_READ_INPUT_REGISTERS) {
		MB_WORD offset = cmd->func_code == MB_FUNC_READ_MULTIPLE_HOLDING_REGISTERS ? MODBUS_OFFSET_HOLDING_REGISTER : MODBUS_OFFSET_INPUT_REGISTER;
		unique_ptr<uint8_t[]> resp(new uint8_t[*readLen]());
		for(uint16_t i = 0; i < cmd->num_addr; i++) {
			uint16_t addr = i + cmd->start_addr + offset;
			uint16_t val = htobe16(accs->getWord(addr));
			modbus_util_write_word(val, resp.get() + 2 * i);
		}
		return resp;
	}
	else if(cmd->func_code == MB_FUNC_WRITE_SINGLE_COIL) {
		accs->setBool(cmd->start_addr + MODBUS_OFFSET_COIL, !!cmd->write_value_words[0]);
	}
	else if(cmd->func_code == MB_FUNC_WRITE_SINGLE_HOLDING_REGISTER) {
		accs->setWord(cmd->start_addr + MODBUS_OFFSET_HOLDING_REGISTER, cmd->write_value_words[0]);
	}
	else if(cmd->func_code == MB_FUNC_WRITE_MULTIPLE_COILS) {
		for(uint16_t i = 0; i < cmd->num_addr; i++) {
			uint16_t addr = i + cmd->start_addr + MODBUS_OFFSET_COIL;
			bool val = cmd->write_value_bytes[i / 8] & (1 << (i % 8));
			accs->setBool(addr, val);
		}
	}
	else if(cmd->func_code == MB_FUNC_WRITE_MULTIPLE_HOLDING_REGISTERS) {
		for(uint16_t i = 0; i < cmd->num_addr; i++) {
			accs->setWord(i + cmd->start_addr + MODBUS_OFFSET_HOLDING_REGISTER, cmd->write_value_words[i]);
		}
	}
	return nullptr;
}