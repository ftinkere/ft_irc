//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"

namespace IRC {
    void cmd_privmsg(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &param = cmd.getParams(); //параметры
        size_t len = param.size(); //длина параметров
        if (param.empty()) {
            sendError(client, server, ERR_NORECIPIENT, cmd.getCommand(), "");
            return;
        }
        if (len < 2) {
            sendError(client, server, ERR_NOTEXTTOSEND, "", "");
            return;
        }
        std::vector<Client *> clients = server.find_clients(cmd.getParams()[0], client); //ищем все ники
        std::string msg;

        for (int j = 1; j < len; ++j) { //собираем параметры для отправки
            msg += param[j];
            if (j != len - 1) {
                msg += ' ';
            }
        }
        for (int i = 0; i < clients.size(); ++i) { //отправляем
            Command cmd(client.get_full_name(), CMD_PRIVMSG);
            cmd << clients[i]->getNick() << msg;
            server.send_command(cmd, clients[i]->getFd());
            //			sendReply(server.getServername(), *clients[i], RPL_AWAY, clients[i]->getNick(), msg, "", "", "", "", "", "");
            if (!clients[i]->getAway().empty())
                sendReply(server.getServername(), client, RPL_AWAY, clients[i]->getNick(), clients[i]->getAway(), "",
                          "", "", "", "", ""); // TODO #chan
        }
    }

    void cmd_notice(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> param = cmd.getParams(); //параметры
        size_t len = param.size(); //длина параметров
        if (param.empty()) {
            return;
        }
        if (!len) {
            return;
        }
        std::vector<Client *> clients = server.find_clients(cmd.getParams()[0], WITMSG, client); //ищем все ники
        std::string msg;

        for (int j = 1; j < len; ++j) { //собираем параметры для отправки
            msg += param[j];
            if (j != len - 1) {
                msg += ' ';
            }
        }
        for (int i = 0; i < clients.size(); ++i) {//отправляем
            Command cmd(client.get_full_name(), CMD_PRIVMSG);
            cmd << clients[i]->getNick() << msg;
            server.send_command(cmd, clients[i]->getFd());
            //			sendReply(server.getServername(), *clients[i], RPL_AWAY, clients[i]->getNick(), msg, "", "", "", "", "", "");
        }
    }

    void cmd_away(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> param = cmd.getParams(); //параметры
        std::string msg;
        if (param.empty()) {
            sendReply(server.getServername(), client, RPL_UNAWAY, "", "", "", "", "", "", "", "");
            client.clearAway();
        } else {
            for (int j = 0; j < param.size(); ++j) { //собираем параметры для отправки
                msg += param[j];
            }
            sendReply(server.getServername(), client, RPL_NOWAWAY, "", "", "", "", "", "", "", "");
            client.setAway(msg);
        }
    }

    void cmd_wallops(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> params = cmd.getParams(); //параметры

        if (params.empty()) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "WALLOPS");
            return;
        }
        if (!client.isFlag(UMODE_NOPER))
        {
            sendError(client, server, ERR_NOPRIVILEGES);
            return;
        }

        std::list<Client>::iterator to = server.clients.begin();
        for (; to != server.clients.end(); ++to) { //отправляем
            Command cmd(client.get_full_name(), CMD_WALLOPS);
            if (to->isFlag(UMODE_WALLOPS)){
                cmd << to->getNick() << params[0];
                server.send_command(cmd, to->getFd());
            }
        }
    }
}