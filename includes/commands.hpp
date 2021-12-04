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

    //вспомогательные функции
    int check_num(const char *str);
    std::string choose_str(std::vector<std::string> const &params, size_t len, int j);
    Channel* check_channel(std::string const &chans, ListenSocket &server, Client & client, int visible);
    bool priv_need_channel(Channel *channel, Client const &client, ListenSocket &server, std::string const &chani);
    bool check_nick(std::list<Client>::iterator& it, Channel *channel, Client const &client, ListenSocket &server, std::string const &nick, std::string const &chani);
    bool erase_member(Channel *channel, Client const &client, ListenSocket &server, std::string const &nick, std::string const &chani);
    void mode_table(Channel *channel, Client const &client, ListenSocket &server, std::string const &chani);
    void mode_flags(Channel *channel, int flag, int const &sign);
    Client* check_mode_nick(Channel *channel, Client const &client, ListenSocket &server, std::string const &nick, std::string const &chan, size_t const &len);
    void mode_flags_chan_nick(Client *moded, std::set<std::string const*> &members, int const &sign);
    bool mode_flags_keys(Channel *channel, Client const &client, ListenSocket &server, int const &sign, size_t const &len, std::string const &nick, std::string const &chan);
    bool mode_flags_limit(Channel *channel, Client const &client, ListenSocket &server, int const &sign, size_t const &len, std::string const &nick, std::string const &chan);
    void mode_table_nicks(Client *oclient, Client const &client, ListenSocket &server);
    void mode_flags_nick(Client *oclient, int flag, int const &sign);
    
    //Массив комманд сервера
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
    void cmd_whois(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_wallops(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_whowas(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_ping(Command const &cmd, Client &client, ListenSocket &server);
    void cmd_pong(Command const &cmd, Client &client, ListenSocket &server);

}
#endif //FT_IRC_COMMANDS_HPP
