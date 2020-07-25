#include <modern_modbus_tcp.h>
modbus_result_t modbus_tcp_parse_err;

void modbus_tcp_parser_init(modbus_tcp_parser_t *parser) {
	parser->state = MB_TCP_AWAIT_TRANSACTION_ID;
	parser->first_byte_word = true;
#ifndef MODBUS_STATIC_BUFFER
	parser->buf_len = 0;
	parser->cmd_buf = NULL;
#endif
}
void modbus_tcp_parser_free(modbus_tcp_parser_t *parser) {
	parser->state = MB_TCP_AWAIT_TRANSACTION_ID;
	parser->first_byte_word = true;
#ifndef MODBUS_STATIC_BUFFER
	if(parser->cmd_buf != NULL && parser->buf_len != 0) {
		free(parser->cmd_buf);
	}
	parser->cmd_buf = NULL;
	parser->buf_len = 0;
#endif
}

modbus_tcp_parser_result_t modbus_tcp_parser_read(uint8_t c, modbus_tcp_parser_t *parser) {
	if(parser->state == MB_TCP_AWAIT_TRANSACTION_ID) {
		if(parser->first_byte_word) {
			parser->word_bytes[0] = c;
		}
		else {
			parser->word_bytes[1] = c;
			parser->frame.transaction_id = be16toh(parser->word_buf);
			parser->state = MB_TCP_AWAIT_PROTOCOL_ID;
		}
		parser->first_byte_word = !parser->first_byte_word;
	}
	else if(parser->state == MB_TCP_AWAIT_PROTOCOL_ID) {
		if(parser->first_byte_word) {
			parser->word_bytes[0] = c;
		}
		else {
			parser->word_bytes[1] = c;
			parser->frame.protocol_id = be16toh(parser->word_buf);
			if(parser->frame.protocol_id != 0) {
				return MB_TCP_RESULT_INVALID;
			}
			parser->state = MB_TCP_AWAIT_LENGTH;
		}
		parser->first_byte_word = !parser->first_byte_word;
	}
	else if(parser->state == MB_TCP_AWAIT_LENGTH) {
		if(parser->first_byte_word) {
			parser->word_bytes[0] = c;
		}
		else {
			parser->word_bytes[1] = c;
			parser->frame.remaining_len = be16toh(parser->word_buf);
			if(parser->frame.remaining_len < 6) {
				return MB_TCP_RESULT_INVALID;
			}
#ifdef MODBUS_STATIC_BUFFER
			if(parser->frame.total_length > MODBUS_STATIC_BUFFER_SIZE-1) {
				return MB_TCP_RESULT_INSUFFICIENT_LEN;
			}
#endif
			parser->state = MB_TCP_AWAIT_UNIT;
		}
		parser->first_byte_word = !parser->first_byte_word;
	}
	else if(parser->state == MB_TCP_AWAIT_UNIT) {
		parser->frame.unit = c;
		parser->state = MB_TCP_AWAIT_CMD;
		// prepare space to read data in
#ifndef MODBUS_STATIC_BUFFER
		if(parser->cmd_buf == NULL || parser->buf_len == 0) {
			parser->cmd_buf = calloc(parser->frame.remaining_len - 1, 1);
			parser->buf_len = parser->frame.remaining_len - 1;
		}
		else if(parser->buf_len < parser->frame.remaining_len - 1) {
			parser->cmd_buf = realloc(parser->cmd_buf, parser->frame.remaining_len - 1);
		}
		if(parser->cmd_buf == NULL) {
			return MB_TCP_RESULT_INSUFFICIENT_LEN;
		}
		parser->write_idx = 0;
#endif
	}
	else if(parser->state == MB_TCP_AWAIT_CMD) {
		parser->cmd_buf[parser->write_idx++] = c;
		if(parser->write_idx >= parser->frame.remaining_len - 1) {
			parser->state = MB_TCP_AWAIT_TRANSACTION_ID;
			modbus_result_t res = modbus_parse_command(parser->cmd_buf, parser->buf_len, &parser->frame.command);
			if(res != MB_RESULT_OK) {
				modbus_tcp_parse_err = res;
				return MB_TCP_RESULT_ERR;
			}
			else {
				return MB_TCP_RESULT_OK;
			}
		}
	}
	return MB_TCP_RESULT_INCOMPLETE;
}