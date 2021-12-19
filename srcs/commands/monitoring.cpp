//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"
#include <Reply.hpp>
#include <algorithm>
#include <sstream>

namespace IRC {

	void cmd_names(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		bool is_empty = false;

		std::vector<channel_iter> mas_it;
		if (!params.empty()) {
			std::vector<std::string> chans = split(params[0], ',');
			size_t len = chans.size();
			for (int i = 0; i < len; ++i) {
				channel_iter it = server.getChannel(chans[i]);
				if (!server.isChannelExist(it)) {
					sendReply(client, server, RPL_ENDOFNAMES, chans[i]);
					continue;
				}
				mas_it.push_back(it);
			}
		} else {
			//если нет каналов выводим все
			for (channel_iter it = server.channels.begin(); it != server.channels.end(); ++it) {
				mas_it.push_back(it);
			}
			is_empty = true;
		}
		for (std::vector<channel_iter>::iterator vec_it = mas_it.begin(); vec_it != mas_it.end(); ++vec_it) {
			Channel &channel = (*vec_it)->second;
			if (channel.isFlag(CMODE_SECRET) && !channel.isClient(client)) {
				sendReply(client, server, RPL_ENDOFNAMES, channel.getName());
				continue;
			}
			sendReply(client, server, RPL_NAMREPLY, (*vec_it)->first,
					  (*vec_it)->second.isFlag(CMODE_SECRET) ? "@" : "=", channel.get_names());
			sendReply(client, server, RPL_ENDOFNAMES, (*vec_it)->first);
		}
		if (is_empty) {
			// если мы не выводили избранные каналы
			std::string cl;
			client_iter lst = server.clients.begin();
			for (; lst != server.clients.end(); ++lst) {
				if (lst->getChannels().empty() && !(lst->isFlag(UMODE_INVIS))) {
					cl += (*lst).getNick() + " ";
				}
			}
			if (!cl.empty()) {
				cl.erase(cl.end() - 1);
			}
			sendReply(client, server, RPL_NAMREPLY, "-", "-", cl);
			sendReply(client, server, RPL_ENDOFNAMES, "-");
		}
	}

	void cmd_admin(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> params = cmd.getParams(); //параметры

		if (params.empty())
			params.push_back(server.getServername());
		if (params[0] != server.getServername()) {
			sendError(client, server, ERR_NOSUCHSERVER, params[0]);
			return;
		}
		sendReply(client, server, RPL_ADMINME, params[0]);
		sendReply(client, server, RPL_ADMINLOC1, server.admin["adminName"]);
		sendReply(client, server, RPL_ADMINLOC2, server.admin["adminNickname"]);
		sendReply(client, server, RPL_ADMINEMAIL, server.admin["adminEmail"]);
	}

	void cmd_whois(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		client_iter to;
		if (params.empty()) {
			sendError(client, server, ERR_NONICKNAMEGIVEN);
			return;
		}
		std::vector<std::string> nicks = split(params[0], ',');
		for (int i = 0; i < nicks.size(); ++i) {
			std::string nick = nicks[i];
//			to = check_mask_nick(RPL_WHOISSERVER, params[0], client, server);
			if (nick.find('@') != std::string::npos)
				to = server.getClientByMask(params[0]);
			else
				to = server.getClient(params[0]);
			if (!server.isClientExist(to)) {
				sendError(client, server, ERR_NOSUCHNICK, nick);
				continue;
			}
			std::string chans_str;
			Client find_client = *to;
			std::list<std::string>::iterator chans = find_client.getChannels().begin(); //берем клиента
			for (; chans != find_client.getChannels().end(); ++chans) {
				Channel &channel = server.channels[*chans];
				if (channel.isFlag(CMODE_SECRET))
					continue;
				else if (channel.opers.find(&find_client.getNick()) != channel.opers.end())//заполняем список каналов
					chans_str += '@' + *chans + ' ';
				else if (channel.voiced.find(&client.getNick()) != channel.voiced.end())
					chans_str += '+' + *chans + ' ';
				else
					chans_str += *chans + ' ';
			}
			sendReply(client, server, RPL_WHOISUSER, nick, find_client.getUser(), find_client.getHost(),
					  find_client.getReal());
			sendReply(client, server, RPL_WHOISCHANNELS, nick, chans_str);
			sendReply(client, server, RPL_WHOISSERVER, nick, server.getServername(), "Вот такой вот сервер");
			if (!find_client.getAway().empty())
				sendReply(client, server, RPL_AWAY, nick, find_client.getAway());
			if (find_client.isFlag(UMODE_OPER))
				sendReply(client, server, RPL_WHOISOPERATOR, nick);
			std::stringstream regTime, onServer;
			onServer << (time(0) - find_client.getRegisterTime());
			regTime << find_client.getRegisterTime();
			sendReply(client, server, RPL_WHOISIDLE, nick, onServer.str(), regTime.str());
			sendReply(client, server, RPL_ENDOFWHOIS, nick);
		}
	}

	void cmd_whowas(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams(); //параметры

		if (params.empty()) {
			sendError(client, server, ERR_NONICKNAMEGIVEN);
			return;
		}
		std::vector<std::string> nicks = split(params[0], ',');
		size_t limit = 1;
		for (int i = 0; i < nicks.size(); ++i) {
			std::string nick = nicks[i];
			bool loop = false;
			if (params.size() > 1)
				limit = check_num(params[1].c_str());// если есть второй аргумент то это будет лимитом
			size_t coun = server.base_client.count(nick); //количество элементов в мапе
			if (limit < 0 || coun < limit)
				limit = coun;
			std::pair<std::multimap<std::string, Client>::iterator, std::multimap<std::string, Client>::iterator> range = server.base_client.equal_range(
					nick);//находим список всех совпадений
			int j = 0;
			for (std::multimap<std::string, Client>::iterator it = range.first;
				 it != range.second && j < limit; ++it, ++j) {
				sendReply(client, server, RPL_WHOWASUSER, nick, it->second.getUser(), it->second.getHost(),
						  it->second.getReal());
				sendReply(client, server, RPL_WHOISSERVER, nick, server.getServername(), "Вот такой вот сервер");
				loop = true;//если зашли в цикл значит такой ник есть
			}
			if (!loop)
				sendError(client, server, ERR_WASNOSUCHNICK, nick);
			sendReply(client, server, RPL_ENDOFWHOWAS, nick);
		}
	}
}