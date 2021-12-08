//
// Created by ftinkere on 08.12.2021.
//

#include "Command.hpp"
#include "Parser.hpp"
#include <Reply.hpp>

namespace IRC {

	void cmd_ping(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const& params = cmd.getParams(); //параметры

		if (params.empty()) {
			sendError(client, server, ERR_NEEDMOREPARAMS, CMD_PING);
			return;
		}
		server.send_command(client, CMD_PONG, params[0]);
	}

	void cmd_pong(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const& params = cmd.getParams(); //параметры

		if (params.empty()) {
			sendError(client, server, ERR_NEEDMOREPARAMS, CMD_PING);
			return;
		}
		client.touchPingpongTime();
		client.unsetPinged();
	}
}