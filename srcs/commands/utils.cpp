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
    //функция проверки прав оператора канала
    {
        if (channel->opers.find(&(client.getNick())) == channel->opers.end()) {
            sendError(client, server, ERR_CHANOPRIVSNEEDED, chani);
            return false;
        }
        return true;
    }

    bool check_nick(std::list<Client>::iterator& it, Channel *channel, Client const &client, ListenSocket &server, std::string const &nick, std::string const &chani)
    //проверка ника в канале
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
    //удаление члена канала
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

    void mode_table(Channel *channel, Client const &client, ListenSocket &server, std::string const &chani)
    //вывод модов канала или ника
    {
        std::string res;
        if (channel->isFlag(CMODE_INVITE))
        {
            res.push_back(' ');
            res.push_back(INVIT);
        }
        if (channel->isFlag(CMODE_MODER))
        {
            res.push_back(' ');
            res.push_back(MODES);
        }
        if (channel->isFlag(CMODE_SECRET))
        {
            res.push_back(' ');
            res.push_back(SECRET);
        }
        if (channel->isFlag(CMODE_NOEXT))
        {
            res.push_back(' ');
            res.push_back(SPEAK);
        }
        if (channel->isFlag(CMODE_TOPIC)){
            res.push_back(' ');
            res.push_back(TOPIC);
        }
        if (!channel->getKey().empty()){
            res.push_back(' ');
            res.push_back(KEY);
            res.push_back('=');
            res += channel->getKey();
        }
        if (channel->getLimit() > 0){
            res.push_back(' ');
            res.push_back(LEN);
            res.push_back('=');
            res += std::to_string(channel->getLimit());
        }
        sendReply(server.getServername(), client, RPL_CHANNELMODEIS, chani, res);
    }

    void mode_flags(Channel *channel, int flag, int const &sign)
    //удаление члена канала
    {
        if (sign == '-') {
            if (channel->isFlag(flag))
                channel->zeroFlag(flag);
        } else {
            if (!channel->isFlag(flag))
                channel->setFlag(flag);
        }
    }

    Client* check_mode_nick(Channel *channel, Client const &client, ListenSocket &server, std::string const &nick, std::string const &chan, size_t const &len)
    //проверка ника в канале для модов
    {
        if (len < 3) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
            return NULL;
        }
        std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
                                                                                      is_nickname(nick));
        if (to == server.clients.end()) {
            sendError(client, server, ERR_NOSUCHNICK, nick);
            return NULL;
        }
        if (channel->users.find(&(*to)) == channel->users.end()) {
            sendError(client, server, ERR_USERNOTINCHANNEL, nick, chan);
            return NULL;
        }
        return &(*to);
    }

    void mode_flags_chan_nick(Client *moded, std::set<std::string const*> &members, int const &sign)
    //добавляем в базу ник
    {
        std::string const &nick = moded->getNick();
        if (sign == '+') {
            if (members.find(&nick) != members.end()) {
                return;
            } else {
                members.insert(&nick);
        }
        } else {
            if (members.find(&nick) == members.end()) {
                return;
            } else {
                members.erase(&nick);
            }
        }
    }

    bool mode_flags_keys(Channel *channel, Client const &client, ListenSocket &server, int const &sign, size_t const &len, std::string const &nick, std::string const &chan)
    //добавляем в базу ключ
    {
        if (len < 3) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
            return false;
        }
        if (sign == '+') 
        {
            if (!channel->getKey().empty()) {
                sendError(client, server, ERR_KEYSET, chan);
                return false;
            } else {
                channel->setKey(nick);
            }
        } else {
            channel->clearKey();
        }
        return true;
    }

    bool mode_flags_limit(Channel *channel, Client const &client, ListenSocket &server, int const &sign, size_t const &len, std::string const &nick, std::string const &chan)
    //добавляем в базу лимита
    {
        if (len < 3) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
            return false;
        }
        if (sign == '+') {
            int a = check_num(nick.c_str());//переводим в число
            if (a > 0) {
                channel->setLimit(a);
            }
            else
                channel->clearLimit();
        } else {
            channel->clearLimit();
        }
        return true;
    }

    void mode_table_nicks(Client *oclient, Client const &client, ListenSocket &server)
    //посмотреть сетку модов ника
    {
        std::string res = "r";
        if (oclient->isFlag(UMODE_NOPER))
            res += " o";
        if (oclient->isFlag(UMODE_INVIS))
            res += " i";
        if (oclient->isFlag(UMODE_WALLOPS))
            res += " w";
        sendReply(server.getServername(), client, RPL_UMODEIS, res);
    }

    void mode_flags_nick(Client *oclient, int flag, int const &sign)
    //установка флага клиента
    {
        if (sign == '+')
            oclient->setFlag(flag);
       else
            oclient->zeroFlag(flag);
    }
}