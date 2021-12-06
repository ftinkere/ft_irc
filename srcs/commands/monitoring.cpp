//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"

namespace IRC {

    void cmd_names(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::vector<std::string> chans;
        int count = 0;
        std::map<std::string, Channel>::iterator it;
        std::vector<std::map<std::string, Channel>::iterator> mas_it;
        std::vector<std::map<std::string, Channel>::iterator>::iterator vec_it;
        size_t len = 0;

        if (!params.empty()) {
            chans = Channel::split(params[0], ',');
            len = chans.size();
            for (int i = 0; i < len; ++i){
                it = server.channels.find(chans[i]);
                if (it == server.channels.end())
                    continue;
                mas_it.push_back(it);
            }
        }
        else {//если нет каналов выводим все
            for(it = server.channels.begin(); it != server.channels.end(); ++it)
                mas_it.push_back(it);
            count = 1;
        }
        for (vec_it = mas_it.begin(); vec_it != mas_it.end(); ++vec_it){
            Channel &channel = (*vec_it)->second;
            if (channel.isFlag(CMODE_SECRET) && channel.users.find(&client) == channel.users.end())
                continue;
            sendReply(server.getServername(), client, RPL_NAMREPLY, (*vec_it)->first, channel.get_names());
            sendReply(server.getServername(), client, RPL_ENDOFNAMES, (*vec_it)->first);
        }
        if (count == 1)
        {// если мы не выводили избранные каналы
            std::string cl;
            std::list<Client>::iterator lst = server.clients.begin();
            for (; lst != server.clients.end(); ++lst) {
                if (lst->getChannels().empty() && !(lst->isFlag(UMODE_INVIS))) {
                    cl += (*lst).getNick() + " ";
                }
            }
            sendReply(server.getServername(), client, RPL_NAMREPLY, "No channels", cl);
            sendReply(server.getServername(), client, RPL_ENDOFNAMES, "No channels");
        }
    }

    void cmd_admin(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> params = cmd.getParams(); //параметры

        if (params.empty())
            params[0] = server.getServername();
        if (params[0] != server.getServername()) {
            sendError(client, server, ERR_NOSUCHSERVER, params[0]);
            return;
        }
        sendReply(server.getServername(), client, RPL_ADMINME, params[0]);
        sendReply(server.getServername(), client, RPL_ADMINLOC1, server.admin["adminName"]);
        sendReply(server.getServername(), client, RPL_ADMINLOC2, server.admin["adminNickname"]);
        sendReply(server.getServername(), client, RPL_ADMINEMAIL, server.admin["adminEmail"]);
    }

    void cmd_whois(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::list<Client>::iterator to;
        if (params.empty()){
            sendError(client, server, ERR_NONICKNAMEGIVEN);
            return;
        }
        std::vector<std::string> nicks = Channel::split(params[0], ',');
        for (int i = 0; i < nicks.size(); ++i)
        {
            std::string nick = nicks[i];
            to = check_mask_nick(RPL_WHOISSERVER, params[0], client, server);
            if (to == server.clients.end()) {
                sendError(client, server, ERR_NOSUCHNICK, nick);
                continue;
            }
            std::string chans_str;
            Client find_client = *to;
            std::list<std::string>::iterator chans = find_client.getChannels().begin(); //берем клиента
            for (;chans != find_client.getChannels().end(); ++chans){
                Channel &channel = server.channels[*chans];
                if (channel.isFlag(CMODE_SECRET))
                    continue;
                else if (channel.opers.find(&find_client.getNick()) != channel.opers.end())//заполняем список каналов
                    chans_str += '@' + *chans + ' ';
                else if (channel.voiced.find(&client.getNick()) != channel.voiced.end())
                    chans_str += '+' + *chans + ' ';
                else
                    chans_str += *chans + ' ';
            }
            sendReply(server.getServername(), client, RPL_WHOISUSER, nick, find_client.getUser(), find_client.getHost(), find_client.getReal());
            sendReply(server.getServername(), client, RPL_WHOISCHANNELS, nick, chans_str);
            sendReply(server.getServername(), client, RPL_WHOISSERVER, nick, server.getServername(), "Вот такой вот сервер");
            if (!find_client.getAway().empty())
                sendReply(server.getServername(), client, RPL_AWAY, nick, find_client.getAway());
            if (find_client.isFlag(UMODE_NOPER))
                sendReply(server.getServername(), client, RPL_WHOISOPERATOR, nick);
            std::stringstream	regTime, onServer;
            onServer << (time(0) - find_client.getTime());
            regTime << find_client.getTime();
            sendReply(server.getServername(), client, RPL_WHOISIDLE, nick, onServer.str(), regTime.str());
            sendReply(server.getServername(), client, RPL_ENDOFWHOIS, nick);
        }
    }

    void cmd_whowas(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры

        if (params.empty()){
            sendError(client, server, ERR_NONICKNAMEGIVEN);
            return;
        }
        std::vector<std::string> nicks = Channel::split(params[0], ',');
        size_t limit = 1;
        for (int i = 0; i < nicks.size(); ++i)
        {
            std::string nick = nicks[i];
            bool loop = false;
            if (params.size() > 1)
                limit = check_num(params[1].c_str());
            // если есть второй аргумент то это будет лимитом
            size_t coun = server.base_client.count(nick); //количество элементов в мапе
            if (limit < 0 || coun < limit)
                limit = coun;

            std::pair<std::multimap<std::string, Client>::iterator, std::multimap<std::string, Client>::iterator> range =  server.base_client.equal_range(nick);//находим список всех совпадений
            int j = 0;
            for (std::multimap<std::string, Client>::iterator it = range.first; it != range.second && j < limit; ++it, ++j)
            {
                sendReply(server.getServername(), client, RPL_WHOWASUSER, nick, it->second.getUser(), it->second.getHost(), it->second.getReal());
                sendReply(server.getServername(), client, RPL_WHOISSERVER, nick, server.getServername(), "Вот такой вот сервер");
                loop = true;//если зашли в цикл значит такой ник есть
            }
            if (!loop)
                sendError(client, server, ERR_WASNOSUCHNICK, nick);
            sendReply(server.getServername(), client, RPL_ENDOFWHOWAS, nick);
        }
    }
}