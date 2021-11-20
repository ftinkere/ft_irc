//
// Created by Frey Tinkerer on 11/16/21.
//

#include <Command.hpp>
#include "Client.hpp"
#include "ListenSocket.hpp"

namespace IRC {

	void cmd_pass(Command const& cmd, Client& client, ListenSocket& server) {
		std::vector<std::string> const& params = cmd.getParams();
		if (params.empty()) {
			// reply not args
		} else {
			client.pass = cmd.getParams()[0];
		}
	}

	void cmd_nick(Command const& cmd, Client& client, ListenSocket& server) {
		// check nick else reply 432 :Erroneous Nickname
		if (!cmd.getParams().empty()) {
			client.nick = cmd.getParams()[0];
			client.try_register(server);
		} // else reply not args
	}

	void cmd_user(Command const& cmd, Client& client, ListenSocket& server) {
		// check user
		if (!cmd.getParams().empty()) {
			client.user = cmd.getParams()[0];
			client.try_register(server);
		} // else reply not args
	}

}