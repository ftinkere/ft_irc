#include "Channel.hpp"

namespace IRC {

    Channel::Channel(std::string const& name) :
		name(name),
		flags(CMODE_NOEXT),
		limit(0) {}

    bool Channel::check_name(std::string const& name) {
        if (name[0] != '#')
            return false;
        std::set<char> sim;
        sim.insert('\r');
        sim.insert('\n');
        sim.insert(' ');
        sim.insert('\0');
        sim.insert(',');
        for (int i = 0; i < name.length(); i++)
        {
            if (sim.find(name[i]) != sim.end())
                return false;
        }
        return true;
    }

    void Channel::add_memeber(Client const& member)
    {
//        base[member] = 0;
		if (users.empty()) {
			opers.insert(&member.getNick());
		}
		users.insert(&member);
	}

    bool Channel::check_limit()
    {
        if (limit > 0)
        {
            if (users.size() == limit)
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

}