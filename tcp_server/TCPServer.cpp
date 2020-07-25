#include <modern_modbus_tcp_server.h>
#include "Socket.h"
#include <arpa/inet.h>
#include "mb_exceptions.h"
#include <fcntl.h>
#include <memory>
#include <iostream>
#include <unistd.h>
#include <spdlog/spdlog.h>
using namespace std;
using namespace modbus::tcp;

TCPServer::TCPServer(int port, std::shared_ptr<Accessors> accs): accs(std::move(accs)) {
	// open the port
	serverFd = checkOSErr(socket(AF_INET, SOCK_STREAM, 0), "Creating server socket: ");
	int optVal = 1;
	setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	checkOSErr(bind(serverFd, (struct sockaddr *) &addr, sizeof(addr)), "Binding to port (check permissions): ");
	checkOSErr(listen(serverFd, 8), "Could not listen on port: ");

	int flags = fcntl(serverFd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	checkOSErr(fcntl(serverFd, F_SETFL, flags), "Could not set server port into nonblocking: ");

	// add server socket to epoll to listen for ppl coming in
	epfd = checkOSErr(epoll_create1(0), "Could not open EPOLL fd");
	struct epoll_event e;
	e.data.fd = serverFd;
	e.events = EPOLLIN;
	checkOSErr(epoll_ctl(epfd, EPOLL_CTL_ADD, serverFd, &e), "Could not add server FD to EPOLL");
}

void TCPServer::tick() {
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
			sockMap[clientFd] = make_unique<Socket>(clientFd, accs);

			epoll_event e;
			e.data.fd = clientFd;
			e.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLET;

			checkOSErr(epoll_ctl(epfd, EPOLL_CTL_ADD, clientFd, &e), "Could not add client to EPOLL: ");

			spdlog::info("Client joined: {}", inet_ntoa(clientAddr.sin_addr));
		}
		// else the event means that a client sent something
		else {
			// if socket is deaded, give up its handler
			if(event & EPOLLHUP || event & EPOLLERR) {
				spdlog::error("Client socket faulted!");
				sockMap.erase(fd);
				checkOSErr(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr), "Could not remove client from EPOLL: ");
			}
			else if(event & EPOLLIN) {
				try {
					sockMap[fd]->drainSocket();
				}
				catch(std::runtime_error &e) {
					spdlog::error("Client issue arose, going to kill it: {}", e.what());
					sockMap.erase(fd);
				}
			}
		}
	}
	// scan all open clients to check which are deserving of death
	auto clientIt = sockMap.begin();
	while(clientIt != sockMap.end()) {
		const auto &kvp = *clientIt;
		if(kvp.second->shouldDie()) {
			checkOSErr(epoll_ctl(epfd, EPOLL_CTL_DEL, kvp.first, nullptr), "Could not remove zombie client form EPOLL: ");
			clientIt = sockMap.erase(clientIt);
			spdlog::info("Client closed connection");
		}
		else {
			clientIt++;
		}
	}
}

TCPServer::~TCPServer() {
	sockMap.clear();
	close(serverFd);
	close(epfd);
}