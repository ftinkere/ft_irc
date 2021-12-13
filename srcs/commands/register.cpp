//
// Created by Frey Tinkerer on 11/16/21.
//

#include "commands.hpp"
#include "Parser.hpp"
#include <Reply.hpp>
#include <algorithm>

//TODO: прописать ответ на выполненые задачи для сервера — отправлять сами сообщения если нужно
//TODO: сделать ответы клиентам при выполнении команд — чем это отличается от выше?
//TODO: разобраться с комментами — так отправляются другим людям с командами

namespace IRC {

	class Channel;

	void cmd_pass(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams();
		if (params.empty()) {
			sendError(client, server, ERR_NEEDMOREPARAMS, CMD_PASS);
		} else if (client.isFlag(UMODE_REGISTERED)) {
			sendError(client, server, ERR_ALREADYREGISTRED);
		} else {
			client.setPass(params[0]);
		}
	}

	void cmd_nick(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams();
		if (params.empty()) {
			sendError(client, server, ERR_NONICKNAMEGIVEN);
			return;
		} else if (!client._name_control(params[0], 0)) {
			sendError(client, server, ERR_ERRONEUSNICKNAME, params[0]);
			return;
		} else if (server.isClientExist(params[0])) {
			sendError(client, server, ERR_NICKNAMEINUSE, params[0]);
			return;
		}
		if (!client.getNick().empty()) {
			Command nick(client.get_full_name(), CMD_NICK, params[0]);
			send_command_to_sharing_channel(nick, client, server);
		}
		client.setNick(params[0]);
		if (!client.isFlag(UMODE_REGISTERED))
			client.try_register(server);
	}

	void cmd_user(Command const &cmd, Client &client, ListenSocket &server) {
		// check user
		//TODO: обработать второй аргумент — НАХУЯ? Просто игнор
		std::vector<std::string> const &params = cmd.getParams();
		if (params.empty() || params.size() < 4) {
			sendError(client, server, ERR_NEEDMOREPARAMS, CMD_USER);
		} else if (client.isFlag(UMODE_REGISTERED)) {
			sendError(client, server, ERR_ALREADYREGISTRED);
		} else {
			client.setUser(params[0]);
			client.setRealname(params[3]);
			client.try_register(server);
		}
	}

	void cmd_quit(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams();

		if (params.empty()) {
			client.disconnect();
		} else {
			client.disconnect(params[0]);
		}
		std::cout << "[DEBUG]: " << client.get_full_name() << " " << cmd.getCommand()
				  << ": " << (params.empty() ? "quit" : params[0]) << std::endl;
	}

}