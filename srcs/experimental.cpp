//
// Created by Frey Tinkerer on 11/4/21.
//

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <vector>
#include <sstream>
#include <Client.hpp>

#include "../includes/Command.hpp"

int main() {
	char name [1024];
	name[1023] = '\0';
	gethostname(name, 1023);

	std::cout << "hostname: " << name << std::endl;


	for (;;) {
		std::string buf;
		std::getline(std::cin, buf);
		if (buf == "q") {
			break;
		}
		buf += "\r\n";
		IRC::Command cmd(buf);
		cmd.exec()
	}

	return 0;
}

//	int socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (socket_fd < 0) {
//		std::cerr << "Can't open socket" << std::endl;
//		return 1;
//	}
//
//	struct sockaddr_in stSockAddr = {0};
//	stSockAddr.sin_family = PF_INET;
//	stSockAddr.sin_port = htons(6664);
//	stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
//
//	if (bind(socket_fd, (struct sockaddr *) &stSockAddr, sizeof(stSockAddr)) < 0) {
//		std::cerr << "Can't bind socket" << std::endl;
//		close(socket_fd);
//		return 1;
//	}
//
//	if (listen(socket_fd, 128) < 0) {
//		std::cerr << "Can't listen socket" << std::endl;
//		close(socket_fd);
//		return 1;
//	}
//
//	fcntl(socket_fd, F_SETFL, O_NONBLOCK);
//	std::vector<struct pollfd> pollfds;
//
//	std::string text;
//	long bytesRead;
//	char buffer[100];
//
//	bool work = true;
//	while (work) {
//		int connect_fd = accept(socket_fd, nullptr, nullptr);
//
//		if (connect_fd >= 0) {
//			std::cout << "[LOG]: accepted" << std::endl;
//			struct pollfd pfd = {0};
//			pfd.fd = connect_fd;
//			pfd.events = POLLIN;
//			pfd.revents = 0;
//			pollfds.push_back(pfd);
//		}
//
//		int poll_ret = poll(pollfds.data(), pollfds.size(), 100);
//		if (poll_ret > 0) {
//			for (int i = 0; i < pollfds.size(); ++i) {
//				if (pollfds[i].revents & POLLIN) {
//					while ((bytesRead = recv(pollfds[i].fd, buffer, 99, 0)) > 0) {
//						buffer[bytesRead] = 0;
//						text += buffer;
//						buffer[0] = 0;
//						if (text.find('\n') != std::string::npos)
//							break;
//					}
//					if (text == "exit\n") {
//						shutdown(pollfds[i].fd, SHUT_RDWR);
//						close (pollfds[i].fd);
//						pollfds.erase(pollfds.begin() + i);
//					} else if (text == "sexit\n") {
//						work = false;
//						break;
//					} else if (pollfds.size() == 1) {
//						std::cout << "[" << i << " to " << i << "]: " << text;
//						std::stringstream tmp;
//						tmp << "[" << i << "]: " << text;
//						send(pollfds[i].fd, (const void*) tmp.str().data(), tmp.str().size(), 0);
//					} else if (pollfds.size() > 1) {
//						int fd = i + 1 == pollfds.size() ? 0 : i + 1;
//						std::cout << "[" << i << " to " << fd << "]: " << text;
//						std::stringstream tmp;
//						tmp << "[" << i << "]: " << text;
//						send(pollfds[fd].fd, (const void*) tmp.str().data(), tmp.str().size(), 0);
//					}
//
//					text.clear();
//				}
//				pollfds[i].revents = 0;
//			}
//		}
//	}