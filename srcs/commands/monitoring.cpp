//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"

namespace IRC {

    void cmd_names(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::vector<std::string> chans;
        //        std::vector<std::string> keys;
        int res;
        int count = 0;

        if (!params.empty()) {
            chans = Channel::split(params[0], ',');
        } else {//если нет каналов выводим все
            for (std::map<std::string, Channel>::iterator it = server.channels.begin();
            it != server.channels.end(); ++it)
                chans.push_back(it->first);
            count = 1;
        }
        size_t len = chans.size();
        for (int i = 0; i < len; ++i) {
            Channel &channel = server.channels.find(chans[i])->second;
            if (!Channel::check_name(chans[i])) {
                continue;
            }
            if (server.channels.find(chans[i]) == server.channels.end()) {
                continue;
            }
            if (channel.isFlag(CMODE_SECRET) && channel.users.find(&client) == channel.users.end()) {
                continue;
            }
            //остановился здесь
            sendReply(server.getServername(), client, RPL_NAMREPLY, chans[i], channel.get_names());
            sendReply(server.getServername(), client, RPL_ENDOFNAMES, chans[i]);
        }
        if (count == 1)// если мы не выводили тзбранные каналы
            {
            std::string cl;
            std::list<Client>::iterator it = server.clients.begin();
            for (; it != server.clients.end(); ++it) {
                if (it->getChannels().empty() && !(it->isFlag(UMODE_INVIS))) {
                    cl += (*it).getNick() + " ";
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
        //TODO: поправить реалнейм
        std::vector<std::string> params = cmd.getParams(); //параметры

        if (params.empty()){
            sendError(client, server, ERR_NONICKNAMEGIVEN);
            return;
        }
        std::vector<std::string> nicks = Channel::split(params[0], ',');
        for (int i = 0; i < nicks.size(); ++i)
        {
            std::string nick = nicks[i];
            std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
                                                          is_nickname(nick));
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
            sendReply(server.getServername(), client, RPL_WHOISUSER, nick, find_client.getUser(), find_client.getHost(), "realname");
            sendReply(server.getServername(), client, RPL_WHOISCHANNELS, nick, chans_str);
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
        //TODO: поправить реалнейм
        //TODO: сделать проверку на имя сервера
        std::vector<std::string> params = cmd.getParams(); //параметры

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
                limit = check_num(params[1].c_str());// если есть второй аргумент то это будет лимитом
                size_t coun = server.base_client.count(nick); //количество элементов в мапе
                if (limit < 0 || coun < limit)
                    limit = coun;
                std::pair<std::multimap<std::string, Client>::iterator, std::multimap<std::string, Client>::iterator> range =  server.base_client.equal_range(nick);//находим список всех совпадений
                int j = 0;
                for (std::multimap<std::string, Client>::iterator i = range.first; i != range.second && j < limit; ++i, ++j)
                {
                    sendReply(server.getServername(), client, RPL_WHOWASUSER, nick, i->second.getUser(), i->second.getHost(), "realname");
                    loop = true;//если зашли в цикл значит такой ник есть
                }
                if (!loop)
                    sendError(client, server, ERR_WASNOSUCHNICK, nick);
                sendReply(server.getServername(), client, RPL_ENDOFWHOWAS, nick);
        }
    }
}