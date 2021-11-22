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
        sim.insert(' ');
        sim.insert('\0');
        sim.insert(',');
        for (int i = 0; i < _name.length(); i++)
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

}