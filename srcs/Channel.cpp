#include "Channel.hpp"

namespace IRC {

    Channel::Channel() : limit(-1) {}

    int Channel::check_name(std::string &_name)
    {
        if (_name[0] != '#')
            return KEY;
        std::set<char> sim;
        sim.insert('\r');
        sim.insert('\n');
        sim.insert('\0');
        for (int i = 0; i < name.length(); i++)
        {
            if (sim.find(_name[i]) != sim.end())
                return ERR_NAME;
        }
        return CHAN;
    }

    void Channel::add_memeber(const std::string &member)
    {
        base[member] = 0;
    }

    bool Channel::check_limit()
    {
        if (limit > -1)
        {
            if (base.size() == limit)
                return false;
        }
        return true;
    }

	std::string Channel::get_names() const {
		std::string ret;

		for (std::set<std::string const*>::const_iterator it = opers.begin(); it != opers.end(); ++it) {
			ret += '@' + **it + ' ';
		}
		for (std::set<Client const*>::const_iterator it = users.begin(); it != users.end(); ++it) {
			if (opers.find(&(*it)->getNick()) == opers.end()) {
				ret += (*it)->getNick() + ' ';
			}
		}
		if (!ret.empty()) {
			ret.erase(ret.end() - 1);
		}
		return ret;
	}

    std::vector<std::string> &Channel::split(const std::string &s, char delim, std::vector<std::string> &elems)
    {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim))
        {
            elems.push_back(item);
        }
        return elems;
    }

    std::vector<std::string> Channel::split(const std::string &s, char delim)
    {
        std::vector<std::string> elems;
        split(s, delim, elems);
        return elems;
    }
}