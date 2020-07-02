#include "ModbusTCPSocket.h"
#include <stdexcept>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include "ModbusError.h"

using namespace std;
using namespace modbus;

struct timeval sockTimeout = {.tv_sec = 1, .tv_usec = 0};

ModbusTCPSocket::ModbusTCPSocket(int sockFd, ModbusCallback cb): sockFd(sockFd), cb(std::move(cb)) {
	int flags = fcntl(sockFd, F_GETFL, 0);
	if(flags < 0) {
		throw runtime_error(string("Could get socket FCNTL flags: ") + strerror(errno));
	}
	flags |= O_NONBLOCK;
	if(fcntl(sockFd, F_SETFL, flags) < 0) {
		throw runtime_error(string("Could not set socket into nonblocking: ") + strerror(errno));
	}
	if(setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, &sockTimeout, sizeof(sockTimeout)) < 0) {
		throw runtime_error(string("Could not set socket timeout: ") + strerror(errno));
	}
};
ModbusTCPSocket::~ModbusTCPSocket() {
	close(sockFd);
}

void ModbusTCPSocket::drainSocket() {
	uint8_t buf[64] = { 0 };
	int n;
	while( (n = read(sockFd, buf, sizeof(buf))) > 0) {
		for(int i = 0; i < n; i++) {
			try {
				readByte(buf[i]);
			}
			catch(ModbusError &e) {
				protocolErrReporter(e);
				close(sockFd);
				return;
			}
		}
	}
	if(n == 0) {
		// client has closed connection on their side, so we close it on our side
		close(sockFd);
	}
	else if(n < 0 && errno != EAGAIN) {
		throw ModbusError(string("Failed to read: ") + strerror(errno));
	}
}

void ModbusTCPSocket::readByte(uint8_t c) {
	if(state == ModbusTCPState::AWAITING_TRANSACTION_ID) {
		if(shortFirstByte) {
			shortBuf[0] = c;
		}
		else {
			shortBuf[1] = c;
			transactionId = be16toh(*(uint8_t*)shortBuf);
			state = ModbusTCPState::AWAITING_PROTOCOL_ID;
		}
		shortFirstByte = !shortFirstByte;
	}
	else if(state == ModbusTCPState::AWAITING_PROTOCOL_ID) {
		if(shortFirstByte) {
			shortBuf[0] = c;
		}
		else {
			shortBuf[1] = c;
			if(*(uint16_t*)shortBuf != 0) {
				throw ModbusError("Invalid Modbus protocol specified (should be 0)");
			}
			state = ModbusTCPState::AWAITING_LENGTH;
		}
		shortFirstByte = !shortFirstByte;
	}
	else if(state == ModbusTCPState::AWAITING_LENGTH) {
		if(shortFirstByte) {
			shortBuf[0] = c;
		}
		else {
			shortBuf[1] = c;
			numBytes = be16toh(*(uint16_t*)shortBuf);
			bytesRead = 0;
			state = ModbusTCPState::AWAITING_IDENTIFIER;
		}
		shortFirstByte = !shortFirstByte;
	}
	else if(state == ModbusTCPState::AWAITING_IDENTIFIER) {
		state = ModbusTCPState::AWAITING_DATA;
		slaveId = c;
		bytesRead++;
	}
	else if(state == ModbusTCPState::AWAITING_DATA) {
		Frame frame;
		bool done = reader.readByte(c, frame);
		bytesRead++;
		// NOTE: accepts supersets of valid packets if the number of bytes stated by the frame > number of bytes needed
		if(bytesRead == numBytes) {
			if(done) {
				// need a lambda to call sendResponse with a "this" context
				cb(Responder(frame, [&](uint8_t func, const uint8_t* msg, int len) {
					sendResponse(func, msg, len);
				}));
				reset();
			}
			else {
				throw ModbusError("Data frame incomplete");
			}
		}
	}
}
bool ModbusTCPSocket::shouldDie() {
	uint8_t c;
	int n = read(sockFd, &c, 1);
	if(n > 0) {
		readByte(c);
		return false;
	}
	else if(n < 0 && errno == EAGAIN) {
		return false;
	}
	// here, we either read 0 bytes or reached an exception from reading
	return true;
}


// #define WTF_IS_HAPPENING
// good for debugging network stuff
// taken from: https://gist.github.com/domnikl/af00cc154e3da1c5d965
#ifdef WTF_IS_HAPPENING
void hexDump(char *desc, void *addr, int len) {
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
#endif

void ModbusTCPSocket::sendResponse(uint8_t func, const uint8_t *msg, int len) {
	// set up the byte manipulation hackery
	uint8_t resp[8 + len]; // probably do a overflow check on this addition
	uint16_t *respShort = (uint16_t*)resp;
	respShort[0] = htobe16(transactionId);
	respShort[1] = 0;
	respShort[2] = htobe16(2+len);
	resp[6] = slaveId;
	resp[7] = func;
	memcpy(resp + 8, msg, len);

	uint8_t finalLen = 8 + len;

#ifdef WTF_IS_HAPPENING
	hexDump("Transmitting: ", resp, finalLen);
	putchar('\n');
#endif

	// we swap the socket into blocking mode during the write (i dont want any loops)
	int flags = fcntl(sockFd, F_GETFL, 0);
	flags &= ~O_NONBLOCK;
	if(fcntl(sockFd, F_SETFL, flags) < 0) {
		throw runtime_error(string("Could not put socket into blocking mode: ") + strerror(errno));
	}

	if(send(sockFd, resp, finalLen, 0) < 0) {
		throw runtime_error(string("Error sending msg data: ") + strerror(errno));
	}

	flags |= O_NONBLOCK;
	if(fcntl(sockFd, F_SETFL, flags) < 0) {
		throw runtime_error(string("Could not put socket back into non-blocking mode: ") + strerror(errno));
	}
}

void ModbusTCPSocket::reset() {
	reader.reset();
	state = ModbusTCPState::AWAITING_TRANSACTION_ID;
}