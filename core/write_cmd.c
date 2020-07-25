#include <modern_modbus_core.h>
uint8_t cmdWriteIdx;

modbus_result_t modbus_write_command(modbus_command_t *cmd, uint8_t *data, uint16_t *len) {
	if (*len < 5) {
		return MB_RESULT_INSUFFICIENT_LEN;
	}
	modbus_util_write_word(cmd->func_code, data);
	modbus_util_write_word(cmd->start_addr, data+1);
	if (modbus_func_is_read(cmd->func_code)) {
		modbus_util_write_word(cmd->num_addr, data+3);
		*len = 5;
	}
	else if (modbus_func_is_write_one(cmd->func_code)) {
		modbus_util_write_word(cmd->write_value_words[0], data + 3);
		*len = 5;
	}
	else if(modbus_func_is_write_multiple(cmd->func_code)) {
		if(*len < 6 || cmd->num_write_bytes > *len - 6) {
			return MB_RESULT_INSUFFICIENT_LEN;
		}
		if(cmd->func_code == MB_FUNC_WRITE_MULTIPLE_HOLDING_REGISTERS) {
			modbus_util_write_word(cmd->num_addr, data+3);
			data[5] = cmd->num_write_bytes;
			data = data+6;
			for(cmdWriteIdx = 0; cmdWriteIdx < cmd->num_addr; cmdWriteIdx++) {
				modbus_util_write_word(cmd->write_value_words[cmdWriteIdx], data + 2 * cmdWriteIdx);
			}
			*len = 6 + cmd->num_write_bytes;
		}
		else {
			modbus_util_write_word(cmd->num_addr, data+3);
			data[5] = cmd->num_write_bytes;
			data = data+6;
			for(cmdWriteIdx = 0; cmdWriteIdx < cmd->num_write_bytes; cmdWriteIdx++) {
				data[cmdWriteIdx] = cmd->write_value_bytes[cmdWriteIdx];
			}
			*len = 6 + cmd->num_write_bytes;
		}
	}
	else {
		return MB_RESULT_UNKNOWN_FUNC;
	}
	return MB_RESULT_OK;
}