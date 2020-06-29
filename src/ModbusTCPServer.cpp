#include "ModbusTCPServer.h"
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <map>

using namespace std;
using namespace modbus;

ModbusTCPServer::ModbusTCPServer(int port, ModbusCallback cb): cb(cb) {
	// open the port
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if(serverFd < 0) {
		throw runtime_error(string("Could not create new socket: ") + strerror(errno));
	}
	int optVal = 1;
	setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(serverFd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		throw runtime_error(string("Could not bind to port ") + to_string(port) + ": " + strerror(errno) + " (do you have permissions?)");
	}
	if(listen(serverFd, 8) < 0) {
		throw runtime_error(string("Could not listen: ") + strerror(errno));
	}
	int flags = fcntl(serverFd, F_GETFL, 0);
	if(fcntl(serverFd, F_SETFL, flags | O_NONBLOCK) < 0) {
		throw runtime_error(string("Could not set server port into nonblocking: ") + strerror(errno));
	}
	// add server socket to epoll to listen for ppl coming in
	epfd = epoll_create1(0);
	if(epfd < 0) {
		throw runtime_error(string("Could not create EPOll: ") + strerror(errno));
	}
	struct epoll_event e;
	e.data.fd = serverFd;
	e.events = EPOLLIN;
	if(epoll_ctl(epfd, EPOLL_CTL_ADD, serverFd, &e) < 0) {
		throw runtime_error(string("Could not add server fd to epoll: ") + strerror(errno));
	}
}

void ModbusTCPServer::tick() {
	int numFds = epoll_wait(epfd, events, 16, 0);
	for(int i = 0; i < numFds; i++) {
		int fd = events[i].data.fd;
		uint32_t event = events[i].events;
		// server event probably means someone is coming in
		if(fd == serverFd && event & EPOLLIN) {
			struct sockaddr_in clientAddr;
			socklen_t clientAddrLen = sizeof(clientAddr);
			int clientFd = accept(serverFd, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrLen);
			if(clientFd < 0) {
				if(errno == EAGAIN) {
					continue;
				}
				throw runtime_error(string("Error accepting new client: ") + strerror(errno));
			}
			sockMap[clientFd] = make_unique<ModbusTCPSocket>(clientFd, cb);

			epoll_event e;
			e.data.fd = clientFd;
			e.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLET;

			if(epoll_ctl(epfd, EPOLL_CTL_ADD, clientFd, &e) < 0) {
				throw runtime_error(string("Could not add client fd to epoll: ") + strerror(errno));
			}
		}
		// else the event means that a client sent something
		else {
			// if socket is deaded, give up its handler
			if(event & EPOLLHUP || event & EPOLLERR) {
				sockMap.erase(fd);
				if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
					throw runtime_error(string("Could not remove client fd to epoll: ") + strerror(errno));
				}
			}
			else if(event & EPOLLIN) {
				sockMap[fd]->drainSocket();
			}
		}
	}

	// scan all open clients to check which are deserving of death
	auto clientIt = sockMap.begin();
	while(clientIt != sockMap.end()) {
		const auto &kvp = *clientIt;
		if(kvp.second->shouldDie()) {
			if(epoll_ctl(epfd, EPOLL_CTL_DEL, kvp.first, nullptr) < 0) {
				throw runtime_error(string("Could not remove client fd to epoll: ") + strerror(errno));
			}
			clientIt = sockMap.erase(clientIt);
		}
	}
}

ModbusTCPServer::~ModbusTCPServer() {
	sockMap.clear();
	close(serverFd);
}