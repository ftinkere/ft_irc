//
// Created by Frey Tinkerer on 11/11/21.
//

//#include <ListenSocket.hpp>
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

	sendReply(server.getServername(), *this, RPL_MOTDSTART, server.getServername(), "", "", "", "", "", "", "");
	sendReply(server.getServername(), *this, RPL_MOTD, "Welcome! Ли сахлии-гар!", "", "", "", "", "", "", "");
	sendReply(server.getServername(), *this, RPL_ENDOFMOTD, "", "", "", "", "", "", "", "");

	return true;
}

bool IRC::Client::_name_control(std::string const& prefix, int v)
{
    int i = 0;
    bool sign = false;

    if (v == 1)
    {
        for(int k = 0; k < prefix.length(); k++)
        {
            if (!std::isalpha(prefix[k]))
                return false;
        }
    }
    else{
        std::string spec = "-[]\\^{}";
        while(prefix.length() > i)
        {
            if (!std::isalnum(prefix[i]))
            {
                for (int j = 0; j < spec.length(); j++)
                {
                    if (prefix[i] == spec[j])
                    {
                        sign = true;
                        break;
                    }
                }
                if (sign == false)
                    return false;
                sign = false;
            }
            i++;
        }
    }
    return true;
}

std::string IRC::Client::get_full_name() const {
    return std::string(this->nick + "!" + this->user + "@" + this->host);
}

void IRC::Client::eraseChannel(std::string const& flag) //удаляем канал из списка пользователя
{
    channels.remove(flag);
}

IRC::Client::Client() {

}
