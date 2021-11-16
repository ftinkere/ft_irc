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
		}
		client.pass = cmd.getParams()[0];
	}

	void cmd_nick(Command const& cmd, Client& client, ListenSocket& server) {

	}

	void cmd_user(Command const& cmd, Client& client, ListenSocket& server) {

	}

}