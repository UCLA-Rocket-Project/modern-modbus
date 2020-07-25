#ifndef MODERN_MODBUS_TCP_H
#define MODERN_MODBUS_TCP_H
#include <modern_modbus_core.h>

#ifndef MODBUS_STATIC_BUFFER
#include <stdlib.h>
#endif

#pragma pack(push, 1)
typedef struct {
	MB_WORD transaction_id;
	MB_WORD protocol_id;
	MB_WORD remaining_len;
	uint8_t unit;
	modbus_command_t command;
} modbus_tcp_frame_t;
#pragma pack(pop)

typedef enum {
	MB_TCP_AWAIT_TRANSACTION_ID,
	MB_TCP_AWAIT_PROTOCOL_ID,
	MB_TCP_AWAIT_LENGTH,
	MB_TCP_AWAIT_UNIT,
	MB_TCP_AWAIT_CMD
} modbus_tcp_parse_state_t;

typedef struct {
	modbus_tcp_frame_t frame;
	modbus_tcp_parse_state_t state;
	MB_WORD write_idx;
	union {
		MB_WORD word_buf;
		uint8_t word_bytes[2];
	};
	bool first_byte_word;
#ifdef MODBUS_STATIC_BUFFER
#ifndef MODBUS_STATIC_BUFFER_SIZE
#define MODBUS_STATIC_BUFFER_SIZE 512
#endif
	uint8_t cmd_buf[MODBUS_STATIC_BUFFER_SIZE];
#else
	MB_WORD buf_len;
	uint8_t *cmd_buf;
#endif
} modbus_tcp_parser_t;

void modbus_tcp_parser_init(modbus_tcp_parser_t *parser);
void modbus_tcp_parser_free(modbus_tcp_parser_t *parser);

typedef enum {
	MB_TCP_RESULT_OK,
	MB_TCP_RESULT_INCOMPLETE,
	MB_TCP_RESULT_ERR,
	MB_TCP_RESULT_INVALID,
	MB_TCP_RESULT_INSUFFICIENT_LEN
} modbus_tcp_parser_result_t;

extern modbus_result_t modbus_tcp_parse_err;

modbus_tcp_parser_result_t modbus_tcp_parser_read(uint8_t c, modbus_tcp_parser_t *parser);

#ifndef MODBUS_NO_STRINGIFY
const char* modbus_tcp_result_str(modbus_tcp_parser_result_t code);
#endif

modbus_result_t modbus_tcp_write_response(modbus_tcp_frame_t *frame, uint8_t *read_buf, uint16_t read_len, uint8_t *buf, uint16_t *len);
modbus_result_t modbus_tcp_write_response_err(modbus_tcp_frame_t *frame, modbus_exception_code_t ex, uint8_t *buf);

#define MODBUS_TCP_RESPONSE_ERR_LEN 9;

#endif