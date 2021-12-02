//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"

namespace IRC {

    void cmd_join(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        size_t len = params.size();
        //        std::vector<std::string> chani;
        //        std::vector<std::string> keys;
        int res;
        int count = 0;

        if (params.empty()) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "JOIN", "");
            return;
        }

        std::vector<std::string> chans;
        {
            std::istringstream stream(params[0]);
            std::string line;
            while (std::getline(stream, line, ',')) {
                chans.push_back(line);
            }
        }

        std::vector<std::string> keys;
        if (params.size() > 1) {
            {
                std::istringstream stream(params[1]);
                std::string line;
                while (std::getline(stream, line, ',')) {
                    keys.push_back(line);
                }
            }
        }
        for (int i = 0; i < chans.size(); ++i) {
            keys.push_back("");
        }

        //        for (int i = 0; i < len; ++i) {
        //            res = Channel::check_name(params[i]);
        //            if (res == ERR_NAME)
        //            {
        //                sendError(client, server, ERR_NOSUCHCHANNEL, params[i], "");
        //                // no such channel
        //                return ;
        //            }
        //            else if (res == CHAN)
        //            {
        //                count++;
        //                if (server.channels.find(params[i]) == server.channels.end())
        //                {
        //                    Channel channel;
        //                    channel.setName(params[i]);
        //                    server.channels.insert(std::make_pair(params[i], channel));
        //                    channel.add_memeber(client);
        //
        //                }
        //                chani.push_back(params[i]);
        //
        //            }
        //            else
        //            {
        //                if (count < 1)
        //                {
        //                    //need more params
        //                    sendError(client, server, ERR_NEEDMOREPARAMS, "JOIN", "");
        //                    return ;
        //                }
        //                keys.push_back(params[i]);
        //            }
        //        }
        for (unsigned int i = 0; i < chans.size(); ++i) {
            if (!Channel::check_name(chans[i])) {
                sendError(client, server, ERR_BADCHANMASK, chans[i], "");
                continue;
            }

            if (server.channels.find(chans[i]) == server.channels.end()) {
                server.channels.insert(std::make_pair(chans[i], Channel(chans[i])));
            }

            Channel &chan = server.channels.find(chans[i])->second;
            std::string const &key = chan.getKey();

            if (!key.empty() && keys[i] != key) {
                sendError(client, server, ERR_BADCHANNELKEY, chans[i], "");
                continue;
            }
            if (chan.isFlag(CMODE_INVITE)) {
                sendError(client, server, ERR_INVITEONLYCHAN, chans[i], "");
                continue;
            }
            if (!chan.check_limit()) {
                sendError(client, server, ERR_CHANNELISFULL, chans[i], "");
                continue;
            }
            chan.add_memeber(client);
            client.setChannels(chans[i]);

            sendReply(server.getServername(), client, RPL_NAMREPLY, chans[i], chan.get_names(), "", "", "", "", "", "");
            sendReply(server.getServername(), client, RPL_ENDOFNAMES, chans[i], "", "", "", "", "", "", "");

            //            if (i < keys.size())
            //            {
            //                if (!key.empty())
            //                {
            //                    if (key == keys[i]){
            //                        if (!(chan.getFlags() & CMODE_INVITE)) {
            //                            if (chan.check_limit()) {
            //								chan.add_memeber(client);
            //                            } else{
            //                                sendError(client, server, ERR_CHANNELISFULL, chans[i], "");
            //                            }
            //                        }
            //                        else{
            //                            sendError(client, server, ERR_INVITEONLYCHAN, chans[i], "");
            //                            return;
            //                        }
            //                    }
            //                    else{
            //
            //                        sendError(client, server, ERR_BADCHANNELKEY, chans[i], "");
            //                        return;
            //                        //no key
            //                    }
            //                }
            //                else
            //                {
            //                    server.channels[chans[i]].add_memeber(client);
            //                }
            //            } else {
            //                if (!(server.channels[chans[i]].getFlags() & CMODE_INVITE)) {
            //                    if (server.channels[chans[i]].check_limit()) {
            //                        server.channels[chans[i]].add_memeber(client);
            //
            //						for (std::set<std::string const*>::iterator it = server.channels[chans[i]].opers.begin(); it != server.channels[chans[i]].opers.end(); ++it) {
            //							std::cout << "[DEBUG]: " << *it << ": " << *(*it) << std::endl;
            //						}
            //						for (std::set<Client const*>::iterator it = server.channels[chans[i]].users.begin(); it != server.channels[chans[i]].users.end(); ++it) {
            //							std::cout << "[DEBUG]: " << (*it)->getFd() << " " << &(*it)->nick << ": " << (*it)->getNick() << std::endl;
            //						}
            //
            //                    } else {
            //                        sendError(client, server, ERR_CHANNELISFULL, chans[i], "");
            //                    }
            //                }
            //                else {
            //                    sendError(client, server, ERR_INVITEONLYCHAN, chans[i], "");
            //                    return;
            //                }
            //            }
            //        }
            //ERR_INVITEONLYCHAN
            //ERR_CHANNELISFULL
            //RPL_TOPIC
            //проверка на невидимость при отправке инфы вновь прибывшему
        }
    }

    void cmd_part(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        //        std::vector<std::string> chani;
        //        std::vector<std::string> keys;
        int res;
        int count = 0;

        if (params.empty()) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "PART", "");
            return;
        }

        std::vector<std::string> chans = Channel::split(params[0], ',');
        size_t len = chans.size();
        for (int i = 0; i < len; ++i) {
            Channel &channel = server.channels.find(chans[i])->second;
            if (!Channel::check_name(chans[i])) {
                sendError(client, server, ERR_NOSUCHCHANNEL, chans[i], "");
                continue;
            }
            if (server.channels.find(chans[i]) == server.channels.end()) {
                sendError(client, server, ERR_NOSUCHCHANNEL, chans[i], "");
                continue;
            }
            if (channel.users.find(&client) == channel.users.end()) {//если клиента нет на канале
                sendError(client, server, ERR_NOTONCHANNEL, chans[i], "");
                continue;
            }
            channel.erase_client(client);
            client.eraseChannel(chans[i]);
        }
    }

    void cmd_topic(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::string msg;
        //        std::vector<std::string> keys;
        int res;
        int count = 0;

        if (params.empty()) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "TOPIC", "");
            return;
        }
        std::string chani = params[0]; //канал

        // std::vector<std::string> chans = split(params[0], ',');
        size_t len = params.size();
        // for(int i = 0; i < len; ++i)

        if (!Channel::check_name(chani)) {
            sendError(client, server, ERR_NOSUCHCHANNEL, chani, "");
            return;
        }
        if (server.channels.find(chani) == server.channels.end()) {
            sendError(client, server, ERR_NOSUCHCHANNEL, chani, "");
            return;
        }
        Channel &channel = server.channels.find(chani)->second;
        if (channel.users.find(&client) == channel.users.end()) {
            sendError(client, server, ERR_NOTONCHANNEL, chani, "");
            return;
        }
        // if (server.channels[chani].opers.find(&client) == server.channels[chani].users.end())
        // {
        // 	sendError(client, server, ERR_CHANOPRIVSNEEDED, chani, "");
        // 	return;
        // }
        // if (!chani.isFlag(CMODE_MODER))
        // {
        // 	sendError(client, server, ERR_NOCHANMODES, chani, "");
        // 	return;
        // }
        for (int j = 1; j < len; ++j) { //собираем параметры для отправки
            msg += params[j];
            if (j != len - 1) {
                msg += ' ';
            }
        }

        if (msg.empty() && params.size() == 1) {
            if (channel.getTopic().empty())
                sendReply(server.getServername(), client, RPL_NOTOPIC, chani);
            else
                sendReply(server.getServername(), client, RPL_TOPIC, chani, channel.getTopic());
        } else if (msg.empty()) {
            if (channel.isFlag(CMODE_TOPIC)) {
                if (channel.opers.find(&(client.getNick())) == channel.opers.end()) {
                    sendError(client, server, ERR_CHANOPRIVSNEEDED, chani);
                    return;
                }
            }
            channel.clearTopic();//очищаем топик
        } else {
            if (channel.isFlag(CMODE_TOPIC)) {
                if (channel.opers.find(&(client.getNick())) == channel.opers.end()) {
                    sendError(client, server, ERR_CHANOPRIVSNEEDED, chani);
                    return;
                }
            }
            channel.setTopic(msg);//устанавливаем топик
        }
    }

    void cmd_list(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::vector<std::string> chans;
        //        std::vector<std::string> keys;
        int res;
        int count = 0;

        if (!params.empty()) {
            chans = Channel::split(params[0], ',');
        } else { //если нет каналов выводим все
            for (std::map<std::string, Channel>::iterator it = server.channels.begin();
            it != server.channels.end(); ++it)
                chans.push_back(it->first);
            count = 1;
        }
        size_t len = chans.size();
        for (int i = 0; i < len; ++i) {
            Channel &channel = server.channels.find(chans[i])->second; // UB
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
            //			if (count != 1)
            //				chans[i].clear();
            sendReply(server.getServername(), client, RPL_LIST, chans[i], std::to_string(channel.users.size()), channel.getTopic());

        }
        sendReply(server.getServername(), client, RPL_LISTEND);
    }

    void cmd_invite(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::vector<std::string> chans;
        //        std::vector<std::string> keys;
        int res;
        int count = 0;

        if (params.empty() || params.size() < 2) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "INVITE", "");
            return;
        }
        if (!Channel::check_name(params[1])) {
            return;
        }
        if (server.channels.find(params[1]) == server.channels.end()) {
            return;
        }
        Channel &channel = server.channels.find(params[1])->second;
        std::list<Client>::iterator it = std::find_if(server.clients.begin(), server.clients.end(),
                                                      is_nickname(params[0]));
        if (it == server.clients.end()) {//если нет ника
            sendError(client, server, ERR_NOSUCHNICK, params[0], "");
            return;
        }
        if (channel.users.find(&client) == channel.users.end()) {//если нет меня на канале
            sendError(client, server, ERR_NOTONCHANNEL, params[1], "");
            return;
        }
        if (channel.users.find(&(*it)) != channel.users.end()) {//если чувак уже на канале
            sendError(client, server, ERR_USERONCHANNEL, params[0], params[1]);
            return;
        }
        if (channel.opers.find(&client.getNick()) == channel.opers.end()) {
            sendError(client, server, ERR_CHANOPRIVSNEEDED, params[1], "");//если нет привелегий
            return;
        }
        sendReply(server.getServername(), client, RPL_INVITING, params[1], params[0], "", "", "", "", "", "");
        sendReply(server.getServername(), *it, RPL_AWAY, client.getNick(), "you were invited to the channel");
        channel.add_memeber(*it);
        it->setChannels(const_cast<std::string &>(params[1]));
    }

    void cmd_kick(Command const &cmd, Client &client, ListenSocket &server) {
        //TODO: что то сделать с комментом надо
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::vector<std::string> chans;
        std::vector<std::string> nicks;

        if (params.empty() || params.size() < 2) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "KICK", "");
            return;
        }
        chans = Channel::split(params[0], ',');
        nicks = Channel::split(params[1], ',');
        size_t len = nicks.size();
        for (int i = 0; i < chans.size(); ++i) {
            if (server.channels.find(chans[i]) == server.channels.end()) {
                sendError(client, server, ERR_NOSUCHCHANNEL, chans[i], "");
                continue;
            }
            if (chans.size() == 1) { //если один канал тогда любое количество ников
                Channel &channel = server.channels.find(chans[0])->second;
                if (channel.users.find(&client) == channel.users.end()) {
                    sendError(client, server, ERR_NOTONCHANNEL, chans[0], "");
                    return;
                }
                if (channel.opers.find(&client.getNick()) == channel.opers.end()) {
                    sendError(client, server, ERR_CHANOPRIVSNEEDED, chans[0], "");//если нет привелегий
                    return;
                }
                for (int j = 0; j < len; ++j) {
                    std::list<Client>::iterator it = std::find_if(server.clients.begin(), server.clients.end(),
                                                                  is_nickname(nicks[j]));
                    if (channel.users.erase(&(*it)) == 0)// пытаемся удалить
                        {
                        sendError(client, server, ERR_USERNOTINCHANNEL, nicks[j], chans[0]);
                        continue;
                        }
                    channel.opers.erase(&(*it).getNick());
                    channel.voiced.erase(&(*it).getNick());
                    it->eraseChannel(chans[0]);
                }
            }
            else//один канал один ник
            {
                Channel &channel = server.channels.find(chans[i])->second;
                if (channel.users.find(&client) == channel.users.end()) {
                    sendError(client, server, ERR_NOTONCHANNEL, chans[i], "");
                    continue;
                }
                if (channel.opers.find(&client.getNick()) == channel.opers.end()) {
                    sendError(client, server, ERR_CHANOPRIVSNEEDED, chans[i], "");//если нет привелегий
                    continue;
                }
                if (nicks.size() <= i)
                    return;
                std::list<Client>::iterator it = std::find_if(server.clients.begin(), server.clients.end(),
                                                              is_nickname(nicks[i]));
                if (channel.users.erase(&(*it)) == 0) {
                    sendError(client, server, ERR_USERNOTINCHANNEL, nicks[i], chans[i]);
                    continue;
                }
                channel.opers.erase(&(*it).getNick());
                channel.voiced.erase(&(*it).getNick());
                it->eraseChannel(chans[i]);
            }
        }
    }
}
