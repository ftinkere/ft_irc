//
// Created by Frey Tinkerer on 11/16/21.
//

#ifndef FT_IRC_COMMANDS_HPP
#define FT_IRC_COMMANDS_HPP

#include "Command.hpp"
#include "Client.hpp"
#include "ListenSocket.hpp"
#include <sstream>

//class Client;

namespace IRC {

    class Command;
    class ListenSocket;
    class Client;

void cmd_pass(Command  &cmd, Client &client, ListenSocket &server);
void cmd_nick(Command  &cmd, Client &client, ListenSocket &server);
void cmd_user(Command  &cmd, Client &client, ListenSocket &server);
void cmd_quit(Command  &cmd, Client &client, ListenSocket &server);

}
#endif //FT_IRC_COMMANDS_HPP
