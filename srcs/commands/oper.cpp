//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"
#include "Parser.hpp"
#include <Reply.hpp>
#include <algorithm>
#include <sstream>

namespace IRC {

	void cmd_mode(Command const &cmd, Client &client, ListenSocket &server) {
		//добавить выводи режимов на экран
		// TODO: полноценный парсинг сложных модов (-bl+i)
		std::vector<std::string> const &params = cmd.getParams();//параметры

		if (params.empty()) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
			return;
		}
		if (params[0][0] == '#') {
			Channel *channel = check_channel(params[0], server, client, 0);
			if (channel == NULL)
				return;
			if (params.size() == 1) {
				//выводим инфу об активных модах
				mode_table(channel, client, server, params[0]);
				return;
			}
			if (!priv_need_channel(channel, client, server, params[0])) {return;}

			std::map<const char, enum Channel::model>::iterator mod;
			mod = Channel::modes.find(params[1][1]);//ищем мод в мапе
			if (params[1].size() != 2 || mod == channel->modes.end()) {
				sendError(client, server, ERR_UNKNOWNMODE, params[1], params[0]);//если не подходит мод
				return;
			}
			size_t res = mod->second;
			char sign = params[1][0];
			if (sign != '-' && sign != '+') {
				sendError(client, server, ERR_UNKNOWNMODE, params[1], params[0]);//если не подходит мод
				return;
			}
			switch (res) {
				case Channel::I:
					mode_flags(channel, CMODE_INVITE, sign);
					break;
				case Channel::M:
					mode_flags(channel, CMODE_MODER, sign);
					break;
				case Channel::S:
					mode_flags(channel, CMODE_SECRET, sign);
					break;
				case Channel::N:
					mode_flags(channel, CMODE_NOEXT, sign);
					break;
				case Channel::T:
					mode_flags(channel, CMODE_TOPIC, sign);
					break;
				case Channel::O: {
					Client *moded = check_mode_nick(channel, client, server, params[2], params[0], params.size());
					if (moded == NULL)
						return;
					mode_flags_chan_nick(moded, channel->opers, sign);
					break;
				}
				case Channel::V: {
					Client *moded = check_mode_nick(channel, client, server, params[2], params[0], params.size());
					if (moded == NULL)
						return;
					mode_flags_chan_nick(moded, channel->voiced, sign);
					break;
				}
				case Channel::K: {
					if (!mode_flags_keys(channel, client, server, sign, params.size(), params[2], params[0]))
						return ;
					break;
				}
				case Channel::L: {
					if (!mode_flags_limit(channel, client, server, sign, params.size(), params[2], params[0]))
						return ;
					break;
				}
			}
			std::string param = res == Channel::K ? "***" : (params.size() > 2 ? params[1] : "");
			Command mode(client.get_full_name(), CMD_MODE, params[0], params[1], param);
			for (channel_client_iter it = channel->users.begin(); it != channel->users.end(); ++it) {
				server.send_command(mode, **it);
			}

		} else {
			int flag = 0;
			if (params.size() == 1)
				flag = 1;//если хочешь просто посмотреть
			Client *oclient = server.thisisnick(params[0], flag, client);
			if (oclient == NULL)
				return;
			if (params.size() == 1) {
				mode_table_nicks(oclient, client, server);
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
				if (mod == 'i')
					mode_flags_nick(oclient, UMODE_INVIS, sign);
				else if (mod == 'w')
					mode_flags_nick(oclient, UMODE_WALLOPS, sign);
				else if (mod == 'o')
					mode_flags_nick(oclient, UMODE_OPER, sign);
			} else if (params[0] == oclient->getNick()) {
				if (mod == 'i')
					mode_flags_nick(oclient, UMODE_INVIS, sign);
				else if (mod == 'w')
					mode_flags_nick(oclient, UMODE_WALLOPS, sign);
				else if (mod == 'o'){
					if (sign == '+') {
						sendError(client, server, ERR_USERSDONTMATCH);//если не опер то не можешь редактировать чужой ник
						return;
					} else
						oclient->zeroFlag(UMODE_OPER);
				}
				Command mode(client.get_full_name(), CMD_MODE, params[0], params[1]);
				server.send_command(mode, *oclient);
				if (oclient->getNick() != client.getNick()) {
					server.send_command(mode, client);
				}
			} else {
				sendError(client, server, ERR_USERSDONTMATCH);//если не опер то не можешь редактировать чужой ник
				return;
			}
		}
	}

    void cmd_oper(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::map<std::string, std::string>::iterator pass;

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
		std::vector<std::string> const &params = cmd.getParams();//параметры
		std::vector<std::string> pass;

		if (params.empty() || params.size() != 2) {
			sendError(client, server, ERR_NEEDMOREPARAMS, CMD_KILL);
			return;
		}

		if (!(client.isFlag(UMODE_OPER))) {
			sendError(client, server, ERR_NOPRIVILEGES);
			return;
		}
//		client_iter it = check_mask_nick(ERR_CANTKILLSERVER, params[0], client, server);
		client_iter it = server.getClient(params[0]);
		if (!server.isClientExist(it)) {
			sendError(client, server, ERR_NOSUCHNICK, params[0]);
			return;
		}
		server.send_command(Command(client.get_full_name(), CMD_KILL, params[0], params[1]), client);
		it->disconnect("Killed (" + client.getNick() + " (" + params[1] + "))");
	}
}// namespace IRC