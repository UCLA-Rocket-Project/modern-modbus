#include <iostream>
#include <ModbusTCPServer.h>
#include <map>
#include <cstdint>

using namespace std;
using namespace modbus;

/**
 * Modbus TCP Example
 * @author saltyJeff
 *
 * Creates an example modbus TCP server.
 */

map<uint16_t, uint16_t> valueMap;


/*
 * The Modbus server (slave) interacts with your code through the use of callbacks.
 *
 * For example, getShort(addr) will be called to return the value of the register at addr, where addr
 * will be 30001-39999 for a input register and 40001-49999 for a holding register
 *
 * Likewise, setShort(addr, value) will be called to set the value of the register at addr to value,
 * where addr will be between 40001-49999
 *
 * There are also getBool/setBool to handle coils and discrete inputs
 */

// example getShorts just return the value of their addresses if the addr is not yet set,
// else it returns the set value
uint16_t getShort(uint16_t addr) {
	if(valueMap.count(addr) > 0) {
		return valueMap[addr];
	}
	return addr;
}
void setShort(uint16_t addr, uint16_t value) {
	valueMap[addr] = value;
}

/*
 * Accessors is just a quick way to bundle together get/set Bool and get/set Short
 * The default setter is a no-op, and the default getter returns 0/false
 */
Accessors accs { .getShort = getShort, .setShort = setShort };

/* This pattern is inspired by the express/hapi web servers in NodeJS.
 *
 * We specify a callback for the server to call with a Response object once a complete command frame is parsed.
 * Through the response object we can send a response back to the modbus master.
 *
 * There are different ways to respond, here we just respond with an accessors object.
 * The library will automagically fill out a proper modbus response and send it to the client
 * by calling the getters/setters within
 * the accessor until it has all the values it needs.
 *
 * If you need a custom response (or don't trust my code :( ) you can analyze the modbus command
 * frame received from the server using res.frame (here we use it to just print out the details
 * of an incoming command). Then you can respond by using res.respRaw
 */
void mbCallback(Responder res) {
	cout << res.frame.str() << endl;
	res.respWithAccessors(accs);
};

int main (int argc, char *argv[]) {
	// to initialize the server, provide a port and a modbus callback.
	// RAII means the server will automatically close its internal ports on deallocation
	ModbusTCPServer server(5020, mbCallback);
	// to be compatible with what ever event loop you prefer, the server only checks its epoll when you call server.tick()
	while(true) {
		server.tick();
	}
	return 0;
}