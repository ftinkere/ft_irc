//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"
#include <Parser.hpp>
#include <Reply.hpp>
#include <algorithm>

namespace IRC {
	void cmd_privmsg(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams();//параметры
		size_t len = params.size();                              //длина параметров
		if (params.empty() || params[0].empty()) {
			sendError(client, server, ERR_NORECIPIENT, cmd.getCommand());
			return;
		}
		if (len < 2) {
			sendError(client, server, ERR_NOTEXTTOSEND);
			return;
		}

		std::vector<int> clients;

		// if server зачем-то return

		std::string chan_s = params[0].substr(params[0][0] == '@' ? 1 : 0);
		channel_iter it = server.getChannel(chan_s);
		if (params[0][0] == '#' || (params[0].size() > 1 && params[0][0] == '@' && params[0][1] == '#')) {
			if (it == server.channels.end()) {
				sendError(client, server, ERR_NOSUCHCHANNEL, chan_s);
				return;
			}
			Channel &chan = it->second;
			if (!check_channel_send(client, server, chan)) { return; }
		}

		if (params[0][0] == '#') {
			Channel &chan = it->second;
			for (channel_client_iter cit = chan.users.begin(); cit != chan.users.end(); ++cit) {
				if ((*cit)->getFd() != client.getFd()) {
					clients.push_back((*cit)->getFd());
				}
			}
		} else if (params[0].size() > 1 && params[0][0] == '@' && params[0][1] == '#') {
			std::string chan_s = params[0].substr(1);
			Channel &chan = it->second;
			for (channel_ov_iter op_it = it->second.opers.begin(); op_it != it->second.opers.end(); ++op_it) {
				client_iter cl_it = server.getClient(**op_it);
				if (cl_it->getFd() != client.getFd()) {
					clients.push_back(cl_it->getFd());
				}
			}
		} else {
			client_iter cl_it = server.getClient(params[0]);
			if (cl_it == server.clients.end()) {
				sendError(client, server, ERR_NOSUCHNICK, params[0]);
				return;
			}
			clients.push_back(cl_it->getFd());
			if (!cl_it->getAway().empty())
				sendReply(client, server, RPL_AWAY, cl_it->getNick(), cl_it->getAway());
		}

		std::string msg = choose_str(params, len, 1);//собираем параметры для отправки

		for (int i = 0; i < clients.size(); ++i) {//отправляем
			Command privmsg(client.get_full_name(), CMD_PRIVMSG, params[0]);
			privmsg << msg;
			server.send_command(privmsg, clients[i]);
		}
	}

	void cmd_notice(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> params = cmd.getParams();//параметры
		size_t len = params.size();                       //длина параметров
		if (params.empty()) {
			return;
		}
		if (!len) {
			return;
		}
		std::vector<int> clients;

		// if server зачем-то return

		if (params[0][0] == '#') {
			channel_iter it = server.channels.find(params[0]);
			if (it == server.channels.end()
				|| (it->second.isFlag(CMODE_NOEXT) && it->second.users.find(&client) == it->second.users.end())
				|| (it->second.isFlag(CMODE_MODER) && it->second.voiced.find(&client.getNick()) == it->second.voiced.end())) { return; }
			for (channel_client_iter cit = it->second.users.begin(); cit != it->second.users.end(); ++cit) {
				clients.push_back((*cit)->getFd());
			}
		} else if (params[0].size() > 1 && params[0][0] == '@' && params[0][1] == '#') {
			channel_iter it = server.channels.find(params[0]);
			if (it == server.channels.end()
				|| (it->second.isFlag(CMODE_NOEXT) && it->second.users.find(&client) == it->second.users.end())
				|| (it->second.isFlag(CMODE_MODER) && it->second.voiced.find(&client.getNick()) == it->second.voiced.end())) { return; }
			for (channel_ov_iter cit = it->second.opers.begin(); cit != it->second.opers.end(); ++cit) {
				client_iter cl_it = server.getClient(**cit);
				clients.push_back((*cl_it).getFd());
			}
		} else {
			client_iter  it_client = server.getClient(params[0]);
//			std::list<Client>::iterator it_client = std::find_if(server.clients.begin(), server.clients.end(), is_nickname(params[0]));
			if (it_client == server.clients.end()) { return; }
			clients.push_back((*it_client).getFd());
		}

		std::string msg = choose_str(params, len, 1); //собираем параметры для отправки

		for (int i = 0; i < clients.size(); ++i) { //отправляем
			Command notice(client.get_full_name(), CMD_NOTICE, params[0]);
			notice << msg;
			server.send_command(notice, clients[i]);
		}
	}

	void cmd_away(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> params = cmd.getParams();//параметры
		if (params.empty()) {
			sendReply(client, server, RPL_UNAWAY);
			client.clearAway();
		} else {
			std::string msg = choose_str(params, params.size(), 0);//собираем параметры для отправки
			sendReply(client, server, RPL_NOWAWAY);
			client.setAway(msg);
		}
	}

	void cmd_wallops(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams();//параметры
		size_t len = params.size();
		std::string msg;
		if (params.empty()) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "WALLOPS");
			return;
		}
		if (!client.isFlag(UMODE_OPER)) {
			sendError(client, server, ERR_NOPRIVILEGES);
			return;
		}

		//ищем все ники
		msg = choose_str(params, len, 0);
		client_iter to = server.clients.begin();
		for (; to != server.clients.end(); ++to) {//отправляем
			if (to->isFlag(UMODE_WALLOPS)) {
				Command wallops(client.get_full_name(), CMD_WALLOPS);
				wallops << to->getNick() << msg;
				server.send_command(wallops, to->getFd());
			}
		}
	}
}// namespace IRC