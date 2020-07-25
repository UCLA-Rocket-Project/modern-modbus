#ifndef MODBUS_NO_STRINGIFY
#include <modern_modbus_core.h>
const char* modbus_func_str(modbus_func_code_t code) {
	switch(code) {
	case MB_FUNC_READ_COILS:
		return "READ COILS";
	case MB_FUNC_READ_DISCRETE_INPUTS:
		return "READ DISCRETE INPUTS";
	case MB_FUNC_READ_INPUT_REGISTERS:
		return "READ INPUT REGISTERS";
	case MB_FUNC_READ_MULTIPLE_HOLDING_REGISTERS:
		return "READ HOLDING REGISTERS";
	case MB_FUNC_WRITE_SINGLE_COIL:
		return "WRITE SINGLE COIL";
	case MB_FUNC_WRITE_SINGLE_HOLDING_REGISTER:
		return "WRITE SINGLE HOLDING REGISTER";
	case MB_FUNC_WRITE_MULTIPLE_COILS:
		return "WRITE MULTIPLE COILS";
	case MB_FUNC_WRITE_MULTIPLE_HOLDING_REGISTERS:
		return "WRITE MULTIPLE HOLDING REGISTERS";
	default:
		return "WTF?? FUNC CODE";
	}
}
const char* modbus_result_str(modbus_result_t code) {
	switch(code) {
	case MB_RESULT_OK:
		return "OK";
	case MB_RESULT_ERR:
		return "ERR";
	case MB_RESULT_INCORRECT_NUM_ADDR:
		return "INCORRECT NUMBER OF ADDRESSES";
	case MB_RESULT_INCORRECT_NUM_BYTES:
		return "INCORRECT NUMBER OF BYTES";
	case MB_RESULT_INSUFFICIENT_LEN:
		return "INSUFFICIENT BUFFER LENGTH";
	case MB_RESULT_TOO_MANY_ADDR:
		return "TOO MANY ADDRESSES REQUESTED";
	case MB_RESULT_UNKNOWN_FUNC:
		return "UNKNOWN FUNCTION CODE";
	default:
		return "WTF?? MODBUS PARSER ERR CODE";
	}
}
const char* modbus_exception_code_str(modbus_exception_code_t ex) {
	switch(ex) {
	case MB_EX_NEGATIVE_ACKNOWLEDGE:
		return "NACK";
	case MB_EX_ACKNOWLEDGE:
		return "ACK-BUSY";
	case MB_EX_ILLEGAL_DATA_ADDR:
		return "ILLEGAL ADDR";
	case MB_EX_ILLEGAL_DATA_VALUE:
		return "ILLEGAL DATA";
	case MB_EX_ILLEGAL_FUNC:
		return "ILLEGAL FUNC";
	case MB_EX_SLAVE_DEVICE_BUSY:
		return "SLAVE BUSY";
	case MB_EX_SLAVE_DEVICE_FAILURE:
		return "SLAVE FAILURE";
	default:
		return "WTF?? MODBUS EXCEPTION";
	}
}
#include <stdio.h>
const char *modbus_cmd_fmt = "CMD: %s, START OFFSET: %d, NUM ADDR: %d, NUM WRITE BYTES: %d, WRITE DATA PTR: %p";
void modbus_command_dump_str(modbus_command_t *cmd, char *buf, int len) {
	snprintf(buf, len, modbus_cmd_fmt, modbus_func_str(cmd->func_code), cmd->start_addr, cmd->num_addr, cmd->num_write_bytes, cmd->write_value_bytes);
}
#endif