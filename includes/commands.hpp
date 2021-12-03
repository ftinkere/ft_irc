//
// Created by Frey Tinkerer on 11/16/21.
//

#ifndef FT_IRC_COMMANDS_HPP
#define FT_IRC_COMMANDS_HPP

#include "Command.hpp"
#include "Client.hpp"
#include "ListenSocket.hpp"
#include "Reply.hpp"
#include "Parser.hpp"
#include "Channel.hpp"
#include <sstream>

//class Command;
//class ListenSocket;
//class Client;
//class Channel;

namespace IRC {

	void cmd_pass(Command const &cmd, Client &client, ListenSocket &server);
	void cmd_nick(Command const &cmd, Client &client, ListenSocket &server);
	void cmd_user(Command const &cmd, Client &client, ListenSocket &server);
	void cmd_quit(Command const &cmd, Client &client, ListenSocket &server);
	void cmd_privmsg(Command const &cmd, Client &client, ListenSocket &server);
	void cmd_notice(Command const &cmd, Client &client, ListenSocket &server);
	void cmd_away(Command const &cmd, Client &client, ListenSocket &server);
	void cmd_join(Command const &cmd, Client &client, ListenSocket &server);
	void cmd_part(Command const &cmd, Client &client, ListenSocket &server);
	void cmd_topic(Command const &cmd, Client &client, ListenSocket &server);
	void cmd_names(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_list(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_invite(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_kick(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_mode(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_oper(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_kill(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_admin(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_ping(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_pong(Command const &cmd, Client &client, ListenSocket &server);

}
#endif //FT_IRC_COMMANDS_HPP
