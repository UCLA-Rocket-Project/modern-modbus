#include "Socket.h"
#include "mb_exceptions.h"
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <spdlog/spdlog.h>

using namespace std;
using namespace modbus::tcp;

struct timeval sockTimeout = {.tv_sec = 1, .tv_usec = 0};

void hexDump(const char *desc, void *addr, int len);

Socket::Socket(int sockFd, shared_ptr<Accessors> accs): sockFd(sockFd), accs(std::move(accs)) {
	int flags = checkOSErr(fcntl(sockFd, F_GETFL, 0), "Get FCNTL flags: ");
	flags |= O_NONBLOCK;
	checkOSErr(fcntl(sockFd, F_SETFL, flags), "Set socket to nonblocking: ");
	checkOSErr(setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, &sockTimeout, sizeof(sockTimeout)), "Set socket timeout: ");
	modbus_tcp_parser_init(&parser);
}
Socket::~Socket() {
	modbus_tcp_parser_free(&parser);
	close(sockFd);
}

void Socket::drainSocket() {
	uint8_t buf[64] = { 0 };
	int n;
	while( (n = read(sockFd, buf, sizeof(buf))) > 0) {
		for(int i = 0; i < n; i++) {
			try {
				readByte(buf[i]);
			}
			catch(runtime_error &e) {
				zombie = true;
				throw e;
			}
		}
	}
	if(n == 0) {
		// client has closed connection on their side, so we close it on our side
		zombie = true;
		throw std::runtime_error("Client closed connection");
	}
	else if(n < 0 && errno != EAGAIN) {
		zombie = true;
		throw OSErr((string &) "Reading socket: ");
	}
}

void Socket::readByte(uint8_t c) {
	uint8_t writeBuf[512];
	if(zombie) {
		return;
	}
	modbus_tcp_parser_result_t res = checkModbusTCPParserErr(modbus_tcp_parser_read(c, &parser), "Reading parser byte");
	if(res == MB_TCP_RESULT_INCOMPLETE) {
		return;
	}
	modbus_command_dump_str(&parser.frame.command, (char*)writeBuf, 256);
	spdlog::debug("Incoming command: {}", writeBuf);

	uint16_t writeLen = sizeof(writeBuf);
	try {
		checkModbusCmdErr(modbus_check_consistency(&parser.frame.command), "Checking consistency");
		uint16_t readLen;
		unique_ptr<uint8_t[]> readBuf = fillResponse(&parser.frame.command, &readLen, accs);
		checkModbusCmdErr(modbus_tcp_write_response(&parser.frame, readBuf.get(), readLen, writeBuf, &writeLen), "Writing response");
	}
	catch(ModbusResponseErr &e) {
		// we got a fault from an accessor, let's report it to the user
		cerr << "Received a fault from an accessor: " << modbus_exception_code_str(e.ex) << endl;
		checkModbusCmdErr(modbus_tcp_write_response_err(&parser.frame, e.ex, writeBuf), "Writing triggered error: ");
		writeLen = MODBUS_TCP_RESPONSE_ERR_LEN;

	}
	catch(ModbusCmdErr &e) {
		// we got an inconsistent frame, let's complain about it to the user
		cerr << "Received a bad frame from the client" << modbus_result_str(e.res) << endl;
		checkModbusCmdErr(modbus_tcp_write_response_err(&parser.frame, MB_EX_NEGATIVE_ACKNOWLEDGE, writeBuf), "Writing bad frame error: ");
		writeLen = MODBUS_TCP_RESPONSE_ERR_LEN;
	}
	catch(runtime_error &e) {
		zombie = true;
		throw e;
	}
	// whatever we want should now be in writeBuf with writeLen set
	// toggle the socket into blocking mode so that the write blocks (otherwise we need a whole loop)
	int flags = checkOSErr(fcntl(sockFd, F_GETFL, 0), "Get FCNTL flags: ");
	flags &= ~O_NONBLOCK;
	checkOSErr(fcntl(sockFd, F_SETFL, flags), "Set socket to blocking: ");

	// NOTE: keeping this here in case more things go wrong
	//hexDump("Writing", writeBuf, writeLen);
	checkOSErr(send(sockFd, writeBuf, writeLen, 0), "Sending message: ");

	flags |= O_NONBLOCK;
	checkOSErr(fcntl(sockFd, F_SETFL, flags), "Set socket to nonblocking: ");

}
bool Socket::shouldDie() {
	return zombie;
}

// taken from: https://gist.github.com/domnikl/af00cc154e3da1c5d965
void hexDump(const char *desc, void *addr, int len) {
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char *) addr;

	// Output description if given.
	if (desc != NULL)
		printf("%s:\n", desc);

	// Process every byte in the data.
	for (i = 0; i < len; i++) {
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0) {
			// Just don't print ASCII for the zeroth line.
			if (i != 0)
				printf("  %s\n", buff);

			// Output the offset.
			printf("  %04x ", i);
		}

		// Now the hex code for the specific character.
		printf(" %02x", pc[i]);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
			buff[i % 16] = '.';
		} else {
			buff[i % 16] = pc[i];
		}

		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}

	// And print the final ASCII bit.
	printf("  %s\n", buff);
}