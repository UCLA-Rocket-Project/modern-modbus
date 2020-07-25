#include <modern_modbus_core.h>
MB_WORD modbus_util_word_buf;
MB_WORD modbus_util_bytes_to_word(uint8_t* bytePtr) {
	uint16_t *ptr = (uint16_t *)bytePtr;
	return be16toh(*ptr);
}
void modbus_util_bytes_to_word_inplace(uint8_t* bytePtr) {
	uint16_t *ptr = (uint16_t *)bytePtr;
	modbus_util_word_buf = be16toh(*ptr);
	*ptr = modbus_util_word_buf;
}
void modbus_util_write_word(MB_WORD word, uint8_t* bytePtr) {
	uint16_t *wordPtr = bytePtr;
	*wordPtr = htobe16(word);
}

union {
	float one_float;
	uint32_t one_long;
	MB_WORD two_words[2];
} double_word;

MB_WORD modbus_util_most_sig_half_long(uint32_t l) {
	double_word.one_long = htobe32(l);
	return double_word.two_words[0];
}
MB_WORD modbus_util_least_sig_half_long(uint32_t l) {
	double_word.one_long = htobe32(l);
	return double_word.two_words[1];
}
MB_WORD modbus_util_most_sig_half_float(float f) {
	double_word.one_float = f;
	double_word.one_long = htobe32(double_word.one_long);
	return double_word.two_words[0];
}
MB_WORD modbus_util_least_sig_half_float(float f) {
	double_word.one_float = f;
	double_word.one_long = htobe32(double_word.one_long);
	return double_word.two_words[1];
}
modbus_result_t modbus_util_read_bytes_needed(modbus_command_t *cmd, uint8_t *len) {
	if(cmd->func_code == MB_FUNC_READ_MULTIPLE_HOLDING_REGISTERS || cmd->func_code == MB_FUNC_READ_INPUT_REGISTERS) {
		if(cmd->num_addr > 255 * 8) {
			return MB_RESULT_TOO_MANY_ADDR;
		}
		*len = cmd->num_addr * 2;
	}
	else if(cmd->func_code == MB_FUNC_READ_COILS || cmd->func_code == MB_FUNC_READ_DISCRETE_INPUTS) {
		if(cmd->num_addr > 255 / 2) {
			return MB_RESULT_TOO_MANY_ADDR;
		}
		*len = (cmd->num_addr + 7) / 8;
	}
	else {
		*len = 0;
	}
	return MB_RESULT_OK;
}