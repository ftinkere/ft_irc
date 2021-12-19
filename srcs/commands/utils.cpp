//
// Created by Luci Atkins on 12/2/21.
//

#include "commands.hpp"
#include <Reply.hpp>
#include <algorithm>
#include <sstream>

class ListenSocket;
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
		for (; j < len; ++j) {//собираем параметры для отправки
			msg += params[j];
			if (j != len - 1) {
				msg += ' ';
			}
		}
		return msg;
	}

	Channel *check_channel(std::string const &chan, ListenSocket &server, Client &client, int visible) {
		//"Функция проверки наличия канала"
		channel_iter it = server.getChannel(chan);
		if (!server.isChannelExist(it)) {
			sendError(client, server, ERR_NOSUCHCHANNEL, chan);
			return NULL;
		}
		Channel &channel = it->second;

		if (visible == 1) {
			if (!channel.isClient(client)) {
				//если клиента нет на канале
				sendError(client, server, ERR_NOTONCHANNEL, chan);
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

	bool priv_need_channel(Channel *channel, Client const &client, ListenSocket &server, std::string const &chani) {
		//функция проверки прав оператора канала
		if (!channel->isOper(client)) {
			sendError(client, server, ERR_CHANOPRIVSNEEDED, chani);
			return false;
		}
		return true;
	}

	bool check_nick(client_iter &it, Channel *channel, Client const &client, ListenSocket &server, std::string const &nick, std::string const &chani) {
		//проверка ника в канале
		if (server.isClientExist(it)) {
			//если нет ника
			sendError(client, server, ERR_NOSUCHNICK, nick);
			return false;
		}
		if (channel->isClient(it)) {
			//если чувак уже на канале
			sendError(client, server, ERR_USERONCHANNEL, nick, chani);
			return false;
		}
		return true;
	}

	bool erase_member(Channel *channel, Client const &client, ListenSocket &server, std::string const &nick, std::string const &chan) {
		//удаление члена канала
		client_iter it = server.getClient(nick);
		if (channel->users.erase(&(*it)) == 0) {// пытаемся удалить
			sendError(client, server, ERR_USERNOTINCHANNEL, nick, chan);
			return false;
		}
		channel->opers.erase(&it->getNick());
		channel->voiced.erase(&it->getNick());
		it->eraseChannel(chan);
		return true;
	}

	bool check_params(Client const& client, ListenSocket const& server, std::vector<std::string> const& params, std::string const& cmd, size_t needed) {
		if (params.size() < needed) {
			sendError(client, server, ERR_NEEDMOREPARAMS, cmd);
			return false;
		}
		return true;
	}

	bool check_join_chan(Client const &client, ListenSocket const &server, Channel const &chan, std::string const &in_key) {
		std::string const &key = chan.getKey();

		if (chan.isClient(client)) {
			return false;
		}
		if (!key.empty() && in_key != key) {
			sendError(client, server, ERR_BADCHANNELKEY, chan.getName());
			return false;
		}
		if (chan.isFlag(CMODE_INVITE)) {
			sendError(client, server, ERR_INVITEONLYCHAN, chan.getName());
			return false;
		}
		if (!chan.check_limit()) {
			sendError(client, server, ERR_CHANNELISFULL, chan.getName());
			return false;
		}
		return true;
	}

	void mode_table(Channel *channel, Client const &client, ListenSocket &server, std::string const &chani)
	//вывод модов канала или ника
	{
		std::string res;
		if (channel->isFlag(CMODE_INVITE))
		{
			res.push_back(INVIT);
		}
		if (channel->isFlag(CMODE_MODER))
		{
			res.push_back(MODES);
		}
		if (channel->isFlag(CMODE_SECRET))
		{
			res.push_back(SECRET);
		}
		if (channel->isFlag(CMODE_NOEXT))
		{
			res.push_back(SPEAK);
		}
		if (channel->isFlag(CMODE_TOPIC)){
			res.push_back(TOPIC);
		}
		if (!channel->getKey().empty()) {
			res.push_back(KEY);
		}
		if (channel->getLimit() > 0){
			res.push_back(LEN);
		}
		std::stringstream ss;
		ss << channel->getLimit();
		sendReply(client, server,  RPL_CHANNELMODEIS, chani, res,
				  channel->getKey().empty() ? "" : "***",
				  channel->getLimit() > 0 ? ss.str() : "");
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
		client_iter to = server.getClient(nick);
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
		std::string res = "+r";
		if (oclient->isFlag(UMODE_OPER))
			res += "o";
		if (oclient->isFlag(UMODE_INVIS))
			res += "i";
		if (oclient->isFlag(UMODE_WALLOPS))
			res += "w";
		sendReply(client, server,  RPL_UMODEIS, res);
	}

	void mode_flags_nick(Client *oclient, int flag, int const &sign)
	//установка флага клиента
	{
		if (sign == '+')
			oclient->setFlag(flag);
		else
			oclient->zeroFlag(flag);
	}

	client_iter check_mask_nick(int flag, std::string const &nick, Client &client, ListenSocket &server)
	//ищем маску или ник
	{
		client_iter it;
		if (flag == ERR_CANTKILLSERVER) {
			sendError(client, server, flag);
			return server.clients.end();
		}
		else if (flag == RPL_WHOISSERVER){
			sendReply(client, server,  flag, nick, server.getServername(), "Вот такой вот сервер");
			sendReply(client, server,  RPL_ENDOFWHOIS, nick);
			return server.clients.end();
		}
		else if (nick.find('@') != std::string::npos) {
			it = server.getClientByMask(nick);
		}
		else
			it = server.getClient(nick);
		return it;
	}

	bool check_channel_exist(Client const &client, ListenSocket const &server, channel_iter const& chan, std::string const& chan_name) {
		if (!server.isChannelExist(chan)) {
			sendError(client, server, ERR_NOSUCHCHANNEL, chan_name);
			return false;
		}
		return true;
	}

	bool check_channel_send(Client const &client, ListenSocket const &server, Channel const& chan) {
		if (((chan.isFlag(CMODE_NOEXT) && !chan.isClient(client)) || (chan.isFlag(CMODE_MODER) && !chan.isVoiced(client)))
			&& !client.isFlag(UMODE_OPER)) {
			sendError(client, server, ERR_CANNOTSENDTOCHAN, chan.getName());
			return false;
		}
		return true;
	}

	void send_command_to_sharing_channel(Command const& cmd, Client const &client, ListenSocket const& server) {
		std::set<Client*> to_send;
		for (std::list<std::string>::const_iterator cl_chan = client.getChannels().begin(); cl_chan != client.getChannels().end(); ++cl_chan) {
			channel_const_iter chan = server.getChannel(*cl_chan);
			if (server.isChannelExist(chan)) {
				to_send.insert(chan->second.users.begin(), chan->second.users.end());
			}
		}
		for (channel_client_iter it = to_send.begin(); it != to_send.end(); ++it) {
			server.send_command(cmd, **it);
		}
		if (to_send.empty()) {
			server.send_command(cmd, client);
		}
	}

} // namespace IRC