#pragma once
extern "C" {
	#include <modern_modbus_core.h>
	#include <modern_modbus_tcp.h>
}
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <string>

namespace modbus { namespace tcp {
	class ModbusCmdErr: public std::runtime_error {
	public:
		modbus_result_t res;
		ModbusCmdErr(modbus_result_t res, std::string &detail): std::runtime_error(detail+modbus_result_str(res)), res(res) {};
	};
	class ModbusTCPParserErr: public std::runtime_error {
	public:
		modbus_tcp_parser_result_t res;
		ModbusTCPParserErr(modbus_tcp_parser_result_t res, std::string &detail): std::runtime_error(detail+modbus_tcp_result_str(res)), res(res) {};
	};
	class ModbusResponseErr: public std::runtime_error {
	public:
		modbus_exception_code_t ex;
		ModbusResponseErr(modbus_exception_code_t ex, std::string &detail): std::runtime_error(detail+modbus_exception_code_str(ex)), ex(ex) {};
	};
	class OSErr: public std::runtime_error {
	public:
		OSErr(std::string &detail): std::runtime_error(detail+strerror(errno)) {};
	};

	inline modbus_result_t checkModbusCmdErr(modbus_result_t res, std::string detail="") {
		if(res != MB_RESULT_OK) {
			throw ModbusCmdErr(res, detail);
		}
		return res;
	}
	inline modbus_tcp_parser_result_t checkModbusTCPParserErr(modbus_tcp_parser_result_t res, std::string detail="") {
		if(res != MB_TCP_RESULT_OK && res != MB_TCP_RESULT_INCOMPLETE) {
			if(res == MB_TCP_RESULT_ERR) {
				std::string newDetail = detail+" (nested from TCP parser)";
				throw ModbusCmdErr(modbus_tcp_parse_err, newDetail);
			}
			throw ModbusTCPParserErr(res, detail);
		}
		return res;
	}
	inline int checkOSErr(int rc, std::string detail="") {
		if(rc < 0) {
			throw OSErr(detail);
		}
		return rc;
	}
}}