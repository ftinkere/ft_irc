//
// Created by Frey Tinkerer on 11/16/21.
//

#include <algorithm>
#include "commands.hpp"
//#include <Parser.hpp>
//#include "Command.hpp"
//#include "Client.hpp"
//#include "ListenSocket.hpp"
//#include "Channel.hpp"

namespace IRC {

	class Channel;

	void cmd_pass(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams();
		if (params.empty()) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "PASS", "");
			// reply not args
		} else if (client.getFlags() & UMODE_REGISTERED) {
			sendError(client, server, ERR_ALREADYREGISTRED, "", "");
		} else {
			client.pass = cmd.getParams()[0];
		}
	}

	void cmd_nick(Command const &cmd, Client &client, ListenSocket &server) {
		std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
													  is_nickname(cmd.getParams()[0]));
		if (cmd.getParams().empty()) {
			sendError(client, server, ERR_NONICKNAMEGIVEN, "", "");
			return;
		}// else reply not args
		if (!client._name_control(cmd.getParams()[0], 0)) //беру только первый аргумент
		{
			sendError(client, server, ERR_ERRONEUSNICKNAME, cmd.getParams()[0], "");
			return;
		}
		if (to != server.clients.end()) {
			sendError(client, server, ERR_NICKNAMEINUSE, cmd.getParams()[0], "");
			return;
		}
		client.nick = cmd.getParams()[0];
		client.try_register(server);
	}

	void cmd_user(Command const &cmd, Client &client, ListenSocket &server) {
		if (cmd.getParams().empty() || cmd.getParams().size() < 4) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "USER", "");
		} else if (client.getFlags() & UMODE_REGISTERED) {
			sendError(client, server, ERR_ALREADYREGISTRED, "", "");
		} else {
			client.realname = cmd.getParams()[3];
			client.user = cmd.getParams()[0];
			client.try_register(server);
		}
	}

	void cmd_quit(Command const &cmd, Client &client, ListenSocket &server) {
		server.quit_client(client.getFd());
		std::cout << "[DEBUG]: " << client.nick << "!" << client.user << "@" << client.host << " " << cmd.getCommand()
				  << ": " << cmd.getParams()[0] << std::endl;
	}

	void cmd_privmsg(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &param = cmd.getParams(); //параметры
		size_t len = param.size(); //длина параметров
		if (param.empty()) {
			sendError(client, server, ERR_NORECIPIENT, cmd.getCommand(), "");
			return;
		}
		if (len < 2) {
			sendError(client, server, ERR_NOTEXTTOSEND, "", "");
			return;
		}
		std::vector<Client *> clients = server.find_clients(cmd.getParams()[0], client); //ищем все ники
		std::string msg;

		for (int j = 1; j < len; ++j) { //собираем параметры для отправки
			msg += param[j];
			if (j != len - 1) {
				msg += ' ';
			}
		}
		for (int i = 0; i < clients.size(); ++i) { //отправляем
			Command cmd(client.get_full_name(), CMD_PRIVMSG);
			// TODO: fix to channel and list (#chan or user1,user2)
			// privmsg #chan,nick
//			cmd << clients[i]->getNick() << msg;
			cmd << cmd.getParams()[0] << msg;
			server.send_command(cmd, clients[i]->getFd());
//			sendReply(server.getServername(), *clients[i], RPL_AWAY, clients[i]->getNick(), msg, "", "", "", "", "", "");
			if (!clients[i]->getAway().empty())
				sendReply(client, server, RPL_AWAY, clients[i]->getNick(), clients[i]->getAway());
		}
	}

	void cmd_notice(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> param = cmd.getParams(); //параметры
		size_t len = param.size(); //длина параметров
		if (param.empty()) {
			return;
		}
		if (!len) {
			return;
		}
		std::vector<Client *> clients = server.find_clients(cmd.getParams()[0], WITMSG, client); //ищем все ники
		std::string msg;

		for (int j = 1; j < len; ++j) { //собираем параметры для отправки
			msg += param[j];
			if (j != len - 1) {
				msg += ' ';
			}
		}
		for (int i = 0; i < clients.size(); ++i) {//отправляем
			Command cmd(client.get_full_name(), CMD_NOTICE);
			cmd << clients[i]->getNick() << msg;
			server.send_command(cmd, clients[i]->getFd());
//			sendReply(server.getServername(), *clients[i], RPL_AWAY, clients[i]->getNick(), msg, "", "", "", "", "", "");
		}
	}

	void cmd_away(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> param = cmd.getParams(); //параметры
		std::string msg;
		if (param.empty()) {
			sendReply(client, server, RPL_UNAWAY);
			client.clearAway();
		} else {
			for (int j = 0; j < param.size(); ++j) { //собираем параметры для отправки
				msg += param[j] + " ";
			}
			sendReply(client, server, RPL_NOWAWAY);
			client.setAway(msg);
		}
	}

	void cmd_join(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		size_t len = params.size();
//        std::vector<std::string> chani;
//        std::vector<std::string> keys;
		int res;
		int count = 0;

		if (params.empty()) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "JOIN", "");
			return;
		}

		std::vector<std::string> chans;
		{
			std::istringstream stream(params[0]);
			std::string line;
			while (std::getline(stream, line, ',')) {
				chans.push_back(line);
			}
		}

		std::vector<std::string> keys;
		if (params.size() > 1) {
			{
				std::istringstream stream(params[1]);
				std::string line;
				while (std::getline(stream, line, ',')) {
					keys.push_back(line);
				}
			}
		}
		for (int i = 0; i < chans.size(); ++i) {
			keys.push_back("");
		}

//        for (int i = 0; i < len; ++i) {
//            res = Channel::check_name(params[i]);
//            if (res == ERR_NAME)
//            {
//                sendError(client, server, ERR_NOSUCHCHANNEL, params[i], "");
//                // no such channel
//                return ;
//            }
//            else if (res == CHAN)
//            {
//                count++;
//                if (server.channels.find(params[i]) == server.channels.end())
//                {
//                    Channel channel;
//                    channel.setName(params[i]);
//                    server.channels.insert(std::make_pair(params[i], channel));
//                    channel.add_memeber(client);
//
//                }
//                chani.push_back(params[i]);
//
//            }
//            else
//            {
//                if (count < 1)
//                {
//                    //need more params
//                    sendError(client, server, ERR_NEEDMOREPARAMS, "JOIN", "");
//                    return ;
//                }
//                keys.push_back(params[i]);
//            }
//        }

		// JOIN #c1,#c2,#c3,#c4 pass1,,pass3
		for (unsigned int i = 0; i < chans.size(); ++i) {
			if (!Channel::check_name(chans[i])) {
				sendError(client, server, ERR_BADCHANMASK, chans[i], "");
				continue;
			}

			if (server.channels.find(chans[i]) == server.channels.end()) {
				server.channels.insert(std::make_pair(chans[i], Channel(chans[i])));
			}

			Channel &chan = server.channels.find(chans[i])->second;
			std::string const &key = chan.getKey();

			if (!key.empty() && keys[i] != key) {
				sendError(client, server, ERR_BADCHANNELKEY, chans[i], "");
				continue;
			}
			if (chan.isFlag(CMODE_INVITE)) {
				sendError(client, server, ERR_INVITEONLYCHAN, chans[i], "");
				continue;
			}
			if (!chan.check_limit()) {
				sendError(client, server, ERR_CHANNELISFULL, chans[i], "");
				continue;
			}
			chan.add_memeber(client);
			client.addChannel(chans[i]);

			if (!chan.getTopic().empty()) {
				sendReply(client, server, RPL_TOPIC, chans[i], chan.getTopic());
			}
			sendReply(client, server, RPL_NAMREPLY, chans[i], chan.get_names());
			sendReply(client, server, RPL_ENDOFNAMES, chans[i]);

//            if (i < keys.size())
//            {
//                if (!key.empty())
//                {
//                    if (key == keys[i]){
//                        if (!(chan.getFlags() & CMODE_INVITE)) {
//                            if (chan.check_limit()) {
//								chan.add_memeber(client);
//                            } else{
//                                sendError(client, server, ERR_CHANNELISFULL, chans[i], "");
//                            }
//                        }
//                        else{
//                            sendError(client, server, ERR_INVITEONLYCHAN, chans[i], "");
//                            return;
//                        }
//                    }
//                    else{
//
//                        sendError(client, server, ERR_BADCHANNELKEY, chans[i], "");
//                        return;
//                        //no key
//                    }
//                }
//                else
//                {
//                    server.channels[chans[i]].add_memeber(client);
//                }
//            } else {
//                if (!(server.channels[chans[i]].getFlags() & CMODE_INVITE)) {
//                    if (server.channels[chans[i]].check_limit()) {
//                        server.channels[chans[i]].add_memeber(client);
//
//						for (std::set<std::string const*>::iterator it = server.channels[chans[i]].opers.begin(); it != server.channels[chans[i]].opers.end(); ++it) {
//							std::cout << "[DEBUG]: " << *it << ": " << *(*it) << std::endl;
//						}
//						for (std::set<Client const*>::iterator it = server.channels[chans[i]].users.begin(); it != server.channels[chans[i]].users.end(); ++it) {
//							std::cout << "[DEBUG]: " << (*it)->getFd() << " " << &(*it)->nick << ": " << (*it)->getNick() << std::endl;
//						}
//
//                    } else {
//                        sendError(client, server, ERR_CHANNELISFULL, chans[i], "");
//                    }
//                }
//                else {
//                    sendError(client, server, ERR_INVITEONLYCHAN, chans[i], "");
//                    return;
//                }
//            }
//        }
//ERR_INVITEONLYCHAN
//ERR_CHANNELISFULL
//RPL_TOPIC
		}
	}

	void cmd_part(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams(); //параметры
//        std::vector<std::string> chani;
//        std::vector<std::string> keys;
		int res;
		int count = 0;

		if (params.empty()) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "PART", "");
			return;
		}

		std::vector<std::string> chans = split(params[0], ',');
		size_t len = chans.size();
		for (int i = 0; i < len; ++i) {
			if (server.channels.find(chans[i]) == server.channels.end()) {
				sendError(client, server, ERR_NOSUCHCHANNEL, chans[i], "");
				continue;
			}
			Channel &channel = server.channels.find(chans[i])->second;
			if (!Channel::check_name(chans[i])) {
				sendError(client, server, ERR_NOSUCHCHANNEL, chans[i], "");
				continue;
			}
			if (channel.users.find(&client) == channel.users.end()) {//если клиента нет на канале
				sendError(client, server, ERR_NOTONCHANNEL, chans[i], "");
				continue;
			}
			channel.erase_client(client);
			client.eraseChannel(chans[i]);
		}
	}

	void cmd_topic(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		std::string msg;

		if (params.empty()) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "TOPIC", "");
			return;
		}
		std::string chani = params[0]; //канал

		// std::vector<std::string> chans = split(params[0], ',');
		size_t len = params.size();
		// for(int i = 0; i < len; ++i)

		if (!Channel::check_name(chani)) {
			sendError(client, server, ERR_NOSUCHCHANNEL, chani, "");
			return;
		}
		if (server.channels.find(chani) == server.channels.end()) {
			sendError(client, server, ERR_NOSUCHCHANNEL, chani, "");
			return;
		}
		Channel &channel = server.channels.find(chani)->second;
		if (channel.users.find(&client) == channel.users.end()) {
			sendError(client, server, ERR_NOTONCHANNEL, chani, "");
			return;
		}
		// if (server.channels[chani].opers.find(&client) == server.channels[chani].users.end())
		// {
		// 	sendError(client, server, ERR_CHANOPRIVSNEEDED, chani, "");
		// 	return;
		// }
		// if (!chani.isFlag(CMODE_MODER))
		// {
		// 	sendError(client, server, ERR_NOCHANMODES, chani, "");
		// 	return;
		// }
		for (int j = 1; j < len; ++j) { //собираем параметры для отправки
			msg += params[j];
			if (j != len - 1) {
				msg += ' ';
			}
		}

		if (msg.empty() && params.size() == 1) {
			if (channel.getTopic().empty())
				sendReply(client, server, RPL_NOTOPIC, chani);
			else
				sendReply(client, server, RPL_TOPIC, chani, channel.getTopic());
		} else if (msg.empty()) {
			if (channel.isFlag(CMODE_TOPIC)) {
				if (channel.opers.find(&(client.getNick())) == channel.opers.end()
					&& !client.isFlag(UMODE_OPER)) {
					sendError(client, server, ERR_CHANOPRIVSNEEDED, chani);
					return;
				}
			}
			channel.clearTopic();//очищаем топик
		} else {
			if (channel.isFlag(CMODE_TOPIC)) {
				if (channel.opers.find(&(client.getNick())) == channel.opers.end()
					&& !client.isFlag(UMODE_OPER)) {
					sendError(client, server, ERR_CHANOPRIVSNEEDED, chani);
					return;
				}
			}
			channel.setTopic(msg);//устанавливаем топик
		}
	}

	void cmd_names(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		std::vector<std::string> chans;
//        std::vector<std::string> keys;
		int res;
		int count = 0;

		// NAMES
		// NAMES #c1,#c2
		if (!params.empty()) {
			chans = split(params[0], ',');
		} else {//если нет каналов выводим все
			for (std::map<std::string, Channel>::iterator it = server.channels.begin();
				 it != server.channels.end(); ++it)
				chans.push_back(it->first);
			count = 1;
		}
		size_t len = chans.size();
		for (int i = 0; i < len; ++i) {
			if (server.channels.find(chans[i]) == server.channels.end()) {
				continue;
			}
			Channel &channel = server.channels.find(chans[i])->second;
			if (!Channel::check_name(chans[i])) {
				continue;
			}
			if (channel.isFlag(CMODE_SECRET) && channel.users.find(&client) == channel.users.end()) {
				continue;
			}
			//остановился здесь
			sendReply(client, server, RPL_NAMREPLY, chans[i], channel.get_names());
			sendReply(client, server, RPL_ENDOFNAMES, chans[i]);
		}
		if (count == 1)// если мы не выводили тзбранные каналы
		{
			std::string cl;
			std::list<Client>::iterator it = server.clients.begin();
			for (; it != server.clients.end(); ++it) {
				if (it->getChannels().empty()) {
					cl += (*it).getNick() + " ";
				}
			}
			sendReply(client, server, RPL_NAMREPLY, "No channels", cl, "", "", "", "", "", "");
			sendReply(client, server, RPL_ENDOFNAMES, "No channels", "", "", "", "", "", "", "");
		}
	}

	void cmd_list(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		std::vector<std::string> chans;
//        std::vector<std::string> keys;
		int res;
		int count = 0;

		if (!params.empty()) {
			chans = split(params[0], ',');
		} else { //если нет каналов выводим все
			for (std::map<std::string, Channel>::iterator it = server.channels.begin();
				 it != server.channels.end(); ++it)
				chans.push_back(it->first);
			count = 1;
		}
		size_t len = chans.size();
		for (int i = 0; i < len; ++i) {
			if (server.channels.find(chans[i]) == server.channels.end()) {
				continue;
			}
			Channel &channel = server.channels.find(chans[i])->second;
			if (!Channel::check_name(chans[i])) {
				continue;
			}
			if (channel.isFlag(CMODE_SECRET) && channel.users.find(&client) == channel.users.end()) {
				continue;
			}
			//остановился здесь
//			if (count != 1)
//				chans[i].clear();
			std::stringstream ss;
			ss << channel.users.size();
			sendReply(client, server, RPL_LIST, chans[i], ss.str(), channel.getTopic());

		}
		sendReply(client, server, RPL_LISTEND);
	}

	void cmd_invite(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		std::vector<std::string> chans;
//        std::vector<std::string> keys;
		int res;
		int count = 0;

		if (params.empty() || params.size() < 2) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "INVITE", "");
			return;
		}
		if (!Channel::check_name(params[1])) {
			return;
		}
		if (server.channels.find(params[1]) == server.channels.end()) {
			return;
		}
		Channel &channel = server.channels.find(params[1])->second;
		std::list<Client>::iterator it = std::find_if(server.clients.begin(), server.clients.end(),
													  is_nickname(params[0]));
		if (it == server.clients.end()) {//если нет ника
			sendError(client, server, ERR_NOSUCHNICK, params[0], "");
			return;
		}
		if (channel.users.find(&client) == channel.users.end()) {//если нет меня на канале
			sendError(client, server, ERR_NOTONCHANNEL, params[1], "");
			return;
		}
		if (channel.users.find(&(*it)) != channel.users.end()) {//если чувак уже на канале
			sendError(client, server, ERR_USERONCHANNEL, params[0], params[1]);
			return;
		}
		if (channel.isFlag(CMODE_INVITE) && channel.opers.find(&client.getNick()) == channel.opers.end()
			&& !client.isFlag(UMODE_OPER)) {
			sendError(client, server, ERR_CHANOPRIVSNEEDED, params[1], "");//если нет привелегий
			return;
		}
		sendReply(client, server, RPL_INVITING, params[1], params[0]);
		Command invite(client.get_full_name(), CMD_INVITE);
		invite << it->getNick() << channel.getName();
		server.send_command(invite, it->getFd());
		channel.add_memeber(*it);
		it->addChannel(params[1]);
	}

	void cmd_kick(Command const &cmd, Client &client, ListenSocket &server) {
		//TODO: что то сделать с комментом надо
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		std::vector<std::string> chans;
		std::vector<std::string> nicks;

		if (params.empty() || params.size() < 2) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "KICK", "");
			return;
		}
		// KICK #c nick1,nick2
		// KICK #c1,#c2, nick1,nick2
		chans = split(params[0], ',');
		nicks = split(params[1], ',');
		size_t len = nicks.size();
		for (int i = 0; i < chans.size(); ++i) {
			if (server.channels.find(chans[i]) == server.channels.end()) {
				sendError(client, server, ERR_NOSUCHCHANNEL, chans[i], "");
				continue;
			}
			if (chans.size() == 1) { //если один канал тогда любое количество ников
				Channel &channel = server.channels.find(chans[0])->second;
				if (channel.users.find(&client) == channel.users.end()) {
					sendError(client, server, ERR_NOTONCHANNEL, chans[0], "");
					return;
				}
				if (channel.opers.find(&client.getNick()) == channel.opers.end()
					&& !client.isFlag(UMODE_OPER)) {
					sendError(client, server, ERR_CHANOPRIVSNEEDED, chans[0], "");//если нет привелегий
					return;
				}
				for (int j = 0; j < len; ++j) {
					std::list<Client>::iterator it = std::find_if(server.clients.begin(), server.clients.end(),
																  is_nickname(nicks[j]));
					if (channel.users.erase(&(*it)) == 0)// пытаемся удалить
					{
						sendError(client, server, ERR_USERNOTINCHANNEL, nicks[j], chans[0]);
						continue;
					}
					channel.opers.erase(&(*it).getNick());
					channel.voiced.erase(&(*it).getNick());
					it->eraseChannel(chans[0]);
				}
			} else//один канал один ник
			{
				Channel &channel = server.channels.find(chans[i])->second;
				if (channel.users.find(&client) == channel.users.end()) {
					sendError(client, server, ERR_NOTONCHANNEL, chans[i], "");
					continue;
				}
				if (channel.opers.find(&client.getNick()) == channel.opers.end()
					&& !client.isFlag(UMODE_OPER)) {
					sendError(client, server, ERR_CHANOPRIVSNEEDED, chans[i], "");//если нет привелегий
					continue;
				}
				if (nicks.size() <= i)
					return;
				std::list<Client>::iterator it = std::find_if(server.clients.begin(), server.clients.end(),
															  is_nickname(nicks[i]));
				if (channel.users.erase(&(*it)) == 0) {
					sendError(client, server, ERR_USERNOTINCHANNEL, nicks[i], chans[i]);
					continue;
				}
				channel.opers.erase(&(*it).getNick());
				channel.voiced.erase(&(*it).getNick());
				it->eraseChannel(chans[i]);
			}
		}
	}

	int check_num(const char *str) {
		for (int i = 0; i < strlen(str); ++i) {
			if (!std::isdigit(str[i]))
				return false;
		}
		int res = std::atoi(str);
		return res;
	}

	void cmd_mode(Command const &cmd, Client &client, ListenSocket &server) {
		//добавить выводи режимов на экран
		std::vector<std::string> const &params = cmd.getParams(); //параметры
//        std::vector<std::string> chans;
//        std::vector<std::string> nicks;
		if (params.empty()) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "MODE", "");
			return;
		}
		if (params[0][0] == '#') {
			Channel *channel = server.thisischannel(params[0], 0, client);
			if (channel == NULL)
				return;
			if (channel->opers.find(&client.getNick()) == channel->opers.end()
				&& !client.isFlag(UMODE_OPER)) {
				sendError(client, server, ERR_CHANOPRIVSNEEDED, params[0], "");//если нет привелегий
				return;
			}
			// MODE #chan +l 2
//			std::map<const char, size_t>::iterator mod;
//			mod = Channel::modes.find(params[1][1]);
//			size_t res = mod->second;
			std::map<const char, enum Channel::model>::iterator it = Channel::modes.find(params[1][1]);
			if (params[1].size() != 2 || it == Channel::modes.end()) {
				sendError(client, server, ERR_UNKNOWNMODE, params[1], params[0]);//если не подходит мод
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
						if (channel->isFlag(CMODE_INVITE))
							channel->zeroFlag(CMODE_INVITE);
					} else {
						if (!channel->isFlag(CMODE_INVITE))
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
						sendError(client, server, ERR_NEEDMOREPARAMS, "MODE", "");
						return;
					}
					std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
																  is_nickname(params[2]));
					if (to == server.clients.end()) {
						sendError(client, server, ERR_NOSUCHNICK, params[2], "");
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
							channel->opers.insert(&to->getNick());
						}
					} else {
						if (channel->opers.find(&nick) == channel->opers.end()
							&& !client.isFlag(UMODE_OPER)) {
							return;
						} else {
							channel->opers.erase(&to->getNick());
						}
					}
					break;
				}
				case Channel::V: {
					if (params.size() < 3) {
						sendError(client, server, ERR_NEEDMOREPARAMS, "MODE", "");
						return;
					}
					std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
																  is_nickname(params[2]));
					if (to == server.clients.end()) {
						sendError(client, server, ERR_NOSUCHNICK, params[2], "");
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
							channel->voiced.insert(&to->getNick());
						}
					} else {
						if (channel->voiced.find(&nick) == channel->voiced.end()) {
							return;
						} else {
							channel->voiced.erase(&to->getNick());
						}
					}
					break;
				}
				case Channel::K: {
					if (params.size() < 3 && sign == '+') {
						sendError(client, server, ERR_NEEDMOREPARAMS, "MODE", "");
						return;
					}
					if (sign == '+') {
						if (!channel->getKey().empty()) {
							sendError(client, server, ERR_KEYSET, params[0], "");
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
					if (params.size() < 3 && sign == '+') {
						sendError(client, server, ERR_NEEDMOREPARAMS, "MODE", "");
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
			Client *oclient = server.thisisnick(params[0], 0, client);
			if (oclient == NULL)
				return;
			std::string models = "iwo";
			if (params[1].size() != 2 || models.find(params[1][1]) == std::string::npos) {
				sendError(client, server, ERR_UMODEUNKNOWNFLAG, "", "");//если не подходит мод
				return;
			}
			char sign = params[1][0];
			char mod = params[1][1];
			if (sign != '-' && sign != '+') {
				sendError(client, server, ERR_UMODEUNKNOWNFLAG, "", "");//если не подходит мод
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
				if (client.getNick() != oclient->getNick()) {
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
					sendError(client, server, ERR_NOPRIVILEGES);
				}
			}
		}
	}

	void cmd_oper(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		std::map<std::string, std::string>::iterator pass;
//        std::vector<std::string> keys;
		int res;
		int count = 0;

		if (params.empty() || params.size() != 2) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "OPER", "");
			return;
		}
		if (server.opers.empty()) { //если мапа пустая значит оперов не может быть
			sendError(client, server, ERR_NOOPERHOST, "", "");
			return;
		}
		pass = server.opers.find(params[0]);
		if (pass == server.opers.end() || pass->second != params[1]) {
			sendError(client, server, ERR_PASSWDMISMATCH, "", "");
			return;
		}
		sendReply(client, server, RPL_YOUREOPER);
		client.setFlag(UMODE_OPER);
	}

	void cmd_kill(Command const &cmd, Client &client, ListenSocket &server) {
		//TODO: что то сделать с комментом надо
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		std::vector<std::string> pass;
		int res;
		int count = 0;

		if (params.empty() || params.size() != 2) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "KILL", "");
			return;
		}

		if (!(client.getFlags() & UMODE_OPER)) {
			sendError(client, server, ERR_NOPRIVILEGES, "", "");
			return;
		}
		std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
													  is_nickname(params[0]));
		if (to == server.clients.end()) {
			sendError(client, server, ERR_NOSUCHNICK, params[0], "");
			return;
		}
		if (params[0] == server.getServername()) {
			sendError(client, server, ERR_CANTKILLSERVER, "", "");
			return;
		}
		server.quit_client(to->getFd());
	}

	void cmd_admin(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> params = cmd.getParams(); //параметры

		if (params.empty())
			params[0] = server.getServername();
		if (params[0] != server.getServername()) {
			sendError(client, server, ERR_NOSUCHSERVER, params[0], "");
			return;
		}
		sendReply(client, server, RPL_ADMINME, params[0]);
		sendReply(client, server, RPL_ADMINLOC1, server.admin["adminName"]);
		sendReply(client, server, RPL_ADMINLOC2, server.admin["adminNickname"]);
		sendReply(client, server, RPL_ADMINEMAIL, server.admin["adminEmail"]);
	}
}