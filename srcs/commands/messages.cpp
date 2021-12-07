//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"

namespace IRC {
    void cmd_privmsg(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        size_t len = params.size(); //длина параметров
        if (params.empty()) {
            sendError(client, server, ERR_NORECIPIENT, cmd.getCommand());
            return;
        }
        if (len < 2) {
            sendError(client, server, ERR_NOTEXTTOSEND);
            return;
        }
        std::vector<Client *> clients = server.find_clients(params[0], client); //ищем все ники
        std::string msg;

        msg = choose_str(params, len, 1);//собираем параметры для отправки
        for (int i = 0; i < clients.size(); ++i) { //отправляем
            Command cmd(client.get_full_name(), CMD_PRIVMSG);
            cmd << clients[i]->getNick() << msg;
            server.send_command(cmd, clients[i]->getFd());
            if (!clients[i]->getAway().empty())
                sendReply(server.getServername(), client, RPL_AWAY, clients[i]->getNick(), clients[i]->getAway());
        }
    }

    void cmd_notice(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        size_t len = params.size(); //длина параметров
        if (params.empty())
            return;
        if (!len)
            return;
        std::vector<Client *> clients = server.find_clients(params[0], WITMSG, client); //ищем все ники
        std::string msg;

        msg = choose_str(params, len, 1);//собираем параметры для отправки
        for (int i = 0; i < clients.size(); ++i) {//отправляем
            Command cmd(client.get_full_name(), CMD_PRIVMSG);
            cmd << clients[i]->getNick() << msg;
            server.send_command(cmd, clients[i]->getFd());
        }
    }

    void cmd_away(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        size_t len = params.size();
        std::string msg;
        if (params.empty()) {
            sendReply(server.getServername(), client, RPL_UNAWAY);
            client.clearAway();
        } else {
            msg = choose_str(params, len, 0);//собираем параметры для отправки
            sendReply(server.getServername(), client, RPL_NOWAWAY);
            client.setAway(msg);
        }
    }

    void cmd_wallops(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        size_t len = params.size();
        std::string msg;
        if (params.empty()) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "WALLOPS");
            return;
        }
        if (!client.isFlag(UMODE_NOPER))
        {
            sendError(client, server, ERR_NOPRIVILEGES);
            return;
        }

        //ищем все ники
        msg = choose_str(params, len, 0);
        std::list<Client>::iterator to = server.clients.begin();
        for (; to != server.clients.end(); ++to) { //отправляем
            Command cmd(client.get_full_name(), CMD_WALLOPS);
            if (to->isFlag(UMODE_WALLOPS)){
                cmd << to->getNick() << msg;
                server.send_command(cmd, to->getFd());
            }
        }
    }
}