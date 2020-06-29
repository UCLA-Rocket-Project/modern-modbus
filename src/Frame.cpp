#include "Frame.h"
#include "FrameReader.h"
#include <endian.h>
#include <sstream>

using namespace std;
using namespace modbus;

const char* modbus::functionCodeToStr(FunctionCode code) {
	switch(code) {
	case UNDEFINED:
		return "UNDEFINED (how did you get this?)";
	case READ_DISCRETE_INPUTS:
		return "READ DISCRETE INPUTS";
	case READ_COILS:
		return "READ COILS";
	case WRITE_SINGLE_COIL:
		return "WRITE SINGLE COIL";
	case WRITE_MULTIPLE_COILS:
		return "WRITE MULTIPLE COILS";
	case READ_INPUT_REGISTERS:
		return "READ INPUT REGISTERS";
	case READ_MULTIPLE_HOLDING_REGISTERS:
		return "READ MULTIPLE HOLDING REGISTERS";
	case WRITE_SINGLE_HOLDING_REGISTER:
		return "WRITE SINGLE HOLDING REGISTER";
	case WRITE_MULTIPLE_HOLDING_REGISTERS:
		break;
	default:
		return "WTF??";
	}
}

string Frame::str() {
	stringstream ret;
	ret << functionCodeToStr(func) << " [ " << startAddr() << ":" << (startAddr() + numAddrs - 1) << " ]";
	return ret.str();
}