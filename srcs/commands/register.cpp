//
// Created by Frey Tinkerer on 11/16/21.
//

#include "commands.hpp"

//TODO:прописать ответ на  выполненые задачи для сервера
//TODO:сделать ответы клиентам при выполнении команд
//TODO: подумать над порядком whowas
//TODO: добавление в базу админа и сервера
//TODO: ping pong

namespace IRC {

	class Channel;

	void cmd_pass(Command const &cmd, Client &client, ListenSocket &server) {
		std::vector<std::string> const &params = cmd.getParams();
		if (params.empty()) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "PASS");
			// reply not args
		} else if (client.isFlag(UMODE_REGISTERED)) {
			sendError(client, server, ERR_ALREADYREGISTRED);
		} else {
		    client.setPass(params[0]);
		}
	}

	void cmd_nick(Command const &cmd, Client &client, ListenSocket &server) {
		// check nick else reply 432 :Erroneous Nickname
		std::vector<std::string> const &params = cmd.getParams();
		std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
													  is_nickname(params[0]));
		if (params.empty()) {
			sendError(client, server, ERR_NONICKNAMEGIVEN);
			return;
		}// else reply not args
		if (!client._name_control(params[0], 0)) //беру только первый аргумент
		{
		    sendError(client, server, ERR_ERRONEUSNICKNAME, params[0]);
			return;
		}
		if (to != server.clients.end()) {
		    sendError(client, server, ERR_NICKNAMEINUSE, params[0]);
			return;
		}
		client.setNick(params[0]);
		if (!client.isFlag(UMODE_REGISTERED))
		    client.try_register(server);
	}

	void cmd_user(Command const &cmd, Client &client, ListenSocket &server) {
		// check user
		std::vector<std::string> const &params = cmd.getParams();
		if (params.empty() || params.size() < 4) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "USER");
		} else if (client.isFlag(UMODE_REGISTERED)) {
			sendError(client, server, ERR_ALREADYREGISTRED);
		} else {
		    client.setUser(params[0]);
			client.try_register(server);
		} // else reply not args
	}

	void cmd_quit(Command const &cmd, Client &client, ListenSocket &server) {
	    std::vector<std::string> const &params = cmd.getParams();
	    std::string quit;
		server.quit_client(client.getFd());
		if (params.empty()) {
		    quit = "quit";
		} else
		    quit = params[0];
		std::cout << "[DEBUG]: " << client.getNick() << "!" << client.getUser() << "@" << client.getHost() << " " << cmd.getCommand()
				  << ": " << quit << std::endl;
	}
}