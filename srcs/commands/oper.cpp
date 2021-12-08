//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"
#include <Reply.hpp>
#include <algorithm>
#include <sstream>
//TODO: all
namespace IRC {

	void cmd_mode(Command const &cmd, Client &client, ListenSocket &server) {
		//добавить выводи режимов на экран
		std::vector<std::string> const &params = cmd.getParams();//параметры

		if (params.empty()) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
			return;
		}
		if (params[0][0] == '#') {
			Channel *channel = server.thisischannel(params[0], 0, client);
			if (channel == NULL)
				return;
			if (params.size() == 1)//выводим инфу об активных модах
			{
				std::string res;
				if (channel->isFlag(CMODE_INVITE)) {
					res.push_back(' ');
					res.push_back(INVIT);
				}
				if (channel->isFlag(CMODE_MODER)) {
					res.push_back(' ');
					res.push_back(MODES);
				}
				if (channel->isFlag(CMODE_SECRET)) {
					res.push_back(' ');
					res.push_back(SECRET);
				}
				if (channel->isFlag(CMODE_NOEXT)) {
					res.push_back(' ');
					res.push_back(SPEAK);
				}
				if (channel->isFlag(CMODE_TOPIC)) {
					res.push_back(' ');
					res.push_back(TOPIC);
				}
				if (!channel->getKey().empty()) {
					res.push_back(' ');
					res.push_back(KEY);
					res.push_back('=');
					res += channel->getKey();
				}
				if (channel->getLimit() > 0) {
					res.push_back(' ');
					res.push_back(LEN);
					res.push_back('=');
					std::stringstream ss;
					ss << channel->getLimit();
					res += ss.str();
				}
				sendReply(client, server, RPL_CHANNELMODEIS, params[0], res);
				return;
			}
			if (channel->opers.find(&client.getNick()) == channel->opers.end() && !client.isFlag(UMODE_OPER)) {
				sendError(client, server, ERR_CHANOPRIVSNEEDED, params[0]);//если нет привелегий
				return;
			}

			std::map<const char, enum Channel::model>::iterator it = Channel::modes.find(params[1][1]);
			//TODO: Странный, сука баг, когда иногда mode not found
//			std::cout << "DEBUG!!!!!! " << params[1][1] << " " << (it == Channel::modes.end()) << " " << "s" << std::endl;
			if (params[1].size() != 2 || it == Channel::modes.end()) {
				sendError(client, server, ERR_UNKNOWNMODE, params.size() > 1 ? params[1] : "", params[0]);//если не подходит мод
				return;
			}
			enum Channel::model mod = it->second;
			char sign = params[1][0];
			if (sign != '-' && sign != '+') {
				sendError(client, server, ERR_UNKNOWNMODE, params[1], params[0]);//если не подходит мод
				return;
			}
			switch (mod) {
				case Channel::I:
					if (sign == '-') {
						channel->zeroFlag(CMODE_INVITE);
					} else {
						channel->setFlag(CMODE_INVITE);
					}
					break;
				case Channel::M:
					if (sign == '-') {
						channel->zeroFlag(CMODE_MODER);
					} else {
						channel->setFlag(CMODE_MODER);
					}
					break;
				case Channel::S:
					if (sign == '-') {
						channel->zeroFlag(CMODE_SECRET);
					} else {
						channel->setFlag(CMODE_SECRET);
					}
					break;
				case Channel::N:
					if (sign == '-') {
						channel->zeroFlag(CMODE_NOEXT);
					} else {
						channel->setFlag(CMODE_NOEXT);
					}
					break;
				case Channel::T:
					if (sign == '-') {
						channel->zeroFlag(CMODE_TOPIC);
					} else {
						channel->setFlag(CMODE_TOPIC);
					}
					break;
				case Channel::O: {
					if (params.size() < 3) {
						sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
						return;
					}
					std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
																  is_nickname(params[2]));
					if (to == server.clients.end()) {
						sendError(client, server, ERR_NOSUCHNICK, params[2]);
						return;
					}
					std::string nick = to->getNick();
					if (channel->users.find(&(*to)) == channel->users.end()) {
						sendError(client, server, ERR_USERNOTINCHANNEL, params[2], params[0]);
						return;
					}
					if (sign == '+') {
						if (channel->opers.find(&nick) != channel->opers.end()) {
							return;
						} else {
							channel->opers.insert(&(to->getNick()));
						}
					} else {
						if (channel->opers.find(&nick) == channel->opers.end()
							&& !client.isFlag(UMODE_OPER)) {
							return;
						} else {
							channel->opers.erase(&(to->getNick()));
						}
					}
					break;
				}
				case Channel::V: {
					if (params.size() < 3) {
						sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
						return;
					}
					std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
																  is_nickname(params[2]));
					if (to == server.clients.end()) {
						sendError(client, server, ERR_NOSUCHNICK, params[2]);
						return;
					}
					std::string nick = to->getNick();
					if (channel->users.find(&(*to)) == channel->users.end()) {
						sendError(client, server, ERR_USERNOTINCHANNEL, params[2], params[0]);
						return;
					}
					if (sign == '+') {
						if (channel->voiced.find(&nick) != channel->voiced.end()) {
							return;
						} else {
							channel->voiced.insert(&((*to).getNick()));
						}
					} else {
						if (channel->voiced.find(&nick) == channel->voiced.end()) {
							return;
						} else {
							channel->voiced.erase(&(to->getNick()));
						}
					}
					break;
				}
				case Channel::K: {
					if (params.size() < 3) {
						sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
						return;
					}
					if (sign == '+') {
						if (!channel->getKey().empty()) {
							sendError(client, server, ERR_KEYSET, params[0]);
							return;
						} else {
							channel->setKey(params[2]);
						}
					} else {
						channel->clearKey();
					}
					break;
				}
				case Channel::L: {
					if (params.size() < 3) {
						sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
						return;
					}
					if (sign == '+') {
						int a = check_num(params[2].c_str());//переводим в число
						if (a != false) {
							channel->setLimit(a);
						}
					} else {
						channel->clearLimit();
					}
					break;
				}
			}
		} else {
			int flag = 0;
			if (params.size() == 1)
				flag = 1;//если хочешь просто посмотреть
			Client *oclient = server.thisisnick(params[0], flag, client);
			if (oclient == NULL)
				return;
			if (params.size() == 1) {
				std::string res = "r";
				if (oclient->isFlag(UMODE_OPER))
					res += " o";
				if (oclient->isFlag(UMODE_INVIS))
					res += " i";
				if (oclient->isFlag(UMODE_WALLOPS))
					res += " w";
				sendReply(client, server, RPL_UMODEIS, res);
				return;
			}
			std::string models = "iwo";
			if (params[1].size() != 2 || models.find(params[1][1]) == std::string::npos) {
				sendError(client, server, ERR_UMODEUNKNOWNFLAG);//если не подходит мод
				return;
			}
			char sign = params[1][0];
			char mod = params[1][1];
			if (sign != '-' && sign != '+') {
				sendError(client, server, ERR_UMODEUNKNOWNFLAG);//если не подходит мод
				return;
			}
			if (client.isFlag(UMODE_OPER)) {
				if (mod == 'i') {
					if (sign == '+')
						oclient->setFlag(UMODE_INVIS);
					else
						oclient->zeroFlag(UMODE_INVIS);
				} else if (mod == 'w') {
					if (sign == '+')
						oclient->setFlag(UMODE_WALLOPS);
					else
						oclient->zeroFlag(UMODE_WALLOPS);
				} else if (mod == 'o') {
					if (sign == '+')
						oclient->setFlag(UMODE_OPER);
					else
						oclient->zeroFlag(UMODE_OPER);
				}
			} else {
				if (client.getNick() != oclient->getNick() && !client.isFlag(UMODE_OPER)) {
					sendError(client, server, ERR_USERSDONTMATCH);
					return;
				} else if (mod == 'i') {
					if (sign == '+')
						oclient->setFlag(UMODE_INVIS);
					else
						oclient->zeroFlag(UMODE_INVIS);
				} else if (mod == 'w') {
					if (sign == '+')
						oclient->setFlag(UMODE_WALLOPS);
					else
						oclient->zeroFlag(UMODE_WALLOPS);
				} else if (mod == 'o') {
					if (!client.isFlag(UMODE_OPER))
						sendError(client, server, ERR_NOPRIVILEGES);
					if (sign == '+')
						oclient->setFlag(UMODE_WALLOPS);
					else
						oclient->zeroFlag(UMODE_WALLOPS);
				}
			}
		}
	}

	void cmd_oper(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams();//параметры
		std::map<std::string, std::string>::iterator pass;
		//        std::vector<std::string> keys;
		int res;
		int count = 0;

		if (params.empty() || params.size() != 2) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "OPER");
			return;
		}
		if (server.opers.empty()) {//если мапа пустая значит оперов не может быть
			sendError(client, server, ERR_NOOPERHOST);
			return;
		}
		pass = server.opers.find(params[0]);
		if (pass == server.opers.end() || pass->second != params[1]) {
			sendError(client, server, ERR_PASSWDMISMATCH);
			return;
		}
		sendReply(client, server, RPL_YOUREOPER);
		if (!(client.getFlags() & UMODE_OPER))
			client.setFlag(UMODE_OPER);
	}

	void cmd_kill(Command const &cmd, Client &client, ListenSocket &server) {
		//TODO: что то сделать с комментом надо
		std::vector<std::string> const &params = cmd.getParams();//параметры
		std::vector<std::string> pass;
		int res;
		int count = 0;

		if (params.empty() || params.size() != 2) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "KILL");
			return;
		}

		if (!(client.isFlag(UMODE_OPER))) {
			sendError(client, server, ERR_NOPRIVILEGES);
			return;
		}
		std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
													  is_nickname(params[0]));
		if (to == server.clients.end()) {
			sendError(client, server, ERR_NOSUCHNICK, params[0]);
			return;
		}
		if (params[0] == server.getServername()) {
			sendError(client, server, ERR_CANTKILLSERVER);
			return;
		}
//		server.quit_client(to->getFd());
		to->disconnect();
	}
}// namespace IRC