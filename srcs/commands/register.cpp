//
// Created by Frey Tinkerer on 11/16/21.
//

#include "Command.hpp"
#include "Client.hpp"
#include "ListenSocket.hpp"

namespace IRC {

	void cmd_pass(Command & cmd, Client& client, ListenSocket& server) {
		std::vector<std::string> const& params = cmd.getParams();
		if (params.empty()) {
		    sendError(client, server, ERR_NEEDMOREPARAMS, "PASS", "");
			// reply not args
		}
		else if (client.getFlags() & UMODE_REGISTERED)
		{
		    sendError(client, server, ERR_ALREADYREGISTRED, "", "");
		}
		else
		{
			client.pass = cmd.getParams()[0];
		}
	}

	void cmd_nick(Command & cmd, Client& client, ListenSocket& server) {
		// check nick else reply 432 :Erroneous Nickname
		Client *to = std::find_if(server.clients.begin(), server.clients.end(),
                                  is_nickname(cmd.getParams()[0])).base();
		if (cmd.getParams().empty())
		{
		    sendError(client, server, ERR_NONICKNAMEGIVEN, "", "");
		    return;
		}// else reply not args
		if (!client._name_control(cmd.getParams()[0], 0)) //беру только первый аргумент
		{
		    sendError(client, server, ERR_ERRONEUSNICKNAME, cmd.getParams()[0], "");
		    return;
		}
		if (to != server.clients.end().base())
		{
		    sendError(client, server, ERR_NICKNAMEINUSE, cmd.getParams()[0], "");
		    return;
		}
		client.nick = cmd.getParams()[0];
		client.try_register(server);
	}

	void cmd_user(Command & cmd, Client& client, ListenSocket& server) {
		// check user
		if (cmd.getParams().empty() || cmd.getParams().size() < 4)
		{
		    sendError(client, server, ERR_NEEDMOREPARAMS, "USER", "");
		}
		else if (client.getFlags() & UMODE_REGISTERED)
		{
		    sendError(client, server, ERR_ALREADYREGISTRED, "", "");
		}
		else{
			client.user = cmd.getParams()[0];
			client.try_register(server);
		} // else reply not args
	}

	void cmd_quit(Command & cmd, Client& client, ListenSocket& server) {
	    server.quit_client(client.getFd());
	    std::cout << "[DEBUG]: " << client.nick << "!" << client.user << "@" << client.host << " " << cmd.getCommand() << ": " << cmd.getParams()[0] << std::endl;
	}

}