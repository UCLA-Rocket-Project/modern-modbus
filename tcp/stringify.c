#ifndef MODBUS_NO_STRINGIFY
#include <modern_modbus_tcp.h>
const char* modbus_tcp_result_str(modbus_tcp_parser_result_t code) {
	switch(code) {
	case MB_TCP_RESULT_INVALID:
		return "INVALID FRAME";
	case MB_TCP_RESULT_INCOMPLETE:
		return "INCOMPLETE";
	case MB_TCP_RESULT_ERR:
		return "UNSPECIFIED ERR (check modbus_tcp_parse_err)";
	case MB_TCP_RESULT_INSUFFICIENT_LEN:
		return "INSUFFICIENT BUFFER LEN";
	case MB_TCP_RESULT_OK:
		return "OK";
	default:
		return "WTF?? TCP PARSER RESULT";
	}
}
#endif