#include <modern_modbus_core.h>

modbus_result_t modbus_check_consistency(modbus_command_t *cmd) {
	if(!modbus_func_is_write_one(cmd->func_code)) {
		// make sure addresses are in bounds
		if(cmd->start_addr > 9999 - cmd->num_addr) {
			return MB_RESULT_TOO_MANY_ADDR;
		}
	}
	if(modbus_func_is_read(cmd->func_code)) {
		if(cmd->num_write_bytes != 0) {
			return MB_RESULT_INCORRECT_NUM_BYTES;
		}
	}
	else if(modbus_func_is_write_one(cmd->func_code)) {
		if(cmd->num_addr != 1) {
			return MB_RESULT_INCORRECT_NUM_ADDR;
		}
		else if(cmd->num_write_bytes != (cmd->func_code == MB_FUNC_WRITE_SINGLE_COIL ? 1 : 2)) {
			return MB_RESULT_INCORRECT_NUM_BYTES;
		}
	}
	else if(cmd->func_code == MB_FUNC_WRITE_MULTIPLE_HOLDING_REGISTERS) {
		if(cmd->num_addr > 255 / 2) {
			return MB_RESULT_TOO_MANY_ADDR;
		}
		else if(cmd->num_write_bytes % 2 != 0 || cmd->num_addr != cmd->num_write_bytes / 2 ) {
			return MB_RESULT_INCORRECT_NUM_BYTES;
		}
	}
	else if(cmd->func_code == MB_FUNC_WRITE_MULTIPLE_COILS) {
		if(cmd->num_addr > 255 * 8) {
			return MB_RESULT_TOO_MANY_ADDR;
		}
		else if(cmd->num_write_bytes != (cmd->num_addr + 7) / 8) {
			return MB_RESULT_INCORRECT_NUM_BYTES;
		}
	}
	else {
		return MB_RESULT_UNKNOWN_FUNC;
	}
	if( (cmd->start_addr + cmd->num_addr) < cmd->start_addr) { // check for overflow
		return MB_RESULT_TOO_MANY_ADDR;
	}
	return MB_RESULT_OK;
}