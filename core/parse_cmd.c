#include <modern_modbus_core.h>

// will only complain on insufficient buffers (ignores extra long ones)
// will read in bytes as told, be sure to check for internal consistency!

modbus_result_t modbus_parse_command(uint8_t *data, uint16_t len, modbus_command_t *cmd) {
	// even the smallest frame must be at least 5 bytes long
	if(len < 5) {
		return MB_RESULT_INSUFFICIENT_LEN;
	}
	cmd->func_code = data[0];
	cmd->start_addr = modbus_util_bytes_to_word(data+1);
	if(modbus_func_is_read(cmd->func_code)) {
		cmd->num_addr = modbus_util_bytes_to_word(data+3);
		cmd->num_write_bytes = 0;
	}
	else if(modbus_func_is_write_one(cmd->func_code)) {
		cmd->num_addr = 1;
		cmd->num_write_bytes = cmd->func_code == MB_FUNC_WRITE_SINGLE_COIL ? 1 : 2;
		cmd->write_value_words = data + 3;
		modbus_util_bytes_to_word_inplace(data+3);
	}
	else if(modbus_func_is_write_multiple(cmd->func_code)) {
		if(len < 6) {
			return MB_RESULT_INSUFFICIENT_LEN;
		}
		cmd->num_addr = modbus_util_bytes_to_word(data+3);
		cmd->num_write_bytes = data[5]; // the number of bytes remaining in frame
		if(cmd->num_write_bytes > len - 6) {
			return MB_RESULT_INSUFFICIENT_LEN;
		}
		if(cmd->func_code == MB_FUNC_WRITE_MULTIPLE_HOLDING_REGISTERS) {
			cmd->write_value_bytes = data + 6;
			for(data = cmd->write_value_bytes;
				data < cmd->write_value_bytes + cmd->num_write_bytes;
				data += 2) {

				modbus_util_bytes_to_word_inplace(data);
			}
		}
		else {
			cmd->write_value_bytes = data + 6;
		}
	}
	else {
		return MB_RESULT_UNKNOWN_FUNC;
	}
	return MB_RESULT_OK;
}