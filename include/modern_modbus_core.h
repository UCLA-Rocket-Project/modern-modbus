#ifndef MODERN_MODBUS_CORE_H
#define MODERN_MODBUS_CORE_H

#include <stdint.h>
#include <endian.h>
#include <stdbool.h>
#include <assert.h>

typedef uint16_t MB_WORD;

typedef enum modbus_func_code_t {
	MB_FUNC_READ_DISCRETE_INPUTS = 2,
	MB_FUNC_READ_COILS = 1,
	MB_FUNC_WRITE_SINGLE_COIL = 5,
	MB_FUNC_WRITE_MULTIPLE_COILS = 15,
	MB_FUNC_READ_INPUT_REGISTERS = 4,
	MB_FUNC_READ_MULTIPLE_HOLDING_REGISTERS = 3,
	MB_FUNC_WRITE_SINGLE_HOLDING_REGISTER = 6,
	MB_FUNC_WRITE_MULTIPLE_HOLDING_REGISTERS = 16
} __attribute__((packed)) modbus_func_code_t;

static inline bool modbus_func_is_read(modbus_func_code_t func) {
	return func == MB_FUNC_READ_DISCRETE_INPUTS || func == MB_FUNC_READ_MULTIPLE_HOLDING_REGISTERS || func == MB_FUNC_READ_COILS | func == MB_FUNC_READ_INPUT_REGISTERS;
}

static inline bool modbus_func_is_write_one(modbus_func_code_t func) {
	return func == MB_FUNC_WRITE_SINGLE_COIL || func == MB_FUNC_WRITE_SINGLE_HOLDING_REGISTER;
}

static inline bool modbus_func_is_write_multiple(modbus_func_code_t func) {
	return func == MB_FUNC_WRITE_MULTIPLE_COILS || func == MB_FUNC_WRITE_MULTIPLE_HOLDING_REGISTERS;
}

typedef enum {
	MB_EX_ILLEGAL_FUNC = 1,
	MB_EX_ILLEGAL_DATA_ADDR = 2,
	MB_EX_ILLEGAL_DATA_VALUE = 3,
	MB_EX_SLAVE_DEVICE_FAILURE= 4,
	MB_EX_ACKNOWLEDGE = 5,
	MB_EX_SLAVE_DEVICE_BUSY = 6,
	MB_EX_NEGATIVE_ACKNOWLEDGE = 7
} modbus_exception_code_t;

typedef enum {
	MB_RESULT_OK,
	MB_RESULT_ERR,
	MB_RESULT_UNKNOWN_FUNC,
	MB_RESULT_INSUFFICIENT_LEN,
	MB_RESULT_INCORRECT_NUM_ADDR,
	MB_RESULT_INCORRECT_NUM_BYTES,
	MB_RESULT_TOO_MANY_ADDR
} modbus_result_t;

typedef struct {
	modbus_func_code_t func_code;
	MB_WORD start_addr;
	MB_WORD num_addr;
	uint8_t num_write_bytes;
	union {
		MB_WORD *write_value_words;
		uint8_t *write_value_bytes;
	};
} modbus_command_t;

/**
 * Parses a byte array to fill out a modbus_command_t.
 * Lifetime: if the command is a write, it will contain a reference to the data buffer
 * Safety: only checks length arguments to make sure no buffer overflow
 * passed into the argument.
 * @param data (in big endian form) the buffer holding frame data
 * @param len the length of the buffer
 * @param cmd a modbus_command_t to fill out
 * @return a modbus_result_t on whether the parse was successful
 */
modbus_result_t modbus_parse_command(uint8_t *data, uint16_t len, modbus_command_t *cmd);

/**
 * Writes a byte array with the data enclosed in a modbus_command_t.
 * Safety: only checks length arguments to make sure no buffer overflow
 * @param cmd the modbus command to write to the buffer
 * @param data pointer to the buffer to write to (will be written in big-endian form)
 * @param len the length of the buffer, will be overwritten with the true # of bytes written
 * @return a modbus_result_t on whether the write was succesful
 */
modbus_result_t modbus_write_command(modbus_command_t *cmd, uint8_t *data, uint16_t *len);

/**
 * Checks the internal consistency of a modbus command
 * @param cmd the modbus command to check
 * @return consistency findings
 */
modbus_result_t modbus_check_consistency(modbus_command_t *cmd);

#ifndef MODBUS_NO_STRINGIFY
const char* modbus_func_str(modbus_func_code_t code);
const char* modbus_result_str(modbus_result_t code);
const char* modbus_exception_code_str(modbus_exception_code_t ex);
void modbus_command_dump_str(modbus_command_t *cmd, char *buf, int len);
#endif

/**
 * Writes the response message to the buffer defined by data and len
 * @param cmd The command to respond to
 * @param read_data If the command was a read, a pointer to the requested read's data
 * @param read_len If the command was a read, length of the read data's buffer
 * @param data The buffer to write into
 * @param len The length of the buffer to write into
 */
modbus_result_t modbus_write_response(modbus_command_t *cmd, uint8_t *read_data, uint16_t read_len, uint8_t *data, uint16_t *len);

/**
 * Writes the response message for an error to the buffer defined by data and len
 * @param cmd The command to respond to
 * @param err The error code to respond with
 * @param data The buffer to write into. All error responses are 2 bytes, no longer, no less
 * @return
 */
modbus_result_t modbus_write_response_err(modbus_command_t *cmd, modbus_exception_code_t err, uint8_t *data);

/** public utilities **/
MB_WORD modbus_util_most_sig_half_long(uint32_t l);
MB_WORD modbus_util_least_sig_half_long(uint32_t l);
MB_WORD modbus_util_most_sig_half_float(float f);
MB_WORD modbus_util_least_sig_half_float(float f);
modbus_result_t modbus_util_read_bytes_needed(modbus_command_t *cmd, uint8_t *len);


/** private utilities (don't look here) **/
extern MB_WORD modbus_util_word_buf;
MB_WORD modbus_util_bytes_to_word(uint8_t* bytePtr);
void modbus_util_bytes_to_word_inplace(uint8_t* bytePtr);
void modbus_util_write_word(MB_WORD word, uint8_t* bytePtr);

#define MODBUS_OFFSET_COIL 1
#define MODBUS_OFFSET_DISCRETE_INPUT 10001
#define MODBUS_OFFSET_INPUT_REGISTER 30001
#define MODBUS_OFFSET_HOLDING_REGISTER 40001

#endif