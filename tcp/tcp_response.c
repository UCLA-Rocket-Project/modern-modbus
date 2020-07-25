#include <modern_modbus_tcp.h>
modbus_result_t tmpResult;
modbus_result_t modbus_tcp_write_response(modbus_tcp_frame_t *frame, uint8_t *read_buf, uint16_t read_len, uint8_t *buf, uint16_t *len) {
	if(*len < 10) {
		return MB_RESULT_INSUFFICIENT_LEN;
	}
	*len -= 7;
	tmpResult = modbus_write_response(&frame->command, read_buf, read_len, buf+7, len);
	if(tmpResult != MB_RESULT_OK) {
		return tmpResult;
	}

	modbus_util_write_word(frame->transaction_id, buf);
	modbus_util_write_word(frame->protocol_id, buf+2);
	modbus_util_write_word(*len+1, buf+4);
	buf[6] = frame->unit;

	*len += 7;
	return MB_RESULT_OK;
}
modbus_result_t modbus_tcp_write_response_err(modbus_tcp_frame_t *frame, modbus_exception_code_t ex, uint8_t *buf) {
	modbus_util_write_word(frame->transaction_id, buf);
	modbus_util_write_word(frame->protocol_id, buf+2);
	modbus_util_write_word(3, buf+4);
	buf[6] = frame->unit;
	buf[7] = frame->command.func_code + 0x80;
	buf[8] = ex;
}