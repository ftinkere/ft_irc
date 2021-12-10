//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"
#include <Parser.hpp>
#include <Reply.hpp>
#include <algorithm>
#include <sstream>

namespace IRC {

	void cmd_join(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		if (!check_params(client, server, params, CMD_JOIN)) { return; }

		std::vector<std::string> chans = split(params[0], ',');
		std::vector<std::string> keys;
		if (params.size() > 1)
			keys = split(params[1], ',');
		for (int i = 0; i < chans.size(); ++i) {
			keys.push_back("");
		}

		for (unsigned int i = 0; i < chans.size(); ++i) {
			if (!Channel::check_name(chans[i])) {
				sendError(client, server, ERR_BADCHANMASK, chans[i]);
				continue;
			}
			if (!server.isChannelExist(chans[i])) {
				server.addNewChannel(chans[i]);
			}

			Channel &chan = server.getChannel(chans[i])->second;
			std::string const &key = chan.getKey();

			if (!key.empty() && keys[i] != key) {
				sendError(client, server, ERR_BADCHANNELKEY, chans[i]);
				continue;
			}
			if (chan.isFlag(CMODE_INVITE)) {
				sendError(client, server, ERR_INVITEONLYCHAN, chans[i]);
				continue;
			}
			if (!chan.check_limit()) {
				sendError(client, server, ERR_CHANNELISFULL, chans[i]);
				continue;
			}

			Command join(client.get_full_name(), CMD_JOIN, chans[i]);
			server.send_command(join, client);
			for (std::set<const std::string*>::iterator it = chan.opers.begin(); it != chan.opers.end(); ++it) {
				server.send_command(join, **it);
			}

			if (!check_join_chan(client, server, chan, keys[i])) { continue; }
			chan.add_memeber(client);
			client.addChannel(chans[i]);

			if (!chan.getTopic().empty()) {
				sendReply(client, server, RPL_TOPIC, chans[i], chan.getTopic());
				// TODO: TOPICWHOTIME 333
			}
			sendReply(client, server, RPL_NAMREPLY, chan.isFlag(CMODE_SECRET) ? "@" : "=", chans[i], chan.get_names());
			sendReply(client, server, RPL_ENDOFNAMES, chans[i]);
			//проверка на невидимость при отправке инфы вновь прибывшему
		}
	}

	void cmd_part(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		if (!check_params(client, server, params, CMD_PART)) { return; }

		std::vector<std::string> chans = split(params[0], ',');
		size_t len = chans.size();
		for (int i = 0; i < len; ++i) {
			Channel *channel = check_channel(chans[i], server, client, 1);
			if (channel == NULL) { continue; }
			channel->erase_client(client);
			client.eraseChannel(chans[i]);
		}
	}

	void cmd_topic(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams();//параметры
		if (!check_params(client, server, params, CMD_TOPIC)) { return; }

		std::string chani = params[0];//канал
		size_t len = params.size();

		Channel *channel = check_channel(chani, server, client, 1);
		if (channel == NULL) { return; }
		std::string msg = choose_str(params, len, 1);

		if (msg.empty() && params.size() == 1) {//если параметры пустые и стоит :
			if (channel->getTopic().empty())    //отправляем топик клиенту
				sendReply(client, server, RPL_NOTOPIC, chani);
			else
				sendReply(client, server, RPL_TOPIC, chani, channel->getTopic());
			return;
		} else if (msg.empty()) {//если просто пустые параметры
			if (channel->isFlag(CMODE_TOPIC)) {
				if (!priv_need_channel(channel, client, server, chani)) { return; }
			}
			channel->clearTopic();//очищаем топик
		} else {                  //если есть параметры
			if (channel->isFlag(CMODE_TOPIC)) {
				if (!priv_need_channel(channel, client, server, chani)) { return; }
			}
			channel->setTopic(msg);//устанавливаем топик
		}
		Command topic(client.get_full_name(), CMD_TOPIC);
		topic << msg;
		for (channel_client_iter it = channel->users.begin(); it != channel->users.end(); ++it) {
			server.send_command(topic, **it);
		}
	}

	void cmd_list(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams();//параметры

		std::vector<channel_iter> mas_it;
		if (!params.empty()) {
			std::vector<std::string> chans = split(params[0], ',');
			size_t len = chans.size();
			for (int i = 0; i < len; ++i) {
				channel_iter it = server.getChannel(chans[i]);
				if (!server.isChannelExist(it))
					continue;
				mas_it.push_back(it);
			}
		} else {
			//если нет каналов выводим все
			channel_iter it;
			for (it = server.channels.begin(); it != server.channels.end(); ++it)
				mas_it.push_back(it);
		}

		sendReply(client, server, RPL_LISTSTART);
		std::vector<channel_iter>::iterator vec_it;
		for (vec_it = mas_it.begin(); vec_it != mas_it.end(); ++vec_it) {
			Channel &channel = (*vec_it)->second;
			if (channel.isFlag(CMODE_SECRET))
				continue;
			std::stringstream ss;
			ss << channel.users.size();
			sendReply(client, server, RPL_LIST, (*vec_it)->first, ss.str(), channel.getTopic());
		}
		sendReply(client, server, RPL_LISTEND);
	}

	void cmd_invite(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams();//параметры
		if (!check_params(client, server, params, CMD_INVITE, 2)) { return; }

		std::string chani = params[1]; //канал
		std::string nick = params[0];

		Channel *channel = check_channel(chani, server, client, 1);
		if (channel == NULL) { return; }

		client_iter it = server.getClient(nick);
		if (!check_nick(it, channel, client, server, nick, chani)) { return; }
		if (!priv_need_channel(channel, client, server, nick)) { return; }

		sendReply(client, server, RPL_INVITING, chani, nick);
		server.send_command(Command(client.get_full_name(), CMD_INVITE, nick, chani), *it);
		channel->add_memeber(*it);
		it->addChannel(chani);
	}

	void cmd_kick(Command const &cmd, Client &client, ListenSocket &server) {
		//TODO: что то сделать с комментом надо
		std::vector<std::string> const &params = cmd.getParams(); //параметры
		if (!check_params(client, server, params, CMD_KICK, 2)) { return; }

		std::string comment = choose_str(params, params.size(), 2);

		std::vector<std::string> chans = split(params[0], ',');
		std::vector<std::string> nicks = split(params[1], ',');
		size_t len = nicks.size();
		for (int i = 0; i < chans.size(); ++i) {
			if (chans.size() == 1) { //если один канал тогда любое количество ников
				Channel *channel = check_channel(chans[0], server, client, 1);
				if (channel == NULL) { return; }
				if (!priv_need_channel(channel, client, server, chans[0])) { return; }

				for (int j = 0; j < len; ++j) {
					if (erase_member(channel, client, server, nicks[j], chans[0])) {
						Command kick(client.get_full_name(), CMD_KICK, channel->getName(), nicks[j], comment);
						server.send_command(kick, nicks[j]);
						for (channel_client_iter it = channel->users.begin(); it != channel->users.end(); ++it) {
							server.send_command(kick, **it);
						}
					}
				}
			} else//один канал один ник
			{
				Channel *channel = check_channel(chans[i], server, client, 1);
				if (channel == NULL) { continue; }
				if (!priv_need_channel(channel, client, server, chans[i])) { continue; }
				if (nicks.size() <= i) { return; }
				if (erase_member(channel, client, server, nicks[i], chans[i])) {
					Command kick(client.get_full_name(), CMD_KICK, channel->getName(), nicks[i], comment);
					server.send_command(Command(client.get_full_name(), CMD_KICK, channel->getName(), nicks[i], comment), nicks[i]);
					for (channel_client_iter it = channel->users.begin(); it != channel->users.end(); ++it) {
						server.send_command(kick, **it);
					}
				}
			}
		}
	}
}// namespace IRC
