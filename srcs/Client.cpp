//
// Created by Frey Tinkerer on 11/11/21.
//

#include <ListenSocket.hpp>
#include "Client.hpp"

IRC::is_nickname_s IRC::is_nickname(std::string nickname) {
		return is_nickname_s(nickname);
}

IRC::is_fd_s IRC::is_fd(int fd) {
	return is_fd_s(fd);

}

bool IRC::Client::try_register(ListenSocket & server) {
//	std::cout << "[DEBUG]: server password: " << server.getPassword() << std::endl;
//	std::cout << "[DEBUG]: nick: " << this->nick << std::endl;
	// TODO: если проблемы с паролем, выкинуть
	if (this->nick.empty() || this->user.empty()) {
		return false;
	} else if (!server.getPassword().empty() && this->pass != server.getPassword()) {
		// reply acces deni
		server.quit_client(this->fd);
		return false;
	}
	setFlag(UMODE_REGISTERED);
	std::cout << "[DEBUG]: " << this->nick << "!" << this->user << "@" << this->host << " are registered." << std::endl;
	return true;
}
