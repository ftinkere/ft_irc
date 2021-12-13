#include "Channel.hpp"
#include <sstream>

namespace IRC {
	std::map<const char, enum Channel::model> Channel::modes;

	Channel::Channel(std::string const &name) :
			name(name),
			flags(CMODE_NOEXT | CMODE_TOPIC),
			limit(0),
			to_delete(false) {}

	bool Channel::check_name(std::string const &name) {
		if (name[0] != '#')
			return false;
		return true;
	}

	void Channel::add_memeber(Client &member) {
//        base[member] = 0;
		if (users.empty()) {
			opers.insert(&member.getNick());
			voiced.insert(&member.getNick());
		}
		users.insert(&member);
	}

	bool Channel::check_limit() const {
		if (limit > 0) {
			if (users.size() == limit)
				return false;
		}
		return true;
	}

	std::string Channel::get_names() const {
		std::string opers_ret;
		std::string voiced_ret;
		std::string ret;

		for (channel_client_iter it = users.begin(); it != users.end(); ++it) {
			if ((*it)->isFlag(UMODE_INVIS)) {
				// не трогаем невидимых
				continue;
			} else if (isOper(it)) {
				// если опер
				opers_ret += '@' + (*it)->getNick() + ' ';
			} else if (isVoiced(it)) {
				// если не опер, но есть право голоса
				voiced_ret += '+' + (*it)->getNick() + ' ';
			} else {
				// если простой смертный
				ret += (*it)->getNick() + ' ';
			}
		}
		ret = opers_ret + voiced_ret + ret;
		if (!ret.empty()) {
			ret.erase(ret.end() - 1);
		}
		return ret;
	}

	std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			elems.push_back(item);
		}
		return elems;
	}

	void Channel::erase_client(Client &cl) {
		voiced.erase(&cl.getNick());
		opers.erase(&cl.getNick());
		users.erase(&cl);
		if (users.empty()) {
			to_delete = true;
		}
	}

	bool Channel::isDelete() { return to_delete; }

	bool Channel::isClient(Client const &client) const { return users.find(const_cast<Client *>(&client)) != users.end(); };
	bool Channel::isClient(client_iter const &client) const { return users.find(&(*client)) != users.end(); };

	bool Channel::isOper(Client const &client) const { return opers.find(&client.getNick()) != opers.end(); };
	bool Channel::isOper(client_iter const &client) const { return opers.find(&client->getNick()) != opers.end(); };
	bool Channel::isOper(channel_client_iter const &client) const { return opers.find(&(*client)->getNick()) != opers.end(); };
	bool Channel::isOper(std::string const &nick) const { return opers.find(&nick) != opers.end(); };

	bool Channel::isVoiced(Client const &client) const { return voiced.find(&client.getNick()) != voiced.end(); };
	bool Channel::isVoiced(client_iter const &client) const { return voiced.find(&client->getNick()) != voiced.end(); };
	bool Channel::isVoiced(channel_client_iter const &client) const { return voiced.find(&(*client)->getNick()) != voiced.end(); };
	bool Channel::isVoiced(std::string const &nick) const { return voiced.find(&nick) != voiced.end(); };

	Channel::Channel():
			flags(CMODE_NOEXT | CMODE_TOPIC),
			limit(0),
			to_delete(false) {}

}