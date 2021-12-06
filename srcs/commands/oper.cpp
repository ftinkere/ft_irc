//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"

namespace IRC {

    void cmd_mode(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        if (params.empty()) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
            return;
        }

        if (params[0][0] == '#') {
            Channel *channel = check_channel(params[0], server, client, 0);
            if (channel == NULL)
                return;
            if (params.size() == 1){
                //выводим инфу об активных модах
                mode_table(channel, client, server, params[0]);
                return;
            }
            if (!priv_need_channel(channel, client, server, params[0])) {return;}
            std::map<const char, size_t>::iterator mod;
            mod = Channel::modes.find(params[1][1]);//ищем мод в мапе
            if (params[1].size() != 2 || mod == channel->modes.end()) {
                sendError(client, server, ERR_UNKNOWNMODE, params[1], params[0]);//если не подходит мод
                return;
            }
            size_t res = mod->second;
            char sign = params[1][0];
            if (sign != '-' && sign != '+') {
                sendError(client, server, ERR_UNKNOWNMODE, params[1], params[0]);//если не подходит мод
                return;
            }
            switch (res) {
                case 0:
                    mode_flags(channel, CMODE_INVITE, sign);
                    break;
                case 1:
                    mode_flags(channel, CMODE_MODER, sign);
                    break;
                case 2:
                    mode_flags(channel, CMODE_SECRET, sign);
                    break;
                case 3:
                    mode_flags(channel, CMODE_NOEXT, sign);
                    break;
                case 4:
                    mode_flags(channel, CMODE_TOPIC, sign);
                    break;
                case 5: {
                    Client *moded = check_mode_nick(channel, client, server, params[2], params[0], params.size());
                    if (moded == NULL)
                        return;
                    mode_flags_chan_nick(moded, channel->opers, sign);
                    break;
                }
                case 6: {
                    Client *moded = check_mode_nick(channel, client, server, params[2], params[0], params.size());
                    if (moded == NULL)
                        return;
                    mode_flags_chan_nick(moded, channel->voiced, sign);
                    break;
                }
                case 7: {
                    if (!mode_flags_keys(channel, client, server, sign, params.size(), params[2], params[0]))
                        return ;
                    break;
                }
                case 8: {
                    if (!mode_flags_limit(channel, client, server, sign, params.size(), params[2], params[0]))
                        return ;
                    break;
                }
            }
        }
        else {
            int flag  = 0;
            if (params.size() == 1)
                flag = 1;//если хочешь просто посмотреть
            Client *oclient = server.thisisnick(params[0], flag, client);
            if (oclient == NULL)
                return;
            if (params.size() == 1)
            {
                mode_table_nicks(oclient, client, server);
                return;
            }
            std::string models = "iwo";
            if (params[1].size() != 2 || models.find(params[1][1]) == std::string::npos) {
                sendError(client, server, ERR_UMODEUNKNOWNFLAG);//если не подходит мод
                return;
            }
            char sign = params[1][0];
            char mod = params[1][1];
            if (sign != '-' && sign != '+') {
                sendError(client, server, ERR_UMODEUNKNOWNFLAG);//если не подходит мод
                return;
            }
            if (client.getFlags() & UMODE_NOPER) {
                if (mod == 'i')
                    mode_flags_nick(oclient, UMODE_INVIS, sign);
                else if (mod == 'w')
                    mode_flags_nick(oclient, UMODE_WALLOPS, sign);
                else if (mod == 'o')
                    mode_flags_nick(oclient, UMODE_NOPER, sign);
            }
            else if (params[0] == oclient->getNick()){
                if (mod == 'i')
                    mode_flags_nick(oclient, UMODE_INVIS, sign);
                else if (mod == 'w')
                    mode_flags_nick(oclient, UMODE_WALLOPS, sign);
                else if (mod == 'o'){
                    if (sign == '+') {
                        sendError(client, server, ERR_USERSDONTMATCH);//если не опер то не можешь редактировать чужой ник
                        return;
                    } else
                        oclient->zeroFlag(UMODE_NOPER);
                }
            }
            else {
                sendError(client, server, ERR_USERSDONTMATCH);//если не опер то не можешь редактировать чужой ник
                return;
            }
        }
    }

    void cmd_oper(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::map<std::string, std::string>::iterator pass;

        if (params.empty() || params.size() != 2) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "OPER");
            return;
        }
        if (server.opers.empty()) { //если мапа пустая значит оперов не может быть
            sendError(client, server, ERR_NOOPERHOST);
            return;
        }
        pass = server.opers.find(params[0]);
        if (pass == server.opers.end() || pass->second != params[1]) {
            sendError(client, server, ERR_PASSWDMISMATCH);
            return;
        }
        sendReply(server.getServername(), client, RPL_YOUREOPER);
        if (!(client.getFlags() & UMODE_NOPER))
            client.setFlag(UMODE_NOPER);
    }

    void cmd_kill(Command const &cmd, Client &client, ListenSocket &server) {
        //TODO: что то сделать с комментом надо
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::list<Client>::iterator it;
        if (params.empty() || params.size() != 2) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "KILL");
            return;
        }

        if (!(client.getFlags() & UMODE_NOPER)) {
            sendError(client, server, ERR_NOPRIVILEGES);
            return;
        }
        it = check_mask_nick(ERR_CANTKILLSERVER, params[0], client, server);
        if (it == server.clients.end()) {
            sendError(client, server, ERR_NOSUCHNICK, params[0]);
            return;
        }

        server.quit_client(it->getFd());
    }
}