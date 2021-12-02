//
// Created by Frey Tinkerer on 11/16/21.
//

#include "commands.hpp"

//TODO:прописать ответ на  выполненые задачи для сервера
//TODO:сделать ли проверку на инвизбл для привмсг и нотис и join????
//TODO:сделать ответы клиентам при выполнении команд
//TODO:проверить регистрацию
//TODO: подумать над порядком whowas
//TODO: добавление в базу админа и сервера

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
		// TODO check registered
		// check nick else reply 432 :Erroneous Nickname
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
		// check user
		if (cmd.getParams().empty() || cmd.getParams().size() < 4) {
			sendError(client, server, ERR_NEEDMOREPARAMS, "USER", "");
		} else if (client.getFlags() & UMODE_REGISTERED) {
			sendError(client, server, ERR_ALREADYREGISTRED, "", "");
		} else {
			client.user = cmd.getParams()[0];
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
		std::cout << "[DEBUG]: " << client.nick << "!" << client.user << "@" << client.host << " " << cmd.getCommand()
				  << ": " << quit << std::endl;
	}
}