//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"

namespace IRC {

    int check_num(const char *str) {
        //"Функция проверки числа"
        for (int i = 0; i < strlen(str); ++i) {
            if (!std::isdigit(str[i]))
                return false;
        }
        int res = std::atoi(str);
        return res;
    }

    std::string choose_str(std::vector<std::string> const &params, size_t len, int j) {
        //"Функция сборки последних параметров для отправки"
        std::string msg;
        for (; j < len; ++j) { //собираем параметры для отправки
            msg += params[j];
            if (j != len - 1) {
                msg += ' ';
            }
        }
        return msg;
    }

    Channel* check_channel(std::string const &chans, ListenSocket &server, Client & client, int visible) {
        //"Функция проверки наличия канала"
        std::map<std::string, Channel>::iterator it = server.channels.find(chans);
        if (it == server.channels.end()) {
            sendError(client, server, ERR_NOSUCHCHANNEL, chans);
            return NULL;
        }
        Channel &channel = it->second;

        if (visible == 1){
            if (channel.users.find(&client) == channel.users.end()) {//если клиента нет на канале
                sendError(client, server, ERR_NOTONCHANNEL, chans);
                return NULL;
            }
        }
//        if (visible == 2)
//        {
//            if (channel.isFlag(CMODE_SECRET))
//                return NULL;
//        }
        return &channel;
    }

    bool priv_need_channel(Channel *channel, Client const &client, ListenSocket &server, std::string const &chani)
    {
        if (channel->opers.find(&(client.getNick())) == channel->opers.end()) {
            sendError(client, server, ERR_CHANOPRIVSNEEDED, chani);
            return false;
        }
        return true;
    }

    bool check_nick(std::list<Client>::iterator& it, Channel *channel, Client const &client, ListenSocket &server, std::string const &nick, std::string const &chani)
    {
        if (it == server.clients.end()) {//если нет ника
            sendError(client, server, ERR_NOSUCHNICK, nick);
            return false;
        }
        if (channel->users.find(&(*it)) != channel->users.end()) {//если чувак уже на канале
            sendError(client, server, ERR_USERONCHANNEL, nick, chani);
            return false;
        }
        return true;
    }

    bool erase_member(Channel *channel, Client const &client, ListenSocket &server, std::string const &nick, std::string const &chani)
    {
        std::list<Client>::iterator it = std::find_if(server.clients.begin(), server.clients.end(),
                                                      is_nickname(nick));
        if (channel->users.erase(&(*it)) == 0)
        {// пытаемся удалить
            sendError(client, server, ERR_USERNOTINCHANNEL, nick, chani);
            return false;
        }
        channel->opers.erase(&(*it).getNick());
        channel->voiced.erase(&(*it).getNick());
        it->eraseChannel(chani);
        return true;
    }
}