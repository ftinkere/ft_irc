//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"
//TODO: all
namespace IRC {

    void cmd_mode(Command const &cmd, Client &client, ListenSocket &server) {
        //добавить выводи режимов на экран
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        //        std::vector<std::string> chans;
        //        std::vector<std::string> nicks;
        if (params.empty()) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
            return;
        }
        if (params[0][0] == '#') {
            Channel *channel = server.thisischannel(params[0], 0, client);
            if (channel == NULL)
                return;
            if (params.size() == 1)//выводим инфу об активных модах
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
                sendReply(server.getServername(), client, RPL_CHANNELMODEIS, params[0], res);
                return;
                }
            if (channel->opers.find(&client.getNick()) == channel->opers.end()) {
                sendError(client, server, ERR_CHANOPRIVSNEEDED, params[0]);//если нет привелегий
                return;
            }
            std::map<const char, size_t>::iterator mod;
            mod = Channel::modes.find(params[1][1]);
            size_t res = mod->second;
            if (params[1].size() != 2 || mod == channel->modes.end()) {
                sendError(client, server, ERR_UNKNOWNMODE, params[1], params[0]);//если не подходит мод
                return;
            }
            char sign = params[1][0];
            if (sign != '-' && sign != '+') {
                sendError(client, server, ERR_UNKNOWNMODE, params[1], params[0]);//если не подходит мод
                return;
            }
            switch (res) {
                case 0:
                    if (sign == '-') {
                        if (channel->isFlag(CMODE_INVITE))
                            channel->zeroFlag(CMODE_INVITE);
                    } else {
                        if (!channel->isFlag(CMODE_INVITE))
                            channel->setFlag(CMODE_INVITE);
                    }
                    break;
                    case 1:
                        if (sign == '-') {
                            if (channel->isFlag(CMODE_MODER))
                                channel->zeroFlag(CMODE_MODER);
                        } else {
                            if (!channel->isFlag(CMODE_MODER))
                                channel->setFlag(CMODE_MODER);
                        }
                        break;
                        case 2:
                            if (sign == '-') {
                                if (channel->isFlag(CMODE_SECRET))
                                    channel->zeroFlag(CMODE_SECRET);
                            } else {
                                if (!channel->isFlag(CMODE_SECRET))
                                    channel->setFlag(CMODE_SECRET);
                            }
                            break;
                            case 3:
                                if (sign == '-') {
                                    if (channel->isFlag(CMODE_NOEXT))
                                        channel->zeroFlag(CMODE_NOEXT);
                                } else {
                                    if (!channel->isFlag(CMODE_NOEXT))
                                        channel->setFlag(CMODE_NOEXT);
                                }
                                break;
                                case 4:
                                    if (sign == '-') {
                                        if (channel->isFlag(CMODE_TOPIC))
                                            channel->zeroFlag(CMODE_TOPIC);
                                    } else {
                                        if (!channel->isFlag(CMODE_TOPIC))
                                            channel->setFlag(CMODE_TOPIC);
                                    }
                                    break;
                                    case 5: {
                                        if (params.size() < 3) {
                                            sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
                                            return;
                                        }
                                        std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
                                                                                      is_nickname(params[2]));
                                        if (to == server.clients.end()) {
                                            sendError(client, server, ERR_NOSUCHNICK, params[2]);
                                            return;
                                        }
                                        std::string nick = (*to).getNick();
                                        if (channel->users.find(&(*to)) == channel->users.end()) {
                                            sendError(client, server, ERR_USERNOTINCHANNEL, params[2], params[0]);
                                            return;
                                        }
                                        if (sign == '+') {
                                            if (channel->opers.find(&nick) != channel->opers.end()) {
                                                return;
                                            } else {
                                                channel->opers.insert(&((*to).getNick()));
                                            }
                                        } else {
                                            if (channel->opers.find(&nick) == channel->opers.end()) {
                                                return;
                                            } else {
                                                channel->opers.erase(&((*to).getNick()));
                                            }
                                        }
                                        break;
                                    }
                                    case 6: {
                                        if (params.size() < 3) {
                                            sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
                                            return;
                                        }
                                        std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
                                                                                      is_nickname(params[2]));
                                        if (to == server.clients.end()) {
                                            sendError(client, server, ERR_NOSUCHNICK, params[2]);
                                            return;
                                        }
                                        std::string nick = (*to).getNick();
                                        if (channel->users.find(&(*to)) == channel->users.end()) {
                                            sendError(client, server, ERR_USERNOTINCHANNEL, params[2], params[0]);
                                            return;
                                        }
                                        if (sign == '+') {
                                            if (channel->voiced.find(&nick) != channel->voiced.end()) {
                                                return;
                                            } else {
                                                channel->voiced.insert(&((*to).getNick()));
                                            }
                                        } else {
                                            if (channel->voiced.find(&nick) == channel->voiced.end()) {
                                                return;
                                            } else {
                                                channel->voiced.erase(&((*to).getNick()));
                                            }
                                        }
                                        break;
                                    }
                                    case 7: {
                                        if (params.size() < 3) {
                                            sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
                                            return;
                                        }
                                        if (sign == '+') {
                                            if (!channel->getKey().empty()) {
                                                sendError(client, server, ERR_KEYSET, params[0]);
                                                return;
                                            } else {
                                                channel->setKey(params[2]);
                                            }
                                        } else {
                                            channel->clearKey();
                                        }
                                        break;
                                    }
                                    case 8: {
                                        if (params.size() < 3) {
                                            sendError(client, server, ERR_NEEDMOREPARAMS, "MODE");
                                            return;
                                        }
                                        if (sign == '+') {
                                            int a = check_num(params[2].c_str());//переводим в число
                                            if (a != false) {
                                                channel->setLimit(a);
                                            }
                                        } else {
                                            channel->clearLimit();
                                        }
                                        break;
                                    }


            }
        } else {
            int flag  = 0;
            if (params.size() == 1)
                flag = 1;//если хочешь просто посмотреть
                Client *oclient = server.thisisnick(params[0], flag, client);
                if (oclient == NULL)
                    return;
                if (params.size() == 1)
                {
                    std::string res = "r";
                    if (oclient->isFlag(UMODE_NOPER))
                        res += " o";
                    if (oclient->isFlag(UMODE_INVIS))
                        res += " i";
                    if (oclient->isFlag(UMODE_WALLOPS))
                        res += " w";
                    sendReply(server.getServername(), client, RPL_UMODEIS, res);
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
                    if (mod == 'i') {
                        if (sign == '+')
                            oclient->setFlag(UMODE_INVIS);
                        else
                            oclient->zeroFlag(UMODE_INVIS);
                    } else if (mod == 'w') {
                        if (sign == '+')
                            oclient->setFlag(UMODE_WALLOPS);
                        else
                            oclient->zeroFlag(UMODE_WALLOPS);
                    } else if (mod == 'o') {
                        if (sign == '+')
                            oclient->setFlag(UMODE_NOPER);
                        else
                            oclient->zeroFlag(UMODE_NOPER);
                    }
                } else {
                    if (mod == 'i') {
                        if (sign == '+')
                            oclient->setFlag(UMODE_INVIS);
                        else
                            oclient->zeroFlag(UMODE_INVIS);
                    } else if (mod == 'w') {
                        if (sign == '+')
                            oclient->setFlag(UMODE_WALLOPS);
                        else
                            oclient->zeroFlag(UMODE_WALLOPS);
                    } else if (mod == 'o') {
                        if (sign == '+') {
                            sendError(client, server, ERR_USERSDONTMATCH);//если не опер то не можешь редактировать чужой ник
                            return;
                        } else
                            oclient->zeroFlag(UMODE_NOPER);
                    }
                }
        }
    }

    void cmd_oper(Command const &cmd, Client &client, ListenSocket &server) {
        std::vector<std::string> const &params = cmd.getParams(); //параметры
        std::map<std::string, std::string>::iterator pass;
        //        std::vector<std::string> keys;
        int res;
        int count = 0;

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
        std::vector<std::string> pass;
        int res;
        int count = 0;

        if (params.empty() || params.size() != 2) {
            sendError(client, server, ERR_NEEDMOREPARAMS, "KILL");
            return;
        }

        if (!(client.getFlags() & UMODE_NOPER)) {
            sendError(client, server, ERR_NOPRIVILEGES);
            return;
        }
        std::list<Client>::iterator to = std::find_if(server.clients.begin(), server.clients.end(),
                                                      is_nickname(params[0]));
        if (to == server.clients.end()) {
            sendError(client, server, ERR_NOSUCHNICK, params[0]);
            return;
        }
        if (params[0] == server.getServername()) {
            sendError(client, server, ERR_CANTKILLSERVER);
            return;
        }
        server.quit_client(to->getFd());
    }
}