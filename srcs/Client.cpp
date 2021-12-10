//
// Created by Frey Tinkerer on 11/11/21.
//

//#include <ListenSocket.hpp>
#include "Client.hpp"
#include <ListenSocket.hpp>
#include <Reply.hpp>

IRC::is_nickname_s IRC::is_nickname(std::string nickname) {
		return is_nickname_s(nickname);
}

IRC::is_mask_s IRC::is_mask(std::string mask) {
    return is_mask_s(mask);
}

IRC::is_fd_s IRC::is_fd(int fd) {
	return is_fd_s(fd);

}

bool IRC::Client::try_register(ListenSocket & server) {
	if (this->isFlag(UMODE_REGISTERED)) {
		return true;
	}
	if (this->nick.empty() || this->user.empty()) {
		return false;
	} else if (!server.getPassword().empty() && this->pass != server.getPassword()) {
		Command msg(server.getServername(), "ERROR");
		msg << "Access denied. Incorrect password";
		server.send_command(msg, this->fd);
		server.quit_client(this->fd);
		return false;
	}
	setFlag(UMODE_REGISTERED);
	std::cout << "[DEBUG]: " << this->nick << "!" << this->user << "@" << this->host << " are registered." << std::endl;
    this->setMask();//закидываем верхнюю строчку в структуру

	int rpls[] = {RPL_WELCOME, RPL_YOURHOST, RPL_CREATED, RPL_MYINFO, RPL_ISUPPORT, RPL_LUSERCLIENT, RPL_LUSEROP, RPL_LUSERUNKNOWN, RPL_LUSERCHANNELS, RPL_LUSERME};
	for(int* it = rpls; it < rpls + 10; ++it) {
		sendReply(*this, server, *it);
	}

	sendReply(*this, server, RPL_MOTDSTART, server.getServername());
	sendReply(*this, server, RPL_MOTD, "Welcome! Ли сахлии-гар!");
	sendReply(*this, server, RPL_ENDOFMOTD);

	pinged = false;
	last_pingpong = time(NULL);

    Client buf = *this;

    server.base_client.insert(std::make_pair(this->nick,buf));//создаем неизменяемую базу клиентов
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
