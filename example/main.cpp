#include <iostream>
#include <modern_modbus_tcp_server.h>
#include <map>
#include <spdlog/spdlog.h>

/** Modern Modbus Example Server.
 * @author saltyJeff
 */

using namespace std;
using namespace modbus::tcp;

// we will be using this map to hold the values of registers
map<MB_WORD, MB_WORD> regs;

// the getters will return the written value if it exists in "regs", else it will just return some random default
bool getBool(MB_WORD addr) {
	return regs.find(addr) == regs.end() ? (addr % 2 != 0) : (regs[addr] != 0);
}
MB_WORD getWord(MB_WORD addr) {
	return regs.find(addr) == regs.end() ? addr % 2 : regs[addr];
}
// the setters will write their values to "regs"
void setBool(MB_WORD addr, bool val) {
	regs[addr] = val;
}
void setWord(MB_WORD addr, MB_WORD val) {
	regs[addr] = val;
}

int main (int argc, char *argv[]) {
	int port = 5020; // standard port is 502 but it requires sudo permissions
	spdlog::default_logger()->set_level(spdlog::level::debug);

	if(argc > 1) {
		port = atoi(argv[1]);
	}
	if(port == 0) {
		throw runtime_error("Port argument invalid! must be a number > 0");
	}
	// you will have to create an accessors struct and populate the following 4 fields with methods.
	Accessors acc;
	acc.getBool = getBool;
	acc.setBool = setBool;
	acc.getWord = getWord;
	acc.setWord = setWord;

	TCPServer server(port, shared_ptr<Accessors>(&acc));
	cout << "Server started PORT: " << port << endl;
	while(true) {
		server.tick();
	}
	return 0;
}