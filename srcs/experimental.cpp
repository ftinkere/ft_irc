//
// Created by Frey Tinkerer on 11/4/21.
//

#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <vector>

int main() {

	int socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_fd < 0) {
		std::cerr << "Can't open socket" << std::endl;
		return EXIT_FAILURE;
	}

	struct sockaddr_in stSockAddr = {0};
	stSockAddr.sin_family = PF_INET;
	stSockAddr.sin_port = htons(6665);
	stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	stSockAddr.sin_len = sizeof(struct sockaddr_in);

	if (bind(socket_fd, (struct sockaddr *) &stSockAddr, sizeof(stSockAddr)) < 0) {
		std::cerr << "Can't bind socket" << std::endl;
		close(socket_fd);
		return EXIT_FAILURE;
	}

	if (listen(socket_fd, 128) < 0) {
		std::cerr << "Can't listen socket" << std::endl;
		close(socket_fd);
		return EXIT_FAILURE;
	}

	fcntl(socket_fd, F_SETFL, O_NONBLOCK);
	std::vector<struct pollfd> pollfds;

	std::string text;
	long bytesRead;
	char buffer[100];

	bool work = true;
	while (work) {
		int connect_fd = accept(socket_fd, nullptr, nullptr);

		if (connect_fd >= 0) {
			std::cout << "[LOG]: accepted" << std::endl;
			struct pollfd pfd = {0};
			pfd.fd = connect_fd;
			pfd.events = POLLIN;
			pfd.revents = 0;
			pollfds.push_back(pfd);
		}

		int poll_ret = poll(pollfds.data(), pollfds.size(), 100);
		if (poll_ret > 0) {
			for (int i = 0; i < pollfds.size(); ++i) {
				if (pollfds[i].revents & POLLIN) {
					while ((bytesRead = recv(pollfds[i].fd, buffer, 99, 0)) > 0) {
						buffer[bytesRead] = 0;
						text += buffer;
						buffer[0] = 0;
						if (text.find('\n') != std::string::npos)
							break;
					}
					std::cout << "[RECV " << i << "]: " << text;
					if (text == "exit\n") {
						shutdown(pollfds[i].fd, SHUT_RDWR);
						close (pollfds[i].fd);
						pollfds.erase(pollfds.begin() + i);
					}
					if (text == "sexit\n") {
						work = false;
						break;
					}
					text.clear();
				}
				pollfds[i].revents = 0;
			}
		}
	}
		return 0;
}
