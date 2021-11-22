//
// Created by Frey Tinkerer on 11/16/21.
//

#include "Command.hpp"
#include "Client.hpp"
#include "ListenSocket.hpp"
//#include "Channel.hpp"

namespace IRC {

    class Channel;
	void cmd_pass(Command const& cmd, Client& client, ListenSocket& server) {
		std::vector<std::string> const& params = cmd.getParams();
		if (params.empty()) {
		    sendError(client, server, ERR_NEEDMOREPARAMS, "PASS", "");
			// reply not args
		}
		else if (client.getFlags() & UMODE_REGISTERED)
		{
		    sendError(client, server, ERR_ALREADYREGISTRED, "", "");
		}
		else
		{
			client.pass = cmd.getParams()[0];
		}
	}

	void cmd_nick(Command const& cmd, Client& client, ListenSocket& server) {
		// check nick else reply 432 :Erroneous Nickname
		Client *to = std::find_if(server.clients.begin(), server.clients.end(),
                                  is_nickname(cmd.getParams()[0])).base();
		if (cmd.getParams().empty())
		{
		    sendError(client, server, ERR_NONICKNAMEGIVEN, "", "");
		    return;
		}// else reply not args
		if (!client._name_control(cmd.getParams()[0], 0)) //беру только первый аргумент
		{
		    sendError(client, server, ERR_ERRONEUSNICKNAME, cmd.getParams()[0], "");
		    return;
		}
		if (to != server.clients.end().base())
		{
		    sendError(client, server, ERR_NICKNAMEINUSE, cmd.getParams()[0], "");
		    return;
		}
		client.nick = cmd.getParams()[0];
		client.try_register(server);
	}

	void cmd_user(Command const& cmd, Client& client, ListenSocket& server) {
		// check user
		if (cmd.getParams().empty() || cmd.getParams().size() < 4)
		{
		    sendError(client, server, ERR_NEEDMOREPARAMS, "USER", "");
		}
		else if (client.getFlags() & UMODE_REGISTERED)
		{
		    sendError(client, server, ERR_ALREADYREGISTRED, "", "");
		}
		else{
			client.user = cmd.getParams()[0];
			client.try_register(server);
		} // else reply not args
	}

	void cmd_quit(Command const& cmd, Client& client, ListenSocket& server) {
	    server.quit_client(client.getFd());
	    std::cout << "[DEBUG]: " << client.nick << "!" << client.user << "@" << client.host << " " << cmd.getCommand() << ": " << cmd.getParams()[0] << std::endl;
	}

	void cmd_privmsg(Command const& cmd, Client& client, ListenSocket& server) {
		std::vector<std::string> const& param = cmd.getParams(); //параметры
		size_t len = param.size(); //длина параметров
		if (param.empty())
		{
			sendError(client, server, ERR_NORECIPIENT, cmd.getCommand(), "");
			return;
		}
		if (!len)
		{
			sendError(client, server, ERR_NOTEXTTOSEND, "", "");
			return;
		}
		std::vector<Client*> clients = server.find_clients(cmd.getParams()[0], client); //ищем все ники
		std::string msg;

		for (int j = 1; j < len; ++j) { //собираем параметры для отправки
			msg += param[j];
			if (j != len - 1) {
				msg += ' ';
			}
		}
		for (int i = 0; i < clients.size(); ++i) {//отправляем
			sendReply(server.getServername(), *clients[i], RPL_AWAY, clients[i]->getNick(), msg, "", "", "", "", "", "");
			if (!clients[i]->getAway().empty())
				sendReply(server.getServername(), client, RPL_AWAY, clients[i]->getNick(), clients[i]->getAway(), "", "", "", "", "", "");
		}
	}

	void cmd_notice(Command const& cmd, Client& client, ListenSocket& server) {
		std::vector<std::string> param = cmd.getParams(); //параметры
		size_t len = param.size(); //длина параметров
		if (param.empty())
		{
			return;
		}
		if (!len)
		{
			return;
		}
		std::vector<Client*> clients = server.find_clients(cmd.getParams()[0], WITMSG, client); //ищем все ники
		std::string msg;

		for(int j = 1; j < len; ++j){ //собираем параметры для отправки
				msg += param[j];
		}
		for(int i = 0; i < clients.size(); ++i){//отправляем
			sendReply(server.getServername(), *clients[i], RPL_AWAY, clients[i]->getNick(), msg, "", "", "", "", "", "");
		}
	}

	void cmd_away(Command const& cmd, Client& client, ListenSocket& server) {
		std::vector<std::string> param = cmd.getParams(); //параметры
		std::string msg;
		if (param.empty())
		{
			sendReply(server.getServername(), client, RPL_UNAWAY, "", "", "", "", "", "", "", "");
			client.clearAway();
		}
		else
		{
			for(int j = 0; j < param.size(); ++j){ //собираем параметры для отправки
				msg += param[j];
			}
			sendReply(server.getServername(), client, RPL_NOWAWAY, "", "", "", "", "", "", "", "");
			client.setAway(msg);
		}
	}

    void cmd_join(Command const& cmd, Client& client, ListenSocket& server) {
        std::vector<std::string> params = cmd.getParams(); //параметры
        size_t len = params.size();
        std::vector<std::string> chani;
        std::vector<std::string> keys;
        int res;
        int count = 0;

        if (params.empty()) {
            sendError(&client, server, ERR_NEEDMOREPARAMS, "JOIN", "");
            return;
        }
        for (int i = 0; i < len; ++i) {
            res = Channel::check_name(params[i]);
            if (res == ERR_NAME)
            {
                sendError(&client, server, ERR_NOSUCHCHANNEL, params[i], "");
                // no such channel
                return ;
            }
            else if (res == CHAN)
            {
                count++;
                if (server.channels.find(params[i]) != server.channels.end())
                {
                    Channel channel;
                    channel.setName(params[i]);
                    server.channels.insert(std::make_pair(params[i], channel));
                    channel.add_memeber(client.getNick());
                }
                chani.push_back(params[i]);

            }
            else
            {
                if (count < 1)
                {
                    //need more params
                    sendError(&client, server, ERR_NEEDMOREPARAMS, "JOIN", "");
                    return ;
                }
                keys.push_back(params[i]);
            }
        }
        size_t size = keys.size();
        for (unsigned int i = 0; i < chani.size(); ++i)
        {
            if (i < size)
            {
                std::string key = server.channels[chani[i]].getKey();
                if (!key.empty())
                {
                    if (key == keys[i]){
                        if (!(server.channels[chani[i]].getFlags() & CMODE_INVITE)) {
                            if (server.channels[chani[i]].check_limit()) {
                                server.channels[chani[i]].add_memeber(client.getNick());
                            } else{
                                sendError(&client, server, ERR_CHANNELISFULL, chani[i], "");
                            }
                        }
                        else{
                            sendError(&client, server, ERR_INVITEONLYCHAN, chani[i], "");
                            return;
                        }
                    }
                    else{

                        sendError(&client, server, ERR_BADCHANNELKEY, chani[i], "");
                        return;
                        //no key
                    }
                }
                else
                {
                    server.channels[chani[i]].add_memeber(client.getNick());
                }
            } else{
                if (!(server.channels[chani[i]].getFlags() & CMODE_INVITE)) {
                    if (server.channels[chani[i]].check_limit()) {
                        server.channels[chani[i]].add_memeber(client.getNick());
                    } else {
                        sendError(&client, server, ERR_CHANNELISFULL, chani[i], "");
                    }
                }
                else {
                    sendError(&client, server, ERR_INVITEONLYCHAN, chani[i], "");
                    return;
                }
            }
        }
//ERR_INVITEONLYCHAN
//ERR_CHANNELISFULL
//RPL_TOPIC
    }
}