#include "ModbusError.h"
#include <iostream>

using namespace std;
using namespace modbus;
ProtocolErrReporter modbus::protocolErrReporter = [](ModbusError& err) {
	cerr << "Modbus protocol error: " << err.what() << '\n';
};
OSErrReporter modbus::osErrReporter = [](runtime_error& err) {
	cerr << "Modbus server OS error: " << err.what() << '\n';
};

void modbus::setProtocolErrReporter(ProtocolErrReporter rep) {
	protocolErrReporter = move(rep);
}
void modbus::setOSErrReporter(OSErrReporter rep) {
	osErrReporter = move(rep);
}