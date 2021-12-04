//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"

namespace IRC {

    void cmd_join(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        size_t len = params.size();
        int res;
        int count = 0;

        if (params.empty()) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "JOIN");
            return;
        }

        std::vector<std::string> chans = Channel::split(params[0], ',');//пилим каналы
        std::vector<std::string> keys;
        if (params.size() > 1) {
            keys = Channel::split(params[1], ',');//пилим пароли
        }
        for (unsigned int i = 0; i < chans.size(); ++i) {
            if (!Channel::check_name(chans[i])) {
                sendError(client, server, ERR_BADCHANMASK, chans[i]);
                continue;
            }

            if (server.channels.find(chans[i]) == server.channels.end()) {
                server.channels.insert(std::make_pair(chans[i], Channel(chans[i])));
            }

            Channel &chan = server.channels.find(chans[i])->second;
            std::string const &key = chan.getKey();

            if (!key.empty() && keys[i] != key) {
                sendError(client, server, ERR_BADCHANNELKEY, chans[i]);
                continue;
            }
            if (chan.isFlag(CMODE_INVITE)) {
                sendError(client, server, ERR_INVITEONLYCHAN, chans[i]);
                continue;
            }
            if (!chan.check_limit()) {
                sendError(client, server, ERR_CHANNELISFULL, chans[i]);
                continue;
            }
            chan.add_memeber(client);
            client.setChannels(chans[i]);

            sendReply(server.getServername(), client, RPL_NAMREPLY, chans[i], chan.get_names());
            sendReply(server.getServername(), client, RPL_ENDOFNAMES, chans[i]);
            if (!chan.getTopic().empty())
                sendReply(server.getServername(), client, RPL_TOPIC, chans[i], chan.getTopic());
        }
    }

    void cmd_part(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры

        if (params.empty()) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "PART");
            return;
        }

        std::vector<std::string> chans = Channel::split(params[0], ',');
        size_t len = chans.size();
        for (int i = 0; i < len; ++i) {
            Channel *channel = check_channel(chans[i], server, client, 1);
            if (channel == NULL) {continue;}
            channel->erase_client(client);
            client.eraseChannel(chans[i]);
        }
    }

    void cmd_topic(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::string msg;

        if (params.empty()) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "TOPIC");
            return;
        }
        std::string chani = params[0]; //канал
        size_t len = params.size();

        Channel *channel = check_channel(chani, server, client, 1);
        if (channel == NULL) {return;}
        msg = choose_str(params, len, 1);

        if (msg.empty() && params.size() == 1) {//если параметры пустые и стоит :
            if (channel->getTopic().empty())//отправляем топик клиенту
                sendReply(server.getServername(), client, RPL_NOTOPIC, chani);
            else
                sendReply(server.getServername(), client, RPL_TOPIC, chani, channel->getTopic());
        } else if (msg.empty()) { //если просто пустые параметры
            if (channel->isFlag(CMODE_TOPIC)) {
                if (!priv_need_channel(channel, client, server, chani)) {return;}
            }
            channel->clearTopic();//очищаем топик
        } else { //если есть параметры
            if (channel->isFlag(CMODE_TOPIC)) {
                if (!priv_need_channel(channel, client, server, chani)) {return;}
            }
            channel->setTopic(msg);//устанавливаем топик
        }
    }

    void cmd_list(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::vector<std::string> chans;
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
        }
        for (vec_it = mas_it.begin(); vec_it != mas_it.end(); ++vec_it){
            Channel &channel = (*vec_it)->second;
            if (channel.isFlag(CMODE_SECRET))
                continue;
            sendReply(server.getServername(), client, RPL_LIST, (*vec_it)->first, std::to_string(channel.users.size()), channel.getTopic());
        }
        sendReply(server.getServername(), client, RPL_LISTEND);
    }

    void cmd_invite(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры

        if (params.empty() || params.size() < 2) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "INVITE");
            return;
        }

        std::string chani = params[1]; //канал
        std::string nick = params[0];
        size_t len = params.size();

        Channel *channel = check_channel(chani, server, client, 1);
        if (channel == NULL) {return;}

        std::list<Client>::iterator it = std::find_if(server.clients.begin(), server.clients.end(),
                                                      is_nickname(nick));
        if (!check_nick(it, channel, client, server, nick, chani)) {return;}
        if (!priv_need_channel(channel, client, server, nick)) {return;}//проврека на права

        sendReply(server.getServername(), client, RPL_INVITING, chani, nick);
        sendReply(server.getServername(), *it, RPL_AWAY, client.getNick(), "you were invited to the channel");
        channel->add_memeber(*it);
        it->setChannels(const_cast<std::string &>(chani));//добавляем канал в список каналов у клиента
    }

    void cmd_kick(Command const &cmd, Client &client, ListenSocket &server) {
        //TODO: что то сделать с комментом надо
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::vector<std::string> chans;
        std::vector<std::string> nicks;

        if (params.empty() || params.size() < 2) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "KICK");
            return;
        }

        chans = Channel::split(params[0], ',');
        nicks = Channel::split(params[1], ',');
        size_t len = nicks.size();
        for (int i = 0; i < chans.size(); ++i) {
            if (chans.size() == 1) { //если один канал тогда любое количество ников
                Channel *channel = check_channel(chans[0], server, client, 1);
                if (channel == NULL) {return;}
                if (!priv_need_channel(channel, client, server, chans[0])) {return;}

                for (int j = 0; j < len; ++j) {
                    erase_member(channel, client, server, nicks[j], chans[0]);
                }
            }
            else//один канал один ник
            {
                Channel *channel = check_channel(chans[i], server, client, 1);
                if (channel == NULL) {continue;}
                if (!priv_need_channel(channel, client, server, chans[i])) {continue;}
                if (nicks.size() <= i)
                    return;
                erase_member(channel, client, server, nicks[i], chans[i]);
            }
        }
    }
}
