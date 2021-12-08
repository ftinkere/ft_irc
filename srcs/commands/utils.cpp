//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"
#include <Reply.hpp>
#include <algorithm>

namespace IRC {

	int check_num(const char *str) {
		//"Функция проверки числа"
		for (int i = 0; i < strlen(str); ++i) {
			if (!std::isdigit(str[i]))
				return false;
		}
		int res = std::atoi(str);
		return res;
	}

	std::string choose_str(std::vector<std::string> const &params, size_t len, int j) {
		//"Функция сборки последних параметров для отправки"
		std::string msg;
		for (; j < len; ++j) {//собираем параметры для отправки
			msg += params[j];
			if (j != len - 1) {
				msg += ' ';
			}
		}
		return msg;
	}

	Channel *check_channel(std::string const &chan, ListenSocket &server, Client &client, int visible) {
		//"Функция проверки наличия канала"
		channel_iter it = server.getChannel(chan);
		if (!server.isChannelExist(it)) {
			sendError(client, server, ERR_NOSUCHCHANNEL, chan);
			return NULL;
		}
		Channel &channel = it->second;

		if (visible == 1) {
			if (!channel.isClient(client)) {
				//если клиента нет на канале
				sendError(client, server, ERR_NOTONCHANNEL, chan);
				return NULL;
			}
		}
		//        if (visible == 2)
		//        {
		//            if (channel.isFlag(CMODE_SECRET))
		//                return NULL;
		//        }
		return &channel;
	}

	bool priv_need_channel(Channel *channel, Client const &client, ListenSocket &server, std::string const &chani) {
		if (!channel->isOper(client)) {
			sendError(client, server, ERR_CHANOPRIVSNEEDED, chani);
			return false;
		}
		return true;
	}

	bool check_nick(client_iter &it, Channel *channel, Client const &client, ListenSocket &server, std::string const &nick, std::string const &chani) {
		if (server.isClientExist(it)) {
			//если нет ника
			sendError(client, server, ERR_NOSUCHNICK, nick);
			return false;
		}
		if (channel->isClient(it)) {
			//если чувак уже на канале
			sendError(client, server, ERR_USERONCHANNEL, nick, chani);
			return false;
		}
		return true;
	}

	bool erase_member(Channel *channel, Client const &client, ListenSocket &server, std::string const &nick, std::string const &chan) {
		client_iter it = server.getClient(nick);
		if (channel->users.erase(&(*it)) == 0) {// пытаемся удалить
			sendError(client, server, ERR_USERNOTINCHANNEL, nick, chan);
			return false;
		}
		channel->opers.erase(&it->getNick());
		channel->voiced.erase(&it->getNick());
		it->eraseChannel(chan);
		return true;
	}

	bool check_params(Client const& client, ListenSocket const& server, std::vector<std::string> const& params, std::string const& cmd, size_t needed) {
		if (params.size() < needed) {
			sendError(client, server, ERR_NEEDMOREPARAMS, cmd);
			return false;
		}
		return true;
	}

	bool check_join_chan(Client const &client, ListenSocket const &server, Channel const &chan, std::string const &in_key) {
		std::string const &key = chan.getKey();

		if (!key.empty() && in_key != key) {
			sendError(client, server, ERR_BADCHANNELKEY, chan.getName());
			return false;
		}
		if (chan.isFlag(CMODE_INVITE)) {
			sendError(client, server, ERR_INVITEONLYCHAN, chan.getName());
			return false;
		}
		if (!chan.check_limit()) {
			sendError(client, server, ERR_CHANNELISFULL, chan.getName());
			return false;
		}
		return true;
	}

	bool check_channel_exist(Client const &client, ListenSocket const &server, channel_iter const& chan, std::string const& chan_name) {
		if (!server.isChannelExist(chan)) {
			sendError(client, server, ERR_NOSUCHCHANNEL, chan_name);
			return false;
		}
		return true;
	}

	bool check_channel_send(Client const &client, ListenSocket const &server, Channel const& chan) {
		if (((chan.isFlag(CMODE_NOEXT) && !chan.isClient(client)) || (chan.isFlag(CMODE_MODER) && !chan.isVoiced(client)))
			&& !client.isFlag(UMODE_OPER)) {
			sendError(client, server, ERR_CANNOTSENDTOCHAN, chan.getName());
			return false;
		}
		return true;
	}

}// namespace IRC