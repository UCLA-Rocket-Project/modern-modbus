#include <modern_modbus_core.h>

uint8_t responseWriteIdx;
modbus_result_t modbus_write_response(modbus_command_t *cmd, uint8_t *read_data, uint16_t read_len, uint8_t *data, uint16_t *len) {
	if(*len < 5) {
		return MB_RESULT_INSUFFICIENT_LEN;
	}
	data[0] = cmd->func_code;
	if(cmd->func_code == MB_FUNC_READ_DISCRETE_INPUTS || cmd->func_code == MB_FUNC_READ_COILS) {
		if(cmd->num_addr > 255 * 8) {
			return MB_RESULT_TOO_MANY_ADDR;
		}
		data[1] = (cmd->num_addr + 7) / 8;
		*len = 2 + data[1];
		if(data[1] > read_len) {
			return MB_RESULT_INSUFFICIENT_LEN;
		}
		data = data + 2;
		for(responseWriteIdx = 0; responseWriteIdx < (cmd->num_addr + 7) / 8; responseWriteIdx++) {
			data[responseWriteIdx] = read_data[responseWriteIdx];
		}
	}
	else if(cmd->func_code == MB_FUNC_READ_INPUT_REGISTERS || cmd->func_code == MB_FUNC_READ_MULTIPLE_HOLDING_REGISTERS) {
		if(cmd->num_addr > 255 / 2) {
			return MB_RESULT_TOO_MANY_ADDR;
		}
		data[1] = cmd->num_addr * 2;
		*len = 2 + data[1];
		if(data[1] > read_len) {
			return MB_RESULT_INSUFFICIENT_LEN;
		}
		data = data + 2;
		for(responseWriteIdx = 0; responseWriteIdx < cmd->num_addr * 2; responseWriteIdx+=2) {
			modbus_util_write_word(*(uint16_t*)(read_data + responseWriteIdx), data+responseWriteIdx);
		}
	}
	else if(modbus_func_is_write_one(cmd->func_code)) {
		// response is the same as the command
		modbus_util_write_word(cmd->start_addr, data+1);
		modbus_util_write_word(cmd->write_value_words[0], data+3);
		*len = 5;
	}
	else if(modbus_func_is_write_multiple(cmd->func_code)) {
		modbus_util_write_word(cmd->start_addr, data+1);
		modbus_util_write_word(cmd->num_addr, data+3);
		*len = 5;
	}
	else {
		return MB_RESULT_UNKNOWN_FUNC;
	}
	return MB_RESULT_OK;
}

modbus_result_t modbus_write_response_err(modbus_command_t *cmd, modbus_exception_code_t err, uint8_t *data) {
	data[0] = cmd->func_code + 0x80;
	data[1] = err;
}