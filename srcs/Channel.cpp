#include "Channel.hpp"

namespace IRC {
    std::map<const char, size_t> Channel::modes;
    Channel::Channel(std::string const& name) :
		name(name),
		flags(CMODE_NOEXT | CMODE_MODER),
		limit(0)
        {}

    bool Channel::check_name(std::string const& name) {
        if (name[0] != '#')
            return false;
        return true;
    }

    void Channel::add_memeber(Client & member)
    {
//        base[member] = 0;
		if (users.empty()) {
			opers.insert(&member.getNick());
            voiced.insert(&member.getNick());
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
		for (std::set<Client*>::const_iterator it = users.begin(); it != users.end(); ++it) {
			if (opers.find(&(*it)->getNick()) == opers.end()) {
                if (!(*it)->isFlag(UMODE_INVIS)) { //если ник видимый
				    ret += (*it)->getNick() + ' ';
				}
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

    void Channel::erase_client(Client & cl)
    {
        voiced.erase(&cl.getNick());
        opers.erase(&cl.getNick());
        users.erase(&cl);
    }

    Channel::Channel() :
    flags(CMODE_NOEXT),
    limit(0) {}

}